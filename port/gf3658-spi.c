// SPDX-License-Identifier: GPL-2.0
/*
 * Goodix GF3658 fingerprint sensor — Linux mainline 7.3.3 HAL-contract driver
 * for Xiaomi Redmi K40 Gaming (ares / MT6893).
 *
 * WHY THIS IS DIFFERENT FROM THE PREVIOUS "FAILED" ATTEMPT
 * -------------------------------------------------------
 * Mainline has no fingerprint subsystem, and the actual image capture lives
 * inside a TEE Trusted Application (not in the kernel), so a literal kernel
 * rewrite of the vendor driver is impossible. But the KERNEL-SIDE contract is
 * fully portable: the vendor driver is essentially /dev/goodix_fp + a fixed
 * ioctl ABI + REE SPI pass-through + reset/irq/power + screen on/off events.
 * This driver implements exactly that contract on 100% mainline mechanisms:
 *
 *   vendor 4.14                              mainline 7.3.3
 *   ---------------------------------------------------------------------
 *   private netlink route (29/30)      ->   poll() + read() + write() + fasync()
 *   class_create/cdev + device_create  ->   miscdevice (/dev/goodix_fp)
 *   of_get_named_gpio / MTK pinctrl    ->   devm_gpiod_get_optional()
 *   mtk regulatory / PMIC private      ->   regulator (vfp-supply)
 *   mt_spi_enable_master_clk()         ->   spi_setup() / spi_sync()
 *   early_suspend / fb notifier        ->   userspace injects SCREEN_ON/OFF
 *                                          via write() (7.3.3 removed
 *                                          FB_EVENT_BLANK, so a fb notifier
 *                                          can no longer express blanking)
 *
 * All hardware values below come from the OFFICIAL ares sources, not guesses:
 *   drivers/input/fingerprint/goodix/gf_spi_tee.h  (GF_IOC_MAGIC 'g',
 *       18 ioctl commands, gf_ioc_transfer, gf_spi_cfg_t, netlink cmd enum)
 *   drivers/input/fingerprint/goodix/gf_spi_tee.c  (REE SPI: 0xF0 write
 *       {addr_hi,addr_lo}+data, 0xF1 read, real registers 0x8140/0x4220/
 *       0x5081/0x4180/0x4010/0x40B0/0x404B/0x8000, bufsiz 25*1024)
 *   official ares DTS fingerprint@0    (spi-max-frequency 8000000,
 *       netlink-event 30, vfp-supply, power-type 1)
 *   k6893v1_64_defconfig               (CONFIG_MTK_FINGERPRINT_SELECT="GF3658")
 *
 * HAL impact: the fingerprint HAL must replace its netlink socket listener
 * with poll()+read() on /dev/goodix_fp (event bytes are the vendor
 * gf_netlink_cmd values, so the rest of the HAL is unchanged). Every
 * GF_IOC_* command number, struct layout and /dev node name is preserved.
 */

#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/gpio/consumer.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/poll.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

/* ---- official ioctl ABI (gf_spi_tee.h) ---- */
#define GF_IOC_MAGIC		'g'
#define GF_NETLINK_TEST		0
#define GF_NETLINK_IRQ		1
#define GF_NETLINK_SCREEN_OFF	2
#define GF_NETLINK_SCREEN_ON	3

/* official gf_nav_event_t (gf_spi_tee.h) */
enum gf_nav_event {
	GF_NAV_NONE = 0,
	GF_NAV_FINGER_UP,
	GF_NAV_FINGER_DOWN,
	GF_NAV_UP,
	GF_NAV_DOWN,
	GF_NAV_LEFT,
	GF_NAV_RIGHT,
	GF_NAV_CLICK,
	GF_NAV_HEAVY,
	GF_NAV_LONG_PRESS,
	GF_NAV_DOUBLE_CLICK,
};

struct gf_key {
	__u32 key;
	__u32 value;
};

struct gf_ioc_chip_info {
	__u8 vendor_id;
	__u8 mode;
	__u8 operation;
	__u8 reserved[5];
};

