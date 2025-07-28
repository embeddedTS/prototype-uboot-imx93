/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __WIZARD_H__
#define __WIZARD_H__

#define WIZARD_I2C_ADDR 0x54

#define WIZARD_MODEL 0
#define WIZARD_REV_INFO 1
#define WIZARD_ADC_CHAN_ADV 2
#define WIZARD_FEATURES0 3
#define WIZARD_CMDS 8
#define WIZARD_GEN_FLAGS 16
#define WIZARD_GEN_INPUTS 24
#define WIZARD_REBOOT_REASON 32
#define WIZARD_SERIAL 34
#define WIZARD_ADC_BASE 128
#define WIZARD_TEMPERATURE 160

enum i2c_cmds_t {
	I2C_NOCMD  = ((u16)0 << 0),
	I2C_REBOOT = ((u16)1 << 0),
	I2C_HALT   = ((u16)1 << 1),
};

enum reboot_reasons_t {
	REBOOT_REASON_POR = 0,
	REBOOT_REASON_CPU_WDT = 1,
	REBOOT_REASON_SOFTWARE_REBOOT = 2,
	REBOOT_REASON_BROWNOUT = 3,
	REBOOT_REASON_RTC_ALARM_REBOOT = 4,
	REBOOT_REASON_WAKE_FROM_PWR_CYCLE = 5,
	REBOOT_REASON_WAKE_FROM_WAKE_SIGNAL = 6,
	REBOOT_REASON_WAKE_FROM_RTC_ALARM = 7,
	REBOOT_REASON_WAKE_FROM_USB_VBUS = 8,
};

int wizard_write(u16 addr, u16 value);
int wizard_read(u16 addr, u16 *value);
int wizard_read_mac(uint8_t *mac_buffer);

const char *get_board_name(void);
u16 get_board_model_register(void);
#endif // __WIZARD_H__
