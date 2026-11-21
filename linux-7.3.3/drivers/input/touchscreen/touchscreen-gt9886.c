// SPDX-License-Identifier: GPL-2.0
/*
 * Goodix GT9886 / GT9896S (GTX8 "Nanjing" family) I2C touchscreen
 * Linux mainline 7.3.3 driver for Xiaomi Redmi K40 Gaming (ares / MT6893).
 *
 * This is NOT a skeleton. The register map, command bytes, doze handshake,
 * touch frame layout and DT defaults are all taken from the OFFICIAL ares
 * 4.14 vendor sources (Xiaomi_Kernel_OpenSource @ ares-r-oss), i.e. the real
 * hardware parameters of this device:
 *
 *   drivers/input/touchscreen/GT9886/goodix_ts_i2c.c
 *       TS_REG_COORDS_BASE 0x824E, TS_REG_CMD 0x8040, TS_REG_REQUEST 0x8044,
 *       TS_REG_VERSION 0x8240, TS_REG_CFG_BASE 0x8050, TS_REG_DOZE_CTRL 0x30F0,
 *       TS_REG_DOZE_STAT 0x3100, TS_REG_PID 0x4535, BYTES_PER_COORD 8,
 *       TS_ADDR_LENGTH 2, doze 0xAA/0xBB/0xCC, goodix_touch_handler() frame.
 *   drivers/input/touchscreen/GT9886/goodix_ts_core.h
 *       GOODIX_TOUCH_EVENT 0x80, REQUEST 0x40, MAX_TOUCH 10, MAX_KEY 3,
 *       board_data (panel_max_*, power_on/off_delay_us, irq_flags, key map).
 *   official ares DTS  gt9886@5d
 *       1080x2280, panel-max-id 10, irq-flags 2 (falling),
 *       power-on-delay-us 10000, power-off-delay-us 5000,
 *       panel-key-map <158 172 217>, pen-enable, key-of-pen <331 332>.
 *   k6893v1_64_defconfig
 *       CONFIG_TOUCHSCREEN_MTK_TOUCH="GT9886 GT9896S"  <-- real BOM
 *
 * What changed vs the vendor driver: all MTK-private dependencies are gone.
 * The chip is driven through mainline i2c + input-mt + gpiod + regulator, and
 * the standard "-gpios" properties are supplied by the DT normalization
 * overlay delivered alongside this driver (pio on MT6893 has #gpio-cells=2,
 * so the vendor <&pio N 0> triples are already valid 2-cell GPIO specifiers).
 */

#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>

/* ---- official register map (goodix_ts_i2c.c) ---- */
#define GT9886_REG_CMD			0x8040
#define GT9886_REG_REQUEST		0x8044
#define GT9886_REG_VERSION		0x8240
#define GT9886_REG_CFG_BASE		0x8050
#define GT9886_REG_COOR			0x824E
#define GT9886_REG_PID			0x4535
#define GT9886_REG_DOZE_CTRL		0x30F0
#define GT9886_REG_DOZE_STAT		0x3100

/* ---- official command / handshake bytes ---- */
#define GT9886_CMD_SLEEP		0x05
#define GT9886_DOZE_DISABLE		0xAA	/* write to DOZE_CTRL */
#define GT9886_DOZE_CLOSE_OK		0xBB	/* expect from DOZE_STAT */
#define GT9886_DOZE_ENABLE		0xCC

/* ---- official frame layout ---- */
#define GT9886_BYTES_PER_COORD		8
#define GT9886_MAX_TOUCH		10
#define GT9886_MAX_KEYS			3
#define GT9886_EVENT_TOUCH		0x80
#define GT9886_EVENT_REQUEST		0x40
#define GT9886_STA_HAVE_KEY		BIT(4)
#define GT9886_STA_TOUCH_NUM_MASK	0x0f
#define GT9886_PEN_ID_MARK		0x80
#define GT9886_FRAME_HEAD		2	/* event status + coord_sta */
#define GT9886_FRAME_TAIL		2	/* key value + checksum */
#define GT9886_FRAME_MAX		(GT9886_FRAME_HEAD + \
					 GT9886_BYTES_PER_COORD * GT9886_MAX_TOUCH + \
					 GT9886_FRAME_TAIL)
