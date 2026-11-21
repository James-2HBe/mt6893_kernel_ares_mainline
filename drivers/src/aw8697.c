/*
 * aw8697.c   aw8697 haptic module
 *
 * Version: v1.1.3
 *
 * Copyright (c) 2019 AWINIC Technology CO., LTD
 * Copyright (C) 2021 XiaoMi, Inc.
 *
 *  Author: Nick Li <zhangzetao@awinic.com.cn>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#define DEBUG
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/debugfs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <linux/syscalls.h>
#include <linux/power_supply.h>
#include <linux/pm_qos.h>
#include "aw8697_config.h"
#include "aw8697_reg.h"
#include "aw8697.h"
#include  "ringbuffer.h"


/******************************************************
 *
 * Marco
 *
 ******************************************************/
#define AW8697_I2C_NAME "aw8697_haptic"
#define AW8697_HAPTIC_NAME "aw8697_haptic"

#define AW8697_VERSION "v1.1.3"

#define AWINIC_RAM_UPDATE_DELAY

#define AW_I2C_RETRIES 2
#define AW_I2C_RETRY_DELAY 2
#define AW_READ_CHIPID_RETRIES 5
#define AW_READ_CHIPID_RETRY_DELAY 2

#define AW8697_MAX_DSP_START_TRY_COUNT    10

#define AW8697_MAX_FIRMWARE_LOAD_CNT 20

#define AW8697_MAX_BST_VO 0x1f

#define OSC_CALIBRATION_T_LENGTH 5100000
#define PM_QOS_VALUE_VB 400
struct pm_qos_request pm_qos_req_vb;
/******************************************************
 *
 * variable
 *
 ******************************************************/
#define AW8697_RTP_NAME_MAX        64
static char *aw8697_ram_name = "aw8697_haptic.bin";
#ifdef TEST_RTP
static char aw8697_rtp_name[][AW8697_RTP_NAME_MAX] = {
	{"aw8697_rtp_1.bin"},
};
#else
static char aw8697_rtp_name[][AW8697_RTP_NAME_MAX] = {
	{"aw8697_rtp_1.bin"}, //8
	{"aw8697_rtp_1.bin"},
	{"aw8697_rtp_1.bin"},
	{"aw8697_rtp_1.bin"},
	{"aw8697_rtp_1.bin"},
	{"aw8697_rtp_1.bin"},
	{"aw8697_rtp_1.bin"},
	{"aw8697_rtp_1.bin"},
	{"aw8697_rtp_1.bin"},
	{"aw8697_rtp_1.bin"},
	{"aw8697_rtp_1.bin"},
	//{"aw8697_rtp_1.bin"},
	//{"aw8697_rtp_1.bin"},
	{"AcousticGuitar_RTP.bin"}, //21
	{"Blues_RTP.bin"},
	{"Candy_RTP.bin"},
	{"Carousel_RTP.bin"},
	{"Celesta_RTP.bin"},
	{"Childhood_RTP.bin"},
	{"Country_RTP.bin"},
	{"Cowboy_RTP.bin"},
	{"Echo_RTP.bin"},
	{"Fairyland_RTP.bin"},
	{"Fantasy_RTP.bin"},
	{"Field_Trip_RTP.bin"},
	{"Glee_RTP.bin"},
	{"Glockenspiel_RTP.bin"},
	{"Ice_Latte_RTP.bin"},
	{"Kung_Fu_RTP.bin"},
	{"Leisure_RTP.bin"},
	{"Lollipop_RTP.bin"},
	{"MiMix2_RTP.bin"},
	{"Mi_RTP.bin"},
	{"MiHouse_RTP.bin"},
	{"MiJazz_RTP.bin"},
	{"MiRemix_RTP.bin"},
	{"Mountain_Spring_RTP.bin"},
	{"Orange_RTP.bin"},
	{"Raindrops_RTP.bin"},
	{"Space_Age_RTP.bin"},
	{"ToyRobot_RTP.bin"},
	{"Vigor_RTP.bin"},
	{"Bottle_RTP.bin"},
	{"Bubble_RTP.bin"},
	{"Bullfrog_RTP.bin"},
	{"Burst_RTP.bin"},
	{"Chirp_RTP.bin"},
	{"Clank_RTP.bin"},
	{"Crystal_RTP.bin"},
	{"FadeIn_RTP.bin"},
	{"FadeOut_RTP.bin"},
	{"Flute_RTP.bin"},
	{"Fresh_RTP.bin"},
	{"Frog_RTP.bin"},
	{"Guitar_RTP.bin"},
	{"Harp_RTP.bin"},
	{"IncomingMessage_RTP.bin"},
	{"MessageSent_RTP.bin"},
	{"Moment_RTP.bin"},
	{"NotificationXylophone_RTP.bin"},
	{"Potion_RTP.bin"},
	{"Radar_RTP.bin"},
	{"Spring_RTP.bin"},
	{"Swoosh_RTP.bin"}, //71
	{"Gesture_UpSlide_RTP.bin"},
	{"FOD_Motion_Planet_RTP.bin"},
	{"Charge_Wire_RTP.bin"},
	{"Charge_Wireless_RTP.bin"},
	{"Unlock_Failed_RTP.bin"},
	{"FOD_Motion1_RTP.bin"},
	{"FOD_Motion2_RTP.bin"},
	{"FOD_Motion3_RTP.bin"},
	{"FOD_Motion4_RTP.bin"},
	{"FOD_Motion_Aurora_RTP.bin"},
	{"FaceID_Wrong2_RTP.bin"}, //82
	{"uninstall_animation_rtp.bin"},
	{"uninstall_dialog_rtp.bin"},
	{"screenshot_rtp.bin"},
	{"lockscreen_camera_entry_rtp.bin"},
	{"launcher_edit_rtp.bin"},
	{"launcher_icon_selection_rtp.bin"},
	{"taskcard_remove_rtp.bin"},
	{"task_cleanall_rtp.bin"},
	{"new_iconfolder_rtp.bin"},
	{"notification_remove_rtp.bin"},
	{"notification_cleanall_rtp.bin"},
	{"notification_setting_rtp.bin"},
	{"game_turbo_rtp.bin"},
	{"NFC_card_rtp.bin"},
	{"wakeup_voice_assistant_rtp.bin"},
	{"NFC_card_slow_rtp.bin"},
	{"aw8697_rtp_1.bin"}, //99
	{"aw8697_rtp_1.bin"}, //100
	{"offline_countdown_RTP.bin"},
	{"scene_bomb_injury_RTP.bin"},
	{"scene_bomb_RTP.bin"}, //103
	{"door_open_RTP.bin"},
	{"aw8697_rtp_1.bin"},
	{"scene_step_RTP.bin"}, //106
	{"crawl_RTP.bin"},
	{"scope_on_RTP.bin"},
	{"scope_off_RTP.bin"},
	{"magazine_quick_RTP.bin"},
	{"grenade_RTP.bin"},
	{"scene_getshot_RTP.bin"}, //112
	{"grenade_explosion_RTP.bin"},
	{"punch_RTP.bin"},
	{"pan_RTP.bin"},
	{"bandage_RTP.bin"},
	{"aw8697_rtp_1.bin"},
	{"scene_jump_RTP.bin"},
	{"vehicle_plane_RTP.bin"}, //119
	{"scene_openparachute_RTP.bin"}, //120
	{"scene_closeparachute_RTP.bin"}, //121
	{"vehicle_collision_RTP.bin"},
	{"vehicle_buggy_RTP.bin"}, //123
	{"vehicle_dacia_RTP.bin"}, //124
	{"vehicle_moto_RTP.bin"}, //125
	{"firearms_akm_RTP.bin"}, //126
	{"firearms_m16a4_RTP.bin"}, //127
	{"aw8697_rtp_1.bin"},
	{"firearms_awm_RTP.bin"}, //129
	{"firearms_mini14_RTP.bin"}, //130
	{"firearms_vss_RTP.bin"}, //131
	{"firearms_qbz_RTP.bin"}, //132
	{"firearms_ump9_RTP.bin"}, //133
	{"firearms_dp28_RTP.bin"}, //134
	{"firearms_s1897_RTP.bin"}, //135
	{"aw8697_rtp_1.bin"},
	{"firearms_p18c_RTP.bin"}, //137
	{"aw8697_rtp_1.bin"},
	{"aw8697_rtp_1.bin"},
	{"CFM_KillOne_RTP.bin"},
	{"CFM_Headshot_RTP.bin"}, //141
	{"CFM_MultiKill_RTP.bin"},
	{"CFM_KillOne_Strong_RTP.bin"},
	{"CFM_Headshot_Strong_RTP.bin"},
	{"CFM_MultiKill_Strong_RTP.bin"},
	{"CFM_Weapon_Grenade_Explode_RTP.bin"},
	{"CFM_Weapon_Grenade_KillOne_RTP.bin"},
	{"CFM_ImpactFlesh_Normal_RTP.bin"},
	{"CFM_Weapon_C4_Installed_RTP.bin"},
	{"CFM_Hero_Appear_RTP.bin"},
	{"CFM_UI_Reward_OpenBox_RTP.bin"},
	{"CFM_UI_Reward_Task_RTP.bin"},
	{"CFM_Weapon_BLT_Shoot_RTP.bin"}, //153
	{"Atlantis_RTP.bin"},
	{"DigitalUniverse_RTP.bin"},
	{"Reveries_RTP.bin"},
	{"FOD_Motion_Triang_RTP.bin"},
	{"FOD_Motion_Flare_RTP.bin"},
	{"FOD_Motion_Ripple_RTP.bin"},
	{"FOD_Motion_Spiral_RTP.bin"},
	{"gamebox_launch_rtp.bin"}, // 161
	{"Gesture_Back_Pull_RTP.bin"},// 162
	{"Gesture_Back_Release_RTP.bin"},// 163
	{"alert_rtp.bin"},// 164
	{"feedback_negative_light_rtp.bin"},// 165
	{"feedback_neutral_rtp.bin"},// 166
	{"feedback_positive_rtp.bin"},// 167
	{"fingerprint_record_rtp.bin"},// 168
	{"lockdown_rtp.bin"},// 169
	{"sliding_damping_rtp.bin"},// 170
	{"todo_alldone_rtp.bin"},// 171
	{"uninstall_animation_icon_rtp.bin"},// 172
	{"signal_button_highlight_rtp.bin"},//173
	{"signal_button_negative_rtp.bin"},
	{"signal_button_rtp.bin"},
	{"signal_clock_high_rtp.bin"},//176
	{"signal_clock_rtp.bin"},
	{"signal_clock_unit_rtp.bin"},
	{"signal_inputbox_rtp.bin"},
	{"signal_key_high_rtp.bin"},
	{"signal_key_unit_rtp.bin"},//181
	{"signal_list_highlight_rtp.bin"},
	{"signal_list_rtp.bin"},
	{"signal_picker_rtp.bin"},
	{"signal_popup_rtp.bin"},
	{"signal_seekbar_rtp.bin"},//186
	{"signal_switch_rtp.bin"},
	{"signal_tab_rtp.bin"},
	{"signal_text_rtp.bin"},
	{"signal_transition_light_rtp.bin"},
	{"signal_transition_rtp.bin"},//191
	{"haptics_video_rtp.bin"},//192
	{"keyboard_clicky_down_rtp.bin"},
	{"keyboard_clicky_up_rtp.bin"},
	{"keyboard_linear_down_rtp.bin"},
	{"keyboard_linear_up_rtp.bin"}, //196
};
#endif
static int CUSTOME_WAVE_ID;
struct aw8697_container *aw8697_rtp;
struct aw8697 *g_aw8697;

/******************************************************
 *
 * functions
 *
 ******************************************************/
static void aw8697_interrupt_clear(struct aw8697 *aw8697);
static int aw8697_haptic_trig_enable_config(struct aw8697 *aw8697);
static int aw8697_haptic_get_vbat(struct aw8697 *aw8697);
 /******************************************************
 *
 * aw8697 i2c write/read
 *
 ******************************************************/
static int aw8697_i2c_write(struct aw8697 *aw8697,
			    unsigned char reg_addr, unsigned char reg_data)
{
	int ret = -1;
	unsigned char cnt = 0;

	while (cnt < AW_I2C_RETRIES) {
		ret =
		    i2c_smbus_write_byte_data(aw8697->i2c, reg_addr, reg_data);
		if (ret < 0) {
			pr_err("%s: i2c_write cnt=%d error=%d\n", __func__, cnt,
			       ret);
		} else {
			break;
		}
		cnt++;
		msleep(AW_I2C_RETRY_DELAY);
	}

	return ret;
}

static int aw8697_i2c_read(struct aw8697 *aw8697,
			   unsigned char reg_addr, unsigned char *reg_data)
{
	int ret = -1;
	unsigned char cnt = 0;

	while (cnt < AW_I2C_RETRIES) {
		ret = i2c_smbus_read_byte_data(aw8697->i2c, reg_addr);
		if (ret < 0) {
			pr_err("%s: i2c_read cnt=%d error=%d\n", __func__, cnt,
			       ret);
		} else {
			*reg_data = ret;
			break;
		}
		cnt++;
		msleep(AW_I2C_RETRY_DELAY);
	}

	return ret;
}

static int aw8697_i2c_write_bits(struct aw8697 *aw8697,
				 unsigned char reg_addr, unsigned int mask,
				 unsigned char reg_data)
{
	unsigned char reg_val = 0;

	aw8697_i2c_read(aw8697, reg_addr, &reg_val);
	reg_val &= mask;
	reg_val |= reg_data;
	aw8697_i2c_write(aw8697, reg_addr, reg_val);

	return 0;
}

static int aw8697_i2c_writes(struct aw8697 *aw8697,
			     unsigned char reg_addr, unsigned char *buf,
			     unsigned int len)
{
	int ret = -1;
	unsigned char *data;

	data = kmalloc(len + 1, GFP_KERNEL);
	if (data == NULL) {
		pr_err("%s: can not allocate memory\n", __func__);
		return -ENOMEM;
	}

	data[0] = reg_addr;
	memcpy(&data[1], buf, len);

	ret = i2c_master_send(aw8697->i2c, data, len + 1);
	if (ret < 0) {
		pr_err("%s: i2c master send error\n", __func__);
	}

	kfree(data);

	return ret;
}

/*****************************************************
 *
 * ram update
 *
 *****************************************************/