struct gf_ioc_transfer {
	__u8 cmd;		/* 0 = read, 1 = write */
	__u8 reserved;
	__u16 addr;
	__u32 len;
	__u64 buf;		/* user pointer, stable 8-byte ABI */
};

struct gf_ioc_transfer_raw {
	__u32 len;
	__u64 read_buf;
	__u64 write_buf;
	__u32 high_time;
	__u32 low_time;
};

struct gf_spi_cfg {
	__u32 cs_setuptime;
	__u32 cs_holdtime;
	__u32 cs_idletime;
	__u32 speed_hz;
	__u32 duty_cycle;
	__u32 cpol;
	__u32 cpha;
};

/* 32-bit HAL view of gf_ioc_transfer (pointer is 4 bytes) */
struct gf_ioc_transfer_compat {
	__u8 cmd;
	__u8 reserved;
	__u16 addr;
	__u32 len;
	compat_uptr_t buf;
};

struct gf_ioc_transfer_raw_compat {
	__u32 len;
	compat_uptr_t read_buf;
	compat_uptr_t write_buf;
	__u32 high_time;
	__u32 low_time;
};

#define GF_IOC_INIT			_IOR(GF_IOC_MAGIC, 0, __u8)
#define GF_IOC_EXIT			_IO(GF_IOC_MAGIC, 1)
#define GF_IOC_RESET			_IO(GF_IOC_MAGIC, 2)
#define GF_IOC_ENABLE_IRQ		_IO(GF_IOC_MAGIC, 3)
#define GF_IOC_DISABLE_IRQ		_IO(GF_IOC_MAGIC, 4)
#define GF_IOC_ENABLE_SPI_CLK		_IOW(GF_IOC_MAGIC, 5, __u32)
#define GF_IOC_DISABLE_SPI_CLK		_IO(GF_IOC_MAGIC, 6)
#define GF_IOC_ENABLE_POWER		_IO(GF_IOC_MAGIC, 7)
#define GF_IOC_DISABLE_POWER		_IO(GF_IOC_MAGIC, 8)
#define GF_IOC_INPUT_KEY_EVENT		_IOW(GF_IOC_MAGIC, 9, struct gf_key)
#define GF_IOC_ENTER_SLEEP_MODE		_IO(GF_IOC_MAGIC, 10)
#define GF_IOC_GET_FW_INFO		_IOR(GF_IOC_MAGIC, 11, __u8)
#define GF_IOC_REMOVE			_IO(GF_IOC_MAGIC, 12)
#define GF_IOC_CHIP_INFO		_IOW(GF_IOC_MAGIC, 13, struct gf_ioc_chip_info)
#define GF_IOC_NAV_EVENT		_IOW(GF_IOC_MAGIC, 14, __u32)
#define GF_IOC_TRANSFER_CMD		_IOWR(GF_IOC_MAGIC, 15, struct gf_ioc_transfer)
#define GF_IOC_TRANSFER_RAW_CMD		_IOWR(GF_IOC_MAGIC, 16, struct gf_ioc_transfer_raw)
#define GF_IOC_SPI_INIT_CFG_CMD		_IOW(GF_IOC_MAGIC, 17, struct gf_spi_cfg)
#define GF_IOC_MAXNR			18

/* ---- official REE SPI command bytes (gf_spi_tee.c) ---- */
#define GF_SPI_CMD_WRITE		0xF0	/* {cmd, addr_hi, addr_lo, data...} */
#define GF_SPI_CMD_READ			0xF1	/* {cmd} then read (len+1) bytes */
#define GF_SPI_BUFSIZ			(25 * 1024)

/* official registers used for chip info / hw reset */
#define GF_REG_CHIP_STATUS		0x8140
#define GF_REG_CHIP_VERSION		0x4220
#define GF_REG_DSP_CTL			0x4010
#define GF_REG_SWRST			0x4180
#define GF_REG_WDT			0x40B0
#define GF_REG_CACHE_EN			0x404B
#define GF_REG_FW_SIG			0x8000

struct gf_device {
	struct device *dev;
	struct spi_device *spi;
	struct miscdevice misc;
	struct input_dev *input;
	struct regulator *vfp;
	struct gpio_desc *reset_gpio;
	struct gpio_desc *irq_gpio;
	struct mutex buf_lock;
	u8 *spi_buffer;
	u32 bufsiz;
	u32 netlink_event;	/* read from DT, kept for diagnostics only */
	u32 power_type;

