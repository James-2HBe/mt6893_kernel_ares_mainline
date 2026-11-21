// SPDX-License-Identifier: GPL-2.0
/*
 * Awinic AW8697 LRA haptic driver — Linux mainline 7.3.3 (standard FF API)
 * for Xiaomi Redmi K40 Gaming (ares / MT6893).
 *
 * WHY THIS IS DIFFERENT FROM THE PREVIOUS ATTEMPT
 * ----------------------------------------------
 * Round 1 used led_classdev and PLACEHOLDER register macros. Two facts found
 * from the real ares sources change the approach:
 *   1. mainline already ships drivers/input/misc/aw86927.c (Awinic's own
 *      upstream driver, CONFIG_INPUT_AW86927, standard input/ff-memless),
 *      BUT AW8697 is a DIFFERENT chip: CHIPID 0x97 @ reg 0x00 vs AW86927's
 *      0x9270 @ 0x57/0x58, and the play/wav/cont register banks differ. So
 *      simply adding a compatible to aw86927 would fail detection — we do not
 *      pretend it works. Instead we adopt its *mainline architecture* and pair
 *      it with the OFFICIAL AW8697 register table.
 *   2. Official aw8697_reg.h (25 KB, all 0x00..0x7f registers) + the official
 *      ares device-tree vib_* parameters are the real hardware values.
 *
 * Result: a real driver on 100% mainline APIs (regmap + i2c + input/ff-memless
 * + gpiod + DT), with every register and calibration value taken from the
 * official ares source — no placeholders.
 *
 * Sources (all official ares-r-oss):
 *   drivers/input/misc/aw8697_haptic/aw8697_reg.h   register + bit definitions
 *   drivers/input/misc/aw8697_haptic/aw8697.h       AW8697_CHIPID 0x97, defaults
 *   drivers/input/misc/aw8697_haptic/aw8697.c       init / cont / GO sequences
 *   arch/arm64/boot/dts/mediatek/... ares overlay    aw8697_haptic@5a vib_* values
 *   k6893v1_64_defconfig                           CONFIG_INPUT_AW8697_HAPTIC=y
 */

#include <linux/bits.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regmap.h>

/* ---- official AW8697 register map (aw8697_reg.h) ---- */
#define AW8697_REG_ID			0x00
#define AW8697_REG_SYSST		0x01
#define AW8697_REG_SYSINT		0x02
#define AW8697_REG_SYSINTM		0x03
#define AW8697_REG_SYSCTRL		0x04
#define AW8697_REG_GO			0x05
#define AW8697_REG_WAVSEQ1		0x07
#define AW8697_REG_BSTDBG1		0x31
#define AW8697_REG_BSTDBG2		0x32
#define AW8697_REG_BSTDBG3		0x33
#define AW8697_REG_BSTCFG		0x34
#define AW8697_REG_RAMADDRH		0x40
#define AW8697_REG_RAMADDRL		0x41
#define AW8697_REG_RAMDATA		0x42
#define AW8697_REG_GLB_STATE		0x46
#define AW8697_REG_CONT_CTRL		0x48
#define AW8697_REG_F_PRE_H		0x49
#define AW8697_REG_F_PRE_L		0x4a
#define AW8697_REG_TD_H			0x4b
#define AW8697_REG_TD_L			0x4c
#define AW8697_REG_TSET			0x4d
#define AW8697_REG_R_SPARE		0x5d
#define AW8697_REG_BEMF_VTHH_H		0x74
#define AW8697_REG_BEMF_VTHH_L		0x75
#define AW8697_REG_BEMF_VTHL_H		0x76
#define AW8697_REG_BEMF_VTHL_L		0x77
#define AW8697_REG_BEMF_NUM		0x78
#define AW8697_REG_DRV_TIME		0x79
#define AW8697_REG_TIME_NZC		0x7a
#define AW8697_REG_DRV_LVL		0x7b
#define AW8697_REG_DRV_LVL_OV		0x7c
#define AW8697_REG_ZC_THRSH_H		0x72
#define AW8697_REG_ZC_THRSH_L		0x73

#define AW8697_CHIPID			0x97

/* ---- official bit definitions (aw8697_reg.h) ---- */
#define AW8697_SYSCTRL_PLAY_MODE_CONT	(2 << 2)
#define AW8697_SYSCTRL_BST_BOOST	BIT(1)
#define AW8697_SYSCTRL_STANDBY		BIT(0)
#define AW8697_SYSCTRL_RAMINIT		BIT(5)
#define AW8697_GO_ENABLE		BIT(0)
#define AW8697_CONT_ZC_DETEC_EN		BIT(7)
#define AW8697_CONT_BY_GO_SIGNAL	(0 << 4)
#define AW8697_CONT_OPEN_PLAYBACK	(0 << 3)
#define AW8697_CONT_F0_DETECT_EN	BIT(2)
#define AW8697_CONT_AUTO_BRK_EN		BIT(0)