static void aw8697_rtp_loaded(const struct firmware *cont, void *context)
{
	struct aw8697 *aw8697 = context;
	pr_info("%s enter\n", __func__);

	if (!cont) {
		pr_err("%s: failed to read %s\n", __func__,
		       aw8697_rtp_name[aw8697->rtp_file_num]);
		release_firmware(cont);
		return;
	}

	pr_info("%s: loaded %s - size: %zu\n", __func__,
		aw8697_rtp_name[aw8697->rtp_file_num], cont ? cont->size : 0);

	/* aw8697 rtp update */
	aw8697_rtp = vmalloc(cont->size + sizeof(int));
	if (!aw8697_rtp) {
		release_firmware(cont);
		pr_err("%s: Error allocating memory\n", __func__);
		return;
	}
	aw8697_rtp->len = cont->size;
	pr_info("%s: rtp size = %d\n", __func__, aw8697_rtp->len);
	memcpy(aw8697_rtp->data, cont->data, cont->size);
	release_firmware(cont);

	aw8697->rtp_init = 1;
	pr_info("%s: rtp update complete\n", __func__);
}

static int aw8697_rtp_update(struct aw8697 *aw8697)
{
	pr_info("%s enter\n", __func__);

	return request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG,
				       aw8697_rtp_name[aw8697->rtp_file_num],
				       aw8697->dev, GFP_KERNEL, aw8697,
				       aw8697_rtp_loaded);
}

static void aw8697_container_update(struct aw8697 *aw8697,
				    struct aw8697_container *aw8697_cont)
{
	int i = 0;
	unsigned int shift = 0;

	pr_info("%s enter\n", __func__);

	mutex_lock(&aw8697->lock);

	aw8697->ram.baseaddr_shift = 2;
	aw8697->ram.ram_shift = 4;

	/* RAMINIT Enable */
	aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSCTRL,
			      AW8697_BIT_SYSCTRL_RAMINIT_MASK,
			      AW8697_BIT_SYSCTRL_RAMINIT_EN);

	/* base addr */
	shift = aw8697->ram.baseaddr_shift;
	aw8697->ram.base_addr =
	    (unsigned int)((aw8697_cont->data[0 + shift] << 8) |
			   (aw8697_cont->data[1 + shift]));
	pr_info("%s: base_addr=0x%4x\n", __func__, aw8697->ram.base_addr);

	aw8697_i2c_write(aw8697, AW8697_REG_BASE_ADDRH,
			 aw8697_cont->data[0 + shift]);
	aw8697_i2c_write(aw8697, AW8697_REG_BASE_ADDRL,
			 aw8697_cont->data[1 + shift]);

	aw8697_i2c_write(aw8697, AW8697_REG_FIFO_AEH,
			 (unsigned char)((aw8697->ram.base_addr >> 2) >> 8));
	aw8697_i2c_write(aw8697, AW8697_REG_FIFO_AEL,
			 (unsigned char)((aw8697->ram.base_addr >> 2) &
					 0x00FF));
	aw8697_i2c_write(aw8697, AW8697_REG_FIFO_AFH,
			 (unsigned
			  char)((aw8697->ram.base_addr -
				 (aw8697->ram.base_addr >> 2)) >> 8));
	aw8697_i2c_write(aw8697, AW8697_REG_FIFO_AFL,
			 (unsigned
			  char)((aw8697->ram.base_addr -
				 (aw8697->ram.base_addr >> 2)) & 0x00FF));

	/* ram */
	shift = aw8697->ram.baseaddr_shift;
	aw8697_i2c_write(aw8697, AW8697_REG_RAMADDRH,
			 aw8697_cont->data[0 + shift]);
	aw8697_i2c_write(aw8697, AW8697_REG_RAMADDRL,
			 aw8697_cont->data[1 + shift]);
	shift = aw8697->ram.ram_shift;
	for (i = shift; i < aw8697_cont->len; i++) {
		aw8697_i2c_write(aw8697, AW8697_REG_RAMDATA,
				 aw8697_cont->data[i]);
	}

	/* RAMINIT Disable */
	aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSCTRL,
			      AW8697_BIT_SYSCTRL_RAMINIT_MASK,
			      AW8697_BIT_SYSCTRL_RAMINIT_OFF);

	mutex_unlock(&aw8697->lock);

	pr_info("%s exit\n", __func__);
}

static void aw8697_ram_loaded(const struct firmware *cont, void *context)
{
	struct aw8697 *aw8697 = context;
	struct aw8697_container *aw8697_fw;
	int i = 0;
	unsigned short check_sum = 0;

	pr_info("%s enter\n", __func__);

	if (!cont) {
		pr_err("%s: failed to read %s\n", __func__, aw8697_ram_name);
		release_firmware(cont);
		return;
	}

	pr_info("%s: loaded %s - size: %zu\n", __func__, aw8697_ram_name,
		cont ? cont->size : 0);
	/*
	for(i=0; i<cont->size; i++) {
		pr_info("%s: addr:0x%04x, data:0x%02x\n", __func__, i, *(cont->data+i));
	}
	*/

	/* check sum */
	for (i = 2; i < cont->size; i++) {
		check_sum += cont->data[i];
	}
	if (check_sum !=
	    (unsigned short)((cont->data[0] << 8) | (cont->data[1]))) {
		pr_err("%s: check sum err: check_sum=0x%04x\n", __func__,
		       check_sum);
		return;
	} else {
		pr_info("%s: check sum pass : 0x%04x\n", __func__, check_sum);
		aw8697->ram.check_sum = check_sum;
	}

	/* aw8697 ram update */
	aw8697_fw = kzalloc(cont->size + sizeof(int), GFP_KERNEL);
	if (!aw8697_fw) {
		release_firmware(cont);
		pr_err("%s: Error allocating memory\n", __func__);
		return;
	}
	aw8697_fw->len = cont->size;
	memcpy(aw8697_fw->data, cont->data, cont->size);
	release_firmware(cont);

	aw8697_container_update(aw8697, aw8697_fw);

	aw8697->ram.len = aw8697_fw->len;

	kfree(aw8697_fw);

	aw8697->ram_init = 1;
	pr_info("%s: fw update complete\n", __func__);

	aw8697_haptic_trig_enable_config(aw8697);

	aw8697_rtp_update(aw8697);
}

static int aw8697_ram_update(struct aw8697 *aw8697)
{
	aw8697->ram_init = 0;
	aw8697->rtp_init = 0;
	return request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG,
				       aw8697_ram_name, aw8697->dev, GFP_KERNEL,
				       aw8697, aw8697_ram_loaded);
}

#ifdef AWINIC_RAM_UPDATE_DELAY
static void aw8697_ram_work_routine(struct work_struct *work)
{
	struct aw8697 *aw8697 =
	    container_of(work, struct aw8697, ram_work.work);

	pr_info("%s enter\n", __func__);

	aw8697_ram_update(aw8697);

}
#endif

static int aw8697_ram_init(struct aw8697 *aw8697)
{
#ifdef AWINIC_RAM_UPDATE_DELAY
	int ram_timer_val = 5000;
	INIT_DELAYED_WORK(&aw8697->ram_work, aw8697_ram_work_routine);
	//schedule_delayed_work(&aw8697->ram_work,
			      //msecs_to_jiffies(ram_timer_val));
	queue_delayed_work(aw8697->work_queue, &aw8697->ram_work,  msecs_to_jiffies(ram_timer_val));
#else
	aw8697_ram_update(aw8697);
#endif
	return 0;
}

/*****************************************************
 *
 * haptic control
 *
 *****************************************************/
static int aw8697_haptic_softreset(struct aw8697 *aw8697)
{
	pr_debug("%s enter\n", __func__);

	aw8697_i2c_write(aw8697, AW8697_REG_ID, 0xAA);
	msleep(1);
	return 0;
}

static int aw8697_haptic_active(struct aw8697 *aw8697)
{
	pr_debug("%s enter\n", __func__);

	aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSCTRL,
			      AW8697_BIT_SYSCTRL_WORK_MODE_MASK,
			      AW8697_BIT_SYSCTRL_ACTIVE);
	aw8697_interrupt_clear(aw8697);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSINTM,
			      AW8697_BIT_SYSINTM_UVLO_MASK,
			      AW8697_BIT_SYSINTM_UVLO_EN);
	return 0;
}

static int aw8697_haptic_play_mode(struct aw8697 *aw8697,
				   unsigned char play_mode)
{
	pr_debug("%s enter\n", __func__);

	switch (play_mode) {
	case AW8697_HAPTIC_STANDBY_MODE:
		aw8697->play_mode = AW8697_HAPTIC_STANDBY_MODE;
		aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSINTM,
				      AW8697_BIT_SYSINTM_UVLO_MASK,
				      AW8697_BIT_SYSINTM_UVLO_OFF);
		aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSCTRL,
				      AW8697_BIT_SYSCTRL_WORK_MODE_MASK,
				      AW8697_BIT_SYSCTRL_STANDBY);
		break;
	case AW8697_HAPTIC_RAM_MODE:
		aw8697->play_mode = AW8697_HAPTIC_RAM_MODE;
		aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSCTRL,
				      AW8697_BIT_SYSCTRL_PLAY_MODE_MASK,
				      AW8697_BIT_SYSCTRL_PLAY_MODE_RAM);
		aw8697_haptic_active(aw8697);
		aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSCTRL,
				      AW8697_BIT_SYSCTRL_BST_MODE_MASK,
				      AW8697_BIT_SYSCTRL_BST_MODE_BOOST);
		break;
	case AW8697_HAPTIC_RAM_LOOP_MODE:
		aw8697->play_mode = AW8697_HAPTIC_RAM_LOOP_MODE;
		aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSCTRL,
				      AW8697_BIT_SYSCTRL_PLAY_MODE_MASK,
				      AW8697_BIT_SYSCTRL_PLAY_MODE_RAM);
		aw8697_haptic_active(aw8697);
		aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSCTRL,
				      AW8697_BIT_SYSCTRL_BST_MODE_MASK,
				      AW8697_BIT_SYSCTRL_BST_MODE_BYPASS);
		break;
	case AW8697_HAPTIC_RTP_MODE:
		aw8697->play_mode = AW8697_HAPTIC_RTP_MODE;
		aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSCTRL,
				      AW8697_BIT_SYSCTRL_PLAY_MODE_MASK,
				      AW8697_BIT_SYSCTRL_PLAY_MODE_RTP);
		aw8697_haptic_active(aw8697);
		aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSCTRL,
				      AW8697_BIT_SYSCTRL_BST_MODE_MASK,
				      AW8697_BIT_SYSCTRL_BST_MODE_BOOST);
		break;
	case AW8697_HAPTIC_TRIG_MODE:
		aw8697->play_mode = AW8697_HAPTIC_TRIG_MODE;
		aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSCTRL,
				      AW8697_BIT_SYSCTRL_PLAY_MODE_MASK,
				      AW8697_BIT_SYSCTRL_PLAY_MODE_RAM);
		aw8697_haptic_active(aw8697);
		break;
	case AW8697_HAPTIC_CONT_MODE:
		aw8697->play_mode = AW8697_HAPTIC_CONT_MODE;
		aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSCTRL,
				      AW8697_BIT_SYSCTRL_PLAY_MODE_MASK,
				      AW8697_BIT_SYSCTRL_PLAY_MODE_CONT);
		aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSCTRL,
				      AW8697_BIT_SYSCTRL_BST_MODE_MASK,
				      AW8697_BIT_SYSCTRL_BST_MODE_BYPASS);
		aw8697_haptic_active(aw8697);
		break;
	default:
		dev_err(aw8697->dev, "%s: play mode %d err",
			__func__, play_mode);
		break;
	}
	return 0;
}

static int aw8697_haptic_play_go(struct aw8697 *aw8697, bool flag)
{
	if (flag == true) {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_GO,
				      AW8697_BIT_GO_MASK, AW8697_BIT_GO_ENABLE);
	} else {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_GO,
				      AW8697_BIT_GO_MASK,
				      AW8697_BIT_GO_DISABLE);
	}
	return 0;
}

static int aw8697_haptic_stop_delay(struct aw8697 *aw8697)
{
	unsigned char reg_val = 0;
	unsigned int cnt = 100;

	while (cnt--) {
		aw8697_i2c_read(aw8697, AW8697_REG_GLB_STATE, &reg_val);
		if ((reg_val & 0x0f) == 0x00) {
			return 0;
		}
		msleep(2);
		pr_debug("%s wait for standby, reg glb_state=0x%02x\n",
			 __func__, reg_val);
	}
	pr_err("%s do not enter standby automatically\n", __func__);

	return 0;
}

static int aw8697_haptic_stop(struct aw8697 *aw8697)
{
	pr_debug("%s enter\n", __func__);

	aw8697_haptic_play_go(aw8697, false);
	aw8697_haptic_stop_delay(aw8697);
	aw8697_haptic_play_mode(aw8697, AW8697_HAPTIC_STANDBY_MODE);

	return 0;
}

static int aw8697_haptic_start(struct aw8697 *aw8697)
{
	pr_debug("%s enter\n", __func__);

	aw8697_haptic_play_go(aw8697, true);

	return 0;
}

static int aw8697_haptic_set_wav_seq(struct aw8697 *aw8697,
				     unsigned char wav, unsigned char seq)
{
	aw8697_i2c_write(aw8697, AW8697_REG_WAVSEQ1 + wav, seq);
	return 0;
}

static int aw8697_haptic_set_wav_loop(struct aw8697 *aw8697,
				      unsigned char wav, unsigned char loop)
{
	unsigned char tmp = 0;

	if (wav % 2) {
		tmp = loop << 0;
		aw8697_i2c_write_bits(aw8697, AW8697_REG_WAVLOOP1 + (wav / 2),
				      AW8697_BIT_WAVLOOP_SEQNP1_MASK, tmp);
	} else {
		tmp = loop << 4;
		aw8697_i2c_write_bits(aw8697, AW8697_REG_WAVLOOP1 + (wav / 2),
				      AW8697_BIT_WAVLOOP_SEQN_MASK, tmp);
	}

	return 0;
}

static int aw8697_haptic_set_repeat_wav_seq(struct aw8697 *aw8697,
					    unsigned char seq)
{
	aw8697_haptic_set_wav_seq(aw8697, 0x00, seq);
	aw8697_haptic_set_wav_loop(aw8697, 0x00,
				   AW8697_BIT_WAVLOOP_INIFINITELY);

	return 0;
}

static int aw8697_haptic_set_bst_vol(struct aw8697 *aw8697,
				     unsigned char bst_vol)
{
	if (bst_vol & 0xe0) {
		bst_vol = 0x1f;
	}
	//printk("%s %d --\n", __func__, __LINE__);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_BSTDBG4,
			      AW8697_BIT_BSTDBG4_BSTVOL_MASK, (bst_vol << 1));
	return 0;
}