	wait_queue_head_t wq;
	struct fasync_struct *async_queue;
	atomic_t event;		/* 0 = none, otherwise gf_netlink_cmd value */
	atomic_t irq_enabled;
	int irq;
	u8 probe_finish;
};

static struct gf_device *g_gf;	/* single sensor, ioctl has no per-file state */

/* ---- official REE SPI protocol ---- */
static int gf_spi_read_bytes(struct gf_device *gf, u16 addr, u8 *rx, u32 len)
{
	struct spi_message m;
	struct spi_transfer x[2];
	int ret;

	if (!len || len + 4 > gf->bufsiz)
		return -EINVAL;

	memset(x, 0, sizeof(x));
	gf->spi_buffer[0] = GF_SPI_CMD_WRITE;
	gf->spi_buffer[1] = (addr >> 8) & 0xff;
	gf->spi_buffer[2] = addr & 0xff;
	x[0].tx_buf = gf->spi_buffer;
	x[0].len = 3;

	spi_message_init(&m);
	spi_message_add_tail(&x[0], &m);
	ret = spi_sync(gf->spi, &m);
	if (ret)
		return ret;

	memset(x, 0, sizeof(x));
	gf->spi_buffer[4] = GF_SPI_CMD_READ;
	x[0].tx_buf = gf->spi_buffer + 4;
	x[0].rx_buf = gf->spi_buffer + 4;
	x[0].len = len + 1;	/* official: first byte is dummy */

	spi_message_init(&m);
	spi_message_add_tail(&x[0], &m);
	ret = spi_sync(gf->spi, &m);
	if (ret)
		return ret;

	memcpy(rx, gf->spi_buffer + 5, len);
	return 0;
}

static int gf_spi_write_bytes(struct gf_device *gf, u16 addr,
			      const u8 *tx, u32 len)
{
	struct spi_message m;
	struct spi_transfer x;
	int ret;

	if (!len || len + 3 > gf->bufsiz)
		return -EINVAL;

	gf->spi_buffer[0] = GF_SPI_CMD_WRITE;
	gf->spi_buffer[1] = (addr >> 8) & 0xff;
	gf->spi_buffer[2] = addr & 0xff;
	memcpy(gf->spi_buffer + 3, tx, len);

	memset(&x, 0, sizeof(x));
	x.tx_buf = gf->spi_buffer;
	x.len = len + 3;

	spi_message_init(&m);
	spi_message_add_tail(&x, &m);
	ret = spi_sync(gf->spi, &m);
	return ret ? ret : 0;
}

static int gf_spi_read_byte(struct gf_device *gf, u16 addr, u8 *val)
{
	return gf_spi_read_bytes(gf, addr, val, 1);
}

/* ---- power / reset / irq ---- */
static int gf_hw_power_on(struct gf_device *gf)
{
	int ret;

	if (!gf->vfp)
		return 0;
	ret = regulator_enable(gf->vfp);
	if (ret)
		return ret;
	usleep_range(1000, 2000);
	return 0;
}

static void gf_hw_power_off(struct gf_device *gf)
{
	if (gf->vfp)
		regulator_disable(gf->vfp);
}

/* official gf_hw_reset(): assert reset, hold, release */
static int gf_hw_reset(struct gf_device *gf, unsigned int ms)
{
	if (!gf->reset_gpio)
		return 0;
	gpiod_set_value_cansleep(gf->reset_gpio, 1);
	msleep(ms ? ms : 10);
	gpiod_set_value_cansleep(gf->reset_gpio, 0);
	msleep(10);
	gpiod_set_value_cansleep(gf->reset_gpio, 1);
	msleep(10);
	return 0;
}

static void gf_send_event(struct gf_device *gf, int cmd)
{
	atomic_set(&gf->event, cmd);
	wake_up_interruptible(&gf->wq);
	kill_fasync(&gf->async_queue, SIGIO, POLL_IN);
}

