// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Technologic Systems, Inc. (dba embeddedTS)
 */

#include <common.h>
#include <env.h>
#include <net.h>
#include <stdbool.h>
#include <stdio.h>

#include "ts-macs.h"
#include "wizard.h"

static void increment_mac(uint8_t *);

/*
 * setup_mac_addresses -- for whatever this board requires.
 *
 * Obtain at least the first MAC from the most viable source, then
 * increment it to assign each additional MAC needed by this platform.
 *
 * The first MAC address should come from the Wizard (aka
 * configuration EEPROM) if possible, fallback to the environment if
 * not, and if not even the environment has a valid MAC, set nothing
 * and let a random one be assigned further down the line.
 */

void setup_mac_addresses(void)
{
	unsigned char enetaddr[6];
	int ret, i;
	char mac_str[16];
	bool from_wizard = 0;
	bool from_env = 0;

	ret = wizard_read_mac(enetaddr);
	if (!ret && is_valid_ethaddr(enetaddr)) {
		from_wizard = 1;
	} else if (!ret && !is_valid_ethaddr(enetaddr)) {
		printf("Valid MAC not found in configuration EEPROM!\n");
	} else if (ret) {
		printf("Error reading configuration EEPROM!\n");
		ret = eth_env_get_enetaddr("ethaddr", enetaddr);
		if (!ret && is_valid_ethaddr(enetaddr)) {
			printf("Valid MAC found in environment\n");
			from_env = 1;
		} else {
			printf("Valid MAC not found in environment!\n");
		}
	}

	if (from_wizard) {
		eth_env_set_enetaddr("ethaddr", enetaddr);
	} else if (!from_env) {
		return;
	}

	for (i = 0; i < N_MAC_ADDRS; i++) {
		snprintf(mac_str, sizeof(mac_str), "eth%daddr", i);
		eth_env_set_enetaddr(mac_str, enetaddr);
		increment_mac(enetaddr);
	}
	return;
}

static void increment_mac(uint8_t *mac)
{
	int i;

	for (i = 5; i >= 3; i--) {
		mac[i] += 1;
		if (mac[i])
			break;
	}
}
