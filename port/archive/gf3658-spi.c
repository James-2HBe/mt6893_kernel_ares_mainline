// SPDX-License-Identifier: GPL-2.0
/*
 * gf3658-spi.c — Goodix GF3658 屏下指纹 SPI 透传（主线重写版，功能受限）
 *
 * 官方 4.14 驱动 gf_spi_tee.c 依赖 teei_fp.h / tee_client_api.h /
 * cpu_ctrl.h / mtk_spi_hal.h 等 MTK TEE 私有头：指纹采集在 TEE TA 内完成，
 * 内核侧只做 SPI 透传 + netlink 事件上报 + HAL ioctl。主线没有 TEE TA、
 * 没有 GF netlink 协议，因此**无法等价移植**。
 *
 * 本驱动实现内核层能独立完成的部分：
 *   - spi_driver probe / vfp-supply 电源 / 厂商 DT 参数
 *   - /dev/goodix_fp miscdevice，read/write 直通 SPI，ioctl 控制电源
 * 明确不含：TEE 采集、netlink(30) 事件、厂商 HAL 的 GF_IOC_* 语义。
 * 这些必须连 Android TEE TA 与厂商 HAL 一起搬，超出内核移植范畴。
 *
 * 厂商 DT：&spi1 { fingerprint@0 {
 *     compatible = "goodix,goodix-fp"; reg = <0>;
 *     spi-max-frequency = <8000000>;
 *     netlink-event = <30>; vfp-supply = <&mt_pmic_vfp_ldo_reg>;
 *     power-type = <1>; }; };
 */
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/uaccess.h>

#define GF_SPI_MAX_BUF	4096
#define GF_IOC_MAGIC	'g'
#define GF_IOC_RESET	_IO(GF_IOC_MAGIC, 0)
#define GF_IOC_POWER	_IOW(GF_IOC_MAGIC, 1, unsigned int)

struct gf3658 {
	struct spi_device	*spi;
	struct miscdevice	 misc;
	struct regulator	*vfp;
	struct mutex		 lock;
	u32			 netlink_event;
	u32			 power_type;
	bool			 powered;
};

/* ares 只有一颗指纹；miscdevice fops 无法直接取 driver_data，用单例 */
static struct gf3658 *gf_dev;

static int gf_open(struct inode *inode, struct file *filp)
{
	if (!gf_dev)
		return -ENODEV;
	filp->private_data = gf_dev;
	return 0;
}

static ssize_t gf_read(struct file *filp, char __user *buf, size_t count,
		       loff_t *off)
{
	struct gf3658 *chip = gf_dev;
	size_t len = min_t(size_t, count, GF_SPI_MAX_BUF);
	u8 *tmp;
	int ret;

	if (!chip)
		return -ENODEV;

	tmp = kmalloc(len, GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;

	mutex_lock(&chip->lock);
	ret = spi_read(chip->spi, tmp, len);
	mutex_unlock(&chip->lock);

	if (!ret && copy_to_user(buf, tmp, len))
		ret = -EFAULT;
	kfree(tmp);

	return ret ? ret : (ssize_t)len;
}

static ssize_t gf_write(struct file *filp, const char __user *buf,
			size_t count, loff_t *off)
{
	struct gf3658 *chip = gf_dev;
	size_t len = min_t(size_t, count, GF_SPI_MAX_BUF);
	u8 *tmp;
	int ret;

	if (!chip)
		return -ENODEV;

	tmp = kmalloc(len, GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;
	if (copy_from_user(tmp, buf, len)) {
		kfree(tmp);
		return -EFAULT;
	}

	mutex_lock(&chip->lock);
	ret = spi_write(chip->spi, tmp, len);
	mutex_unlock(&chip->lock);
	kfree(tmp);

	return ret ? ret : (ssize_t)len;
}

static long gf_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct gf3658 *chip = gf_dev;
	unsigned int val;
	int ret = 0;

	if (!chip)
		return -ENODEV;

	mutex_lock(&chip->lock);
	switch (cmd) {
	case GF_IOC_POWER:
		if (get_user(val, (unsigned int __user *)arg)) {
			ret = -EFAULT;
			break;
		}
		if (chip->vfp) {
			if (val)
				ret = regulator_enable(chip->vfp);
			else
				ret = regulator_disable(chip->vfp);
		}
		if (!ret)
			chip->powered = !!val;
		break;
	case GF_IOC_RESET:
		/* 厂商用 pinctrl 状态复位，主线无对应 gpio 描述，仅占位 */
		break;
	default:
		ret = -ENOTTY;
	}
	mutex_unlock(&chip->lock);

	return ret;
}

static const struct file_operations gf_fops = {
	.owner		= THIS_MODULE,
	.open		= gf_open,
	.read		= gf_read,
	.write		= gf_write,
	.unlocked_ioctl	= gf_ioctl,
};

static int gf_probe(struct spi_device *spi)
{
	struct device *dev = &spi->dev;
	struct gf3658 *chip;
	int ret;

	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->spi = spi;
	mutex_init(&chip->lock);
	of_property_read_u32(dev->of_node, "netlink-event",
			     &chip->netlink_event);
	of_property_read_u32(dev->of_node, "power-type", &chip->power_type);

	chip->vfp = devm_regulator_get_optional(dev, "vfp");
	if (IS_ERR(chip->vfp)) {
		if (PTR_ERR(chip->vfp) == -ENODEV) {
			chip->vfp = NULL;
		} else {
			ret = PTR_ERR(chip->vfp);
			dev_err(dev, "获取 vfp-supply 失败: %d\n", ret);
			return ret;
		}
	}

	chip->misc.minor = MISC_DYNAMIC_MINOR;
	chip->misc.name  = "goodix_fp";
	chip->misc.fops  = &gf_fops;
	chip->misc.parent = dev;

	ret = misc_register(&chip->misc);
	if (ret) {
		dev_err(dev, "注册 /dev/goodix_fp 失败: %d\n", ret);
		return ret;
	}

	gf_dev = chip;
	spi_set_drvdata(spi, chip);

	dev_info(dev,
		 "GF3658 SPI 透传已注册 (netlink-event=%u, power-type=%u); "
		 "TEE 采集/netlink 事件未实现\n",
		 chip->netlink_event, chip->power_type);
	return 0;
}

static void gf_remove(struct spi_device *spi)
{
	if (gf_dev) {
		misc_deregister(&gf_dev->misc);
		gf_dev = NULL;
	}
}

static const struct of_device_id gf_of_match[] = {
	{ .compatible = "goodix,goodix-fp" },
	{ }
};
MODULE_DEVICE_TABLE(of, gf_of_match);

static struct spi_driver gf_driver = {
	.driver = {
		.name		= "gf3658",
		.of_match_table	= gf_of_match,
	},
	.probe = gf_probe,
	.remove = gf_remove,
};
module_spi_driver(gf_driver);

MODULE_AUTHOR("ported from Xiaomi ares-r-oss / Goodix");
MODULE_DESCRIPTION("Goodix GF3658 fingerprint SPI passthrough (no TEE)");
MODULE_LICENSE("GPL");