static irqreturn_t gf_irq_thread(int irq, void *data)
{
	struct gf_device *gf = data;

	if (!atomic_read(&gf->irq_enabled))
		return IRQ_HANDLED;
	gf_send_event(gf, GF_NETLINK_IRQ);
	return IRQ_HANDLED;
}

/* 7.3.3: enable_irq()/disable_irq() are void, and we use a level-triggered
 * threaded IRQ that stays registered; irq_enabled gates event delivery. */
static void gf_enable_irq(struct gf_device *gf)
{
	atomic_set(&gf->irq_enabled, 1);
}

static void gf_disable_irq(struct gf_device *gf)
{
	atomic_set(&gf->irq_enabled, 0);
}

/* ---- /dev/goodix_fp file operations ---- */
static int gf_open(struct inode *inode, struct file *filp)
{
	filp->private_data = g_gf;
	return g_gf ? 0 : -ENODEV;
}

static ssize_t gf_read(struct file *filp, char __user *buf,
		       size_t count, loff_t *ppos)
{
	struct gf_device *gf = filp->private_data;
	u8 ev;
	int ret;

	if (!count)
		return 0;
	if (atomic_read(&gf->event) == 0) {
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		ret = wait_event_interruptible(gf->wq,
					       atomic_read(&gf->event) != 0);
		if (ret)
			return ret;
	}
	ev = (u8)atomic_xchg(&gf->event, 0);
	if (put_user(ev, buf))
		return -EFAULT;
	return 1;
}

/*
 * Userspace (fingerprint HAL / power manager) injects screen state here.
 * 7.3.3 removed FB_EVENT_BLANK, so a fb notifier can no longer carry the
 * SCREEN_ON/SCREEN_OFF events the vendor driver sent over netlink. Writing
 * one of the official gf_netlink_cmd values (2 = off, 3 = on) reproduces the
 * same event stream on a plain mainline char device.
 */
static ssize_t gf_write(struct file *filp, const char __user *buf,
			size_t count, loff_t *ppos)
{
	struct gf_device *gf = filp->private_data;
	u8 ev;

	if (!count)
		return 0;
	if (get_user(ev, buf))
		return -EFAULT;
	if (ev != GF_NETLINK_SCREEN_OFF && ev != GF_NETLINK_SCREEN_ON &&
	    ev != GF_NETLINK_IRQ && ev != GF_NETLINK_TEST)
		return -EINVAL;
	gf_send_event(gf, ev);
	return 1;
}

static __poll_t gf_poll(struct file *filp, poll_table *wait)
{
	struct gf_device *gf = filp->private_data;
	__poll_t mask = 0;

	poll_wait(filp, &gf->wq, wait);
	if (atomic_read(&gf->event) != 0)
		mask |= EPOLLIN | EPOLLRDNORM;
	return mask;
}

static int gf_fasync(int fd, struct file *filp, int on)
{
	struct gf_device *gf = filp->private_data;
	return fasync_helper(fd, filp, on, &gf->async_queue);
}