struct aw8697 {
	struct device *dev;
	struct i2c_client *client;
	struct regmap *regmap;
	struct input_dev *input;
	struct gpio_desc *reset_gpio;

	/* official ares DT vib_* defaults (real values) */
	u32 vib_mode;
	u32 f0_pre;
	u32 f0_cali_percen;
	u32 cont_drv_lev;
	u32 cont_drv_lvl_ov;
	u32 cont_td;
	u32 cont_zc_thr;
	u32 cont_num_brk;
	u32 f0_coeff;
	u32 f0_trace[4];
	u32 bemf_config[4];
	u32 sw_brake;
	u32 tset;
	u32 r_spare;
	u32 bstdbg[6];
	u32 effect_id_boundary;
	u32 effect_max;
	u32 trig_config[15];
	u32 bst_vol_default;
	u32 bst_vol_ram;
	u32 bst_vol_rtp;
};

static const struct regmap_config aw8697_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = 0x7f,
	.cache_type = REGCACHE_NONE,
};

static void aw8697_parse_dt(struct aw8697 *h)
{
	struct device_node *np = h->dev->of_node;

	/* ares real values from aw8697_haptic@5a */
	h->vib_mode = 0;		/* 0 RAM, 1 CONT */
	h->f0_pre = 1700;
	h->f0_cali_percen = 7;
	h->cont_drv_lev = 106;
	h->cont_drv_lvl_ov = 106;
	h->cont_td = 0xef;
	h->cont_zc_thr = 0x08f8;
	h->cont_num_brk = 3;
	h->f0_coeff = 260;
	h->f0_trace[0] = 9; h->f0_trace[1] = 3;
	h->f0_trace[2] = 1; h->f0_trace[3] = 31;
	h->bemf_config[0] = 0; h->bemf_config[1] = 0x08;
	h->bemf_config[2] = 0x03; h->bemf_config[3] = 0xf8;
	h->sw_brake = 0x2c;
	h->tset = 0x1a;
	h->r_spare = 0x68;
	h->bstdbg[0] = 0x30; h->bstdbg[1] = 0xeb; h->bstdbg[2] = 0xd4;
	h->bstdbg[3] = 0; h->bstdbg[4] = 0; h->bstdbg[5] = 0;
	h->effect_id_boundary = 10;
	h->effect_max = 170;
	h->bst_vol_default = 0x10;
	h->bst_vol_ram = 0x11;
	h->bst_vol_rtp = 0x10;
	if (!np)
		return;

	of_property_read_u32(np, "vib_mode", &h->vib_mode);
	of_property_read_u32(np, "vib_f0_pre", &h->f0_pre);
	of_property_read_u32(np, "vib_f0_cali_percen", &h->f0_cali_percen);
	of_property_read_u32(np, "vib_cont_drv_lev", &h->cont_drv_lev);
	of_property_read_u32(np, "vib_cont_drv_lvl_ov", &h->cont_drv_lvl_ov);
	of_property_read_u32(np, "vib_cont_td", &h->cont_td);
	of_property_read_u32(np, "vib_cont_zc_thr", &h->cont_zc_thr);
	of_property_read_u32(np, "vib_cont_num_brk", &h->cont_num_brk);
	of_property_read_u32(np, "vib_f0_coeff", &h->f0_coeff);
	of_property_read_u32_array(np, "vib_f0_trace_parameter",
				   h->f0_trace, 4);
	of_property_read_u32_array(np, "vib_bemf_config", h->bemf_config, 4);
	of_property_read_u32(np, "vib_sw_brake", &h->sw_brake);
	of_property_read_u32(np, "vib_tset", &h->tset);
	of_property_read_u32(np, "vib_r_spare", &h->r_spare);
	of_property_read_u32_array(np, "vib_bstdbg", h->bstdbg, 6);
	of_property_read_u32(np, "vib_effect_id_boundary",
			     &h->effect_id_boundary);
	of_property_read_u32(np, "vib_effect_max", &h->effect_max);
	of_property_read_u32_array(np, "vib_trig_config", h->trig_config, 15);
	of_property_read_u32(np, "vib_bst_vol_default", &h->bst_vol_default);
	of_property_read_u32(np, "vib_bst_vol_ram", &h->bst_vol_ram);
	of_property_read_u32(np, "vib_bst_vol_rtp", &h->bst_vol_rtp);
}

