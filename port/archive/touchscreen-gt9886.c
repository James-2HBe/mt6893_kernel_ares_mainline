// SPDX-License-Identifier: GPL-2.0
/*
 * touchscreen-gt9886.c — Goodix GT9886 触控（主线框架骨架）
 *
 * ⚠️ 这是**框架骨架，不是可用的触控驱动**，请勿据此判断触控已能工作。
 *
 * 原因：官方 4.14 GT9886 驱动依赖未开源的 goodix_ts_core.h /
 * goodix_cfg_bin.h / goodix_default_fw.h / mtk_panel_ext.h，且使用 GTX8
 * 私有固件升级与触点帧协议。主线里 goodix.c 是 GT9xx 协议、
 * goodix_berlin_* 是 GT9916 协议，均与 GT9886 不同，无对应驱动，
 * 也没有这些头文件，因此无法等价移植。
 *
 * 本文件只搭好主线 i2c/input/regulator/gpiod 骨架，便于后续补齐协议：
 *   - probe 解析厂商 DT 的面板尺寸、电源、irq-flags
 *   - 注册 input 多点设备（ABS_MT_* / input_mt_init_slots）
 *   - 中断里清状态标志（帧解析留空，明确打印一次警告）
 * ⚠️ GT9886_REG_* 全为占位，必须按 datasheet 或补齐官方头文件后重写。
 *
 * 另注：ares 真机触控 IC 型号未确认 —— 官方 R 版 dts 写
 * "goodix,gt9886"，lineage 板级 dts 写 "focaltech,fts"（FT3658U）。
 * 搬驱动前必须先确认真机型号，否则会选错驱动。
 *
 * 厂商 DT：&i2c? { gt9886@5d {
 *     compatible = "goodix,gt9886"; reg = <0x5d>;
 *     interrupts = <21 IRQ_TYPE_EDGE_FALLING 21 0>;
 *     vtouch-supply = <&mt_pmic_vtp_ldo_reg>;
 *     goodix,reset-gpio = <&pio 20 0x0>;
 *     goodix,panel-max-x = <1080>; goodix,panel-max-y = <2280>;
 *     goodix,panel-max-w = <256>; }; };
 */
#include <linux/device.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>

/* 占位寄存器（GTX8 为 16 位地址，此处仅用低 8 位做 smbus 占位访问） */
#define GT9886_REG_CHIPID	0x4100
#define GT9886_REG_STATUS	0x4104
#define GT9886_MAX_TOUCHES	10

struct gt9886 {
	struct i2c_client	*client;
	struct input_dev	*input;
	struct regulator	*vtouch;
	struct gpio_desc	*reset;
	u32			 max_x;
	u32			 max_y;
	u32			 max_w;
	u32			 irq_flags;
};

static irqreturn_t gt9886_irq_thread(int irq, void *data)
{
	struct gt9886 *ts = data;
	int ret;

	ret = i2c_smbus_read_byte_data(ts->client, GT9886_REG_STATUS & 0xff);
	if (ret >= 0)
		i2c_smbus_write_byte_data(ts->client,
					  GT9886_REG_STATUS & 0xff, 0);

	dev_warn_once(&ts->client->dev,
		      "GT9886 触点帧协议未实现，未上报触摸事件；"
		      "见 docs/PORTED-DRIVERS.md\n");
	return IRQ_HANDLED;
}

static int gt9886_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct gt9886 *ts;
	int ret;

	ts = devm_kzalloc(dev, sizeof(*ts), GFP_KERNEL);
	if (!ts)
		return -ENOMEM;
	ts->client = client;

	of_property_read_u32(dev->of_node, "goodix,panel-max-x", &ts->max_x);
	of_property_read_u32(dev->of_node, "goodix,panel-max-y", &ts->max_y);
	of_property_read_u32(dev->of_node, "goodix,panel-max-w", &ts->max_w);
	of_property_read_u32(dev->of_node, "goodix,irq-flags", &ts->irq_flags);
	if (!ts->max_x)
		ts->max_x = 1080;
	if (!ts->max_y)
		ts->max_y = 2280;
	if (!ts->max_w)
		ts->max_w = 256;

	ts->vtouch = devm_regulator_get_optional(dev, "vtouch");
	if (!IS_ERR(ts->vtouch)) {
		ret = regulator_enable(ts->vtouch);
		if (ret)
			dev_warn(dev, "使能 vtouch-supply 失败: %d\n", ret);
	} else {
		ts->vtouch = NULL;
	}

	/* 厂商写的是 goodix,reset-gpio（私有），标准 reset-gpios 缺失时跳过 */
	ts->reset = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ts->reset))
		return PTR_ERR(ts->reset);

	ts->input = devm_input_allocate_device(dev);
	if (!ts->input)
		return -ENOMEM;

	ts->input->name = "gt9886-touchscreen";
	ts->input->id.bustype = BUS_I2C;
	input_set_abs_params(ts->input, ABS_MT_POSITION_X, 0, ts->max_x, 0, 0);
	input_set_abs_params(ts->input, ABS_MT_POSITION_Y, 0, ts->max_y, 0, 0);
	input_set_abs_params(ts->input, ABS_MT_TOUCH_MAJOR, 0, ts->max_w, 0, 0);

	ret = input_mt_init_slots(ts->input, GT9886_MAX_TOUCHES,
				  INPUT_MT_DIRECT);
	if (ret)
		return ret;

	ret = input_register_device(ts->input);
	if (ret) {
		dev_err(dev, "注册 input 设备失败: %d\n", ret);
		return ret;
	}

	if (client->irq > 0) {
		ret = devm_request_threaded_irq(dev, client->irq, NULL,
						gt9886_irq_thread,
						IRQF_TRIGGER_FALLING |
						IRQF_ONESHOT,
						"gt9886", ts);
		if (ret) {
			dev_err(dev, "申请 IRQ %d 失败: %d\n", client->irq, ret);
			return ret;
		}
	}

	i2c_set_clientdata(client, ts);
	dev_info(dev,
		 "GT9886 框架骨架已注册 (%ux%u, irq=%d); "
		 "⚠️ 帧协议/固件升级未实现\n",
		 ts->max_x, ts->max_y, client->irq);
	return 0;
}

static const struct of_device_id gt9886_of_match[] = {
	{ .compatible = "goodix,gt9886" },
	{ .compatible = "goodix,gt9896s" },
	{ }
};
MODULE_DEVICE_TABLE(of, gt9886_of_match);

static struct i2c_driver gt9886_driver = {
	.driver = {
		.name		= "gt9886-touchscreen",
		.of_match_table	= gt9886_of_match,
	},
	.probe = gt9886_probe,
};
module_i2c_driver(gt9886_driver);

MODULE_AUTHOR("ported for Xiaomi ares / MT6893");
MODULE_DESCRIPTION("Goodix GT9886 touchscreen framework skeleton (protocol TBD)");
MODULE_LICENSE("GPL");