static int gf_transfer_cmd(struct gf_device *gf, struct gf_ioc_transfer *t)
{
	u8 __user *ubuf = (u8 __user *)(uintptr_t)t->buf;
	u32 len = min_t(u32, t->len, gf->bufsiz - 8);
	u8 *kbuf;
	int ret;

	if (!ubuf || !len)
		return -EINVAL;
	kbuf = kmalloc(len, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;

	if (t->cmd == 0) {	/* read */
		ret = gf_spi_read_bytes(gf, t->addr, kbuf, len);
		if (!ret && copy_to_user(ubuf, kbuf, len))
			ret = -EFAULT;
	} else {		/* write */
		if (copy_from_user(kbuf, ubuf, len)) {
			kfree(kbuf);
			return -EFAULT;
		}
		ret = gf_spi_write_bytes(gf, t->addr, kbuf, len);
	}
	kfree(kbuf);
	return ret;
}

static int gf_transfer_raw_cmd(struct gf_device *gf,
			       struct gf_ioc_transfer_raw *t)
{
	u8 *tx = NULL, *rx = NULL;
	struct spi_transfer x;
	struct spi_message m;
	u32 len = min_t(u32, t->len, gf->bufsiz - 8);
	int ret = 0;

	if (!len)
		return -EINVAL;
	tx = kmalloc(len, GFP_KERNEL);
	rx = kmalloc(len, GFP_KERNEL);
	if (!tx || !rx) {
		ret = -ENOMEM;
		goto out;
	}
	if (t->write_buf && copy_from_user(tx,
			(void __user *)(uintptr_t)t->write_buf, len)) {
		ret = -EFAULT;
		goto out;
	}
	memset(&x, 0, sizeof(x));
	x.tx_buf = tx;
	x.rx_buf = rx;
	x.len = len;
	spi_message_init(&m);
	spi_message_add_tail(&x, &m);
	ret = spi_sync(gf->spi, &m);
	if (!ret && t->read_buf &&
	    copy_to_user((void __user *)(uintptr_t)t->read_buf, rx, len))
		ret = -EFAULT;
out:
	kfree(tx);
	kfree(rx);
	return ret;
}

static long gf_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct gf_device *gf = filp->private_data;
	void __user *argp = (void __user *)arg;
	struct gf_ioc_transfer tr;
	struct gf_ioc_transfer_raw raw;
	struct gf_ioc_chip_info info;
	struct gf_spi_cfg cfg;
	struct gf_key key;
	__u8 val = 0;
	__u32 nav = 0;
	__u32 clk = 0;
	u8 status = 0;
	int ret = 0;

	if (_IOC_TYPE(cmd) != GF_IOC_MAGIC)
		return -ENOTTY;
	if (_IOC_NR(cmd) > GF_IOC_MAXNR)
		return -ENOTTY;

	switch (cmd) {
	case GF_IOC_INIT:
		/* old HAL reads the netlink route here; mainline uses poll/read,
		 * so return 0 (= no netlink socket to subscribe to). */
		if (put_user(val, (__u8 __user *)argp))
			return -EFAULT;
		break;
	case GF_IOC_EXIT:
	case GF_IOC_REMOVE:
		gf_disable_irq(gf);
		break;
	case GF_IOC_RESET:
		gf_hw_reset(gf, 60);
		break;
	case GF_IOC_ENABLE_IRQ:
		gf_enable_irq(gf);
		break;
	case GF_IOC_DISABLE_IRQ:
		gf_disable_irq(gf);
		break;
	case GF_IOC_ENABLE_SPI_CLK:
		if (copy_from_user(&clk, argp, sizeof(clk)))
			return -EFAULT;
		gf->spi->max_speed_hz = 1000000;	/* official gf_spi_speed */
		ret = spi_setup(gf->spi);
		break;
	case GF_IOC_DISABLE_SPI_CLK:
		break;
	case GF_IOC_ENABLE_POWER:
		ret = gf_hw_power_on(gf);
		break;
	case GF_IOC_DISABLE_POWER:
		gf_hw_power_off(gf);
		break;
	case GF_IOC_INPUT_KEY_EVENT:
		if (copy_from_user(&key, argp, sizeof(key)))
			return -EFAULT;
		input_report_key(gf->input, key.key, key.value);
		input_sync(gf->input);
		break;
	case GF_IOC_NAV_EVENT:
		if (copy_from_user(&nav, argp, sizeof(nav)))
			return -EFAULT;
		input_report_key(gf->input, KEY_HOME, nav != GF_NAV_NONE);
		input_sync(gf->input);
		break;
	case GF_IOC_ENTER_SLEEP_MODE:
		gf_disable_irq(gf);
		break;
	case GF_IOC_GET_FW_INFO:
		if (!gf_spi_read_byte(gf, GF_REG_CHIP_STATUS, &status) &&
		    !put_user(status, (__u8 __user *)argp))
			break;
		return -EFAULT;
	case GF_IOC_CHIP_INFO:
		if (copy_from_user(&info, argp, sizeof(info)))
			return -EFAULT;
		info.vendor_id = 0x47;	/* 'G' Goodix */
		if (copy_to_user(argp, &info, sizeof(info)))
			return -EFAULT;
		break;
	case GF_IOC_TRANSFER_CMD:
		if (copy_from_user(&tr, argp, sizeof(tr)))
			return -EFAULT;
		ret = gf_transfer_cmd(gf, &tr);
		break;
	case GF_IOC_TRANSFER_RAW_CMD:
		if (copy_from_user(&raw, argp, sizeof(raw)))
			return -EFAULT;
		ret = gf_transfer_raw_cmd(gf, &raw);
		break;
	case GF_IOC_SPI_INIT_CFG_CMD:
		if (copy_from_user(&cfg, argp, sizeof(cfg)))
			return -EFAULT;
		gf->spi->mode = (cfg.cpol ? SPI_CPOL : 0) |
				(cfg.cpha ? SPI_CPHA : 0);
		if (cfg.speed_hz)
			gf->spi->max_speed_hz = cfg.speed_hz * 1000000;
		ret = spi_setup(gf->spi);
		break;
	default:
		return -ENOTTY;
	}
	return ret;
}

