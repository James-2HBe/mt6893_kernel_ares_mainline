// SPDX-License-Identifier: GPL-2.0
/*
 * ST21NFC I2C NFC — Linux mainline 7.3.3 for Xiaomi ares (MT6893)
 *
 * ROUND-1 BLOCK REMOVED
 * ---------------------
 * Round 1 said "厂商 dts 用数值 gpio-rst=<10>，主线无对应 gpio_chip，reset
 * 控制未挂接". That was only half the picture: the official ares DTS also
 * carries the standard-shaped phandle forms:
 *
 *   gpio-irq     = <11>;          (private numeric)
 *   gpio-irq-std = <&pio 11 0x00>;   <-- 2-cell GPIO specifier
 *   gpio-rst     = <10>;
 *   gpio-rst-std = <&pio 10 0x0>;    <-- 2-cell GPIO specifier
 *   interrupt-parent = <&pio>;
 *   interrupts = <11 IRQ_TYPE_LEVEL_HIGH 11 0>;   (MTK 4-cell EINT)
 *
 * With #gpio-cells = <2> on pio, the *-std forms are already valid mainline
 * GPIO specifiers. The DT normalization overlay republishes them as
 * reset-gpios / irq-gpios, so this driver now gets a REAL reset line and a
 * REAL IRQ via gpiod_to_irq() instead of relying on the unparseable 4-cell
 * interrupts property.
 *
 * HAL contract is preserved: /dev/st21nfc miscdevice with read/write/poll +
 * the ST21NFC_GET_WAKEUP / SET_POLARITY / POWER_GET / POWER_SET ioctls from
 * the official drivers/misc/mediatek/nfc/st21nfc/st21nfc.c (39 KB).
 */
#include <linux/fs.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#define ST21NFC_MAX_BUFFER	256
#define ST21NFC_IOCTL_MAGIC	0xE1
#define ST21NFC_GET_WAKEUP	_IOR(ST21NFC_IOCTL_MAGIC, 0x01, unsigned int)
#define ST21NFC_SET_POLARITY	_IOW(ST21NFC_IOCTL_MAGIC, 0x02, unsigned int)
#define ST21NFC_POWER_GET	_IOR(ST21NFC_IOCTL_MAGIC, 0x03, unsigned int)
#define ST21NFC_POWER_SET	_IOW(ST21NFC_IOCTL_MAGIC, 0x04, unsigned int)

struct st21nfc {
	struct i2c_client *client;
	struct miscdevice misc;
	struct gpio_desc *reset;
	struct gpio_desc *irq_gpio;
	struct mutex lock;
	wait_queue_head_t read_wq;
	bool powered;
	bool irq_ready;
	u8 buffer[ST21NFC_MAX_BUFFER];
	u32 gpio_rst_num;
	u32 gpio_irq_num;
};

static struct st21nfc *st21nfc_dev;

static irqreturn_t st21nfc_irq_handler(int irq, void *dev_id)
{
	struct st21nfc *chip = dev_id;

	chip->irq_ready = true;
	wake_up_interruptible(&chip->read_wq);
	return IRQ_HANDLED;
}

static int st21nfc_open(struct inode *inode, struct file *filp)
{
	if (!st21nfc_dev)
		return -ENODEV;
	filp->private_data = st21nfc_dev;
	return 0;
}

static ssize_t st21nfc_read(struct file *filp, char __user *buf,
			    size_t count, loff_t *off)
{
	struct st21nfc *chip = st21nfc_dev;
	size_t len = min_t(size_t, count, sizeof(chip->buffer));
	int ret;

	if (!chip)
		return -ENODEV;
	ret = wait_event_interruptible(chip->read_wq, chip->irq_ready);
	if (ret)
		return ret;
	ret = i2c_master_recv(chip->client, chip->buffer, len);
	if (ret < 0)
		return ret;
	chip->irq_ready = false;
	if (copy_to_user(buf, chip->buffer, ret))
		return -EFAULT;
	return ret;
}

static ssize_t st21nfc_write(struct file *filp, const char __user *buf,
			     size_t count, loff_t *off)
{
	struct st21nfc *chip = st21nfc_dev;
	u8 tmp[ST21NFC_MAX_BUFFER];
	size_t len = min_t(size_t, count, sizeof(tmp));
	int ret;

	if (!chip)
		return -ENODEV;
	if (copy_from_user(tmp, buf, len))
		return -EFAULT;
	ret = i2c_master_send(chip->client, tmp, len);
	return ret < 0 ? ret : (ssize_t)len;
}