static int aw8697_haptic_set_bst_peak_cur(struct aw8697 *aw8697,
					  unsigned char peak_cur)
{
	peak_cur &= AW8697_BSTCFG_PEAKCUR_LIMIT;
	pr_info("%s  %d enter\n", __func__, __LINE__);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_BSTCFG,
			      AW8697_BIT_BSTCFG_PEAKCUR_MASK, peak_cur);
	return 0;
}

static int aw8697_haptic_set_gain(struct aw8697 *aw8697, unsigned char gain)
{
	unsigned char comp_gain = 0;
	if (aw8697->ram_vbat_comp == AW8697_HAPTIC_RAM_VBAT_COMP_ENABLE)
	{
		aw8697_haptic_get_vbat(aw8697);
		pr_debug("%s: ref %d vbat %d ", __func__, AW8697_VBAT_REFER,
				aw8697->vbat);
		comp_gain = gain * AW8697_VBAT_REFER / aw8697->vbat;
		if (comp_gain > (128 * AW8697_VBAT_REFER / AW8697_VBAT_MIN)) {
			comp_gain = 128 * AW8697_VBAT_REFER / AW8697_VBAT_MIN;
			pr_debug("%s: comp gain limit is %d ", comp_gain);
		}
		pr_info("%s: enable vbat comp, level = %x comp level = %x", __func__,
			   gain, comp_gain);
		aw8697_i2c_write(aw8697, AW8697_REG_DATDBG, comp_gain);
	} else {
		pr_debug("%s: disable compsensation, vbat=%d, vbat_min=%d, vbat_ref=%d",
				__func__, aw8697->vbat, AW8697_VBAT_MIN, AW8697_VBAT_REFER);
		aw8697_i2c_write(aw8697, AW8697_REG_DATDBG, gain);
	}
	return 0;
}

static int aw8697_haptic_set_pwm(struct aw8697 *aw8697, unsigned char mode)
{
	switch (mode) {
	case AW8697_PWM_48K:
		aw8697_i2c_write_bits(aw8697, AW8697_REG_PWMDBG,
				      AW8697_BIT_PWMDBG_PWM_MODE_MASK,
				      AW8697_BIT_PWMDBG_PWM_48K);
		break;
	case AW8697_PWM_24K:
		aw8697_i2c_write_bits(aw8697, AW8697_REG_PWMDBG,
				      AW8697_BIT_PWMDBG_PWM_MODE_MASK,
				      AW8697_BIT_PWMDBG_PWM_24K);
		break;
	case AW8697_PWM_12K:
		aw8697_i2c_write_bits(aw8697, AW8697_REG_PWMDBG,
				      AW8697_BIT_PWMDBG_PWM_MODE_MASK,
				      AW8697_BIT_PWMDBG_PWM_12K);
		break;
	default:
		break;
	}
	return 0;
}

static int aw8697_haptic_play_repeat_seq(struct aw8697 *aw8697,
					 unsigned char flag)
{
	pr_debug("%s enter\n", __func__);

	if (flag) {
		aw8697_haptic_play_mode(aw8697, AW8697_HAPTIC_RAM_LOOP_MODE);
		aw8697_haptic_start(aw8697);
	}

	return 0;
}

/*****************************************************
 *
 * motor protect
 *
 *****************************************************/
static int aw8697_haptic_swicth_motorprotect_config(struct aw8697 *aw8697,
						    unsigned char addr,
						    unsigned char val)
{
	pr_info("%s enter\n", __func__);

	if (addr == 1) {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_DETCTRL,
				      AW8697_BIT_DETCTRL_PROTECT_MASK,
				      AW8697_BIT_DETCTRL_PROTECT_SHUTDOWN);
		aw8697_i2c_write_bits(aw8697, AW8697_REG_PWMPRC,
				      AW8697_BIT_PWMPRC_PRC_MASK,
				      AW8697_BIT_PWMPRC_PRC_ENABLE);
		aw8697_i2c_write_bits(aw8697, AW8697_REG_PRLVL,
				      AW8697_BIT_PRLVL_PR_MASK,
				      AW8697_BIT_PRLVL_PR_ENABLE);
	} else if (addr == 0) {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_DETCTRL,
				      AW8697_BIT_DETCTRL_PROTECT_MASK,
				      AW8697_BIT_DETCTRL_PROTECT_NO_ACTION);
		aw8697_i2c_write_bits(aw8697, AW8697_REG_PWMPRC,
				      AW8697_BIT_PWMPRC_PRC_MASK,
				      AW8697_BIT_PWMPRC_PRC_DISABLE);
		aw8697_i2c_write_bits(aw8697, AW8697_REG_PRLVL,
				      AW8697_BIT_PRLVL_PR_MASK,
				      AW8697_BIT_PRLVL_PR_DISABLE);
	} else if (addr == 0x2d) {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_PWMPRC,
				      AW8697_BIT_PWMPRC_PRCTIME_MASK, val);
	} else if (addr == 0x3e) {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_PRLVL,
				      AW8697_BIT_PRLVL_PRLVL_MASK, val);
	} else if (addr == 0x3f) {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_PRTIME,
				      AW8697_BIT_PRTIME_PRTIME_MASK, val);
	} else {
		/*nothing to do; */
	}
	return 0;
}

/*****************************************************
 *
 * offset calibration
 *
 *****************************************************/
static int aw8697_haptic_offset_calibration(struct aw8697 *aw8697)
{
	unsigned int cont = 2000;
	unsigned char reg_val = 0;

	pr_info("%s enter\n", __func__);

	aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSCTRL,
			      AW8697_BIT_SYSCTRL_RAMINIT_MASK,
			      AW8697_BIT_SYSCTRL_RAMINIT_EN);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_DETCTRL,
			      AW8697_BIT_DETCTRL_DIAG_GO_MASK,
			      AW8697_BIT_DETCTRL_DIAG_GO_ENABLE);
	while (1) {
		aw8697_i2c_read(aw8697, AW8697_REG_DETCTRL, &reg_val);
		if ((reg_val & 0x01) == 0 || cont == 0)
			break;
		cont--;
	}
	aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSCTRL,
			      AW8697_BIT_SYSCTRL_RAMINIT_MASK,
			      AW8697_BIT_SYSCTRL_RAMINIT_OFF);

	return 0;
}

/*****************************************************
 *
 * trig config
 *
 *****************************************************/

static int aw8697_haptic_trig_param_init(struct aw8697 *aw8697)
{
	pr_info("%s enter\n", __func__);

	aw8697->trig[0].enable = aw8697->info.trig_config[0][0];
	aw8697->trig[0].default_level = aw8697->info.trig_config[0][1];
	aw8697->trig[0].dual_edge = aw8697->info.trig_config[0][2];
	aw8697->trig[0].frist_seq = aw8697->info.trig_config[0][3];
	aw8697->trig[0].second_seq = aw8697->info.trig_config[0][4];

	aw8697->trig[1].enable = aw8697->info.trig_config[1][0];;
	aw8697->trig[1].default_level = aw8697->info.trig_config[1][1];
	aw8697->trig[1].dual_edge = aw8697->info.trig_config[1][2];
	aw8697->trig[1].frist_seq = aw8697->info.trig_config[1][3];
	aw8697->trig[1].second_seq = aw8697->info.trig_config[1][4];

	aw8697->trig[2].enable = aw8697->info.trig_config[2][0];;
	aw8697->trig[2].default_level = aw8697->info.trig_config[2][1];
	aw8697->trig[2].dual_edge = aw8697->info.trig_config[2][2];
	aw8697->trig[2].frist_seq = aw8697->info.trig_config[2][3];
	aw8697->trig[2].second_seq = aw8697->info.trig_config[2][4];

	return 0;
}

static int aw8697_haptic_trig_param_config(struct aw8697 *aw8697)
{
	pr_info("%s enter\n", __func__);

	if (aw8697->trig[0].default_level) {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_TRG_CFG1,
				      AW8697_BIT_TRGCFG1_TRG1_POLAR_MASK,
				      AW8697_BIT_TRGCFG1_TRG1_POLAR_NEG);
	} else {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_TRG_CFG1,
				      AW8697_BIT_TRGCFG1_TRG1_POLAR_MASK,
				      AW8697_BIT_TRGCFG1_TRG1_POLAR_POS);
	}
	if (aw8697->trig[1].default_level) {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_TRG_CFG1,
				      AW8697_BIT_TRGCFG1_TRG2_POLAR_MASK,
				      AW8697_BIT_TRGCFG1_TRG2_POLAR_NEG);
	} else {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_TRG_CFG1,
				      AW8697_BIT_TRGCFG1_TRG2_POLAR_MASK,
				      AW8697_BIT_TRGCFG1_TRG2_POLAR_POS);
	}
	if (aw8697->trig[2].default_level) {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_TRG_CFG1,
				      AW8697_BIT_TRGCFG1_TRG3_POLAR_MASK,
				      AW8697_BIT_TRGCFG1_TRG3_POLAR_NEG);
	} else {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_TRG_CFG1,
				      AW8697_BIT_TRGCFG1_TRG3_POLAR_MASK,
				      AW8697_BIT_TRGCFG1_TRG3_POLAR_POS);
	}

	if (aw8697->trig[0].dual_edge) {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_TRG_CFG1,
				      AW8697_BIT_TRGCFG1_TRG1_EDGE_MASK,
				      AW8697_BIT_TRGCFG1_TRG1_EDGE_POS_NEG);
	} else {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_TRG_CFG1,
				      AW8697_BIT_TRGCFG1_TRG1_EDGE_MASK,
				      AW8697_BIT_TRGCFG1_TRG1_EDGE_POS);
	}
	if (aw8697->trig[1].dual_edge) {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_TRG_CFG1,
				      AW8697_BIT_TRGCFG1_TRG2_EDGE_MASK,
				      AW8697_BIT_TRGCFG1_TRG2_EDGE_POS_NEG);
	} else {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_TRG_CFG1,
				      AW8697_BIT_TRGCFG1_TRG2_EDGE_MASK,
				      AW8697_BIT_TRGCFG1_TRG2_EDGE_POS);
	}
	if (aw8697->trig[2].dual_edge) {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_TRG_CFG1,
				      AW8697_BIT_TRGCFG1_TRG3_EDGE_MASK,
				      AW8697_BIT_TRGCFG1_TRG3_EDGE_POS_NEG);
	} else {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_TRG_CFG1,
				      AW8697_BIT_TRGCFG1_TRG3_EDGE_MASK,
				      AW8697_BIT_TRGCFG1_TRG3_EDGE_POS);
	}

	if (aw8697->trig[0].frist_seq) {
		aw8697_i2c_write(aw8697, AW8697_REG_TRG1_WAV_P,
				 aw8697->trig[0].frist_seq);
	}
	if (aw8697->trig[0].second_seq && aw8697->trig[0].dual_edge) {
		aw8697_i2c_write(aw8697, AW8697_REG_TRG1_WAV_N,
				 aw8697->trig[0].second_seq);
	}
	if (aw8697->trig[1].frist_seq) {
		aw8697_i2c_write(aw8697, AW8697_REG_TRG2_WAV_P,
				 aw8697->trig[1].frist_seq);
	}
	if (aw8697->trig[1].second_seq && aw8697->trig[1].dual_edge) {
		aw8697_i2c_write(aw8697, AW8697_REG_TRG2_WAV_N,
				 aw8697->trig[1].second_seq);
	}
	if (aw8697->trig[2].frist_seq) {
		aw8697_i2c_write(aw8697, AW8697_REG_TRG3_WAV_P,
				 aw8697->trig[1].frist_seq);
	}
	if (aw8697->trig[2].second_seq && aw8697->trig[2].dual_edge) {
		aw8697_i2c_write(aw8697, AW8697_REG_TRG3_WAV_N,
				 aw8697->trig[1].second_seq);
	}

	return 0;
}

static int aw8697_haptic_trig_enable_config(struct aw8697 *aw8697)
{
	pr_info("%s enter\n", __func__);

	aw8697_i2c_write_bits(aw8697, AW8697_REG_TRG_CFG2,
			      AW8697_BIT_TRGCFG2_TRG1_ENABLE_MASK,
			      aw8697->trig[0].enable);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_TRG_CFG2,
			      AW8697_BIT_TRGCFG2_TRG2_ENABLE_MASK,
			      aw8697->trig[1].enable);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_TRG_CFG2,
			      AW8697_BIT_TRGCFG2_TRG3_ENABLE_MASK,
			      aw8697->trig[2].enable);

	return 0;
}

static int aw8697_haptic_auto_boost_config(struct aw8697 *aw8697,
					   unsigned char flag)
{
	aw8697->auto_boost = flag;
	if (flag) {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_BST_AUTO,
					AW8697_BIT_BST_AUTO_BST_AUTOSW_MASK,
					AW8697_BIT_BST_AUTO_BST_AUTOMATIC_BOOST);
	} else {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_BST_AUTO,
					AW8697_BIT_BST_AUTO_BST_AUTOSW_MASK,
					AW8697_BIT_BST_AUTO_BST_MANUAL_BOOST);
	}
	return 0;
}

/*****************************************************
 *
 * vbat mode
 *
 *****************************************************/
static int aw8697_haptic_cont_vbat_mode(struct aw8697 *aw8697,
					unsigned char flag)
{
	if (flag == AW8697_HAPTIC_CONT_VBAT_HW_COMP_MODE) {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_ADCTEST,
				      AW8697_BIT_ADCTEST_VBAT_MODE_MASK,
				      AW8697_BIT_ADCTEST_VBAT_HW_COMP);
	} else {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_ADCTEST,
				      AW8697_BIT_ADCTEST_VBAT_MODE_MASK,
				      AW8697_BIT_ADCTEST_VBAT_SW_COMP);
	}
	return 0;
}