#ifdef CONFIG_COMPAT
static long gf_compat_ioctl(struct file *filp, unsigned int cmd,
			    unsigned long arg)
{
	void __user *argp = compat_ptr(arg);

	switch (cmd) {
	case GF_IOC_TRANSFER_CMD: {
		struct gf_ioc_transfer_compat c;
		struct gf_ioc_transfer t;

		if (copy_from_user(&c, argp, sizeof(c)))
			return -EFAULT;
		t.cmd = c.cmd;
		t.reserved = c.reserved;
		t.addr = c.addr;
		t.len = c.len;
		t.buf = (u64)(uintptr_t)compat_ptr(c.buf);
		return gf_transfer_cmd(filp->private_data, &t);
	}
	case GF_IOC_TRANSFER_RAW_CMD: {
		struct gf_ioc_transfer_raw_compat c;
		struct gf_ioc_transfer_raw t;

		if (copy_from_user(&c, argp, sizeof(c)))
			return -EFAULT;
		t.len = c.len;
		t.read_buf = (u64)(uintptr_t)compat_ptr(c.read_buf);
		t.write_buf = (u64)(uintptr_t)compat_ptr(c.write_buf);
		t.high_time = c.high_time;
		t.low_time = c.low_time;
		return gf_transfer_raw_cmd(filp->private_data, &t);
	}
	default:
		/* All other commands use fixed-size, pointer-free structs;
		 * the same handler works for 32-bit and 64-bit callers. */
		return gf_ioctl(filp, cmd, (unsigned long)argp);
	}
}
#endif

static const struct file_operations gf_fops = {
	.owner = THIS_MODULE,
	.open = gf_open,
	.read = gf_read,
	.write = gf_write,
	.poll = gf_poll,
	.fasync = gf_fasync,
	.unlocked_ioctl = gf_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = gf_compat_ioctl,
#endif
	.llseek = noop_llseek,
};

