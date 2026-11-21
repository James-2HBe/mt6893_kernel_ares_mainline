// SPDX-License-Identifier: GPL-2.0
/*
 * st21nfc.c — ST21NFC I2C NFC 直通驱动（主线重写版）
 *
 * 重写自小米官方 ares-r-oss
 *   drivers/misc/mediatek/nfc/st21nfc/st21nfc.c（39KB）
 * 保留 Android HAL 依赖的 /dev/st21nfc miscdevice 直通语义，剥离：
 *   - <linux/of_gpio.h> + 数值 GPIO → 标准 gpiod（厂商 dts 的
 *     gpio-rst = <10> 是 MTK 私有写法，主线无法解析，见下方说明）
 *   - 私有 st21nfc.h 的 ioctl 定义 → 本文件的最小 ioctl 集
 *   - 4.14 双参数 probe / LEGACY 条件分支
 *
 * 厂商 DT：&i2c6 { st21nfc@08 {
 *     compatible = "mediatek,nfc";
 *     reg = <0x08>;
 *     interrupt-parent = <&pio>;
 *     interrupts = <11 IRQ_TYPE_LEVEL_HIGH 11 0>;
 *     gpio-rst = <10>;  gpio-irq = <11>;
 * }; };
 * ⚠️ 主线 pinctrl 驱动不导出这些数值 GPIO 为 gpio_chip，故驱动用
 *    devm_gpiod_get_optional("reset") 读标准 reset-gpios；厂商属性仅记录。
 *    要让 reset 生效，需在板级 dts 补 reset-gpios = <&pio 10 ...>。
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
	struct i2c_client	*client;
	struct miscdevice	 misc;
	struct gpio_desc	*reset;
	struct mutex		 lock;
	wait_queue_head_t	 read_wq;
	bool			 powered;
	bool			 irq_ready;
	u8			 buffer[ST21NFC_MAX_BUFFER];
	u32			 gpio_rst_num;
	u32			 gpio_irq_num;
};

/* ares 只有一颗 NFC；miscdevice 的 fops 无法直接拿 driver_data，用单例 */
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
		/* 极性由 DT interrupts 描述，主线无需运行时改写 */
		break;
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
	.owner		= THIS_MODULE,
	.open		= st21nfc_open,
	.read		= st21nfc_read,
	.write		= st21nfc_write,
	.unlocked_ioctl	= st21nfc_ioctl,
	.poll		= st21nfc_poll,
};

static int st21nfc_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct st21nfc *chip;
	int ret;

	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->client = client;
	mutex_init(&chip->lock);
	init_waitqueue_head(&chip->read_wq);
	of_property_read_u32(dev->of_node, "gpio-rst", &chip->gpio_rst_num);
	of_property_read_u32(dev->of_node, "gpio-irq", &chip->gpio_irq_num);

	chip->reset = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(chip->reset))
		return PTR_ERR(chip->reset);
	if (!chip->reset)
		dev_info(dev,
			 "厂商 dts 用数值 gpio-rst=%u，主线无对应 gpio_chip；"
			 "reset 控制未挂接（需补 reset-gpios）\n",
			 chip->gpio_rst_num);

	chip->misc.minor = MISC_DYNAMIC_MINOR;
	chip->misc.name  = "st21nfc";
	chip->misc.fops  = &st21nfc_fops;
	chip->misc.parent = dev;

	ret = misc_register(&chip->misc);
	if (ret) {
		dev_err(dev, "注册 /dev/st21nfc 失败: %d\n", ret);
		return ret;
	}
	st21nfc_dev = chip;

	if (client->irq > 0) {
		ret = devm_request_irq(dev, client->irq, st21nfc_irq_handler,
				       IRQF_TRIGGER_HIGH, "st21nfc", chip);
		if (ret)
			dev_warn(dev, "申请 IRQ %d 失败: %d\n", client->irq, ret);
	}

	dev_info(dev, "ST21NFC 已注册（irq=%d, gpio-rst=%u, gpio-irq=%u）\n",
		 client->irq, chip->gpio_rst_num, chip->gpio_irq_num);
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
	{ }
};
MODULE_DEVICE_TABLE(of, st21nfc_of_match);

static struct i2c_driver st21nfc_driver = {
	.driver = {
		.name		= "st21nfc",
		.of_match_table	= st21nfc_of_match,
	},
	.probe = st21nfc_probe,
	.remove = st21nfc_remove,
};
module_i2c_driver(st21nfc_driver);

MODULE_AUTHOR("ported from Xiaomi ares-r-oss / ST Microelectronics");
MODULE_DESCRIPTION("ST21NFC I2C passthrough driver for Xiaomi ares");
MODULE_LICENSE("GPL");
