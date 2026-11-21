// SPDX-License-Identifier: GPL-2.0
/*
 * leds-aw2028.c — Awinic AW2028 三通道 RGB LED 驱动（主线重写版）
 *
 * 主线 drivers/leds/leds-aw200xx.c 只支持 aw20036/54/72/20108，不含 AW2028；
 * 本驱动按主线 leds-aw2013.c 的 Awinic 同族 3 通道寄存器框架重写，注册 3 个
 * led_classdev，用 regmap 做 I2C 访问、brightness_set_blocking 做亮度控制。
 *
 * ⚠️ 寄存器偏移沿用 AW2013 同族布局，需按 AW2028 datasheet 校准
 *    （宏集中在下方 AW2028_REG_*，校准时只改这里）。
 *
 * 厂商 DT：&i2c11 { aw2028@65 { compatible = "awinic, aw2028_i2c";
 *                              reg = <0x65>; }; };
 * 注意厂商 compatible 里有一个空格，故 of_match 同时接受规范写法与原样。
 */
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/regmap.h>
#include <linux/slab.h>

#define AW2028_MAX_LEDS        3
#define AW2028_REG_RSTR        0x00
#define AW2028_RSTR_RESET      0x55
#define AW2028_REG_GCR         0x01
#define AW2028_GCR_ENABLE      BIT(0)
#define AW2028_REG_LCTR        0x30
#define AW2028_REG_LCFG(x)     (0x31 + (x))
#define AW2028_REG_PWM(x)      (0x34 + (x))
#define AW2028_REG_MAX         0x77

static const char * const aw2028_led_names[AW2028_MAX_LEDS] = {
	"aw2028:red", "aw2028:green", "aw2028:blue",
};

struct aw2028_led {
	struct aw2028 *chip;
	struct led_classdev cdev;
	u32 num;
};

struct aw2028 {
	struct i2c_client *client;
	struct regmap *regmap;
	struct mutex lock;
	struct aw2028_led leds[AW2028_MAX_LEDS];
};

static const struct regmap_config aw2028_regmap_config = {
	.reg_bits	= 8,
	.val_bits	= 8,
	.max_register	= AW2028_REG_MAX,
};

static int aw2028_set_brightness(struct led_classdev *cdev,
				 enum led_brightness brightness)
{
	struct aw2028_led *led = container_of(cdev, struct aw2028_led, cdev);
	struct aw2028 *chip = led->chip;
	int ret;

	mutex_lock(&chip->lock);
	ret = regmap_write(chip->regmap, AW2028_REG_PWM(led->num), brightness);
	if (!ret)
		ret = regmap_update_bits(chip->regmap, AW2028_REG_LCTR,
					 BIT(led->num),
					 brightness ? BIT(led->num) : 0);
	mutex_unlock(&chip->lock);

	return ret;
}

static int aw2028_chip_init(struct aw2028 *chip)
{
	int ret, i;

	ret = regmap_write(chip->regmap, AW2028_REG_RSTR, AW2028_RSTR_RESET);
	if (ret)
		return ret;

	ret = regmap_write(chip->regmap, AW2028_REG_GCR, AW2028_GCR_ENABLE);
	if (ret)
		return ret;

	/* 三个通道设为最大电流档；具体档位需按 AW2028 datasheet 校准 */
	for (i = 0; i < AW2028_MAX_LEDS; i++) {
		ret = regmap_write(chip->regmap, AW2028_REG_LCFG(i), 0x03);
		if (ret)
			return ret;
	}

	return 0;
}

static int aw2028_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct aw2028 *chip;
	int i, ret;

	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->client = client;
	mutex_init(&chip->lock);

	chip->regmap = devm_regmap_init_i2c(client, &aw2028_regmap_config);
	if (IS_ERR(chip->regmap))
		return PTR_ERR(chip->regmap);

	ret = aw2028_chip_init(chip);
	if (ret) {
		dev_err(dev, "AW2028 芯片初始化失败: %d\n", ret);
		return ret;
	}

	/* 厂商 dts 无子节点，注册 3 个默认通道 */
	for (i = 0; i < AW2028_MAX_LEDS; i++) {
		struct aw2028_led *led = &chip->leds[i];

		led->chip = chip;
		led->num  = i;
		led->cdev.name = aw2028_led_names[i];
		led->cdev.max_brightness = 255;
		led->cdev.brightness_set_blocking = aw2028_set_brightness;

		ret = devm_led_classdev_register(dev, &led->cdev);
		if (ret) {
			dev_err(dev, "注册 LED 通道 %d 失败: %d\n", i, ret);
			return ret;
		}
	}

	dev_info(dev, "AW2028 三通道 LED 已注册\n");
	return 0;
}

static const struct of_device_id aw2028_of_match[] = {
	{ .compatible = "awinic,aw2028" },
	/* 厂商 dts 原样（含空格），保留以直接 probe 现有设备树 */
	{ .compatible = "awinic, aw2028_i2c" },
	{ }
};
MODULE_DEVICE_TABLE(of, aw2028_of_match);

static struct i2c_driver aw2028_driver = {
	.driver = {
		.name		= "aw2028-leds",
		.of_match_table	= aw2028_of_match,
	},
	.probe = aw2028_probe,
};
module_i2c_driver(aw2028_driver);

MODULE_AUTHOR("ported for Xiaomi ares / MT6893");
MODULE_DESCRIPTION("Awinic AW2028 3-channel RGB LED driver");
MODULE_LICENSE("GPL");