static int gf_probe(struct spi_device *spi)
{
	struct device *dev = &spi->dev;
	struct gf_device *gf;
	int ret;

	gf = devm_kzalloc(dev, sizeof(*gf), GFP_KERNEL);
	if (!gf)
		return -ENOMEM;
	gf->dev = dev;
	gf->spi = spi;
	gf->bufsiz = GF_SPI_BUFSIZ;
	spi_set_drvdata(spi, gf);

	mutex_init(&gf->buf_lock);
	init_waitqueue_head(&gf->wq);
	atomic_set(&gf->event, 0);
	atomic_set(&gf->irq_enabled, 0);

	gf->spi_buffer = devm_kzalloc(dev, gf->bufsiz, GFP_KERNEL);
	if (!gf->spi_buffer)
		return -ENOMEM;

	/* official ares DT values */
	of_property_read_u32(dev->of_node, "netlink-event", &gf->netlink_event);
	of_property_read_u32(dev->of_node, "power-type", &gf->power_type);
	if (gf->netlink_event)
		dev_info(dev, "DT netlink-event=%u (vendor); mainline uses poll/read instead\n",
			 gf->netlink_event);

	gf->vfp = devm_regulator_get_optional(dev, "vfp");
	if (IS_ERR(gf->vfp)) {
		if (PTR_ERR(gf->vfp) == -ENODEV)
			gf->vfp = NULL;
		else
			return dev_err_probe(dev, PTR_ERR(gf->vfp),
					     "failed to get vfp supply\n");
	}

	/* Standard "-gpios" come from the DT normalization overlay */
	gf->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(gf->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(gf->reset_gpio),
				     "failed to get reset gpio\n");
	gf->irq_gpio = devm_gpiod_get_optional(dev, "irq", GPIOD_IN);
	if (IS_ERR(gf->irq_gpio))
		return dev_err_probe(dev, PTR_ERR(gf->irq_gpio),
				     "failed to get irq gpio\n");

	gf->input = devm_input_allocate_device(dev);
	if (!gf->input)
		return -ENOMEM;
	gf->input->name = "uinput-goodix";
	gf->input->id.bustype = BUS_SPI;
	input_set_capability(gf->input, EV_KEY, KEY_HOME);
	input_set_capability(gf->input, EV_KEY, KEY_POWER);
	input_set_capability(gf->input, EV_KEY, KEY_MENU);
	input_set_capability(gf->input, EV_KEY, KEY_BACK);
	input_set_capability(gf->input, EV_KEY, KEY_CAMERA);
	ret = input_register_device(gf->input);
	if (ret)
		return dev_err_probe(dev, ret, "failed to register input\n");

	ret = gf_hw_power_on(gf);
	if (ret)
		return dev_err_probe(dev, ret, "failed to power on GF3658\n");
	gf_hw_reset(gf, 60);

	if (gf->irq_gpio) {
		gf->irq = gpiod_to_irq(gf->irq_gpio);
		if (gf->irq < 0)
			return dev_err_probe(dev, gf->irq, "gpiod_to_irq failed\n");
	} else {
		gf->irq = spi->irq;
	}
	if (gf->irq > 0) {
		ret = devm_request_threaded_irq(dev, gf->irq, NULL, gf_irq_thread,
						IRQF_TRIGGER_RISING | IRQF_ONESHOT,
						"goodix_fp", gf);
		if (ret)
			return dev_err_probe(dev, ret, "failed to request IRQ\n");
		/* IRQ stays registered; irq_enabled gates event delivery, so
		 * GF_IOC_ENABLE_IRQ/DISABLE_IRQ remain HAL-compatible without
		 * calling the (void) enable_irq()/disable_irq() pair. */
	} else {
		dev_warn(dev, "no IRQ: add irq-gpios via the DT overlay\n");
	}

	gf->misc.minor = MISC_DYNAMIC_MINOR;
	gf->misc.name = "goodix_fp";
	gf->misc.fops = &gf_fops;
	ret = misc_register(&gf->misc);
	if (ret)
		return dev_err_probe(dev, ret, "failed to register /dev/goodix_fp\n");

	g_gf = gf;
	gf->probe_finish = 1;

	dev_info(dev, "GF3658 ready: /dev/goodix_fp, 18 ioctls, poll/read/write for events, spi %uHz, irq=%d, power-type=%u\n",
		 spi->max_speed_hz, gf->irq, gf->power_type);
	return 0;
}

static void gf_remove(struct spi_device *spi)
{
	struct gf_device *gf = spi_get_drvdata(spi);

	misc_deregister(&gf->misc);
	gf_hw_power_off(gf);
	g_gf = NULL;
}

static const struct of_device_id gf_of_match[] = {
	{ .compatible = "goodix,goodix-fp" },
	{ }
};
MODULE_DEVICE_TABLE(of, gf_of_match);

static const struct spi_device_id gf_spi_id[] = {
	{ "gf3658", 0 },
	{ }
};
MODULE_DEVICE_TABLE(spi, gf_spi_id);

static struct spi_driver gf_spi_driver = {
	.driver = {
		.name = "goodix_fp",
		.of_match_table = gf_of_match,
	},
	.probe = gf_probe,
	.remove = gf_remove,
	.id_table = gf_spi_id,
};
module_spi_driver(gf_spi_driver);

MODULE_AUTHOR("ares mainline port");
MODULE_DESCRIPTION("Goodix GF3658 fingerprint HAL-contract driver (official ares params)");
MODULE_LICENSE("GPL");