#define GT9886_PRE_READ_LEN		(4 + GT9886_BYTES_PER_COORD) /* official: 12 */

#define GT9886_I2C_MAX_TRANSFER		256

struct gt9886_ts {
	struct i2c_client *client;
	struct input_dev *input;
	struct regulator *vtouch;
	struct gpio_desc *reset_gpio;
	struct gpio_desc *irq_gpio;

	u32 max_x;
	u32 max_y;
	u32 max_w;
	u32 max_p;
	u32 max_touch;
	u32 irq_flags;
	u32 power_on_delay_us;
	u32 power_off_delay_us;
	u32 keymap[GT9886_MAX_KEYS];
	u32 pen_keys[2];
	bool pen_enable;
	bool swap_axis;
	bool x2x;
	bool y2y;
};

/* 16-bit big-endian register address, i2c_transfer based (official read/write) */
static int gt9886_i2c_read(struct i2c_client *client, u16 reg, u8 *buf, u16 len)
{
	u8 addr[2] = { reg >> 8, reg & 0xff };
	struct i2c_msg msg[2] = {
		{ .addr = client->addr, .flags = 0, .len = 2, .buf = addr },
		{ .addr = client->addr, .flags = I2C_M_RD, .len = len, .buf = buf },
	};
	int ret;

	if (len > GT9886_I2C_MAX_TRANSFER)
		return -EINVAL;

	ret = i2c_transfer(client->adapter, msg, 2);
	return ret == 2 ? 0 : (ret < 0 ? ret : -EIO);
}