static int aw8697_haptic_get_vbat(struct aw8697 *aw8697)
{
	unsigned char reg_val = 0;
	unsigned char reg_val_sysctrl = 0;
	unsigned char reg_val_detctrl = 0;

	aw8697_haptic_stop(aw8697);
	aw8697_i2c_read(aw8697, AW8697_REG_SYSCTRL, &reg_val_sysctrl);
	aw8697_i2c_read(aw8697, AW8697_REG_DETCTRL, &reg_val_detctrl);
	/*step 1:EN_RAMINIT*/
	aw8697_i2c_write_bits(aw8697,
				AW8697_REG_SYSCTRL,
				AW8697_BIT_SYSCTRL_RAMINIT_MASK,
				AW8697_BIT_SYSCTRL_RAMINIT_EN);

	/*step 2 :launch offset cali */
	aw8697_i2c_write_bits(aw8697,
				AW8697_REG_DETCTRL,
				AW8697_BIT_DETCTRL_DIAG_GO_MASK,
				AW8697_BIT_DETCTRL_DIAG_GO_ENABLE);
	/*step 3 :delay */
	usleep_range(2000, 2500);

	/*step 4 :launch power supply testing */
	aw8697_i2c_write_bits(aw8697,
				AW8697_REG_DETCTRL,
				AW8697_BIT_DETCTRL_VBAT_GO_MASK,
				AW8697_BIT_DETCTRL_VABT_GO_ENABLE);
	usleep_range(2000, 2500);

	aw8697_i2c_read(aw8697, AW8697_REG_VBATDET, &reg_val);
	aw8697->vbat = 6100 * reg_val / 256;

	/*step 5: return val*/
	aw8697_i2c_write(aw8697, AW8697_REG_SYSCTRL, reg_val_sysctrl);

	return 0;
}

static int aw8697_haptic_ram_vbat_comp(struct aw8697 *aw8697, bool flag)
{
	pr_debug("%s: enter\n", __func__);
	if (flag)
		aw8697->ram_vbat_comp = AW8697_HAPTIC_RAM_VBAT_COMP_ENABLE;
	else
		aw8697->ram_vbat_comp = AW8697_HAPTIC_RAM_VBAT_COMP_DISABLE;
	return 0;
}

/*****************************************************
 *
 * f0
 *
 *****************************************************/
static int aw8697_haptic_set_f0_preset(struct aw8697 *aw8697)
{
	unsigned int f0_reg = 0;

	pr_info("%s enter\n", __func__);

	f0_reg = 1000000000 / (aw8697->info.f0_pre * aw8697->info.f0_coeff);
	aw8697_i2c_write(aw8697, AW8697_REG_F_PRE_H,
			 (unsigned char)((f0_reg >> 8) & 0xff));
	aw8697_i2c_write(aw8697, AW8697_REG_F_PRE_L,
			 (unsigned char)((f0_reg >> 0) & 0xff));

	return 0;
}

#ifndef USE_CONT_F0_CALI
static int aw8697_haptic_read_f0(struct aw8697 *aw8697)
{
	int ret = 0;
	unsigned char reg_val = 0;
	unsigned int f0_reg = 0;
	unsigned long f0_tmp = 0;

	pr_info("%s enter\n", __func__);

	ret = aw8697_i2c_read(aw8697, AW8697_REG_F_LRA_F0_H, &reg_val);
	f0_reg = (reg_val << 8);
	ret = aw8697_i2c_read(aw8697, AW8697_REG_F_LRA_F0_L, &reg_val);
	f0_reg |= (reg_val << 0);
	if (!f0_reg) {
		pr_info("%s: not get f0_reg value is 0!\n", __func__);
		aw8697->f0_cali_status = false;
		return 0;
	}
	aw8697->f0_cali_status = true;
	f0_tmp = 1000000000 / (f0_reg * aw8697->info.f0_coeff);
	aw8697->f0 = (unsigned int)f0_tmp;
	pr_info("%s f0=%d\n", __func__, aw8697->f0);

	return 0;
}
#endif

#ifndef USE_CONT_F0_CALI
static int aw8697_haptic_read_cont_f0(struct aw8697 *aw8697)
{
	int ret = 0;
	unsigned char reg_val = 0;
	unsigned int f0_reg = 0;
	unsigned long f0_tmp = 0;

	pr_debug("%s enter\n", __func__);

	ret = aw8697_i2c_read(aw8697, AW8697_REG_F_LRA_CONT_H, &reg_val);
	f0_reg = (reg_val << 8);
	ret = aw8697_i2c_read(aw8697, AW8697_REG_F_LRA_CONT_L, &reg_val);
	f0_reg |= (reg_val << 0);
	if (!f0_reg) {
		pr_info("%s: not get f0_reg value is 0!\n", __func__);
		return 0;
	}
	f0_tmp = 1000000000 / (f0_reg * aw8697->info.f0_coeff);
	aw8697->cont_f0 = (unsigned int)f0_tmp;
	pr_info("%s f0=%d\n", __func__, aw8697->cont_f0);

	return 0;
}
#else
static int aw8697_haptic_read_cont_f0(struct aw8697 *aw8697)
{
	int ret = 0;
	unsigned char reg_val = 0;
	unsigned int f0_reg = 0;
	unsigned long f0_tmp = 0;

	pr_debug("%s enter\n", __func__);

	ret = aw8697_i2c_read(aw8697, AW8697_REG_F_LRA_CONT_H, &reg_val);
	f0_reg = (reg_val << 8);
	ret = aw8697_i2c_read(aw8697, AW8697_REG_F_LRA_CONT_L, &reg_val);
	f0_reg |= (reg_val << 0);
	if (!f0_reg) {
		pr_info("%s: not get f0_reg value is 0!\n", __func__);
		return 0;
	}
	f0_tmp = 1000000000 / (f0_reg * aw8697->info.f0_coeff);
	aw8697->cont_f0 = (unsigned int)f0_tmp;
	aw8697->cont_f0 -= 12;
	aw8697->f0 = aw8697->cont_f0;
	pr_info("%s f0=%d\n", __func__, aw8697->cont_f0);

	return 0;
}
#endif

#ifndef USE_CONT_F0_CALI
static int aw8697_haptic_read_beme(struct aw8697 *aw8697)
{
	int ret = 0;
	unsigned char reg_val = 0;
	pr_info("%s  %d enter\n", __func__, __LINE__);
	ret = aw8697_i2c_read(aw8697, AW8697_REG_WAIT_VOL_MP, &reg_val);
	aw8697->max_pos_beme = (reg_val << 0);
	ret = aw8697_i2c_read(aw8697, AW8697_REG_WAIT_VOL_MN, &reg_val);
	aw8697->max_neg_beme = (reg_val << 0);

	pr_info("%s max_pos_beme=%d\n", __func__, aw8697->max_pos_beme);
	pr_info("%s max_neg_beme=%d\n", __func__, aw8697->max_neg_beme);

	return 0;
}
#else
static int aw8697_haptic_read_cont_bemf(struct aw8697 *aw8697)
{
	int ret = 0;
	unsigned char reg_val = 0;
	unsigned int bemf = 0;

	ret = aw8697_i2c_read(aw8697, AW8697_REG_BEMF_VOL_H, &reg_val);
	bemf |= (reg_val<<8);
	ret = aw8697_i2c_read(aw8697, AW8697_REG_BEMF_VOL_L, &reg_val);
	bemf |= (reg_val<<0);

	pr_info("%s bemf=%d\n", __func__, bemf);

	return 0;
}
#endif

/*****************************************************
 *
 * rtp
 *
 *****************************************************/
static void aw8697_haptic_set_rtp_aei(struct aw8697 *aw8697, bool flag)
{
	if (flag) {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSINTM,
				      AW8697_BIT_SYSINTM_FF_AE_MASK,
				      AW8697_BIT_SYSINTM_FF_AE_EN);
	} else {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSINTM,
				      AW8697_BIT_SYSINTM_FF_AE_MASK,
				      AW8697_BIT_SYSINTM_FF_AE_OFF);
	}
}

static unsigned char aw8697_haptic_rtp_get_fifo_afi(struct aw8697 *aw8697)
{
	unsigned char ret = 0;
	unsigned char reg_val = 0;

	if (aw8697->osc_cali_flag == 1) {
		aw8697_i2c_read(aw8697, AW8697_REG_SYSST, &reg_val);
		reg_val &= AW8697_BIT_SYSST_FF_AFS;
		ret = reg_val >> 3;
	} else {
		aw8697_i2c_read(aw8697, AW8697_REG_SYSINT, &reg_val);
		reg_val &= AW8697_BIT_SYSINT_FF_AFI;
		ret = reg_val >> 3;
	}

	return ret;
}

/*****************************************************
 *
 * rtp
 *
 *****************************************************/
static int aw8697_haptic_rtp_init(struct aw8697 *aw8697)
{
	unsigned int buf_len = 0;
	unsigned int period_size = aw8697->ram.base_addr >> 2 ;

	pm_qos_add_request(&pm_qos_req_vb, PM_QOS_CPU_DMA_LATENCY, PM_QOS_VALUE_VB);
	aw8697->rtp_cnt = 0;
	while ((!aw8697_haptic_rtp_get_fifo_afi(aw8697)) &&
	       (aw8697->play_mode == AW8697_HAPTIC_RTP_MODE) &&
	       !atomic_read(&aw8697->exit_in_rtp_loop)) {
		if (aw8697->is_custom_wave == 0) {
			if ((aw8697_rtp->len - aw8697->rtp_cnt) <
			(aw8697->ram.base_addr >> 2)) {
				buf_len = aw8697_rtp->len - aw8697->rtp_cnt;
			} else {
				buf_len = (aw8697->ram.base_addr >> 2);
			}
			aw8697_i2c_writes(aw8697, AW8697_REG_RTP_DATA,
					  &aw8697_rtp->data[aw8697->rtp_cnt], buf_len);
			aw8697->rtp_cnt += buf_len;
			if (aw8697->rtp_cnt == aw8697_rtp->len) {
				aw8697->rtp_cnt = 0;
				aw8697_haptic_set_rtp_aei(aw8697, false);
				pm_qos_remove_request(&pm_qos_req_vb);
				return 0;
			}
		} else {
			buf_len = read_rb(aw8697_rtp->data,  period_size);
			aw8697_i2c_writes(aw8697, AW8697_REG_RTP_DATA, aw8697_rtp->data, buf_len);
			if (buf_len < period_size) {
				pr_info("%s: custom rtp update complete\n", __func__);
				aw8697->rtp_cnt = 0;
				aw8697_haptic_set_rtp_aei(aw8697, false);
				pm_qos_remove_request(&pm_qos_req_vb);
				return 0;
			}
		}
	}
	if (aw8697->play_mode == AW8697_HAPTIC_RTP_MODE && !atomic_read(&aw8697->exit_in_rtp_loop)) {
		aw8697_haptic_set_rtp_aei(aw8697, true);
	}
	pr_info("%s: exit\n", __func__);
	pm_qos_remove_request(&pm_qos_req_vb);
	return 0;
}

static int16_t aw8697_haptic_effect_strength(struct aw8697 *aw8697)
{
	pr_debug("%s enter\n", __func__);
	pr_debug("%s: aw8697->play.vmax_mv =0x%x\n", __func__, aw8697->play.vmax_mv);
#if 0
	switch (aw8697->play.vmax_mv) {
	case AW8697_LIGHT_MAGNITUDE:
		aw8697->level = 0x80;
		break;
	case AW8697_MEDIUM_MAGNITUDE:
		aw8697->level = 0x50;
		break;
	case AW8697_STRONG_MAGNITUDE:
		aw8697->level = 0x30;
		break;
	default:
		break;
	}
#else
	if (aw8697->play.vmax_mv >= 0x7FFF)
		aw8697->level = 0x80; /*128*/
	else if (aw8697->play.vmax_mv <= 0x3FFF)
		aw8697->level = 0x1E; /*30*/
    else
		aw8697->level = (aw8697->play.vmax_mv - 16383) / 128;
	if( aw8697->level < 0x1E)
		aw8697->level = 0x1E; /*30*/
#endif

	pr_info("%s: aw8697->level =0x%x\n", __func__, aw8697->level);
	return 0;
}

static int aw8697_haptic_play_effect_seq(struct aw8697 *aw8697,
					 unsigned char flag)
{
	if (aw8697->effect_id > aw8697->info.effect_id_boundary)
		return 0;
	//printk("%s:aw8697->effect_id =%d\n", __func__, aw8697->effect_id);
	//printk("%s:aw8697->activate_mode =%d\n", __func__, aw8697->activate_mode);

	if (flag) {
		if (aw8697->activate_mode == AW8697_HAPTIC_ACTIVATE_RAM_MODE) {
			aw8697_haptic_set_wav_seq(aw8697, 0x00,
						(char)aw8697->effect_id + 1);
			aw8697_haptic_set_wav_seq(aw8697, 0x01, 0x00);
			aw8697_haptic_set_wav_loop(aw8697, 0x00, 0x00);
			aw8697_haptic_play_mode(aw8697, AW8697_HAPTIC_RAM_MODE);
			if (aw8697->info.bst_vol_ram <= AW8697_MAX_BST_VO)
				aw8697_haptic_set_bst_vol(aw8697, aw8697->info.bst_vol_ram);
			else
				aw8697_haptic_set_bst_vol(aw8697, aw8697->vmax);
			aw8697_haptic_effect_strength(aw8697);
			aw8697_haptic_set_gain(aw8697, aw8697->level);
			aw8697_haptic_start(aw8697);
		}
		if (aw8697->activate_mode == AW8697_HAPTIC_ACTIVATE_RAM_LOOP_MODE) {
			aw8697_haptic_set_repeat_wav_seq(aw8697, (aw8697->info.effect_id_boundary + 1));
			aw8697_haptic_effect_strength(aw8697);
			aw8697_haptic_set_gain(aw8697, aw8697->level);
			aw8697_haptic_play_repeat_seq(aw8697, true);
		}
	}

	return 0;
}

static void aw8697_haptic_upload_lra(struct aw8697 *aw8697, unsigned char flag)
{
	if (flag) {
		//printk("%s f0_cali_lra=%d\n", __func__, aw8697->f0_calib_data);
		aw8697_i2c_write(aw8697, AW8697_REG_TRIM_LRA,
				 (char)aw8697->f0_calib_data);
	} else {
		//printk("%s rtp_cali_lra=%d\n", __func__, aw8697->lra_calib_data);
		aw8697_i2c_write(aw8697, AW8697_REG_TRIM_LRA,
				 (char)aw8697->lra_calib_data);
	}

}

