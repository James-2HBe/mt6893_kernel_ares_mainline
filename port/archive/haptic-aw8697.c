// SPDX-License-Identifier: GPL-2.0
/*
 * haptic-aw8697.c — Awinic AW8697 线性马达驱动（主线重写版）
 *
 * 官方 4.14 驱动 aw8697.c（83KB）依赖 aw8697.h / aw8697_reg.h /
 * aw8697_config.h / ringbuffer.h（源码仓库未提供），且使用 4.14 私有
 * power_supply / pm_qos / of_gpio 接口，无法直接搬移。本驱动按主线
 * led_classdev 重写：brightness 即振动强度，0 停止；保留厂商 DT 的
 * vib_* 参数解析，寄存器集中在 AW8697_REG_* 一组宏里。
 *
 * 为什么用 leds-class 而不是 input/ff-memless：当前基线
 * CONFIG_INPUT_FF_MEMLESS=m（模块），外部模块严格 modpost 会缺符号；
 * CONFIG_LEDS_CLASS=y，无额外依赖。若需要标准 force-feedback 接口，
 * 打开 CONFIG_INPUT_FF_MEMLESS=y 后可改用 input_ff_create_memless()。
 *
 * ⚠️ AW8697_REG_* 数值需按 datasheet 校准；厂商 dts 的 reset-gpio /
 *    irq-gpio 是 MTK 私有写法，需补标准 reset-gpios 才能控制复位。
 *
 * 厂商 DT：&i2c7 { aw8697_haptic@5a {
 *     compatible = "awinic,aw8697_haptic"; reg = <0x5A>;
 *     vib_mode = <0>; vib_f0_pre = <1700>; vib_cont_drv_lev = <106>; ... }; };
 */
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/slab.h>

/* 最小寄存器集（占位值，按 AW8697 datasheet 校准） */
#define AW8697_REG_SYSCTRL	0x00
#define AW8697_REG_CHIPID	0x01
#define AW8697_REG_CONT_PLAY	0x02
#define AW8697_REG_GO		0x03
#define AW8697_SYSCTRL_EN	BIT(0)

struct aw8697 {
	struct i2c_client	*client;
	struct led_classdev	 cdev;
	struct gpio_desc	*reset;
	struct mutex		 lock;
	u32			 vib_mode;
	u32			 f0_pre;
	u32			 cont_drv_lev;
	bool			 enabled;
};

static int aw8697_write(struct aw8697 *chip, u8 reg, u8 val)
{
	return i2c_smbus_write_byte_data(chip->client, reg, val);
}

static int aw8697_set_level(struct aw8697 *chip, int level)
{
	int ret;

	mutex_lock(&chip->lock);
	ret = aw8697_write(chip, AW8697_REG_CONT_PLAY, (u8)level);
	if (!ret)
		ret = aw8697_write(chip, AW8697_REG_GO, level ? 1 : 0);
	if (!ret)
		chip->enabled = !!level;
	mutex_unlock(&chip->lock);

	return ret;
}

static int aw8697_brightness_set(struct led_classdev *cdev,
				 enum led_brightness brightness)
{
	struct aw8697 *chip = container_of(cdev, struct aw8697, cdev);

	return aw8697_set_level(chip, brightness);
}

static int aw8697_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct aw8697 *chip;
	int ret, id;

	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->client = client;
	mutex_init(&chip->lock);
	of_property_read_u32(dev->of_node, "vib_mode", &chip->vib_mode);
	of_property_read_u32(dev->of_node, "vib_f0_pre", &chip->f0_pre);
	of_property_read_u32(dev->of_node, "vib_cont_drv_lev",
			     &chip->cont_drv_lev);

	chip->reset = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(chip->reset)) {
		ret = PTR_ERR(chip->reset);
		dev_err(dev, "读取 reset-gpios 失败: %d\n", ret);
		return ret;
	}
	if (chip->reset) {
		gpiod_set_value_cansleep(chip->reset, 1);
		usleep_range(2000, 3000);
		gpiod_set_value_cansleep(chip->reset, 0);
		usleep_range(2000, 3000);
	} else {
		dev_info(dev,
			 "厂商 dts 用 reset-gpio 私有写法，无标准 reset-gpios；"
			 "跳过硬件复位\n");
	}

	id = i2c_smbus_read_byte_data(client, AW8697_REG_CHIPID);
	if (id < 0) {
		dev_err(dev, "读 AW8697 chipid 失败: %d\n", id);
		return id;
	}
	dev_info(dev, "AW8697 chipid=0x%02x, vib_mode=%u, f0_pre=%u\n",
		 id, chip->vib_mode, chip->f0_pre);

	ret = aw8697_write(chip, AW8697_REG_SYSCTRL, AW8697_SYSCTRL_EN);
	if (ret) {
		dev_err(dev, "使能 AW8697 失败: %d\n", ret);
		return ret;
	}

	chip->cdev.name = "aw8697:haptic";
	chip->cdev.max_brightness = 255;
	chip->cdev.brightness_set_blocking = aw8697_brightness_set;

	ret = devm_led_classdev_register(dev, &chip->cdev);
	if (ret) {
		dev_err(dev, "注册 led_classdev 失败: %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct of_device_id aw8697_of_match[] = {
	{ .compatible = "awinic,aw8697_haptic" },
	{ }
};
MODULE_DEVICE_TABLE(of, aw8697_of_match);

static struct i2c_driver aw8697_driver = {
	.driver = {
		.name		= "aw8697-haptic",
		.of_match_table	= aw8697_of_match,
	},
	.probe = aw8697_probe,
};
module_i2c_driver(aw8697_driver);

MODULE_AUTHOR("ported from Xiaomi ares-r-oss / AWINIC");
MODULE_DESCRIPTION("Awinic AW8697 linear haptic driver (leds-class)");
MODULE_LICENSE("GPL");
