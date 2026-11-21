// SPDX-License-Identifier: GPL-2.0
/*
 * Xiaomi ares pop-up shoulder keys (gamekey) — Linux mainline 7.3.3
 *
 * ROUND-1 BLOCK REMOVED
 * ---------------------
 * Round 1 said "厂商 dts 的 gamekey 节点是空的，需补 key-gpios"。That is no
 * longer true: the official ares DTS DOES carry the real pins, just under
 * private property names:
 *
 *   &gamekey {
 *       key_left   = <&pio 111 0x00>;
 *       hall_left  = <&pio 4  0x00>;
 *       key_right  = <&pio 112 0x00>;
 *       hall_right = <&pio 5  0x00>;
 *       pinctrl-names = "gamekey_active", "gamekey_suspend";
 *       pinctrl-0 = <&gk_hall_act &gk_key_act>;
 *       pinctrl-1 = <&gk_hall_sup &gk_key_sup>;
 *   };
 *
 * Since MT6893 pio has #gpio-cells = <2>, <&pio 111 0x00> is already a valid
 * mainline GPIO specifier. The DT normalization overlay merely republishes
 * them under standard "key-gpios" / "hall-gpios" names, so this driver gets
 * real, working GPIOs via devm_gpiod_get_array() instead of a stub.
 *
 * Behaviour: 2 shoulder keys (pio111/112) + 2 hall sensors (pio4/5).
 * Key GPIOs are edge-triggered; hall GPIOs are polled through the same
 * threaded handler (hall reports BTN_TRIGGER_HAPPY for the slide state).
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

/* ares shoulder keys: L/R + 2 hall; keymap matches the vendor keylayout */
static const unsigned int gamekey_keycodes[GAMEKEY_MAX_KEYS] = {
	KEY_F1,			/* key_left  pio111 */
	KEY_F2,			/* key_right pio112 */
	BTN_TRIGGER_HAPPY1,	/* hall_left  pio4  */
	BTN_TRIGGER_HAPPY2,	/* hall_right pio5  */
};

struct gamekey {
	struct input_dev *input;
	struct gpio_descs *keys;
	struct gpio_descs *halls;
	int nkeys;
};

static irqreturn_t gamekey_irq(int irq, void *dev_id)
{
	struct gamekey *gk = dev_id;
	int i, val;
	int idx = 0;

	for (i = 0; i < gk->keys->ndescs && idx < GAMEKEY_MAX_KEYS; i++, idx++) {
		val = gpiod_get_value_cansleep(gk->keys->desc[i]);
		if (val >= 0)
			input_report_key(gk->input, gamekey_keycodes[idx], val);
	}
	if (gk->halls) {
		for (i = 0; i < gk->halls->ndescs && idx < GAMEKEY_MAX_KEYS;
		     i++, idx++) {
			val = gpiod_get_value_cansleep(gk->halls->desc[i]);
			if (val >= 0)
				input_report_key(gk->input,
						 gamekey_keycodes[idx], val);
		}
	}
	input_sync(gk->input);
	return IRQ_HANDLED;
}

static int gamekey_request_irqs(struct device *dev, struct gamekey *gk,
				struct gpio_descs *descs)
{
	int i, irq, ret;

	for (i = 0; i < descs->ndescs; i++) {
		irq = gpiod_to_irq(descs->desc[i]);
		if (irq < 0) {
			dev_warn(dev, "gpio %d has no irq: %d\n", i, irq);
			continue;
		}
		ret = devm_request_threaded_irq(dev, irq, NULL, gamekey_irq,
				IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
				IRQF_ONESHOT, "ares-gamekey", gk);
		if (ret)
			return dev_err_probe(dev, ret,
					     "failed to request gpio %d irq\n", i);
	}
	return 0;
}

static int gamekey_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct gamekey *gk;
	struct input_dev *input;
	int i, ret;

	gk = devm_kzalloc(dev, sizeof(*gk), GFP_KERNEL);
	if (!gk)
		return -ENOMEM;

	/* Standard names come from the DT normalization overlay; the vendor
	 * key_left/hall_left/... properties remain in the DTS as reference. */
	gk->keys = devm_gpiod_get_array_optional(dev, "key", GPIOD_IN);
	if (IS_ERR(gk->keys))
		return dev_err_probe(dev, PTR_ERR(gk->keys),
				     "failed to get key-gpios\n");
	gk->halls = devm_gpiod_get_array_optional(dev, "hall", GPIOD_IN);
	if (IS_ERR(gk->halls))
		return dev_err_probe(dev, PTR_ERR(gk->halls),
				     "failed to get hall-gpios\n");

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
	if (ret)
		return dev_err_probe(dev, ret, "failed to register input\n");

	if (!gk->keys && !gk->halls) {
		dev_warn(dev, "no key-gpios/hall-gpios: apply the DT normalization "
			 "overlay to enable ares shoulder keys\n");
		return 0;
	}

	if (gk->keys) {
		ret = gamekey_request_irqs(dev, gk, gk->keys);
		if (ret)
			return ret;
		gk->nkeys += gk->keys->ndescs;
	}
	if (gk->halls) {
		ret = gamekey_request_irqs(dev, gk, gk->halls);
		if (ret)
			return ret;
		gk->nkeys += gk->halls->ndescs;
	}

	dev_info(dev, "ares pop-up gamekey ready: %d key GPIOs + %d hall GPIOs\n",
		 gk->keys ? gk->keys->ndescs : 0,
		 gk->halls ? gk->halls->ndescs : 0);
	return 0;
}

static const struct of_device_id gamekey_of_match[] = {
	{ .compatible = "xm,gamekey" },
	{ }
};
MODULE_DEVICE_TABLE(of, gamekey_of_match);

static struct platform_driver gamekey_driver = {
	.driver = {
		.name = "ares-gamekey",
		.of_match_table = gamekey_of_match,
	},
	.probe = gamekey_probe,
};
module_platform_driver(gamekey_driver);

MODULE_AUTHOR("ares mainline port");
MODULE_DESCRIPTION("Xiaomi ares pop-up shoulder keys (real pio111/112/4/5)");
MODULE_LICENSE("GPL");