static int aw8697_clock_OSC_trim_calibration(unsigned long int theory_time, unsigned long int real_time)
{
	unsigned int real_code = 0;
	unsigned int LRA_TRIM_CODE = 0;
	unsigned int DFT_LRA_TRIM_CODE = 0;
	unsigned int Not_need_cali_threshold = 10;/*0.1 percent not need calibrate*/

	if (theory_time == real_time) {
		pr_info("aw_osctheory_time == real_time:%ld  theory_time = %ld not need to cali\n", real_time, theory_time);
		return 0;
	} else if (theory_time < real_time) {
		if ((real_time - theory_time) > (theory_time / 50)) {
			pr_info("aw_osc(real_time - theory_time) > (theory_time/50) not to cali\n");
			return DFT_LRA_TRIM_CODE;
		}

		if ((real_time - theory_time) < (Not_need_cali_threshold*theory_time/10000)) {
			pr_info("aw_oscmicrosecond:%ld  theory_time = %ld not need to cali\n", real_time, theory_time);
			return DFT_LRA_TRIM_CODE;
		}

		real_code = ((real_time - theory_time) * 4000) / theory_time;
		real_code = ((real_code%10 < 5) ? 0 : 1) + real_code/10;
		real_code = 32 + real_code;
	} else if (theory_time > real_time) {
		if ((theory_time - real_time) > (theory_time / 50)) {
			pr_info("aw_osc((theory_time - real_time) > (theory_time / 50)) not to cali\n");
			return DFT_LRA_TRIM_CODE;
		}
		if ((theory_time - real_time) < (Not_need_cali_threshold * theory_time/10000)) {
			pr_info("aw_oscmicrosecond:%ld  theory_time = %ld not need to cali\n", real_time, theory_time);
			return DFT_LRA_TRIM_CODE;
		}
		real_code = ((theory_time - real_time) * 4000) / theory_time;
		real_code = ((real_code%10 < 5) ? 0 : 1) + real_code/10;
		real_code = 32 - real_code;
	}
	if (real_code > 31)
		LRA_TRIM_CODE = real_code - 32;
	else
		LRA_TRIM_CODE = real_code + 32;
	pr_info("aw_oscmicrosecond:%ld  theory_time = %ld real_code =0X%02X LRA_TRIM_CODE 0X%02X\n", real_time, theory_time, real_code, LRA_TRIM_CODE);

	return LRA_TRIM_CODE;
}

static int aw8697_rtp_trim_lra_calibration(struct aw8697 *aw8697)
{
	unsigned char reg_val = 0;
	unsigned int fre_val = 0;
	unsigned int theory_time = 0;
	unsigned int lra_rtim_code = 0;

	aw8697_i2c_read(aw8697, AW8697_REG_PWMDBG, &reg_val);
	fre_val = (reg_val & 0x006f) >> 5;

	if (fre_val == 3)
		theory_time = (aw8697->rtp_len / 12000) * 1000000; /*12K */
	if (fre_val == 2)
		theory_time = (aw8697->rtp_len / 24000) * 1000000; /*24K */
	if (fre_val == 1 || fre_val == 0)
		theory_time = (aw8697->rtp_len / 48000) * 1000000; /*48K */

	printk("microsecond:%ld  theory_time = %d\n", aw8697->microsecond, theory_time);

	lra_rtim_code = aw8697_clock_OSC_trim_calibration(theory_time, aw8697->microsecond);
	if (lra_rtim_code > 0) {
		aw8697->lra_calib_data = lra_rtim_code;
		aw8697_i2c_write(aw8697, AW8697_REG_TRIM_LRA, (char)lra_rtim_code);
	}
	return 0;
}
static unsigned char aw8697_haptic_osc_read_int(struct aw8697 *aw8697)
{
	unsigned char reg_val = 0;

	aw8697_i2c_read(aw8697, AW8697_REG_DBGSTAT, &reg_val);
	return reg_val;
}

static int aw8697_rtp_osc_calibration(struct aw8697 *aw8697)
{
	const struct firmware *rtp_file;
	int ret = -1;
	unsigned int buf_len = 0;
	unsigned char osc_int_state = 0;
	aw8697->rtp_cnt = 0;
	aw8697->timeval_flags = 1;
	aw8697->osc_cali_flag = 1;

	pr_info("%s enter\n", __func__);
	/* fw loaded */
	ret = request_firmware(&rtp_file,
		aw8697_rtp_name[/*aw8697->rtp_file_num*/ 0],
		aw8697->dev);
	if (ret < 0) {
		pr_err("%s: failed to read %s\n", __func__,
			aw8697_rtp_name[/*aw8697->rtp_file_num*/ 0]);
		return ret;
	}
	/*awinic add stop,for irq interrupt during calibrate*/
	aw8697_haptic_stop(aw8697);
	aw8697->rtp_init = 0;
	mutex_lock(&aw8697->rtp_lock);
	vfree(aw8697_rtp);
	aw8697_rtp = vmalloc(rtp_file->size+sizeof(int));
	if (!aw8697_rtp) {
		release_firmware(rtp_file);
		mutex_unlock(&aw8697->rtp_lock);
		pr_err("%s: error allocating memory\n", __func__);
		return -ENOMEM;
	}
	aw8697_rtp->len = rtp_file->size;
	aw8697->rtp_len = rtp_file->size;
	pr_info("%s: rtp file [%s] size = %d\n", __func__,
		aw8697_rtp_name[/*aw8697->rtp_file_num*/ 0], aw8697_rtp->len);
	memcpy(aw8697_rtp->data, rtp_file->data, rtp_file->size);
	release_firmware(rtp_file);
	mutex_unlock(&aw8697->rtp_lock);

	/* gain */
	aw8697_haptic_ram_vbat_comp(aw8697, false);

	/* rtp mode config */
	aw8697_haptic_play_mode(aw8697, AW8697_HAPTIC_RTP_MODE);

	aw8697_i2c_write_bits(aw8697, AW8697_REG_DBGCTRL,
		AW8697_BIT_DBGCTRL_INT_MODE_MASK, AW8697_BIT_DBGCTRL_INT_MODE_EDGE);
	disable_irq(gpio_to_irq(aw8697->irq_gpio));
	/* haptic start */
	aw8697_haptic_start(aw8697);
	pm_qos_add_request(&pm_qos_req_vb, PM_QOS_CPU_DMA_LATENCY, PM_QOS_VALUE_VB);
	while (1) {
		if (!aw8697_haptic_rtp_get_fifo_afi(aw8697)) {
			mutex_lock(&aw8697->rtp_lock);
			if ((aw8697_rtp->len - aw8697->rtp_cnt) < (aw8697->ram.base_addr>>2))
				buf_len = aw8697_rtp->len-aw8697->rtp_cnt;
			else
				buf_len = (aw8697->ram.base_addr >> 2);
			if (aw8697->rtp_cnt != aw8697_rtp->len) {
				if (aw8697->timeval_flags == 1) {
					ktime_get_real_ts64(&aw8697->start);
					aw8697->timeval_flags = 0;
				}
				aw8697_i2c_writes(aw8697, AW8697_REG_RTP_DATA, &aw8697_rtp->data[aw8697->rtp_cnt], buf_len);
				aw8697->rtp_cnt += buf_len;
			}
			mutex_unlock(&aw8697->rtp_lock);
		}
		osc_int_state = aw8697_haptic_osc_read_int(aw8697);
		if (osc_int_state&AW8697_BIT_SYSINT_DONEI) {
			ktime_get_real_ts64(&aw8697->end);
			pr_info("%s vincent playback done aw8697->rtp_cnt= %d \n", __func__, aw8697->rtp_cnt);
			break;
		}

		ktime_get_real_ts64(&aw8697->end);
		aw8697->microsecond = (aw8697->end.tv_sec - aw8697->start.tv_sec)*1000000 +
					(aw8697->end.tv_nsec - aw8697->start.tv_nsec) / 1000000;
		if (aw8697->microsecond > OSC_CALIBRATION_T_LENGTH) {
			pr_info("%s vincent time out aw8697->rtp_cnt %d osc_int_state %02x\n", __func__, aw8697->rtp_cnt, osc_int_state);
			break;
		}
	}
	pm_qos_remove_request(&pm_qos_req_vb);
	enable_irq(gpio_to_irq(aw8697->irq_gpio));

	aw8697->osc_cali_flag = 0;
	aw8697->microsecond = (aw8697->end.tv_sec - aw8697->start.tv_sec)*1000000 +
				(aw8697->end.tv_nsec - aw8697->start.tv_nsec) / 1000000;
	/*calibration osc*/
	pr_info("%s 2018_microsecond:%ld \n", __func__, aw8697->microsecond);
	pr_info("%s exit\n", __func__);
	return 0;
}


static void aw8697_rtp_work_routine(struct work_struct *work)
{
	const struct firmware *rtp_file;
	int ret = -1;
	struct aw8697 *aw8697 = container_of(work, struct aw8697, rtp_work);

	if ((aw8697->effect_id < aw8697->info.effect_id_boundary) &&
	    (aw8697->effect_id > aw8697->info.effect_max))
		return;

	pr_info("%s: effect_id = %d state=%d activate_mode = %d\n", __func__,
		aw8697->effect_id, aw8697->state, aw8697->activate_mode);
	mutex_lock(&aw8697->lock);
	aw8697_haptic_upload_lra(aw8697, false);
	aw8697_haptic_set_rtp_aei(aw8697, false);
	aw8697_interrupt_clear(aw8697);
	//wait for irq to exit


	atomic_set(&aw8697->exit_in_rtp_loop, 1);
	while (atomic_read(&aw8697->is_in_rtp_loop)) {
		pr_info("%s  goint to waiting irq exit\n", __func__);
		mutex_unlock(&aw8697->lock);
		ret = wait_event_interruptible(aw8697->wait_q, atomic_read(&aw8697->is_in_rtp_loop) == 0);
		pr_info("%s  wakeup \n", __func__);
		mutex_lock(&aw8697->lock);
		if (ret == -ERESTARTSYS) {
			atomic_set(&aw8697->exit_in_rtp_loop, 0);
			wake_up_interruptible(&aw8697->stop_wait_q);
			mutex_unlock(&aw8697->lock);
			pr_err("%s wake up by signal return erro\n", __func__);
			return;
		}
	}

	atomic_set(&aw8697->exit_in_rtp_loop, 0);
	wake_up_interruptible(&aw8697->stop_wait_q);


		//how to force exit this call
		if (aw8697->is_custom_wave == 1 && aw8697->state) {
			pr_err("%s buffer size %d, availbe size %d \n", __func__, aw8697->ram.base_addr >> 2, get_rb_avalible_size());
			while (get_rb_avalible_size() < aw8697->ram.base_addr && !rb_shoule_exit()) {
			mutex_unlock(&aw8697->lock);
			ret = wait_event_interruptible(aw8697->stop_wait_q, (get_rb_avalible_size() >= aw8697->ram.base_addr) || rb_shoule_exit());
			pr_info("%s  wakeup  \n", __func__);
			pr_err("%s after wakeup sbuffer size %d, availbe size %d \n", __func__, aw8697->ram.base_addr >> 2, get_rb_avalible_size());
			if (ret == -ERESTARTSYS) {
			pr_err("%s wake up by signal return erro\n", __func__);
			return ;
		}
			mutex_lock(&aw8697->lock);

		}
	}

	aw8697_haptic_stop(aw8697);

	if (aw8697->state) {
		pm_stay_awake(aw8697->dev);
		if (aw8697->info.bst_vol_ram <= AW8697_MAX_BST_VO)
			aw8697_haptic_set_bst_vol(aw8697, aw8697->info.bst_vol_rtp);
		else
			aw8697_haptic_set_bst_vol(aw8697, aw8697->vmax);
		aw8697_haptic_ram_vbat_comp(aw8697, false);
		aw8697_haptic_effect_strength(aw8697);
		aw8697_haptic_set_gain(aw8697, aw8697->level);
		aw8697->rtp_init = 0;
		if (aw8697->is_custom_wave == 0) {
			aw8697->rtp_file_num =
			    aw8697->effect_id - aw8697->info.effect_id_boundary;
			printk("%s: aw8697->rtp_file_num =%d\n", __func__,
			       aw8697->rtp_file_num);
			if (aw8697->rtp_file_num < 0)
				aw8697->rtp_file_num = 0;
			if (aw8697->rtp_file_num > ((sizeof(aw8697_rtp_name) / AW8697_RTP_NAME_MAX) - 1))
				aw8697->rtp_file_num = (sizeof(aw8697_rtp_name) / AW8697_RTP_NAME_MAX) - 1;

			/* fw loaded */
			ret = request_firmware(&rtp_file,
					       aw8697_rtp_name[aw8697->rtp_file_num],
					       aw8697->dev);
			if (ret < 0) {
				pr_err("%s: failed to read %s\n", __func__,
				       aw8697_rtp_name[aw8697->rtp_file_num]);
				pm_relax(aw8697->dev);
				mutex_unlock(&aw8697->lock);
				return;
			}

			vfree(aw8697_rtp);
			aw8697_rtp = vmalloc(rtp_file->size + sizeof(int));
			if (!aw8697_rtp) {
				release_firmware(rtp_file);
				pr_err("%s: error allocating memory\n", __func__);
				pm_relax(aw8697->dev);
				mutex_unlock(&aw8697->lock);
				return;
			}
			aw8697_rtp->len = rtp_file->size;
			pr_info("%s: rtp file [%s] size = %d\n", __func__,
				aw8697_rtp_name[aw8697->rtp_file_num], aw8697_rtp->len);
			memcpy(aw8697_rtp->data, rtp_file->data, rtp_file->size);
			release_firmware(rtp_file);
		} else  {
			vfree(aw8697_rtp);
			aw8697_rtp = vmalloc(aw8697->ram.base_addr >> 2);
			if (!aw8697_rtp) {
				pr_err("%s: error allocating memory\n", __func__);
				pm_relax(aw8697->dev);
				mutex_unlock(&aw8697->lock);
				return;
			}
		}
		aw8697->rtp_init = 1;

		/* rtp mode config */
		aw8697_haptic_play_mode(aw8697, AW8697_HAPTIC_RTP_MODE);

		/* haptic start */
		aw8697_haptic_start(aw8697);

		aw8697_haptic_rtp_init(aw8697);
	} else {
		aw8697->rtp_cnt = 0;
		aw8697->rtp_init = 0;
		pm_relax(aw8697->dev);
	}
	mutex_unlock(&aw8697->lock);
}

static enum hrtimer_restart aw8697_haptic_audio_timer_func(struct hrtimer
							   *timer)
{
	struct aw8697 *aw8697 =
	    container_of(timer, struct aw8697, haptic_audio.timer);

	pr_debug("%s enter\n", __func__);
	//schedule_work(&aw8697->haptic_audio.work);
	queue_work(aw8697->work_queue, &aw8697->haptic_audio.work);

	hrtimer_start(&aw8697->haptic_audio.timer,
		      ktime_set(aw8697->haptic_audio.timer_val / 1000000,
				(aw8697->haptic_audio.timer_val % 1000000) *
				1000), HRTIMER_MODE_REL);
	return HRTIMER_NORESTART;
}