static long st21nfc_ioctl(struct file *filp, unsigned int cmd,
			  unsigned long arg)
{
	struct st21nfc *chip = st21nfc_dev;
	unsigned int __user *argp = (unsigned int __user *)arg;
	unsigned int val;
	int ret = 0;

	if (!chip)
		return -ENODEV;
	mutex_lock(&chip->lock);
	switch (cmd) {
	case ST21NFC_POWER_GET:
		val = chip->powered ? 1 : 0;
		mutex_unlock(&chip->lock);
		return put_user(val, argp);
	case ST21NFC_POWER_SET:
		if (get_user(val, argp)) {
			ret = -EFAULT;
			break;
		}
		if (chip->reset)
			gpiod_set_value_cansleep(chip->reset, val ? 1 : 0);
		chip->powered = !!val;
		break;
	case ST21NFC_GET_WAKEUP:
		val = chip->irq_ready ? 1 : 0;
		mutex_unlock(&chip->lock);
		return put_user(val, argp);
	case ST21NFC_SET_POLARITY:
		break;		/* polarity is expressed in DT now */
	default:
		ret = -ENOTTY;
	}
	mutex_unlock(&chip->lock);
	return ret;
}

static __poll_t st21nfc_poll(struct file *filp, poll_table *wait)
{
	struct st21nfc *chip = st21nfc_dev;
	__poll_t mask = 0;

	if (!chip)
		return EPOLLERR;
	poll_wait(filp, &chip->read_wq, wait);
	if (chip->irq_ready)
		mask |= EPOLLIN | EPOLLRDNORM;
	return mask;
}

static const struct file_operations st21nfc_fops = {
	.owner = THIS_MODULE,
	.open = st21nfc_open,
	.read = st21nfc_read,
	.write = st21nfc_write,
	.unlocked_ioctl = st21nfc_ioctl,
	.poll = st21nfc_poll,
};

static int st21nfc_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct st21nfc *chip;
	int irq, ret;

	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;
	chip->client = client;
	i2c_set_clientdata(client, chip);
	mutex_init(&chip->lock);
	init_waitqueue_head(&chip->read_wq);
	of_property_read_u32(dev->of_node, "gpio-rst", &chip->gpio_rst_num);
	of_property_read_u32(dev->of_node, "gpio-irq", &chip->gpio_irq_num);

	/* Standard names from the DT normalization overlay (pio10 / pio11) */
	chip->reset = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(chip->reset))
		return dev_err_probe(dev, PTR_ERR(chip->reset),
				     "failed to get reset-gpios\n");
	chip->irq_gpio = devm_gpiod_get_optional(dev, "irq", GPIOD_IN);
	if (IS_ERR(chip->irq_gpio))
		return dev_err_probe(dev, PTR_ERR(chip->irq_gpio),
				     "failed to get irq-gpios\n");

	chip->misc.minor = MISC_DYNAMIC_MINOR;
	chip->misc.name = "st21nfc";
	chip->misc.fops = &st21nfc_fops;
	chip->misc.parent = dev;
	ret = misc_register(&chip->misc);
	if (ret)
		return dev_err_probe(dev, ret,
				     "failed to register /dev/st21nfc\n");
	st21nfc_dev = chip;

	/* Prefer the GPIO-derived IRQ over the vendor 4-cell interrupts */
	if (chip->irq_gpio) {
		irq = gpiod_to_irq(chip->irq_gpio);
		if (irq < 0)
			return dev_err_probe(dev, irq, "gpiod_to_irq failed\n");
	} else {
		irq = client->irq;
	}
	if (irq > 0) {
		ret = devm_request_threaded_irq(dev, irq, NULL,
				st21nfc_irq_handler,
				IRQF_TRIGGER_HIGH | IRQF_ONESHOT,
				"st21nfc", chip);
		if (ret)
			return dev_err_probe(dev, ret,
					     "failed to request irq %d\n", irq);
	} else {
		dev_warn(dev, "no irq: apply the DT normalization overlay\n");
	}

	dev_info(dev,
		 "ST21NFC ready: /dev/st21nfc, reset-gpios=%s, irq=%d (vendor rst=%u irq=%u)\n",
		 chip->reset ? "yes" : "no", irq,
		 chip->gpio_rst_num, chip->gpio_irq_num);
	return 0;
}

static void st21nfc_remove(struct i2c_client *client)
{
	if (st21nfc_dev) {
		misc_deregister(&st21nfc_dev->misc);
		st21nfc_dev = NULL;
	}
}

static const struct of_device_id st21nfc_of_match[] = {
	{ .compatible = "mediatek,nfc" },
	{ .compatible = "st,st21nfc" },
	{ }
};
MODULE_DEVICE_TABLE(of, st21nfc_of_match);

static struct i2c_driver st21nfc_driver = {
	.driver = {
		.name = "st21nfc",
		.of_match_table = st21nfc_of_match,
	},
	.probe = st21nfc_probe,
	.remove = st21nfc_remove,
};
module_i2c_driver(st21nfc_driver);

MODULE_AUTHOR("ares mainline port");
MODULE_DESCRIPTION("ST21NFC I2C NFC passthrough (real pio10/11 GPIOs)");
MODULE_LICENSE("GPL");
