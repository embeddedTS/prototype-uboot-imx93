/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __SUPER_H__
#define __SUPER_H__

#define SUPER_I2C_ADDR 0x54

#define SUPER_MODEL 0
#define SUPER_REV_INFO 1
#define SUPER_ADC_CHAN_ADV 2
#define SUPER_FEATURES0 3
#define SUPER_CMDS 8
#define SUPER_GEN_FLAGS 16
#define SUPER_GEN_INPUTS 24
#define SUPER_REBOOT_REASON 32
#define SUPER_SERIAL 34
#define SUPER_ADC_BASE 128
#define SUPER_TEMPERATURE 160

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

int super_write(u16 addr, u16 value);
int super_read(u16 addr, u16 *value);
int wizard_read_mac(uint8_t *mac_buffer);
u16 wizard_byte_order(u16 addr_value);

const char *get_board_name(void);
u16 get_board_model_register(void);
u16 get_board_model_register_early(void);
#endif // __SUPER_H__