static void aw8697_haptic_audio_work_routine(struct work_struct *work)
{
	struct aw8697 *aw8697 =
	    container_of(work, struct aw8697, haptic_audio.work);

	pr_info("%s enter\n", __func__);

	mutex_lock(&aw8697->haptic_audio.lock);
	memcpy(&aw8697->haptic_audio.ctr,
	       &aw8697->haptic_audio.data[aw8697->haptic_audio.cnt],
	       sizeof(struct haptic_ctr));
	pr_debug("%s: cnt=%d, cmd=%d, play=%d, wavseq=%d, loop=%d, gain=%d\n",
		 __func__,
		 aw8697->haptic_audio.cnt,
		 aw8697->haptic_audio.ctr.cmd,
		 aw8697->haptic_audio.ctr.play,
		 aw8697->haptic_audio.ctr.wavseq,
		 aw8697->haptic_audio.ctr.loop, aw8697->haptic_audio.ctr.gain);
	mutex_unlock(&aw8697->haptic_audio.lock);
	if (AW8697_HAPTIC_CMD_ENABLE == aw8697->haptic_audio.ctr.cmd) {
		if (AW8697_HAPTIC_PLAY_ENABLE == aw8697->haptic_audio.ctr.play) {
			pr_info("%s: haptic_audio_play_start\n", __func__);
			mutex_lock(&aw8697->lock);
			aw8697_haptic_stop(aw8697);
			aw8697_haptic_play_mode(aw8697, AW8697_HAPTIC_RAM_MODE);

			aw8697_haptic_set_wav_seq(aw8697, 0x00,
						  aw8697->haptic_audio.
						  ctr.wavseq);
			aw8697_haptic_set_wav_seq(aw8697, 0x01, 0x00);

			aw8697_haptic_set_wav_loop(aw8697, 0x00,
						   aw8697->haptic_audio.
						   ctr.loop);

			aw8697_haptic_set_gain(aw8697,
					       aw8697->haptic_audio.ctr.gain);

			aw8697_haptic_start(aw8697);
			mutex_unlock(&aw8697->lock);
		} else if (AW8697_HAPTIC_PLAY_STOP ==
			   aw8697->haptic_audio.ctr.play) {
			mutex_lock(&aw8697->lock);
			aw8697_haptic_stop(aw8697);
			mutex_unlock(&aw8697->lock);
		} else if (AW8697_HAPTIC_PLAY_GAIN ==
			   aw8697->haptic_audio.ctr.play) {
			mutex_lock(&aw8697->lock);
			aw8697_haptic_set_gain(aw8697,
					       aw8697->haptic_audio.ctr.gain);
			mutex_unlock(&aw8697->lock);
		}
		mutex_lock(&aw8697->haptic_audio.lock);
		memset(&aw8697->haptic_audio.data[aw8697->haptic_audio.cnt],
		       0, sizeof(struct haptic_ctr));
		mutex_unlock(&aw8697->haptic_audio.lock);
	}

	mutex_lock(&aw8697->haptic_audio.lock);
	aw8697->haptic_audio.cnt++;
	if (aw8697->haptic_audio.data[aw8697->haptic_audio.cnt].cmd == 0) {
		aw8697->haptic_audio.cnt = 0;
		pr_debug("%s: haptic play buffer restart\n", __func__);
	}
	mutex_unlock(&aw8697->haptic_audio.lock);

}

/*****************************************************
 *
 * haptic cont
 *
 *****************************************************/
static int aw8697_haptic_cont(struct aw8697 *aw8697)
{
	pr_info("%s enter\n", __func__);

	/* work mode */
	aw8697_haptic_play_mode(aw8697, AW8697_HAPTIC_CONT_MODE);

	/* preset f0 */
	aw8697_haptic_set_f0_preset(aw8697);

	/* lpf */
	aw8697_i2c_write_bits(aw8697, AW8697_REG_DATCTRL,
			      AW8697_BIT_DATCTRL_FC_MASK,
			      AW8697_BIT_DATCTRL_FC_1000HZ);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_DATCTRL,
			      AW8697_BIT_DATCTRL_LPF_ENABLE_MASK,
			      AW8697_BIT_DATCTRL_LPF_ENABLE);

	/* cont config */
	aw8697_i2c_write_bits(aw8697, AW8697_REG_CONT_CTRL,
			      AW8697_BIT_CONT_CTRL_ZC_DETEC_MASK,
			      AW8697_BIT_CONT_CTRL_ZC_DETEC_ENABLE);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_CONT_CTRL,
			      AW8697_BIT_CONT_CTRL_WAIT_PERIOD_MASK,
			      AW8697_BIT_CONT_CTRL_WAIT_1PERIOD);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_CONT_CTRL,
			      AW8697_BIT_CONT_CTRL_MODE_MASK,
			      AW8697_BIT_CONT_CTRL_BY_GO_SIGNAL);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_CONT_CTRL,
			      AW8697_BIT_CONT_CTRL_EN_CLOSE_MASK,
			      AW8697_CONT_PLAYBACK_MODE);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_CONT_CTRL,
			      AW8697_BIT_CONT_CTRL_F0_DETECT_MASK,
			      AW8697_BIT_CONT_CTRL_F0_DETECT_DISABLE);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_CONT_CTRL,
			      AW8697_BIT_CONT_CTRL_O2C_MASK,
			      AW8697_BIT_CONT_CTRL_O2C_DISABLE);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_CONT_CTRL,
			      AW8697_BIT_CONT_CTRL_AUTO_BRK_MASK,
			      AW8697_BIT_CONT_CTRL_AUTO_BRK_ENABLE);

	/* TD time */
	aw8697_i2c_write(aw8697, AW8697_REG_TD_H,
			 (unsigned char)(aw8697->info.cont_td >> 8));
	aw8697_i2c_write(aw8697, AW8697_REG_TD_L,
			 (unsigned char)(aw8697->info.cont_td >> 0));
	aw8697_i2c_write(aw8697, AW8697_REG_TSET, aw8697->info.tset);

	/* zero cross */
	aw8697_i2c_write(aw8697, AW8697_REG_ZC_THRSH_H,
			 (unsigned char)(aw8697->info.cont_zc_thr >> 8));
	aw8697_i2c_write(aw8697, AW8697_REG_ZC_THRSH_L,
			 (unsigned char)(aw8697->info.cont_zc_thr >> 0));

	aw8697_i2c_write_bits(aw8697, AW8697_REG_BEMF_NUM,
			      AW8697_BIT_BEMF_NUM_BRK_MASK,
			      aw8697->info.cont_num_brk);
	aw8697_i2c_write(aw8697, AW8697_REG_TIME_NZC, 0x23);	// 35*171us=5.985ms

	/* f0 driver level */
	aw8697_i2c_write(aw8697, AW8697_REG_DRV_LVL, aw8697->info.cont_drv_lvl);
	aw8697_i2c_write(aw8697, AW8697_REG_DRV_LVL_OV,
			 aw8697->info.cont_drv_lvl_ov);

	/* cont play go */
	aw8697_haptic_play_go(aw8697, true);

	return 0;
}

#ifndef USE_CONT_F0_CALI
/*****************************************************
 *
 * haptic f0 cali
 *
 *****************************************************/
static int aw8697_haptic_get_f0(struct aw8697 *aw8697)
{
	int ret = 0;
	/*unsigned char i = 0; */
	unsigned char reg_val = 0;
	unsigned char f0_pre_num = 0;
	unsigned char f0_wait_num = 0;
	unsigned char f0_repeat_num = 0;
	unsigned char f0_trace_num = 0;
	unsigned int t_f0_ms = 0;
	unsigned int t_f0_trace_ms = 0;
	/*unsigned int f0_cali_cnt = 50; */

	pr_info("%s enter\n", __func__);

	aw8697->f0 = aw8697->info.f0_pre;

	/* f0 calibrate work mode */
	aw8697_haptic_stop(aw8697);
	aw8697_haptic_play_mode(aw8697, AW8697_HAPTIC_CONT_MODE);

	aw8697_i2c_write_bits(aw8697, AW8697_REG_CONT_CTRL,
			      AW8697_BIT_CONT_CTRL_EN_CLOSE_MASK,
			      AW8697_BIT_CONT_CTRL_OPEN_PLAYBACK);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_CONT_CTRL,
			      AW8697_BIT_CONT_CTRL_F0_DETECT_MASK,
			      AW8697_BIT_CONT_CTRL_F0_DETECT_ENABLE);

	/* LPF */
	aw8697_i2c_write_bits(aw8697, AW8697_REG_DATCTRL,
			      AW8697_BIT_DATCTRL_FC_MASK,
			      AW8697_BIT_DATCTRL_FC_1000HZ);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_DATCTRL,
			      AW8697_BIT_DATCTRL_LPF_ENABLE_MASK,
			      AW8697_BIT_DATCTRL_LPF_ENABLE);

	/* LRA OSC Source */
	if (aw8697->f0_cali_flag == AW8697_HAPTIC_CALI_F0) {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_ANACTRL,
				      AW8697_BIT_ANACTRL_LRA_SRC_MASK,
				      AW8697_BIT_ANACTRL_LRA_SRC_REG);
	} else {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_ANACTRL,
				      AW8697_BIT_ANACTRL_LRA_SRC_MASK,
				      AW8697_BIT_ANACTRL_LRA_SRC_EFUSE);
	}

	/* preset f0 */
	aw8697_haptic_set_f0_preset(aw8697);

	/* f0 driver level */
	aw8697_i2c_write(aw8697, AW8697_REG_DRV_LVL, aw8697->info.cont_drv_lvl);

	/* f0 trace parameter */
	f0_pre_num = aw8697->info.f0_trace_parameter[0];
	f0_wait_num = aw8697->info.f0_trace_parameter[1];
	f0_repeat_num = aw8697->info.f0_trace_parameter[2];
	f0_trace_num = aw8697->info.f0_trace_parameter[3];
	aw8697_i2c_write(aw8697, AW8697_REG_NUM_F0_1,
			 (f0_pre_num << 4) | (f0_wait_num << 0));
	aw8697_i2c_write(aw8697, AW8697_REG_NUM_F0_2, (f0_repeat_num << 0));
	aw8697_i2c_write(aw8697, AW8697_REG_NUM_F0_3, (f0_trace_num << 0));

	/* clear aw8697 interrupt */
	ret = aw8697_i2c_read(aw8697, AW8697_REG_SYSINT, &reg_val);

	/* play go and start f0 calibration */
	aw8697_haptic_play_go(aw8697, true);

	/* f0 trace time */
	t_f0_ms = 1000 * 10 / aw8697->info.f0_pre;
	t_f0_trace_ms =
	    t_f0_ms * (f0_pre_num + f0_wait_num +
		       (f0_trace_num + f0_wait_num) * (f0_repeat_num - 1)) + 50;
	msleep(t_f0_trace_ms);

	#if 0
	for (i = 0; i < f0_cali_cnt; i++) {
		mdelay(200);
		ret = aw8697_i2c_read(aw8697, AW8697_REG_SYSINT, &reg_val);
		/* f0 calibrate done */
		if (reg_val & 0x01) {
			aw8697_haptic_read_f0(aw8697);
			aw8697_haptic_read_beme(aw8697);
			break;
		}
		msleep(200);
		pr_info("%s f0 cali sleep 10ms\n", __func__);
	}

	if (i == f0_cali_cnt) {
		ret = -1;
	} else {
		ret = 0;
	}
	#endif

	aw8697_haptic_read_f0(aw8697);
	aw8697_haptic_read_beme(aw8697);
	/* restore default config */
	aw8697_i2c_write_bits(aw8697, AW8697_REG_CONT_CTRL,
			      AW8697_BIT_CONT_CTRL_EN_CLOSE_MASK,
			      AW8697_CONT_PLAYBACK_MODE);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_CONT_CTRL,
			      AW8697_BIT_CONT_CTRL_F0_DETECT_MASK,
			      AW8697_BIT_CONT_CTRL_F0_DETECT_DISABLE);

	return ret;
}
#else
/*****************************************************
 *
 * haptic cont mode f0 cali
 *
 *****************************************************/
static int aw8697_haptic_get_f0(struct aw8697 *aw8697)
{
	int ret = 0;
	unsigned char i = 0;
	unsigned char reg_val = 0;
	unsigned int t_f0_trace_ms = 0;
	unsigned int f0_cali_cnt = 50;

	pr_info("%s enter\n", __func__);

	aw8697->f0 = aw8697->info.f0_pre;

	/* f0 calibrate work mode */
	aw8697_haptic_stop(aw8697);
	aw8697_haptic_play_mode(aw8697, AW8697_HAPTIC_CONT_MODE);

	aw8697_i2c_write_bits(aw8697, AW8697_REG_CONT_CTRL,
		AW8697_BIT_CONT_CTRL_MODE_MASK, AW8697_BIT_CONT_CTRL_BY_DRV_TIME);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_CONT_CTRL,
		AW8697_BIT_CONT_CTRL_EN_CLOSE_MASK, AW8697_BIT_CONT_CTRL_OPEN_PLAYBACK);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_CONT_CTRL,
		AW8697_BIT_CONT_CTRL_EN_CLOSE_MASK, AW8697_BIT_CONT_CTRL_CLOSE_PLAYBACK);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_CONT_CTRL,
		AW8697_BIT_CONT_CTRL_F0_DETECT_MASK, AW8697_BIT_CONT_CTRL_F0_DETECT_DISABLE);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_CONT_CTRL,
		AW8697_BIT_CONT_CTRL_AUTO_BRK_MASK, AW8697_BIT_CONT_CTRL_AUTO_BRK_DISABLE);

	/* LPF */
	//aw8697_i2c_write_bits(aw8697, AW8697_REG_DATCTRL,
	//        AW8697_BIT_DATCTRL_FC_MASK, AW8697_BIT_DATCTRL_FC_1000HZ);
	//aw8697_i2c_write_bits(aw8697, AW8697_REG_DATCTRL,
	//        AW8697_BIT_DATCTRL_LPF_ENABLE_MASK, AW8697_BIT_DATCTRL_LPF_ENABLE);

	/* LRA OSC Source */
	if(aw8697->f0_cali_flag == AW8697_HAPTIC_CALI_F0) {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_ANACTRL,
			AW8697_BIT_ANACTRL_LRA_SRC_MASK, AW8697_BIT_ANACTRL_LRA_SRC_REG);
	} else {
		aw8697_i2c_write_bits(aw8697, AW8697_REG_ANACTRL,
			AW8697_BIT_ANACTRL_LRA_SRC_MASK, AW8697_BIT_ANACTRL_LRA_SRC_EFUSE);
	}

	/* f0 driver level */
	aw8697_i2c_write(aw8697, AW8697_REG_DRV_LVL, aw8697->info.cont_drv_lvl);
	aw8697_i2c_write(aw8697, AW8697_REG_DRV_LVL_OV, aw8697->info.cont_drv_lvl_ov);

	/* TD time */
	aw8697_i2c_write(aw8697, AW8697_REG_TD_H, aw8697->info.cont_td >> 8);
	aw8697_i2c_write(aw8697, AW8697_REG_TD_L, aw8697->info.cont_td);
	aw8697_i2c_write(aw8697, AW8697_REG_TSET, aw8697->info.tset);

	/* drive time  */
	aw8697_i2c_write(aw8697, AW8697_REG_DRV_TIME, 0x75);

	/* preset f0 */
	aw8697_haptic_set_f0_preset(aw8697);

	/* clear aw8697 interrupt */
	ret = aw8697_i2c_read(aw8697, AW8697_REG_SYSINT, &reg_val);

	/* play go and start f0 calibration */
	aw8697_haptic_play_go(aw8697, true);

	/* f0 trace time */
	t_f0_trace_ms = 0xfe * 684 / 1000;
	msleep(t_f0_trace_ms);

	for(i=0; i<f0_cali_cnt; i++) {
		ret = aw8697_i2c_read(aw8697, AW8697_REG_SYSINT, &reg_val);
		/* f0 calibrate done */
		if(reg_val & 0x01) {
			aw8697_haptic_read_cont_f0(aw8697);
			aw8697_haptic_read_cont_bemf(aw8697);
			break;
		}
		msleep(10);
		pr_info("%s f0 cali sleep 10ms\n", __func__);
	}

	if(i == f0_cali_cnt) {
		ret = -1;
	} else {
		ret = 0;
	}

	/* restore default config */
	aw8697_i2c_write_bits(aw8697, AW8697_REG_CONT_CTRL,
		AW8697_BIT_CONT_CTRL_EN_CLOSE_MASK, AW8697_CONT_PLAYBACK_MODE);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_CONT_CTRL,
		AW8697_BIT_CONT_CTRL_F0_DETECT_MASK, AW8697_BIT_CONT_CTRL_F0_DETECT_DISABLE);

	return ret;
}
#endif

