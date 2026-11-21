// SPDX-License-Identifier: GPL-2.0
/*
 * input-gamekey.c — ares 弹出式肩键（主线重写版）
 *
 * 厂商 4.14 用 MTK 私有 gamekey 框架（CONFIG_MTK_GAMEKEY）+ 霍尔 GPIO，
 * 主线无对应子系统。本驱动按标准 platform_driver + input_dev + gpiod 重写：
 *   - 读取可选 "key-gpios"（标准 DT 写法），每个 GPIO 双边沿中断上报按键
 *   - 厂商 dts 的 gamekey 节点是空的（无任何 GPIO 属性），此时只注册
 *     input 设备并打印警告 —— 需补 GPIO 描述才会产生事件
 *
 * 厂商 DT：gamekey { compatible = "xm,gamekey"; };
 * 建议补：key-gpios = <&pio 111 GPIO_ACTIVE_LOW>, <&pio 112 GPIO_ACTIVE_LOW>;
 *         （具体 pin 号按真机原理图，见 docs/PORTED-DRIVERS.md）
 */
#include <linux/gpio/consumer.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#define GAMEKEY_MAX_KEYS	4

/* 左/右肩键 + 两个霍尔；键码需按真机 userspace keylayout 调整 */
static const unsigned int gamekey_keycodes[GAMEKEY_MAX_KEYS] = {
	KEY_F1, KEY_F2, KEY_F3, KEY_F4,
};

struct gamekey {
	struct input_dev	*input;
	struct gpio_descs	*keys;
};

static irqreturn_t gamekey_irq(int irq, void *dev_id)
{
	struct gamekey *gk = dev_id;
	int i, val;

	for (i = 0; i < gk->keys->ndescs && i < GAMEKEY_MAX_KEYS; i++) {
		val = gpiod_get_value_cansleep(gk->keys->desc[i]);
		if (val < 0)
			continue;
		input_report_key(gk->input, gamekey_keycodes[i], val);
	}
	input_sync(gk->input);

	return IRQ_HANDLED;
}

static int gamekey_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct gamekey *gk;
	struct input_dev *input;
	int i, ret, irq, nkeys;

	gk = devm_kzalloc(dev, sizeof(*gk), GFP_KERNEL);
	if (!gk)
		return -ENOMEM;

	input = devm_input_allocate_device(dev);
	if (!input)
		return -ENOMEM;

	input->name = "ares-gamekey";
	input->id.bustype = BUS_HOST;
	__set_bit(EV_KEY, input->evbit);
	for (i = 0; i < GAMEKEY_MAX_KEYS; i++)
		__set_bit(gamekey_keycodes[i], input->keybit);

	gk->input = input;
	ret = input_register_device(input);
	if (ret) {
		dev_err(dev, "注册 input 设备失败: %d\n", ret);
		return ret;
	}

	gk->keys = devm_gpiod_get_array_optional(dev, "key", GPIOD_IN);
	if (IS_ERR(gk->keys)) {
		ret = PTR_ERR(gk->keys);
		dev_err(dev, "读取 key-gpios 失败: %d\n", ret);
		return ret;
	}
	if (!gk->keys) {
		dev_warn(dev,
			 "厂商 dts 的 gamekey 节点无 key-gpios：仅注册 input 设备，"
			 "不会产生按键事件；需补 GPIO 描述\n");
		return 0;
	}

	nkeys = min_t(int, gk->keys->ndescs, GAMEKEY_MAX_KEYS);
	for (i = 0; i < nkeys; i++) {
		irq = gpiod_to_irq(gk->keys->desc[i]);
		if (irq < 0) {
			dev_warn(dev, "GPIO %d 无中断号: %d\n", i, irq);
			continue;
		}
		ret = devm_request_irq(dev, irq, gamekey_irq,
				       IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
				       "ares-gamekey", gk);
		if (ret) {
			dev_err(dev, "申请 GPIO %d 中断失败: %d\n", i, ret);
			return ret;
		}
	}

	dev_info(dev, "已注册 %d 个肩键 GPIO\n", nkeys);
	return 0;
}

static const struct of_device_id gamekey_of_match[] = {
	{ .compatible = "xm,gamekey" },
	{ }
};
MODULE_DEVICE_TABLE(of, gamekey_of_match);

static struct platform_driver gamekey_driver = {
	.driver = {
		.name		= "ares-gamekey",
		.of_match_table	= gamekey_of_match,
	},
	.probe = gamekey_probe,
};
module_platform_driver(gamekey_driver);

MODULE_AUTHOR("ported for Xiaomi ares / MT6893");
MODULE_DESCRIPTION("Xiaomi ares pop-up gamekey (shoulder keys) input driver");
MODULE_LICENSE("GPL");
