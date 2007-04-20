/*
 * Copyright (C) 2007
 *       pancake <pancake@youterm.com>
 *
 * 0xFFFF is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * 0xFFFF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 0xFFFF; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <usb.h>

/* wait for status = 0 */
int get_status()
{
	int ret = 0;
	if (usb_control_msg(dev, 192, 1, 0, 0, (char *)&ret, 4, 2000) == -1) {
		fprintf(stderr, "Cannot get device status.\n");
		exit(1);
	}

	return ret;
}

int get_peripheral_host_mode(int foo)
{
	//unsigned short mode = 0;

	// XXX BROKEN
#if 0
	if (usb_control_msg(dev, 64, 16, 2, 0, 2, 0, 0, 2000)) {
		fprintf(stderr, "Cannot query device.\n");
		return -1;
	}

	printf("Device is in \"%s\" mode (%hd)\n", (mode)?"host":"peripheral", mode);
#endif

	return 0;
}
/*
 * boot the omap mobo
 */

int boot_board(char *cmdline)
{
	if (usb_control_msg(dev, CMD_WRITE, 130, 0, 0, cmdline, strlen(cmdline), 2000) == -1) {
		fprintf(stderr, "Cannot boot kernel.\n");
		perror("boot_board");
		return -1;
	}

	printf("Booting kernel with arguments: '%s'.\n", cmdline);

	return 0;
}

/*
 * reboot the omap mobo
 */
int reboot_board()
{
	// 131 = reboot
	if (usb_control_msg(dev, CMD_WRITE, 131, 0, 0, 0, 0, 2000) == -1) {
		fprintf(stderr, "Cannot reboot board.\n");
		return -1;
	}

	printf("Mobo rebooted!\n");

	return 0;
}

int set_rd_mode(unsigned short mode)
{
	if (((short)mode)==-1)
		return 1;

	if (usb_control_msg(dev, CMD_WRITE, 16, mode, 0, 0, 0, 2000) == -1) {
		fprintf(stderr, "Cannot set R&D flags.\n");
		return 1;
	}

	printf("rd mode changed to %s\n", mode?"on":"off");

	return 0;
}

// mode = 1 || 0
int set_usb_host_mode(unsigned short mode)
{
	if (usb_control_msg(dev, CMD_WRITE, 16, mode, 2, 0, 0, 2000) == -1) {
		fprintf(stderr, "Cannot change the usb-host mode.\n");
		return -1;
	}

	printf("USB host mode is %s.\n", mode?"enabled":"disabled");

	return 0;
}

/*
 * query root device
 */
int query_hw_revision()
{
	unsigned char string[512];
	int i = 0;

	if (usb_control_msg(dev, CMD_QUERY, 4, 0, 0, (char *)string, 512, 2000) == -1) {
		fprintf(stderr, "Cannot query hw revision.\n");
		return -1;
	}

	printf("HW revision string: '");
	for(i=0;i<44;i++) { // XXX ??
		if (string[i]==0) {
			printf(" ");
		} else {
			if (string[i]>19)
				printf("%c", string[i]);
		}
	}
	printf("'\n");

	return 0;
}

int query_rdmode_device()
{
	char isrd = 1;

	if (usb_control_msg(dev, CMD_QUERY, 17, 0, 0, (char *)&isrd, 1, 2000) == -1) {
		fprintf(stderr, "Cannot query rd mode.\n");
		return -1;
	}
	printf("RD mode is: %s\n", isrd?"on":"off");

	return isrd;
}

int query_root_device()
{
	unsigned char opcode;

	if (usb_control_msg(dev, 192, 17, 0, 1, (char *)&opcode, 1, 2000) < 0) {
		fprintf(stderr, "Cannot query root device\n");
		return -1;
	}

	if (opcode>2) { // use sizeof || enum
		printf("Invalid root device received from the device '%d'.\n", opcode);
	}

	printf("Root device is: %s\n", root_devices[opcode]);

	return 0;
}

int query_nolo_version()
{
	unsigned int version; // ensure uint is at least 32 bits

	//if (usb_control_msg(dev, 192, 3, 0, 1, (char *)&version, 4 , 2000) < 0) {
	if (usb_control_msg(dev, 192, 3, 0, 0, (char *)&version, 4 , 2000) < 0) {
		fprintf(stderr, "Cannot query nolo version. Old bootloader version?\n");
		exit(1);
	}

	printf("NOLO Version %d.%d.%d\n",
		version >> 20 & 15,
		version >> 16 & 15,
		version >> 8 & 255);

	if ((version & 255)> 1)
		printf("Invalid API version (%d)\n", version&255);

	return 0;
}