static int aw8697_haptic_f0_calibration(struct aw8697 *aw8697)
{
	int ret = 0;
	unsigned char reg_val = 0;
	unsigned int f0_limit = 0;
	char f0_cali_lra = 0;
	int f0_cali_step = 0;

	pr_info("%s enter\n", __func__);

	aw8697->f0_cali_flag = AW8697_HAPTIC_CALI_F0;

	aw8697_i2c_write(aw8697, AW8697_REG_TRIM_LRA, 0x00);
	if (aw8697_haptic_get_f0(aw8697)) {
		pr_err("%s get f0 error, user defafult f0\n", __func__);
	} else {
		/* max and min limit */
		f0_limit = aw8697->f0;
		if (aw8697->f0 * 100 <
		    aw8697->info.f0_pre * (100 - aw8697->info.f0_cali_percen)) {
			f0_limit = aw8697->info.f0_pre;
		}
		if (aw8697->f0 * 100 >
		    aw8697->info.f0_pre * (100 + aw8697->info.f0_cali_percen)) {
			f0_limit = aw8697->info.f0_pre;
		}

		/* calculate cali step */
		f0_cali_step =
		    100000 * ((int)f0_limit -
			      (int)aw8697->info.f0_pre) / ((int)f0_limit * 25);
		pr_info("%s  line=%d f0_cali_step=%d\n", __func__, __LINE__,
		       f0_cali_step);
		pr_info("%s line=%d  f0_limit=%d\n", __func__, __LINE__,
		       (int)f0_limit);
		pr_info("%s line=%d  aw8697->info.f0_pre=%d\n", __func__,
		       __LINE__, (int)aw8697->info.f0_pre);

		if (f0_cali_step >= 0) {	/*f0_cali_step >= 0 */
			if (f0_cali_step % 10 >= 5) {
				f0_cali_step = f0_cali_step / 10 + 1 + 32;
			} else {
				f0_cali_step = f0_cali_step / 10 + 32;
			}
		} else {	/*f0_cali_step < 0 */
			if (f0_cali_step % 10 <= -5) {
				f0_cali_step = 32 + (f0_cali_step / 10 - 1);
			} else {
				f0_cali_step = 32 + f0_cali_step / 10;
			}
		}

		if (f0_cali_step > 31) {
			f0_cali_lra = (char)f0_cali_step - 32;
		} else {
			f0_cali_lra = (char)f0_cali_step + 32;
		}
		aw8697->f0_calib_data = (int)f0_cali_lra;
		printk("%s f0_cali_lra=%d\n", __func__, (int)f0_cali_lra);

		/* update cali step */
		aw8697_i2c_write(aw8697, AW8697_REG_TRIM_LRA,
				 (char)f0_cali_lra);
		aw8697_i2c_read(aw8697, AW8697_REG_TRIM_LRA, &reg_val);
		pr_info("%s final trim_lra=0x%02x\n", __func__, reg_val);
	}

	/* restore default work mode */
	aw8697_haptic_play_mode(aw8697, AW8697_HAPTIC_STANDBY_MODE);
	aw8697->play_mode = AW8697_HAPTIC_RAM_MODE;
	aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSCTRL,
			      AW8697_BIT_SYSCTRL_PLAY_MODE_MASK,
			      AW8697_BIT_SYSCTRL_PLAY_MODE_RAM);
	aw8697_haptic_stop(aw8697);

	return ret;
}

/*****************************************************
 *
 * haptic fops
 *
 *****************************************************/
static int aw8697_file_open(struct inode *inode, struct file *file)
{
	if (!try_module_get(THIS_MODULE))
		return -ENODEV;
	pr_info("%s enter\n", __func__);
	file->private_data = (void *)g_aw8697;

	return 0;
}

static int aw8697_file_release(struct inode *inode, struct file *file)
{
	file->private_data = (void *)NULL;
	pr_info("%s enter\n", __func__);
	module_put(THIS_MODULE);

	return 0;
}

static long aw8697_file_unlocked_ioctl(struct file *file, unsigned int cmd,
				       unsigned long arg)
{
	struct aw8697 *aw8697 = (struct aw8697 *)file->private_data;

	int ret = 0;
	pr_info("%s enter\n", __func__);
	dev_info(aw8697->dev, "%s: cmd=0x%x, arg=0x%lx\n", __func__, cmd, arg);

	mutex_lock(&aw8697->lock);

	if (_IOC_TYPE(cmd) != AW8697_HAPTIC_IOCTL_MAGIC) {
		dev_err(aw8697->dev, "%s: cmd magic err\n", __func__);
		return -EINVAL;
	}

	switch (cmd) {
	default:
		dev_err(aw8697->dev, "%s, unknown cmd\n", __func__);
		break;
	}

	mutex_unlock(&aw8697->lock);

	return ret;
}

static ssize_t aw8697_file_read(struct file *filp, char *buff, size_t len,
				loff_t *offset)
{
	struct aw8697 *aw8697 = (struct aw8697 *)filp->private_data;
	int ret = 0;
	int i = 0;
	unsigned char reg_val = 0;
	unsigned char *pbuff = NULL;
	pr_info("%s enter\n", __func__);
	mutex_lock(&aw8697->lock);

	dev_info(aw8697->dev, "%s: len=%zu\n", __func__, len);

	switch (aw8697->fileops.cmd) {
	case AW8697_HAPTIC_CMD_READ_REG:
		pbuff = (unsigned char *)kzalloc(len, GFP_KERNEL);
		if (pbuff != NULL) {
			for (i = 0; i < len; i++) {
				aw8697_i2c_read(aw8697, aw8697->fileops.reg + i,
						&reg_val);
				pbuff[i] = reg_val;
			}
			for (i = 0; i < len; i++) {
				dev_info(aw8697->dev, "%s: pbuff[%d]=0x%02x\n",
					 __func__, i, pbuff[i]);
			}
			ret = copy_to_user(buff, pbuff, len);
			if (ret) {
				dev_err(aw8697->dev, "%s: copy to user fail\n",
					__func__);
			}
			kfree(pbuff);
		} else {
			dev_err(aw8697->dev, "%s: alloc memory fail\n",
				__func__);
		}
		break;
	default:
		dev_err(aw8697->dev, "%s, unknown cmd %d \n", __func__,
			aw8697->fileops.cmd);
		break;
	}

	mutex_unlock(&aw8697->lock);

	return len;
}

static ssize_t aw8697_file_write(struct file *filp, const char *buff,
				 size_t len, loff_t *off)
{
	struct aw8697 *aw8697 = (struct aw8697 *)filp->private_data;
	int i = 0;
	int ret = 0;
	unsigned char *pbuff = NULL;
	pr_info("%s enter\n", __func__);
	pbuff = (unsigned char *)kzalloc(len, GFP_KERNEL);
	if (pbuff == NULL) {
		dev_err(aw8697->dev, "%s: alloc memory fail\n", __func__);
		return len;
	}
	ret = copy_from_user(pbuff, buff, len);
	if (ret) {
		dev_err(aw8697->dev, "%s: copy from user fail\n", __func__);
		return len;
	}

	for (i = 0; i < len; i++) {
		dev_info(aw8697->dev, "%s: pbuff[%d]=0x%02x\n",
			 __func__, i, pbuff[i]);
	}

	mutex_lock(&aw8697->lock);

	aw8697->fileops.cmd = pbuff[0];

	switch (aw8697->fileops.cmd) {
	case AW8697_HAPTIC_CMD_READ_REG:
		if (len == 2) {
			aw8697->fileops.reg = pbuff[1];
		} else {
			dev_err(aw8697->dev, "%s: read cmd len %zu err\n",
				__func__, len);
		}
		break;
	case AW8697_HAPTIC_CMD_WRITE_REG:
		if (len > 2) {
			for (i = 0; i < len - 2; i++) {
				dev_info(aw8697->dev,
					 "%s: write reg0x%02x=0x%02x\n",
					 __func__, pbuff[1] + i, pbuff[i + 2]);
				aw8697_i2c_write(aw8697, pbuff[1] + i,
						 pbuff[2 + i]);
			}
		} else {
			dev_err(aw8697->dev, "%s: write cmd len %zu err\n",
				__func__, len);
		}
		break;
	default:
		dev_err(aw8697->dev, "%s, unknown cmd %d \n", __func__,
			aw8697->fileops.cmd);
		break;
	}

	mutex_unlock(&aw8697->lock);

	if (pbuff != NULL) {
		kfree(pbuff);
	}
	return len;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = aw8697_file_read,
	.write = aw8697_file_write,
	.unlocked_ioctl = aw8697_file_unlocked_ioctl,
	.open = aw8697_file_open,
	.release = aw8697_file_release,
};

static struct miscdevice aw8697_haptic_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = AW8697_HAPTIC_NAME,
	.fops = &fops,
};

static int aw8697_haptic_init(struct aw8697 *aw8697)
{
	int ret = 0;
	unsigned char i = 0;
	unsigned char reg_val = 0;
	unsigned char bemf_config = 0;

	pr_info("%s enter\n", __func__);
	ret = misc_register(&aw8697_haptic_misc);
	if (ret) {
		dev_err(aw8697->dev, "%s: misc fail: %d\n", __func__, ret);
		return ret;
	}

	/* haptic audio */
	aw8697->haptic_audio.delay_val = 1;
	aw8697->haptic_audio.timer_val = 21318;
	aw8697->f0_cali_status = true;

	hrtimer_init(&aw8697->haptic_audio.timer, CLOCK_MONOTONIC,
		     HRTIMER_MODE_REL);
	aw8697->haptic_audio.timer.function = aw8697_haptic_audio_timer_func;
	INIT_WORK(&aw8697->haptic_audio.work, aw8697_haptic_audio_work_routine);

	mutex_init(&aw8697->haptic_audio.lock);

	/* haptic init */
	mutex_lock(&aw8697->lock);

	aw8697->activate_mode = aw8697->info.mode;
	aw8697->osc_cali_run = 0;
	ret = aw8697_i2c_read(aw8697, AW8697_REG_WAVSEQ1, &reg_val);
	aw8697->index = reg_val & 0x7F;
	ret = aw8697_i2c_read(aw8697, AW8697_REG_DATDBG, &reg_val);
	aw8697->gain = reg_val & 0xFF;
	ret = aw8697_i2c_read(aw8697, AW8697_REG_BSTDBG4, &reg_val);
	aw8697->vmax = (reg_val >> 1) & 0x1F;
	for (i = 0; i < AW8697_SEQUENCER_SIZE; i++) {
		ret = aw8697_i2c_read(aw8697, AW8697_REG_WAVSEQ1 + i, &reg_val);
		aw8697->seq[i] = reg_val;
	}

	aw8697_haptic_play_mode(aw8697, AW8697_HAPTIC_STANDBY_MODE);

	aw8697_haptic_set_pwm(aw8697, AW8697_PWM_24K);

	aw8697_i2c_write(aw8697, AW8697_REG_BSTDBG1, aw8697->info.bstdbg[0]);
	aw8697_i2c_write(aw8697, AW8697_REG_BSTDBG2, aw8697->info.bstdbg[1]);
	aw8697_i2c_write(aw8697, AW8697_REG_BSTDBG3, aw8697->info.bstdbg[2]);
	aw8697_i2c_write(aw8697, AW8697_REG_TSET, aw8697->info.tset);
	aw8697_i2c_write(aw8697, AW8697_REG_R_SPARE, aw8697->info.r_spare);

	aw8697_i2c_write_bits(aw8697, AW8697_REG_ANADBG,
			      AW8697_BIT_ANADBG_IOC_MASK,
			      AW8697_BIT_ANADBG_IOC_4P65A);

	aw8697_haptic_set_bst_peak_cur(aw8697, AW8697_DEFAULT_PEAKCUR);

	aw8697_haptic_swicth_motorprotect_config(aw8697, 0x00, 0x00);

	aw8697_haptic_auto_boost_config(aw8697, false);

	if ((aw8697->info.trig_config[0][0] == 1) ||
		(aw8697->info.trig_config[1][0] == 1) ||
		(aw8697->info.trig_config[2][0] == 1)) {
		aw8697_haptic_trig_param_init(aw8697);
		aw8697_haptic_trig_param_config(aw8697);
	}

	aw8697_haptic_offset_calibration(aw8697);

	/* vbat compensation */
	aw8697_haptic_cont_vbat_mode(aw8697,
				     AW8697_HAPTIC_CONT_VBAT_HW_COMP_MODE);
	aw8697->ram_vbat_comp = AW8697_HAPTIC_RAM_VBAT_COMP_ENABLE;

	mutex_unlock(&aw8697->lock);

	/* f0 calibration */
	mutex_lock(&aw8697->lock);
#ifndef USE_CONT_F0_CALI
	aw8697_haptic_f0_calibration(aw8697);
#endif
	mutex_unlock(&aw8697->lock);
	/* beme config */
	bemf_config = aw8697->info.bemf_config[0];
	aw8697_i2c_write(aw8697, AW8697_REG_BEMF_VTHH_H, bemf_config);
	bemf_config = aw8697->info.bemf_config[1];
	aw8697_i2c_write(aw8697, AW8697_REG_BEMF_VTHH_L, bemf_config);
	bemf_config = aw8697->info.bemf_config[2];
	aw8697_i2c_write(aw8697, AW8697_REG_BEMF_VTHL_H, bemf_config);
	bemf_config = aw8697->info.bemf_config[3];
	aw8697_i2c_write(aw8697, AW8697_REG_BEMF_VTHL_L, bemf_config);
	return ret;
}

