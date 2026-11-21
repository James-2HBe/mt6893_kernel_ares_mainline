// SPDX-License-Identifier: GPL-2.0
/*
 * regulator-wl2866d.c — Will Semiconductor WL2866D 相机 LDO 驱动（主线版）
 *
 * 重写自小米官方 ares-r-oss 4.14 内核 drivers/regulator/wl2866d.c。
 * 保留芯片寄存器定义与电压换算；删除厂商私有部分：
 *   - cdev/class/sysfs 调试接口（camera_ldo 字符设备，供厂商 HAL 用）
 *   - 4.14 双参数 i2c probe() / i2c_device_id / simple_strtoul
 *   - CONFIG_DRM_PANEL_K10_42_02_0A_DSC_CMD 的面板特判
 *   - 手工父电源投票（官方 supply_name 恒为 "none"，实际不存在）
 * 全部改用主线 devm_* / regmap / regulator 框架。
 *
 * 厂商 DT 契约（保持不变，逐项由本驱动解析）：
 *   &i2c4 { ldo@28 {
 *       compatible = "willsemi,wl2866d1";
 *       reg = <0x28>;
 *       index = <0>;
 *       on,wl2866d1@0 {
 *           offset = /bits/ 16 <0x0000>;
 *           regulator-name = "wl2866d-dvdd1";
 *           regulator-min-microvolt = <600000>;
 *           regulator-max-microvolt = <1800000>;
 *           min-dropout-voltage = <80000>;
 *           iout_ua = <500000>;
 *           init-voltage = <1050000>;
 *       };
 *   }; };
 *
 * 电压档位：offset 0/1 为 DVDD（600mV 起，6mV/档）；
 *           offset 2/3 为 AVDD（1200mV 起，12.5mV/档）。
 */
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/math.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regmap.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/slab.h>

#define WL2866D_REG_PID         0x00
#define WL2866D_REG_DISCHARGE   0x02
#define WL2866D_REG_LDO0        0x03
#define WL2866D_REG_ENABLE      0x0e

#define LDO_VSET_REG(offset)    ((offset) + WL2866D_REG_LDO0)

#define VSET_DVDD_BASE_UV       600000
#define VSET_DVDD_STEP_UV       6000
#define VSET_AVDD_BASE_UV       1200000
#define VSET_AVDD_STEP_UV       12500

#define WL2866D_MAX_LDO         4

struct wl2866d_ldo {
	struct regulator_desc		desc;
	struct regulator_init_data	init_data;
	struct regulator_dev		*rdev;
	struct regmap			*regmap;
	struct device			*dev;
	struct device_node		*of_node;
	u16				offset;
	unsigned int			index;
};

struct wl2866d_chip {
	struct regmap	*regmap;
	struct device	*dev;
	unsigned int	 index;
};

static const struct regmap_config wl2866d_regmap_config = {
	.reg_bits	= 8,
	.val_bits	= 8,
	.max_register	= 0x0f,
};

static int wl2866d_is_enabled(struct regulator_dev *rdev)
{
	struct wl2866d_ldo *ldo = rdev_get_drvdata(rdev);
	unsigned int val;
	int ret;

	ret = regmap_read(ldo->regmap, WL2866D_REG_ENABLE, &val);
	if (ret)
		return ret;

	return !!(val & BIT(ldo->offset));
}

static int wl2866d_enable(struct regulator_dev *rdev)
{
	struct wl2866d_ldo *ldo = rdev_get_drvdata(rdev);

	return regmap_update_bits(ldo->regmap, WL2866D_REG_ENABLE,
				  BIT(ldo->offset), BIT(ldo->offset));
}

static int wl2866d_disable(struct regulator_dev *rdev)
{
	struct wl2866d_ldo *ldo = rdev_get_drvdata(rdev);

	return regmap_update_bits(ldo->regmap, WL2866D_REG_ENABLE,
				  BIT(ldo->offset), 0);
}

static int wl2866d_get_voltage(struct regulator_dev *rdev)
{
	struct wl2866d_ldo *ldo = rdev_get_drvdata(rdev);
	unsigned int vset;
	int ret;

	ret = regmap_read(ldo->regmap, LDO_VSET_REG(ldo->offset), &vset);
	if (ret)
		return ret;

	if (vset == 0)
		return ldo->init_data.constraints.min_uV;

	if (ldo->offset < 2)
		return VSET_DVDD_BASE_UV + vset * VSET_DVDD_STEP_UV;

	return VSET_AVDD_BASE_UV + vset * VSET_AVDD_STEP_UV;
}

static int wl2866d_set_voltage(struct regulator_dev *rdev, int min_uV,
			       int max_uV, unsigned int *selector)
{
	struct wl2866d_ldo *ldo = rdev_get_drvdata(rdev);
	int base, step, vset;
	unsigned int val;

	if (min_uV > max_uV)
		return -EINVAL;

	if (ldo->offset < 2) {
		base = VSET_DVDD_BASE_UV;
		step = VSET_DVDD_STEP_UV;
	} else {
		base = VSET_AVDD_BASE_UV;
		step = VSET_AVDD_STEP_UV;
	}

	if (min_uV < base)
		return -EINVAL;

	vset = DIV_ROUND_UP(min_uV - base, step);
	if (vset > 0xff || base + vset * step > max_uV)
		return -EINVAL;

	*selector = 0;
	val = vset;
	return regmap_write(ldo->regmap, LDO_VSET_REG(ldo->offset), val);
}