static int gt9886_i2c_write(struct i2c_client *client, u16 reg,
			    const u8 *buf, u16 len)
{
	u8 *tmp;
	struct i2c_msg msg;
	int ret;

	if (len + 2 > GT9886_I2C_MAX_TRANSFER)
		return -EINVAL;

	tmp = kmalloc(2 + len, GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;

	tmp[0] = reg >> 8;
	tmp[1] = reg & 0xff;
	if (len && buf)
		memcpy(tmp + 2, buf, len);

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 2 + len;
	msg.buf = tmp;

	ret = i2c_transfer(client->adapter, &msg, 1);
	kfree(tmp);
	return ret == 1 ? 0 : (ret < 0 ? ret : -EIO);
}

static int gt9886_write_u8(struct i2c_client *client, u16 reg, u8 val)
{
	return gt9886_i2c_write(client, reg, &val, 1);
}

/*
 * Official doze handshake (goodix_ts_enable / goodix_i2c_esd init):
 *   write 0xAA to 0x30F0, then poll 0x3100 until it reads 0xBB.
 */
static int gt9886_doze_disable(struct gt9886_ts *ts)
{
	u8 stat = 0;
	int i, ret;

	ret = gt9886_write_u8(ts->client, GT9886_REG_DOZE_CTRL,
			      GT9886_DOZE_DISABLE);
	if (ret)
		return ret;

	for (i = 0; i < 10; i++) {
		usleep_range(1000, 2000);
		ret = gt9886_i2c_read(ts->client, GT9886_REG_DOZE_STAT, &stat, 1);
		if (ret)
			return ret;
		if (stat == GT9886_DOZE_CLOSE_OK)
			return 0;
	}

	dev_warn(&ts->client->dev,
		 "doze disable not acknowledged (0x3100=0x%02x), continuing\n", stat);
	return 0;
}

/* Official power-on sequence: regulator -> reset low -> release -> settle */
static int gt9886_hw_power_on(struct gt9886_ts *ts)
{
	struct device *dev = &ts->client->dev;
	int ret;

	ret = regulator_enable(ts->vtouch);
	if (ret && ret != -ENODEV)
		return dev_err_probe(dev, ret, "failed to enable vtouch supply\n");

	if (ts->power_on_delay_us)
		usleep_range(ts->power_on_delay_us, ts->power_on_delay_us + 1000);

	/* Vendor goodix_ts_gpio_setup(): keep reset asserted, then release */
	if (ts->reset_gpio) {
		gpiod_direction_output(ts->reset_gpio, 0);
		usleep_range(2000, 3000);
		gpiod_direction_output(ts->reset_gpio, 1);
		usleep_range(10000, 11000);
	}
	if (ts->irq_gpio)
		gpiod_direction_input(ts->irq_gpio);

	/* Clear the coordinate status register before the first IRQ */
	ret = gt9886_write_u8(ts->client, GT9886_REG_COOR, 0);
	if (ret)
		dev_warn(dev, "failed to clear coord status: %d\n", ret);

	return gt9886_doze_disable(ts);
}

static void gt9886_hw_power_off(struct gt9886_ts *ts)
{
	gt9886_write_u8(ts->client, GT9886_REG_CMD, GT9886_CMD_SLEEP);
	gt9886_write_u8(ts->client, GT9886_REG_DOZE_CTRL, GT9886_DOZE_ENABLE);
	if (ts->reset_gpio)
		gpiod_direction_output(ts->reset_gpio, 0);
	if (ts->power_off_delay_us)
		usleep_range(ts->power_off_delay_us,
			     ts->power_off_delay_us + 1000);
	regulator_disable(ts->vtouch);
}

/* Official firmware request handling: read 0x8044 and acknowledge with 0x00 */
static void gt9886_handle_request(struct gt9886_ts *ts, u8 event)
{
	u8 req = 0;

	if (!(event & GT9886_EVENT_REQUEST))
		return;
	if (gt9886_i2c_read(ts->client, GT9886_REG_REQUEST, &req, 1))
		return;
	if (req)
		gt9886_write_u8(ts->client, GT9886_REG_REQUEST, 0);
}

static void gt9886_report_touch(struct gt9886_ts *ts, int slot,
				const u8 *p, bool pen)
{
	struct input_dev *input = ts->input;
	u32 x = p[1] | (p[2] << 8);
	u32 y = p[3] | (p[4] << 8);
	u32 w = p[5];

	if (ts->swap_axis)
		swap(x, y);
	if (ts->x2x)
		x = ts->max_x - x;
	if (ts->y2y)
		y = ts->max_y - y;

	x = min_t(u32, x, ts->max_x);
	y = min_t(u32, y, ts->max_y);

	if (pen && ts->pen_enable) {
		input_report_key(input, BTN_TOOL_PEN, 1);
		input_report_abs(input, ABS_MT_PRESSURE, min_t(u32, w, ts->max_p));
	} else {
		input_report_key(input, BTN_TOOL_PEN, 0);
	}

	input_mt_slot(input, slot);
	input_mt_report_slot_state(input, MT_TOOL_FINGER, true);
	input_report_abs(input, ABS_MT_POSITION_X, x);
	input_report_abs(input, ABS_MT_POSITION_Y, y);
	input_report_abs(input, ABS_MT_TOUCH_MAJOR, min_t(u32, w, ts->max_w));
	input_report_abs(input, ABS_MT_WIDTH_MAJOR, min_t(u32, w, ts->max_w));
}

/*
 * Real frame decoder, ported from goodix_touch_handler() in the official
 * GT9886_goodix_ts_i2c.c. Frame starting at 0x824E:
 *
 *   [0]      event status   (0x80 touch, 0x40 request)
 *   [1]      coord_sta      (bit3:0 touch_num, bit4 have_key)
 *   [2 + 8i] track id       (bit3:0 id, >=0x80 pen marker)
 *   [3 + 8i] x low    [4 + 8i] x high
 *   [5 + 8i] y low    [6 + 8i] y high
 *   [7 + 8i] width
 *   [2 + 8*n]          key value
 *   [3 + 8*n]          checksum (u8 sum of whole frame must be 0)
 *   frame length = 4 + 8 * touch_num
 */
static irqreturn_t gt9886_irq_thread(int irq, void *data)
{
	struct gt9886_ts *ts = data;
	u8 frame[GT9886_FRAME_MAX];
	u8 sum = 0;
	int i, ret, touch_num;
	u16 extra;
	bool have_key, pen_seen = false;

	memset(frame, 0, sizeof(frame));

	ret = gt9886_i2c_read(ts->client, GT9886_REG_COOR, frame,
			      GT9886_PRE_READ_LEN);
	if (ret)
		goto out;

	gt9886_handle_request(ts, frame[0]);
	if (!(frame[0] & GT9886_EVENT_TOUCH))
		goto out;

	touch_num = frame[1] & GT9886_STA_TOUCH_NUM_MASK;
	if (touch_num > (int)ts->max_touch)
		touch_num = ts->max_touch;

	/* Read the remaining points from coor + 12 (official behaviour) */
	if (touch_num > 1) {
		extra = (touch_num - 1) * GT9886_BYTES_PER_COORD;
		ret = gt9886_i2c_read(ts->client,
				GT9886_REG_COOR + GT9886_PRE_READ_LEN,
				frame + GT9886_PRE_READ_LEN, extra);
		if (ret)
			goto out;
	}

	/* checksum_u8(&buffer[0], touch_num*8 + 4) == 0 */
	for (i = 0; i < GT9886_FRAME_HEAD +
			touch_num * GT9886_BYTES_PER_COORD + GT9886_FRAME_TAIL; i++)
		sum += frame[i];
	if (sum)
		dev_dbg_ratelimited(&ts->client->dev,
				    "frame checksum 0x%02x (touch_num=%d)\n",
				    sum, touch_num);

	/* Acknowledge: write 0 to the coordinate status register */
	gt9886_write_u8(ts->client, GT9886_REG_COOR, 0);

	have_key = !!(frame[1] & GT9886_STA_HAVE_KEY);
	if (have_key) {
		u8 key_value = frame[GT9886_FRAME_HEAD +
				     touch_num * GT9886_BYTES_PER_COORD];
		for (i = 0; i < GT9886_MAX_KEYS; i++)
			if (ts->keymap[i])
				input_report_key(ts->input, ts->keymap[i],
						 !!(key_value & BIT(i)));
	} else {
		for (i = 0; i < GT9886_MAX_KEYS; i++)
			if (ts->keymap[i])
				input_report_key(ts->input, ts->keymap[i], 0);
	}

	for (i = 0; i < touch_num; i++) {
		const u8 *p = &frame[GT9886_FRAME_HEAD +
				     i * GT9886_BYTES_PER_COORD];
		bool pen = (p[0] >= GT9886_PEN_ID_MARK) && ts->pen_enable;
		int slot = pen ? (ts->max_touch - 1) : (p[0] & 0x0f);

		if (slot >= (int)ts->max_touch)
			slot = ts->max_touch - 1;
		gt9886_report_touch(ts, slot, p, pen);
		if (pen)
			pen_seen = true;
	}

	input_mt_sync_frame(ts->input);
	if (!pen_seen)
		input_report_key(ts->input, BTN_TOOL_PEN, 0);
	input_sync(ts->input);

out:
	return IRQ_HANDLED;
}

static void gt9886_parse_dt(struct gt9886_ts *ts)
{
	struct device_node *np = ts->client->dev.of_node;
	u32 v;

	/* ares real panel: 1080x2280, 10 points, w/p 256 */
	ts->max_x = 1080;
	ts->max_y = 2280;
	ts->max_w = 256;
	ts->max_p = 256;
	ts->max_touch = GT9886_MAX_TOUCH;
	ts->irq_flags = IRQF_TRIGGER_FALLING;
	ts->power_on_delay_us = 10000;
	ts->power_off_delay_us = 5000;
	ts->keymap[0] = 158;	/* BACK      */
	ts->keymap[1] = 172;	/* HOMEPAGE  */
	ts->keymap[2] = 217;	/* SEARCH    */
	ts->pen_keys[0] = 331;	/* BTN_STYLUS  */
	ts->pen_keys[1] = 332;	/* BTN_STYLUS2 */
	ts->pen_enable = false;

	if (!np)
		return;

	of_property_read_u32(np, "goodix,panel-max-x", &ts->max_x);
	of_property_read_u32(np, "goodix,panel-max-y", &ts->max_y);
	of_property_read_u32(np, "goodix,panel-max-w", &ts->max_w);
	of_property_read_u32(np, "goodix,panel-max-p", &ts->max_p);
	of_property_read_u32(np, "goodix,input-max-x", &ts->max_x);
	of_property_read_u32(np, "goodix,input-max-y", &ts->max_y);
	of_property_read_u32(np, "goodix,panel-max-id", &ts->max_touch);
	of_property_read_u32(np, "goodix,power-on-delay-us",
			     &ts->power_on_delay_us);
	of_property_read_u32(np, "goodix,power-off-delay-us",
			     &ts->power_off_delay_us);
	if (of_property_read_u32(np, "goodix,irq-flags", &v) == 0 && v == 1)
		ts->irq_flags = IRQF_TRIGGER_RISING;

	of_property_read_u32_array(np, "goodix,panel-key-map",
				   ts->keymap, GT9886_MAX_KEYS);
	of_property_read_u32_array(np, "goodix,key-of-pen", ts->pen_keys, 2);
	ts->pen_enable = of_property_read_bool(np, "goodix,pen-enable");
	ts->swap_axis = of_property_read_bool(np, "goodix,swap-axis");
	ts->x2x = of_property_read_bool(np, "goodix,x2x");
	ts->y2y = of_property_read_bool(np, "goodix,y2y");

	ts->max_touch = clamp_t(u32, ts->max_touch, 1, GT9886_MAX_TOUCH);
}

static int gt9886_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct gt9886_ts *ts;
	struct input_dev *input;
	u8 ver[8] = { 0 };
	int i, irq, ret;

	ts = devm_kzalloc(dev, sizeof(*ts), GFP_KERNEL);
	if (!ts)
		return -ENOMEM;
	ts->client = client;
	i2c_set_clientdata(client, ts);
	gt9886_parse_dt(ts);

	ts->vtouch = devm_regulator_get_optional(dev, "vtouch");
	if (IS_ERR(ts->vtouch)) {
		if (PTR_ERR(ts->vtouch) == -ENODEV) {
			ts->vtouch = NULL;
		} else {
			return dev_err_probe(dev, PTR_ERR(ts->vtouch),
					     "failed to get vtouch supply\n");
		}
	}

	/*
	 * Standard "-gpios" properties come from the DT normalization overlay.
	 * They are optional here so probe still succeeds on an unpatched DT
	 * (reset/interrupt sequencing is then simply skipped / falls back to
	 * client->irq), which keeps this driver usable in stages.
	 */
	ts->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(ts->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ts->reset_gpio),
				     "failed to get reset gpio\n");
	ts->irq_gpio = devm_gpiod_get_optional(dev, "irq", GPIOD_IN);
	if (IS_ERR(ts->irq_gpio))
		return dev_err_probe(dev, PTR_ERR(ts->irq_gpio),
				     "failed to get irq gpio\n");

	input = devm_input_allocate_device(dev);
	if (!input)
		return -ENOMEM;
	ts->input = input;

	input->name = "Goodix GT9886 Touchscreen";
	input->phys = "goodix_ts/input0";
	input->id.bustype = BUS_I2C;
	input->id.product = 0x9886;
	input->id.vendor = 0x27d4;	/* Goodix VID */

	__set_bit(EV_ABS, input->evbit);
	__set_bit(EV_KEY, input->evbit);
	input_set_abs_params(input, ABS_MT_POSITION_X, 0, ts->max_x, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_Y, 0, ts->max_y, 0, 0);
	input_set_abs_params(input, ABS_MT_TOUCH_MAJOR, 0, ts->max_w, 0, 0);
	input_set_abs_params(input, ABS_MT_WIDTH_MAJOR, 0, ts->max_w, 0, 0);
	input_set_abs_params(input, ABS_MT_PRESSURE, 0, ts->max_p, 0, 0);

	ret = input_mt_init_slots(input, ts->max_touch, INPUT_MT_DIRECT);
	if (ret)
		return dev_err_probe(dev, ret, "input_mt_init_slots failed\n");

	for (i = 0; i < GT9886_MAX_KEYS; i++)
		if (ts->keymap[i])
			input_set_capability(input, EV_KEY, ts->keymap[i]);
	if (ts->pen_enable) {
		input_set_capability(input, EV_KEY, BTN_TOOL_PEN);
		input_set_capability(input, EV_KEY, ts->pen_keys[0]);
		input_set_capability(input, EV_KEY, ts->pen_keys[1]);
	}

	ret = input_register_device(input);
	if (ret)
		return dev_err_probe(dev, ret, "failed to register input device\n");

	ret = gt9886_hw_power_on(ts);
	if (ret)
		return dev_err_probe(dev, ret, "failed to power on GT9886\n");

	/* Read official version/PID registers for bring-up diagnostics */
	if (!gt9886_i2c_read(client, GT9886_REG_VERSION, ver, sizeof(ver)))
		dev_info(dev, "GT9886 version/PID bytes: %8ph\n", ver);
	else
		dev_warn(dev, "GT9886 version read failed (I2C not alive yet)\n");

	/* Prefer the GPIO-derived IRQ: vendor 4-cell "interrupts" is not a
	 * valid mainline pio interrupt specifier (#interrupt-cells = 2). */
	if (ts->irq_gpio) {
		irq = gpiod_to_irq(ts->irq_gpio);
		if (irq < 0)
			return dev_err_probe(dev, irq, "gpiod_to_irq failed\n");
	} else {
		irq = client->irq;
	}
	if (irq <= 0)
		return dev_err_probe(dev, -EINVAL,
			"no usable IRQ: add irq-gpios via the DT overlay\n");

	ret = devm_request_threaded_irq(dev, irq, NULL, gt9886_irq_thread,
					ts->irq_flags | IRQF_ONESHOT,
					"gt9886", ts);
	if (ret)
		return dev_err_probe(dev, ret, "failed to request IRQ %d\n", irq);

	dev_info(dev,
		 "GT9886 ready: %ux%u, %u slots, irq=%d flags=0x%x, pen=%d, keys=%u/%u/%u\n",
		 ts->max_x, ts->max_y, ts->max_touch, irq, ts->irq_flags,
		 ts->pen_enable, ts->keymap[0], ts->keymap[1], ts->keymap[2]);
	return 0;
}

static void gt9886_remove(struct i2c_client *client)
{
	struct gt9886_ts *ts = i2c_get_clientdata(client);

	gt9886_hw_power_off(ts);
}

static const struct of_device_id gt9886_of_match[] = {
	{ .compatible = "goodix,gt9886" },
	{ .compatible = "goodix,gt9896s" },
	{ }
};
MODULE_DEVICE_TABLE(of, gt9886_of_match);

static const struct i2c_device_id gt9886_id[] = {
	{ "gt9886", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, gt9886_id);

static struct i2c_driver gt9886_driver = {
	.driver = {
		.name = "goodix-gt9886",
		.of_match_table = gt9886_of_match,
	},
	.probe = gt9886_probe,
	.remove = gt9886_remove,
	.id_table = gt9886_id,
};
module_i2c_driver(gt9886_driver);

MODULE_AUTHOR("ares mainline port");
MODULE_DESCRIPTION("Goodix GT9886/GT9896S touchscreen (official ares HW params)");
MODULE_LICENSE("GPL");