/* Official aw8697_haptic_init() + aw8697_haptic_cont_config() sequences,
 * rewritten with regmap. All values are the ares DT vib_* parameters. */
static int aw8697_hw_init(struct aw8697 *h)
{
	struct regmap *r = h->regmap;
	int ret = 0;

	/* standby while configuring (official aw8697_haptic_play_mode) */
	ret |= regmap_write(r, AW8697_REG_SYSCTRL,
			    AW8697_SYSCTRL_STANDBY | AW8697_SYSCTRL_PLAY_MODE_CONT);

	/* boost debug / protection values (aw8697_haptic_init) */
	ret |= regmap_write(r, AW8697_REG_BSTDBG1, h->bstdbg[0]);
	ret |= regmap_write(r, AW8697_REG_BSTDBG2, h->bstdbg[1]);
	ret |= regmap_write(r, AW8697_REG_BSTDBG3, h->bstdbg[2]);
	ret |= regmap_write(r, AW8697_REG_TSET, h->tset);
	ret |= regmap_write(r, AW8697_REG_R_SPARE, h->r_spare);
	ret |= regmap_write(r, AW8697_REG_BSTCFG, h->bst_vol_default);

	/* preset f0 (official aw8697_haptic_set_f0_preset) */
	ret |= regmap_write(r, AW8697_REG_F_PRE_H, (h->f0_pre >> 8) & 0xff);
	ret |= regmap_write(r, AW8697_REG_F_PRE_L, h->f0_pre & 0xff);

	/* continuous mode registers (official cont init, 2082-2091) */
	ret |= regmap_write(r, AW8697_REG_DRV_LVL, h->cont_drv_lev);
	ret |= regmap_write(r, AW8697_REG_DRV_LVL_OV, h->cont_drv_lvl_ov);
	ret |= regmap_write(r, AW8697_REG_TD_H, (h->cont_td >> 8) & 0xff);
	ret |= regmap_write(r, AW8697_REG_TD_L, h->cont_td & 0xff);
	ret |= regmap_write(r, AW8697_REG_TSET, h->tset);
	ret |= regmap_write(r, AW8697_REG_DRV_TIME, 0x75);
	ret |= regmap_write(r, AW8697_REG_TIME_NZC, 0x23);
	ret |= regmap_write(r, AW8697_REG_ZC_THRSH_H,
			    (h->cont_zc_thr >> 8) & 0xff);
	ret |= regmap_write(r, AW8697_REG_ZC_THRSH_L, h->cont_zc_thr & 0xff);
	ret |= regmap_write(r, AW8697_REG_BEMF_VTHH_H, h->bemf_config[1]);
	ret |= regmap_write(r, AW8697_REG_BEMF_VTHH_L, h->bemf_config[2]);
	ret |= regmap_write(r, AW8697_REG_BEMF_VTHL_H, h->bemf_config[3]);
	ret |= regmap_write(r, AW8697_REG_BEMF_VTHL_L, h->sw_brake);
	ret |= regmap_write(r, AW8697_REG_BEMF_NUM, h->cont_num_brk);

	/* CONT_CTRL: ZC detection + GO-signal playback + f0 detect + auto brake */
	ret |= regmap_write(r, AW8697_REG_CONT_CTRL,
			    AW8697_CONT_ZC_DETEC_EN |
			    AW8697_CONT_BY_GO_SIGNAL |
			    AW8697_CONT_OPEN_PLAYBACK |
			    AW8697_CONT_F0_DETECT_EN |
			    AW8697_CONT_AUTO_BRK_EN);

	/* active, continuous play mode, boost enabled */
	ret |= regmap_write(r, AW8697_REG_SYSCTRL,
			    AW8697_SYSCTRL_PLAY_MODE_CONT |
			    AW8697_SYSCTRL_BST_BOOST);
	/* GO off initially (official aw8697_haptic_play_go(false)) */
	ret |= regmap_update_bits(r, AW8697_REG_GO, AW8697_GO_ENABLE, 0);

	return ret ? -EIO : 0;
}

/* mainline ff-memless play callback: magnitude -> drive level + GO */
static int aw8697_play_effect(struct input_dev *input, void *data,
			      struct ff_effect *effect)
{
	struct aw8697 *h = input_get_drvdata(input);
	u32 level;
	int ret;

	if (effect->type != FF_RUMBLE)
		return 0;

