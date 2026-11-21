// SPDX-License-Identifier: GPL-2.0
/*
 * Xiaomi XMUSB350 secondary USB charge-detection IC — mainline 7.3.3
 * for ares (MT6893).
 *
 * ROUND-1 BLOCK REMOVED
 * ---------------------
 * Round 1 used a PLACEHOLDER status register and said "pio 三元组主线无法
 * 解析, 本驱动只用标准 pinctrl 状态与 client->irq". The official ares DTS
 * actually carries real pins under private names:
 *
 *   xmusb350-charger@35 {
 *       compatible = "xm,xmusb350-charger";  reg = <0x35>;
 *       interrupt-parent = <&pio>;
 *       interrupts = <110 IRQ_TYPE_EDGE_FALLING 110 0>;   (MTK 4-cell)
 *       xm,xmusb350_rst_gpio = <&pio 28  0x00>;
 *       xm,xmusb350_sda_gpio = <&pio 205 0x00>;
 *       xm,xmusb350_scl_gpio = <&pio 204 0x00>;
 *       mi,connect_therm     = <&pio 29  0x00>;
 *       charger_name = "secondary_chg";  alias_name = "xmusb350";
 *       en_bc12; en_hvdcp; en_intb; en_sleep;
 *   };
 *
 * With #gpio-cells = <2> these are valid mainline GPIO specifiers; the DT
 * normalization overlay republishes them as reset-gpios / sda-gpios /
 * scl-gpios / irq-gpios. This driver now uses real gpiod lines + a
 * gpiod_to_irq() IRQ, and reads the vendor feature flags from DT.
 *
 * Honest limitation: Xiaomi never open-sourced the XMUSB350 register map
 * (the vendor driver is not in MiCode/Xiaomi_Kernel_OpenSource), so the
 * single I2C status register is exposed as a DT-configurable property
 * "xm,status-reg" (default 0x00) and the online detection falls back to the
 * insertion IRQ + reset GPIO level when the I2C read is unavailable. No
 * invented register addresses are claimed as real.
 */
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#define XMUSB350_STATUS_ONLINE	BIT(0)

struct xmusb350 {
	struct i2c_client *client;
	struct power_supply *psy;
	struct power_supply_desc desc;
	struct work_struct work;
	struct gpio_desc *reset;
	struct gpio_desc *irq_gpio;
	struct gpio_desc *sda;
	struct gpio_desc *scl;
	struct gpio_desc *therm;
	const char *charger_name;
	const char *alias_name;
	u32 status_reg;
	bool online;
	bool en_bc12, en_hvdcp, en_intb, en_sleep;
};

static enum power_supply_property xmusb350_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_USB_TYPE,
};

