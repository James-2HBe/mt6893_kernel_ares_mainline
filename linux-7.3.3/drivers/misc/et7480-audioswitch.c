// SPDX-License-Identifier: GPL-2.0
/*
 * et7480-audioswitch.c — ET7480 音频通路开关（主线重写版）
 *
 * 官方 4.14 没有独立驱动源码：节点只是 &accdet 的 et7480-i2c-handle 引用，
 * 由 MTK accdet 框架在用户态切换。主线没有 accdet，故按标准 i2c_driver
 * + pinctrl 状态切换 + sysfs 重写，且不猜任何寄存器：
 *   - 取厂商 dts 的两个 pinctrl 状态 "uart_enable" / "uart_disable"
 *   - 通过 sysfs 属性 et7480_switch 读写 0/1 切换通路
 *   - 状态切换失败时仍记录软件状态，便于无 pinctrl 的板级调试
 *
 * 厂商 DT：&i2c9 { et7480@42 {
 *     compatible = "mediatek,et7480-audioswitch";
 *     reg = <0x42>;
 *     pinctrl-names = "uart_enable", "uart_disable";
 *     pinctrl-0 = <&uart_gpio_enable>;
 *     pinctrl-1 = <&uart_gpio_disable>;
 * }; };
 */
#include <linux/device.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/pinctrl/consumer.h>
#include <linux/slab.h>
#include <linux/sysfs.h>

struct et7480 {
	struct i2c_client	*client;
	struct pinctrl		*pctrl;
	struct pinctrl_state	*enable;
	struct pinctrl_state	*disable;
	struct mutex		 lock;
	bool			 uart_enabled;
};

static int et7480_apply(struct et7480 *chip, bool enable)
{
	struct pinctrl_state *state = enable ? chip->enable : chip->disable;
	int ret = 0;

	if (!IS_ERR_OR_NULL(state))
		ret = pinctrl_select_state(chip->pctrl, state);

	chip->uart_enabled = enable;
	return ret;
}

static ssize_t et7480_switch_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct et7480 *chip = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%u\n", chip->uart_enabled);
}

static ssize_t et7480_switch_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t len)
{
	struct et7480 *chip = dev_get_drvdata(dev);
	bool enable;
	int ret;

	ret = kstrtobool(buf, &enable);
	if (ret)
		return ret;

	mutex_lock(&chip->lock);
	ret = et7480_apply(chip, enable);
	mutex_unlock(&chip->lock);
	if (ret)
		return ret;

	return len;
}
static DEVICE_ATTR_RW(et7480_switch);

static int et7480_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct et7480 *chip;
	int ret;

	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	chip->client = client;
	mutex_init(&chip->lock);
	dev_set_drvdata(dev, chip);

	chip->pctrl = devm_pinctrl_get(dev);
	if (IS_ERR(chip->pctrl)) {
		dev_info(dev, "无 pinctrl 状态（%ld），仅提供 sysfs 开关\n",
			 PTR_ERR(chip->pctrl));
		chip->pctrl = NULL;
	} else {
		chip->enable  = pinctrl_lookup_state(chip->pctrl, "uart_enable");
		chip->disable = pinctrl_lookup_state(chip->pctrl, "uart_disable");
	}

	ret = device_create_file(dev, &dev_attr_et7480_switch);
	if (ret)
		return ret;

	/* 默认关闭 uart 通路 */
	et7480_apply(chip, false);

	i2c_set_clientdata(client, chip);
	dev_info(dev, "ET7480 音频通路开关已注册\n");
	return 0;
}

static void et7480_remove(struct i2c_client *client)
{
	device_remove_file(&client->dev, &dev_attr_et7480_switch);
}

static const struct of_device_id et7480_of_match[] = {
	{ .compatible = "mediatek,et7480-audioswitch" },
	{ }
};
MODULE_DEVICE_TABLE(of, et7480_of_match);

static struct i2c_driver et7480_driver = {
	.driver = {
		.name		= "et7480-audioswitch",
		.of_match_table	= et7480_of_match,
	},
	.probe = et7480_probe,
	.remove = et7480_remove,
};
module_i2c_driver(et7480_driver);

MODULE_AUTHOR("ported for Xiaomi ares / MT6893");
MODULE_DESCRIPTION("ET7480 audio path switch driver (pinctrl based)");
MODULE_LICENSE("GPL");