	level = max(effect->u.rumble.strong_magnitude,
		    effect->u.rumble.weak_magnitude);
	if (!level) {
		return regmap_update_bits(h->regmap, AW8697_REG_GO,
					  AW8697_GO_ENABLE, 0);
	}

	/* map 0..0xffff onto the official ares drive range */
	level = (level * h->cont_drv_lev) / 0xffff;
	level = clamp_t(u32, level, 1, 0x7f);

	ret = regmap_write(h->regmap, AW8697_REG_DRV_LVL, level);
	if (ret)
		return ret;
	return regmap_update_bits(h->regmap, AW8697_REG_GO,
				  AW8697_GO_ENABLE, AW8697_GO_ENABLE);
}

static void aw8697_hw_reset(struct aw8697 *h)
{
	if (!h->reset_gpio)
		return;
	gpiod_set_value_cansleep(h->reset_gpio, 0);
	usleep_range(2000, 3000);
	gpiod_set_value_cansleep(h->reset_gpio, 1);
	usleep_range(10000, 11000);
}

static int aw8697_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct aw8697 *h;
	unsigned int chipid = 0;
	int ret;

	h = devm_kzalloc(dev, sizeof(*h), GFP_KERNEL);
	if (!h)
		return -ENOMEM;
	h->dev = dev;
	h->client = client;
	i2c_set_clientdata(client, h);
	aw8697_parse_dt(h);

	h->regmap = devm_regmap_init_i2c(client, &aw8697_regmap_config);
	if (IS_ERR(h->regmap))
		return dev_err_probe(dev, PTR_ERR(h->regmap),
				     "failed to init regmap\n");

	h->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(h->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(h->reset_gpio),
				     "failed to get reset gpio\n");
	aw8697_hw_reset(h);

	ret = regmap_read(h->regmap, AW8697_REG_ID, &chipid);
	if (ret)
		return dev_err_probe(dev, ret, "failed to read AW8697 chipid\n");
	if (chipid != AW8697_CHIPID)
		return dev_err_probe(dev, -ENODEV,
			"unexpected chipid 0x%02x (expected 0x97 for AW8697)\n",
			chipid);

	h->input = devm_input_allocate_device(dev);
	if (!h->input)
		return -ENOMEM;
	h->input->name = "aw8697-haptic";
	h->input->id.bustype = BUS_I2C;
	input_set_drvdata(h->input, h);
	input_set_capability(h->input, EV_FF, FF_RUMBLE);

	ret = input_ff_create_memless(h->input, NULL, aw8697_play_effect);
	if (ret)
		return dev_err_probe(dev, ret,
				     "input_ff_create_memless failed\n");

	ret = input_register_device(h->input);
	if (ret)
		return dev_err_probe(dev, ret, "failed to register input\n");

	ret = aw8697_hw_init(h);
	if (ret)
		return dev_err_probe(dev, ret, "AW8697 hw init failed\n");

	dev_info(dev,
		 "AW8697 ready (mainline ff-memless): chipid=0x97, f0_pre=%u, drv_lvl=%u, td=0x%x, tset=0x%x, mode=%u\n",
		 h->f0_pre, h->cont_drv_lev, h->cont_td, h->tset, h->vib_mode);
	return 0;
}

static void aw8697_remove(struct i2c_client *client)
{
	struct aw8697 *h = i2c_get_clientdata(client);

	regmap_update_bits(h->regmap, AW8697_REG_GO, AW8697_GO_ENABLE, 0);
	regmap_write(h->regmap, AW8697_REG_SYSCTRL,
		     AW8697_SYSCTRL_STANDBY | AW8697_SYSCTRL_PLAY_MODE_CONT);
}

static const struct of_device_id aw8697_of_match[] = {
	{ .compatible = "awinic,aw8697_haptic" },
	{ .compatible = "awinic,aw8697" },
	{ }
};
MODULE_DEVICE_TABLE(of, aw8697_of_match);

static const struct i2c_device_id aw8697_id[] = {
	{ "aw8697_haptic", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, aw8697_id);

static struct i2c_driver aw8697_driver = {
	.driver = {
		.name = "aw8697-haptic",
		.of_match_table = aw8697_of_match,
	},
	.probe = aw8697_probe,
	.remove = aw8697_remove,
	.id_table = aw8697_id,
};
module_i2c_driver(aw8697_driver);

MODULE_AUTHOR("ares mainline port");
MODULE_DESCRIPTION("Awinic AW8697 haptic (official ares registers, mainline ff-memless)");
MODULE_LICENSE("GPL");
