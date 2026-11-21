// SPDX-License-Identifier: GPL-2.0
/*
 * xmusb350-charger.c — 小米 XMUSB350 USB 充电识别（主线重写版）
 *
 * 官方 4.14 未归档该驱动源码（线索：drivers/power/supply 或
 * drivers/misc/mediatek，defconfig 里有 CONFIG_MTK_CHARGER）。主线按
 * power_supply 框架重写：注册一个 USB charger psy，通过 I2C 读状态 +
 * 插入中断更新 online 并 power_supply_changed() 通知用户态。
 *
 * ⚠️ XMUSB350_REG_* 数值需按 datasheet 校准；厂商 dts 的
 *    xm,xmusb350_{rst,sda,scl}_gpio 是 MTK pio 三元组，主线无法解析，
 *    本驱动只用标准 pinctrl 状态与 client->irq。
 *
 * 厂商 DT：&i2c11 { xmusb350-charger@35 {
 *     compatible = "xm,xmusb350-charger"; reg = <0x35>;
 *     interrupts = <110 IRQ_TYPE_EDGE_FALLING 110 0>;
 *     charger_name = "secondary_chg"; alias_name = "xmusb350";
 *     en_bc12; en_hvdcp; }; };
 */
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#define XMUSB350_REG_STATUS	0x00
#define XMUSB350_STATUS_ONLINE	BIT(0)

struct xmusb350 {
	struct i2c_client	*client;
	struct power_supply	*psy;
	struct power_supply_desc desc;
	struct work_struct	 work;
	const char		*charger_name;
	const char		*alias_name;
	bool			 online;
	bool			 en_bc12;
	bool			 en_hvdcp;
};

static enum power_supply_property xmusb350_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_STATUS,
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
	default:
		return -EINVAL;
	}

	return 0;
}

static void xmusb350_work(struct work_struct *work)
{
	struct xmusb350 *chip = container_of(work, struct xmusb350, work);
	int ret;

	ret = i2c_smbus_read_byte_data(chip->client, XMUSB350_REG_STATUS);
	if (ret < 0)
		return;

	chip->online = !!(ret & XMUSB350_STATUS_ONLINE);
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
	int ret;

	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->client = client;
	i2c_set_clientdata(client, chip);
	INIT_WORK(&chip->work, xmusb350_work);
	chip->en_bc12  = of_property_read_bool(dev->of_node, "en_bc12");
	chip->en_hvdcp = of_property_read_bool(dev->of_node, "en_hvdcp");
	of_property_read_string(dev->of_node, "charger_name",
				&chip->charger_name);
	of_property_read_string(dev->of_node, "alias_name", &chip->alias_name);

	chip->desc.name		 = chip->charger_name ? : "xmusb350";
	chip->desc.type		 = POWER_SUPPLY_TYPE_USB;
	chip->desc.properties	 = xmusb350_props;
	chip->desc.num_properties = ARRAY_SIZE(xmusb350_props);
	chip->desc.get_property	 = xmusb350_get_property;

	cfg.drv_data = chip;

	chip->psy = devm_power_supply_register(dev, &chip->desc, &cfg);
	if (IS_ERR(chip->psy)) {
		ret = PTR_ERR(chip->psy);
		dev_err(dev, "注册 power_supply 失败: %d\n", ret);
		return ret;
	}

	if (client->irq > 0) {
		ret = devm_request_irq(dev, client->irq, xmusb350_irq,
				       IRQF_TRIGGER_FALLING, "xmusb350", chip);
		if (ret) {
			dev_err(dev, "申请 IRQ %d 失败: %d\n", client->irq, ret);
			return ret;
		}
	}

	schedule_work(&chip->work);
	dev_info(dev, "XMUSB350 已注册（name=%s, bc12=%d, hvdcp=%d）\n",
		 chip->desc.name, chip->en_bc12, chip->en_hvdcp);
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
		.name		= "xmusb350-charger",
		.of_match_table	= xmusb350_of_match,
	},
	.probe = xmusb350_probe,
	.remove = xmusb350_remove,
};
module_i2c_driver(xmusb350_driver);

MODULE_AUTHOR("ported for Xiaomi ares / MT6893");
MODULE_DESCRIPTION("Xiaomi XMUSB350 USB charge-detection driver");
MODULE_LICENSE("GPL");