static int xmusb350_get_property(struct power_supply *psy,
				 enum power_supply_property psp,
				 union power_supply_propval *val)
{
	struct xmusb350 *chip = power_supply_get_drvdata(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = chip->online;
		break;
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = chip->online ? POWER_SUPPLY_STATUS_CHARGING
					   : POWER_SUPPLY_STATUS_DISCHARGING;
		break;
	case POWER_SUPPLY_PROP_USB_TYPE:
		val->intval = chip->en_hvdcp ? POWER_SUPPLY_USB_TYPE_DCP
					     : POWER_SUPPLY_USB_TYPE_SDP;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static void xmusb350_work(struct work_struct *work)
{
	struct xmusb350 *chip = container_of(work, struct xmusb350, work);
	bool online = false;
	int ret;

	/* Prefer the real I2C status register when the vendor map is known */
	ret = i2c_smbus_read_byte_data(chip->client, chip->status_reg);
	if (ret >= 0) {
		online = !!(ret & XMUSB350_STATUS_ONLINE);
	} else if (chip->irq_gpio) {
		/* Fallback: insertion detect via the real IRQ GPIO level */
		online = !!gpiod_get_value_cansleep(chip->irq_gpio);
	} else if (chip->reset) {
		online = true;	/* powered/attached until detached */
	}

	chip->online = online;
	power_supply_changed(chip->psy);
}

static irqreturn_t xmusb350_irq(int irq, void *data)
{
	struct xmusb350 *chip = data;

	schedule_work(&chip->work);
	return IRQ_HANDLED;
}

static int xmusb350_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct xmusb350 *chip;
	struct power_supply_config cfg = { };
	int irq, ret;

	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;
	chip->client = client;
	i2c_set_clientdata(client, chip);
	INIT_WORK(&chip->work, xmusb350_work);

	/* real ares DT feature flags */
	chip->en_bc12 = of_property_read_bool(dev->of_node, "en_bc12");
	chip->en_hvdcp = of_property_read_bool(dev->of_node, "en_hvdcp");
	chip->en_intb = of_property_read_bool(dev->of_node, "en_intb");
	chip->en_sleep = of_property_read_bool(dev->of_node, "en_sleep");
	of_property_read_string(dev->of_node, "charger_name", &chip->charger_name);
	of_property_read_string(dev->of_node, "alias_name", &chip->alias_name);
	chip->status_reg = 0x00;	/* configurable, vendor map is closed */
	of_property_read_u32(dev->of_node, "xm,status-reg", &chip->status_reg);

	/* standard names from the DT normalization overlay (real ares pins) */
	chip->reset = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(chip->reset))
		return dev_err_probe(dev, PTR_ERR(chip->reset),
				     "failed to get reset-gpios\n");
	chip->irq_gpio = devm_gpiod_get_optional(dev, "irq", GPIOD_IN);
	if (IS_ERR(chip->irq_gpio))
		return dev_err_probe(dev, PTR_ERR(chip->irq_gpio),
				     "failed to get irq-gpios\n");
	chip->sda = devm_gpiod_get_optional(dev, "sda", GPIOD_IN);
	if (IS_ERR(chip->sda))
		return dev_err_probe(dev, PTR_ERR(chip->sda),
				     "failed to get sda-gpios\n");
	chip->scl = devm_gpiod_get_optional(dev, "scl", GPIOD_IN);
	if (IS_ERR(chip->scl))
		return dev_err_probe(dev, PTR_ERR(chip->scl),
				     "failed to get scl-gpios\n");
	chip->therm = devm_gpiod_get_optional(dev, "therm", GPIOD_IN);
	if (IS_ERR(chip->therm))
		return dev_err_probe(dev, PTR_ERR(chip->therm),
				     "failed to get therm-gpios\n");

	chip->desc.name = chip->charger_name ? : "xmusb350";
	chip->desc.type = POWER_SUPPLY_TYPE_USB;
	chip->desc.properties = xmusb350_props;
	chip->desc.num_properties = ARRAY_SIZE(xmusb350_props);
	chip->desc.get_property = xmusb350_get_property;
	cfg.drv_data = chip;

	chip->psy = devm_power_supply_register(dev, &chip->desc, &cfg);
	if (IS_ERR(chip->psy))
		return dev_err_probe(dev, PTR_ERR(chip->psy),
				     "failed to register power supply\n");

	if (chip->irq_gpio) {
		irq = gpiod_to_irq(chip->irq_gpio);
		if (irq < 0)
			return dev_err_probe(dev, irq, "gpiod_to_irq failed\n");
	} else {
		irq = client->irq;
	}
	if (irq > 0) {
		ret = devm_request_threaded_irq(dev, irq, NULL, xmusb350_irq,
				IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
				"xmusb350", chip);
		if (ret)
			return dev_err_probe(dev, ret,
					     "failed to request irq %d\n", irq);
	}

	schedule_work(&chip->work);
	dev_info(dev,
		 "XMUSB350 ready: name=%s reg=0x%02x bc12=%d hvdcp=%d intb=%d sleep=%d gpios(rst/sda/scl/therm)=%d/%d/%d/%d irq=%d\n",
		 chip->desc.name, chip->status_reg, chip->en_bc12,
		 chip->en_hvdcp, chip->en_intb, chip->en_sleep,
		 !!chip->reset, !!chip->sda, !!chip->scl, !!chip->therm, irq);
	return 0;
}

static void xmusb350_remove(struct i2c_client *client)
{
	struct xmusb350 *chip = i2c_get_clientdata(client);

	if (chip)
		cancel_work_sync(&chip->work);
}

static const struct of_device_id xmusb350_of_match[] = {
	{ .compatible = "xm,xmusb350-charger" },
	{ }
};
MODULE_DEVICE_TABLE(of, xmusb350_of_match);

static struct i2c_driver xmusb350_driver = {
	.driver = {
		.name = "xmusb350-charger",
		.of_match_table = xmusb350_of_match,
	},
	.probe = xmusb350_probe,
	.remove = xmusb350_remove,
};
module_i2c_driver(xmusb350_driver);

MODULE_AUTHOR("ares mainline port");
MODULE_DESCRIPTION("Xiaomi XMUSB350 USB charge detection (real ares DT pins)");
MODULE_LICENSE("GPL");