static const struct regulator_ops wl2866d_ops = {
	.enable		= wl2866d_enable,
	.disable	= wl2866d_disable,
	.is_enabled	= wl2866d_is_enabled,
	.set_voltage	= wl2866d_set_voltage,
	.get_voltage	= wl2866d_get_voltage,
};

static int wl2866d_register_ldo(struct wl2866d_chip *chip,
				struct device_node *child)
{
	struct device *dev = chip->dev;
	struct wl2866d_ldo *ldo;
	struct regulation_constraints *c;
	struct regulator_config config = { };
	const char *name;
	u32 val;
	unsigned int selector = 0;
	int ret, max_uv;

	/* 没有 regulator-name 的子节点不是 LDO 描述，跳过 */
	if (of_property_read_string(child, "regulator-name", &name))
		return 0;

	ldo = devm_kzalloc(dev, sizeof(*ldo), GFP_KERNEL);
	if (!ldo)
		return -ENOMEM;

	ret = of_property_read_u16(child, "offset", &ldo->offset);
	if (ret) {
		dev_err(dev, "%s: 缺 offset 属性: %d\n", name, ret);
		return ret;
	}
	if (ldo->offset >= WL2866D_MAX_LDO) {
		dev_err(dev, "%s: offset %u 超出 0..%d\n",
			name, ldo->offset, WL2866D_MAX_LDO - 1);
		return -EINVAL;
	}

	ldo->regmap  = chip->regmap;
	ldo->dev     = dev;
	ldo->of_node = child;
	ldo->index   = chip->index;

	c = &ldo->init_data.constraints;
	c->name = name;
	if (!of_property_read_u32(child, "regulator-min-microvolt", &val))
		c->min_uV = val;
	if (!of_property_read_u32(child, "regulator-max-microvolt", &val))
		c->max_uV = val;
	c->valid_ops_mask   = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE;
	c->valid_modes_mask = REGULATOR_MODE_NORMAL;

	ldo->desc.name	      = name;
	ldo->desc.type	      = REGULATOR_VOLTAGE;
	ldo->desc.owner	      = THIS_MODULE;
	ldo->desc.ops	      = &wl2866d_ops;
	ldo->desc.n_voltages  = 1;

	config.dev	    = dev;
	config.init_data    = &ldo->init_data;
	config.driver_data  = ldo;
	config.of_node	    = child;

	ldo->rdev = devm_regulator_register(dev, &ldo->desc, &config);
	if (IS_ERR(ldo->rdev)) {
		ret = PTR_ERR(ldo->rdev);
		dev_err(dev, "%s: regulator 注册失败: %d\n", name, ret);
		return ret;
	}

	/* 厂商 init-voltage：注册后写入初始档位 */
	if (!of_property_read_u32(child, "init-voltage", &val) && val > 0) {
		max_uv = c->max_uV ? c->max_uV : val;
		ret = wl2866d_set_voltage(ldo->rdev, val, max_uv, &selector);
		if (ret)
			dev_warn(dev, "%s: 写初始电压 %u uV 失败: %d\n",
				 name, val, ret);
	}

	dev_info(dev, "registered %s (offset %u, %d..%d uV)\n",
		 name, ldo->offset, c->min_uV, c->max_uV);
	return 0;
}

static int wl2866d_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct wl2866d_chip *chip;
	struct device_node *child;
	unsigned int pid;
	int ret;

	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->dev = dev;
	of_property_read_u32(dev->of_node, "index", &chip->index);

	chip->regmap = devm_regmap_init_i2c(client, &wl2866d_regmap_config);
	if (IS_ERR(chip->regmap)) {
		ret = PTR_ERR(chip->regmap);
		dev_err(dev, "regmap 初始化失败: %d\n", ret);
		return ret;
	}

	ret = regmap_read(chip->regmap, WL2866D_REG_PID, &pid);
	if (ret) {
		dev_err(dev, "读 WL2866D PID 失败（芯片未响应？）: %d\n", ret);
		return ret;
	}
	dev_info(dev, "WL2866D product id = 0x%02x, index = %u\n",
		 pid, chip->index);

	/* 官方驱动上电时打开全部 LDO 的放电电阻 */
	ret = regmap_write(chip->regmap, WL2866D_REG_DISCHARGE, 0xff);
	if (ret)
		dev_warn(dev, "写放电寄存器 0x02 失败: %d\n", ret);

	for_each_available_child_of_node(dev->of_node, child) {
		ret = wl2866d_register_ldo(chip, child);
		if (ret) {
			of_node_put(child);
			return ret;
		}
	}

	return 0;
}

static const struct of_device_id wl2866d_of_match[] = {
	{ .compatible = "willsemi,wl2866d1" },
	{ .compatible = "willsemi,wl2866d2" },
	{ }
};
MODULE_DEVICE_TABLE(of, wl2866d_of_match);

static struct i2c_driver wl2866d_driver = {
	.driver = {
		.name		= "wl2866d-regulator",
		.of_match_table	= wl2866d_of_match,
	},
	.probe = wl2866d_probe,
};
module_i2c_driver(wl2866d_driver);

MODULE_AUTHOR("ported from Xiaomi ares-r-oss (liuqinhong@xiaomi.com)");
MODULE_DESCRIPTION("Will Semiconductor WL2866D camera LDO regulator driver");
MODULE_LICENSE("GPL");
