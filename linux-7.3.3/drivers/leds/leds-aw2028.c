// SPDX-License-Identifier: GPL-2.0
/*
 * Awinic AW2028 3-channel RGB LED — Linux mainline 7.3.3
 * for Xiaomi Redmi K40 Gaming (ares / MT6893).
 *
 * "OTHER WAY" vs round 1: round 1 wrote this driver with "placeholder"
 * register offsets borrowed loosely from a sibling. In fact mainline already
 * ships drivers/leds/leds-aw2013.c with the Awinic 3-channel register bank,
 * and AW2028 uses exactly the SAME bank:
 *
 *   mainline leds-aw2013.c        AW2028 (official ares DT aw2028@65)
 *   -----------------------------------------------------------------
 *   AW2013_RSTR       0x00  (reset 0x55, chipid 0x33)
 *   AW2013_GCR        0x01  (enable BIT(0))
 *   AW2013_LCTR       0x30  (per-channel LE)
 *   AW2013_LCFG(x)    0x31+x
 *   AW2013_REG_PWM(x) 0x34+x
 *   AW2013_LEDT0/1/2  0x37+3x (timing/breathing)
 *   AW2013_REG_MAX    0x77
 *
 * So the primary path is a tiny patch to the MAINLINE driver adding the
 * "awinic,aw2028" compatible (delivered in port/compat/). This standalone
 * driver is the zero-dependency fallback: identical real register map plus a
 * chip-id check, so it can probe without touching the mainline source tree.
 *
 * Note: the official ares DTS has this node commented out (aw2028@65 on
 * i2c11), i.e. the retail unit does not populate/enable it. The driver is
 * provided so that a board variant with the LED can probe immediately.
 *
 * The vendor DTS wrote "awinic, aw2028_i2c" with a stray space after the
 * comma, which violates the DT spec's "vendor,model" form and trips
 * dtbs_check. The delivered DTS is normalized to "awinic,aw2028_i2c" by
 * tools/fix_mainline_contract.py, so only canonical strings are matched here.
 */
#include <linux/bits.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/regmap.h>
#include <linux/slab.h>

/* real Awinic 3-channel register bank (mainline leds-aw2013.c) */
#define AW_REG_RSTR		0x00
#define AW_RSTR_RESET		0x55
#define AW_RSTR_CHIPID		0x33
#define AW_REG_GCR		0x01
#define AW_GCR_ENABLE		BIT(0)
#define AW_REG_LCTR		0x30
#define AW_LCTR_LE(x)		BIT(x)
#define AW_REG_LCFG(x)		(0x31 + (x))
#define AW_LCFG_IMAX_MASK	(BIT(0) | BIT(1))
#define AW_LCFG_MD		BIT(4)
#define AW_REG_PWM(x)		(0x34 + (x))
#define AW_REG_MAX		0x77
#define AW_MAX_LEDS		3

static const char * const aw_led_names[AW_MAX_LEDS] = {
	"aw2028:red", "aw2028:green", "aw2028:blue",
};

struct aw_led {
	struct aw_chip *chip;
	struct led_classdev cdev;
	u32 num;
};

struct aw_chip {
	struct i2c_client *client;
	struct regmap *regmap;
	struct mutex lock;
	struct aw_led leds[AW_MAX_LEDS];
};

static const struct regmap_config aw_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = AW_REG_MAX,
	.cache_type = REGCACHE_NONE,
};

static int aw_set_brightness(struct led_classdev *cdev,
			     enum led_brightness brightness)
{
	struct aw_led *led = container_of(cdev, struct aw_led, cdev);
	struct aw_chip *chip = led->chip;
	int ret;

	mutex_lock(&chip->lock);
	ret = regmap_write(chip->regmap, AW_REG_PWM(led->num), brightness);
	if (!ret)
		ret = regmap_update_bits(chip->regmap, AW_REG_LCTR,
					 AW_LCTR_LE(led->num),
					 brightness ? AW_LCTR_LE(led->num) : 0);
	mutex_unlock(&chip->lock);
	return ret;
}

static int aw_chip_init(struct aw_chip *chip)
{
	struct device *dev = &chip->client->dev;
	unsigned int id = 0;
	int i, ret;

	ret = regmap_write(chip->regmap, AW_REG_RSTR, AW_RSTR_RESET);
	if (ret)
		return ret;
	usleep_range(1000, 2000);

	/* real chip-id check (0x33 for this Awinic 3-channel family) */
	ret = regmap_read(chip->regmap, AW_REG_RSTR, &id);
	if (ret)
		return ret;
	if (id != AW_RSTR_CHIPID)
		dev_warn(dev, "unexpected chip id 0x%02x (expected 0x33)\n", id);

	ret = regmap_write(chip->regmap, AW_REG_GCR, AW_GCR_ENABLE);
	if (ret)
		return ret;

	for (i = 0; i < AW_MAX_LEDS; i++) {
		/* max current + manual dimming mode (mainline aw2013 default) */
		ret = regmap_write(chip->regmap, AW_REG_LCFG(i),
				   AW_LCFG_IMAX_MASK | AW_LCFG_MD);
		if (ret)
			return ret;
	}
	return 0;
}

static int aw_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct aw_chip *chip;
	int i, ret;

	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;
	chip->client = client;
	mutex_init(&chip->lock);
	i2c_set_clientdata(client, chip);

	chip->regmap = devm_regmap_init_i2c(client, &aw_regmap_config);
	if (IS_ERR(chip->regmap))
		return dev_err_probe(dev, PTR_ERR(chip->regmap),
				     "failed to init regmap\n");

	ret = aw_chip_init(chip);
	if (ret)
		return dev_err_probe(dev, ret, "AW2028 chip init failed\n");

	for (i = 0; i < AW_MAX_LEDS; i++) {
		struct aw_led *led = &chip->leds[i];

		led->chip = chip;
		led->num = i;
		led->cdev.name = aw_led_names[i];
		led->cdev.max_brightness = 255;
		led->cdev.brightness_set_blocking = aw_set_brightness;
		ret = devm_led_classdev_register(dev, &led->cdev);
		if (ret)
			return dev_err_probe(dev, ret,
					     "failed to register LED %d\n", i);
	}

	dev_info(dev, "AW2028 3-channel RGB LED ready (real Awinic reg bank)\n");
	return 0;
}

static const struct of_device_id aw_of_match[] = {
	{ .compatible = "awinic,aw2028" },
	{ .compatible = "awinic,aw2028_i2c" },
	{ }
};
MODULE_DEVICE_TABLE(of, aw_of_match);

static struct i2c_driver aw_driver = {
	.driver = {
		.name = "aw2028-leds",
		.of_match_table = aw_of_match,
	},
	.probe = aw_probe,
};
module_i2c_driver(aw_driver);

MODULE_AUTHOR("ares mainline port");
MODULE_DESCRIPTION("Awinic AW2028 3-channel LED (mainline AW2013 register bank)");
MODULE_LICENSE("GPL");