/*****************************************************
 *
 * vibrator
 *
 *****************************************************/
static enum hrtimer_restart qti_hap_stop_timer(struct hrtimer *timer)
{
	struct aw8697 *aw8697 = container_of(timer, struct aw8697,
					     stop_timer);
	int rc;

	pr_info("%s enter\n", __func__);
	aw8697->play.length_us = 0;
	rc = aw8697_haptic_play_go(aw8697, false);	// qti_haptics_play(aw8697, false);
	if (rc < 0)
		dev_err(aw8697->dev, "Stop playing failed, rc=%d\n", rc);

	return HRTIMER_NORESTART;
}

static enum hrtimer_restart qti_hap_disable_timer(struct hrtimer *timer)
{
	struct aw8697 *aw8697 = container_of(timer, struct aw8697,
					     hap_disable_timer);
	int rc;

	pr_info("%s enter\n", __func__);
	rc = aw8697_haptic_play_go(aw8697, false);	//qti_haptics_module_en(aw8697, false);
	if (rc < 0)
		dev_err(aw8697->dev, "Disable haptics module failed, rc=%d\n",
			rc);

	return HRTIMER_NORESTART;
}

static enum hrtimer_restart aw8697_vibrator_timer_func(struct hrtimer *timer)
{
	struct aw8697 *aw8697 = container_of(timer, struct aw8697, timer);

	pr_info("%s enter\n", __func__);

	aw8697->state = 0;
	//schedule_work(&aw8697->vibrator_work);
	queue_work(aw8697->work_queue, &aw8697->vibrator_work);

	return HRTIMER_NORESTART;
}

static void aw8697_vibrator_work_routine(struct work_struct *work)
{
	struct aw8697 *aw8697 =
	    container_of(work, struct aw8697, vibrator_work);

	pr_debug("%s enter\n", __func__);
	pr_info("%s: effect_id = %d state=%d activate_mode = %d duration = %d\n", __func__,
		aw8697->effect_id, aw8697->state, aw8697->activate_mode, aw8697->duration);
	mutex_lock(&aw8697->lock);
	aw8697_haptic_upload_lra(aw8697, true);
	aw8697_haptic_stop(aw8697);
	if (aw8697->state) {
		if (aw8697->activate_mode == AW8697_HAPTIC_ACTIVATE_RAM_MODE) {
			aw8697_haptic_ram_vbat_comp(aw8697, false);
			aw8697_haptic_play_effect_seq(aw8697, true);
		} else if (aw8697->activate_mode ==
			   AW8697_HAPTIC_ACTIVATE_RAM_LOOP_MODE) {
			aw8697_haptic_ram_vbat_comp(aw8697, true);
			aw8697_haptic_play_effect_seq(aw8697, true);
			hrtimer_start(&aw8697->timer,
				      ktime_set(aw8697->duration / 1000,
						(aw8697->duration % 1000) *
						1000000), HRTIMER_MODE_REL);
		} else if (aw8697->activate_mode ==
			   AW8697_HAPTIC_ACTIVATE_CONT_MODE) {
			aw8697_haptic_cont(aw8697);
			hrtimer_start(&aw8697->timer,
				      ktime_set(aw8697->duration / 1000,
						(aw8697->duration % 1000) *
						1000000), HRTIMER_MODE_REL);
		} else {
			/*other mode */
		}
	}
	mutex_unlock(&aw8697->lock);
}

static int aw8697_vibrator_init(struct aw8697 *aw8697)
{
	pr_info("%s enter\n", __func__);

	hrtimer_init(&aw8697->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	aw8697->timer.function = aw8697_vibrator_timer_func;
	INIT_WORK(&aw8697->vibrator_work, aw8697_vibrator_work_routine);
	INIT_WORK(&aw8697->rtp_work, aw8697_rtp_work_routine);

	mutex_init(&aw8697->lock);
	atomic_set(&aw8697->is_in_rtp_loop, 0);
	atomic_set(&aw8697->exit_in_rtp_loop, 0);
	atomic_set(&aw8697->is_in_write_loop, 0);
	init_waitqueue_head(&aw8697->wait_q);
	init_waitqueue_head(&aw8697->stop_wait_q);

	return 0;
}

/******************************************************
 *
 * irq
 *
 ******************************************************/
static void aw8697_interrupt_clear(struct aw8697 *aw8697)
{
	unsigned char reg_val = 0;
	aw8697_i2c_read(aw8697, AW8697_REG_SYSINT, &reg_val);
}

static void aw8697_interrupt_setup(struct aw8697 *aw8697)
{
	unsigned char reg_val = 0;


	aw8697_i2c_read(aw8697, AW8697_REG_SYSINT, &reg_val);

	/* edge int mode */
	aw8697_i2c_write_bits(aw8697, AW8697_REG_DBGCTRL,
			      AW8697_BIT_DBGCTRL_INT_MODE_MASK,
			      AW8697_BIT_DBGCTRL_INT_MODE_EDGE);

	/* int enable */
	aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSINTM,
			      AW8697_BIT_SYSINTM_BSTERR_MASK,
			      AW8697_BIT_SYSINTM_BSTERR_OFF);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSINTM,
			      AW8697_BIT_SYSINTM_OV_MASK,
			      AW8697_BIT_SYSINTM_OV_EN);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSINTM,
			      AW8697_BIT_SYSINTM_UVLO_MASK,
			      AW8697_BIT_SYSINTM_UVLO_EN);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSINTM,
			      AW8697_BIT_SYSINTM_OCD_MASK,
			      AW8697_BIT_SYSINTM_OCD_EN);
	aw8697_i2c_write_bits(aw8697, AW8697_REG_SYSINTM,
			      AW8697_BIT_SYSINTM_OT_MASK,
			      AW8697_BIT_SYSINTM_OT_EN);
}

static irqreturn_t aw8697_irq(int irq, void *data)
{
	struct aw8697 *aw8697 = data;
	unsigned char reg_val = 0;
	unsigned char dbg_val = 0;
	unsigned int buf_len = 0;
	unsigned period_size =  aw8697->ram.base_addr >> 2;


	atomic_set(&aw8697->is_in_rtp_loop, 1);
	aw8697_i2c_read(aw8697, AW8697_REG_SYSINT, &reg_val);
	//pr_info("%s: reg SYSINT=0x%x\n", __func__, reg_val);
	aw8697_i2c_read(aw8697, AW8697_REG_DBGSTAT, &dbg_val);
	//pr_info("%s: reg DBGSTAT=0x%x\n", __func__, dbg_val);

	if (reg_val & AW8697_BIT_SYSINT_OVI) {
		pr_err("%s chip ov int error\n", __func__);
	}
	if (reg_val & AW8697_BIT_SYSINT_UVLI) {
		pr_err("%s chip uvlo int error\n", __func__);
	}
	if (reg_val & AW8697_BIT_SYSINT_OCDI) {
		pr_err("%s chip over current int error\n", __func__);
	}
	if (reg_val & AW8697_BIT_SYSINT_OTI) {
		pr_err("%s chip over temperature int error\n", __func__);
	}
	if (reg_val & AW8697_BIT_SYSINT_DONEI) {
		pr_info("%s chip playback done\n", __func__);
	}

	if (reg_val & AW8697_BIT_SYSINT_FF_AEI) {
		pr_debug("%s: aw8697 rtp fifo almost empty int\n", __func__);
		if (aw8697->rtp_init) {
			while ((!aw8697_haptic_rtp_get_fifo_afi(aw8697)) &&
			       (aw8697->play_mode == AW8697_HAPTIC_RTP_MODE) && !atomic_read(&aw8697->exit_in_rtp_loop)) {
				mutex_lock(&aw8697->rtp_lock);
				if (!aw8697_rtp) {
					pr_info("%s:aw8697_rtp is null break\n",
					__func__);
					mutex_unlock(&aw8697->rtp_lock);
					break;
				}

				if (aw8697->is_custom_wave == 1) {
					buf_len = read_rb(aw8697_rtp->data,  period_size);
					aw8697_i2c_writes(aw8697, AW8697_REG_RTP_DATA, aw8697_rtp->data, buf_len);
					if (buf_len < period_size) {
						pr_info("%s: rtp update complete\n",
							__func__);
						aw8697_haptic_set_rtp_aei(aw8697,
									  false);
						aw8697->rtp_cnt = 0;
						aw8697->rtp_init = 0;
						mutex_unlock(&aw8697->rtp_lock);
						break;
					}
				} else {
					if ((aw8697_rtp->len - aw8697->rtp_cnt) < period_size) {
						buf_len =
						aw8697_rtp->len - aw8697->rtp_cnt;
					} else {
						buf_len = period_size;
					}
					aw8697_i2c_writes(aw8697, AW8697_REG_RTP_DATA,
								&aw8697_rtp->
								data[aw8697->rtp_cnt],
								buf_len);
					aw8697->rtp_cnt += buf_len;
					if (aw8697->rtp_cnt == aw8697_rtp->len) {
						pr_info("%s: rtp update complete\n",
							__func__);
						aw8697_haptic_set_rtp_aei(aw8697,
									  false);
						aw8697->rtp_cnt = 0;
						aw8697->rtp_init = 0;
						mutex_unlock(&aw8697->rtp_lock);
						break;
					}
				}
				mutex_unlock(&aw8697->rtp_lock);
			}
		} else {
			pr_err("%s: aw8697 rtp init = %d, init error\n",
				__func__, aw8697->rtp_init);
		}
	}

	if (reg_val & AW8697_BIT_SYSINT_FF_AFI) {
		pr_debug("%s: aw8697 rtp mode fifo full empty\n", __func__);
	}

	if (aw8697->play_mode != AW8697_HAPTIC_RTP_MODE || atomic_read(&aw8697->exit_in_rtp_loop)) {
		aw8697_haptic_set_rtp_aei(aw8697, false);
	}

	aw8697_i2c_read(aw8697, AW8697_REG_SYSINT, &reg_val);
	pr_debug("%s: reg SYSINT=0x%x\n", __func__, reg_val);
	aw8697_i2c_read(aw8697, AW8697_REG_SYSST, &reg_val);
	pr_debug("%s: reg SYSST=0x%x\n", __func__, reg_val);
	atomic_set(&aw8697->is_in_rtp_loop, 0);
	wake_up_interruptible(&aw8697->wait_q);
	pr_debug("%s exit\n", __func__);
	return IRQ_HANDLED;
}

/*****************************************************
 *
 * device tree
 *
 *****************************************************/
static int aw8697_parse_dt(struct device *dev, struct aw8697 *aw8697,
			   struct device_node *np)
{
	unsigned int val = 0;
	unsigned int bstdbg[6];
	unsigned int f0_trace_parameter[4];
	unsigned int bemf_config[4];
	unsigned int rtp_time[175];
	unsigned int trig_config[15];
	struct qti_hap_config *config = &aw8697->config;
	struct device_node *child_node;
	struct qti_hap_effect *effect;
	int rc = 0, tmp, i = 0, j;

	aw8697->reset_gpio = of_get_named_gpio(np, "reset-gpio", 0);
	if (aw8697->reset_gpio < 0) {
		dev_err(dev,
			"%s: no reset gpio provided, will not HW reset device\n",
			__func__);
		return -EINVAL;
	} else {
		dev_info(dev, "%s: reset gpio provided ok\n", __func__);
	}
	aw8697->irq_gpio = of_get_named_gpio(np, "irq-gpio", 0);
	if (aw8697->irq_gpio < 0) {
		dev_err(dev, "%s: no irq gpio provided.\n", __func__);
	} else {
		dev_info(dev, "%s: irq gpio provided ok.\n", __func__);
	}

	val = of_property_read_u32(np, "vib_mode", &aw8697->info.mode);
	if (val != 0)
		printk("vib_mode not found\n");
	val = of_property_read_u32(np, "vib_f0_pre", &aw8697->info.f0_pre);
	if (val != 0)
		printk("vib_f0_pre not found\n");
	val =
	    of_property_read_u32(np, "vib_f0_cali_percen",
				 &aw8697->info.f0_cali_percen);
	if (val != 0)
		printk("vib_f0_cali_percen not found\n");
	val =
	    of_property_read_u32(np, "vib_cont_drv_lev",
				 &aw8697->info.cont_drv_lvl);
	if (val != 0)
		printk("vib_cont_drv_lev not found\n");
	val =
	    of_property_read_u32(np, "vib_cont_drv_lvl_ov",
				 &aw8697->info.cont_drv_lvl_ov);
	if (val != 0)
		printk("vib_cont_drv_lvl_ov not found\n");
	val = of_property_read_u32(np, "vib_cont_td", &aw8697->info.cont_td);
	if (val != 0)
		printk("vib_cont_td not found\n");
	val =
	    of_property_read_u32(np, "vib_cont_zc_thr",
				 &aw8697->info.cont_zc_thr);
	if (val != 0)
		printk("vib_cont_zc_thr not found\n");
	val =
	    of_property_read_u32(np, "vib_cont_num_brk",
				 &aw8697->info.cont_num_brk);
	if (val != 0)
		printk("vib_cont_num_brk not found\n");
	val = of_property_read_u32(np, "vib_f0_coeff", &aw8697->info.f0_coeff);
	if (val != 0)
		printk("vib_f0_coeff not found\n");

	val = of_property_read_u32(np, "vib_tset", &aw8697->info.tset);
	if (val != 0)
		printk("%s vib_tset not found\n", __func__);
	val = of_property_read_u32(np, "vib_r_spare", &aw8697->info.r_spare);
	if (val != 0)
		printk("%s vib_r_spare not found\n", __func__);
	val = of_property_read_u32_array(np, "vib_bstdbg",
					 bstdbg, ARRAY_SIZE(bstdbg));
	if (val != 0)