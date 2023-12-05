#include <stdint.h>
#include <stddef.h>

#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"
#include "kernel/i2c-dev.h"

#define WII_CONTROLLER_ADDR 0x52

/* Sets register reg_addr to value on the device at i2c1 addr */
/* Returns 2 on success, negative on failure */
static int32_t i2c_set_reg(uint16_t addr, uint8_t reg_addr, uint8_t value) {
	uint8_t buf[2];
	buf[0] = reg_addr;
	buf[1] = value;
	return i2c_write(WII_CONTROLLER_ADDR, buf, 2);
}

/* Reads count bytes from address reg_addr from the i2c1 device at address addr */
/* Returns the number of bytes read, negative on error */
static int32_t i2c_get_regs(uint16_t addr, uint8_t reg_addr, uint8_t *buf, size_t count) {
	i2c_write(WII_CONTROLLER_ADDR, &reg_addr, 1);
	/* Give the controller a bit of time */
	usleep(200);
	return i2c_read(WII_CONTROLLER_ADDR, buf, count);
}

/* Please see https://wiibrew.org/wiki/Wiimote/Extension_Controllers for more information */
static int init_controller(void) {
	uint16_t id;

	/* Write first part of init sequence */
	if (i2c_set_reg(WII_CONTROLLER_ADDR, 0xF0, 0x55) != 2) {
		return -1;
	}

	usleep(2000); /* Give the controller some time */

	if (i2c_set_reg(WII_CONTROLLER_ADDR, 0xFB, 0x00) != 2) {
		return -2;
	}
	usleep(2000); /* Give the controller some time */

	/* Read the identification byte from the controller */
	if (i2c_get_regs(WII_CONTROLLER_ADDR, 0xFE, (uint8_t *)&id, 2) != 2) {
		return -3;
	}
	int result = -4;
	switch (id) {
		case 0x0000:
			printf("Nunchuk");
			break;
		case 0x0101:
			printf("Classic Controller");
			result = 0;
			break;
		case 0x0013:
			printf("Drawsome Graphics tablet");
			break;
		case 0x0103:
			printf("Guitar Hero/DJ Hero controller");
			break;
		case 0x0111:
			printf("Taiko no Tatsujin TaTaCon");
			break;
		case 0x0112:
			printf("uDraw GameTablet");
			break;
		case 0x0310:
			printf("Densha de GO! Shinkansen Controller");
			break;
		case 0x0402:
			printf("Wii Balance Board");
			break;
		case 0x0005:
		case 0x0405:
			printf("Wii Motion Plus");
			break;
		/* Unlikely these will be seen */
		case 0x0505:
			printf("Wii Motion Plus (Nunchuck passthru)");
			break;
		case 0x0705:
			printf("Wii Motion Plus (Classic Controller Passthru)");
			break;
		default:
			printf("No valid controller detected!");
			return 1;
	}
	printf(" detected\n");
	return result;
}

/*
 * Reads the registers from the device for the buttons
 * Register layout from https://wiibrew.org/wiki/Wiimote/Extension_Controllers/Classic_Controller_Pro
 *
 * |-----------------------------------------------------|
 * | Byte|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 * |-----------------------------------------------------|
 * |  0  |  RX<4:3>  |              LX<5:0>              |
 * |-----------------------------------------------------|
 * |  1  |  RX<2:1>  |              LY<5:0>              |
 * |-----------------------------------------------------|
 * |  2  |RX<0>|  LT<4:3>  |           RY<4:0>           |
 * |-----------------------------------------------------|
 * |  3  |     LT<2:0>     |           RT<4:0>           |
 * |-----------------------------------------------------|
 * |  4  | BDR | BDD | BLT |  -  |  H  |  +  | BRT |-----|
 * |-----------------------------------------------------|
 * |  5  | ZL  |  B  |  Y  |  A  |  X  |  ZR | BDL | BDU |
 * |-----------------------------------------------------|
 *
 * RX, RY, LX, LY are the analog sticks
 * RT LT are analog shoulder buttons (will read 0 or 31 on classic controller pro)
 * B{LT,RT} are the digital switch of shoulder buttons
 * DPAD buttons are BD{L|R|U|D}
 * Other buttons are the ABXY, plus, minus, and home, and the triggers
 */
#define BDR (1<<15)
#define BDD (1<<14)
#define BLT (1<<13)
#define BMI (1<<12)
#define BHO (1<<11)
#define BPL (1<<10)
#define BRT (1<<9)
/* bit 10 is unused */
#define BZL (1<<7)
#define BB (1<<6)
#define BY (1<<5)
#define BA (1<<4)
#define BX (1<<3)
#define BZR (1<<2)
#define BDL (1<<1)
#define BDU (1<<0)
/*
 * Returns negative on error, 0 on success
 */
int read_buttons(uint8_t buf[6]) {
	int results = i2c_get_regs(WII_CONTROLLER_ADDR, 0x00, buf, 6);
	if (results < 0) return results;
	else if (results != 6) return -3;
	return 0;
}

int main() {
	printf("Initializing Wii Extension Controller...\n");
	int result = init_controller();
	if (result < 0) {
		printf("Error configuring controller: %d\n", result);
		return -1;
	} else if (result) {
		printf("A controller was connected, but not a classic controller, exiting.\n");
		return -2;
	}

	uint16_t buttons_pressed = 0; // just ignore the analog sticks and triggers
	uint16_t buttons_pressed_prev = 0; // just ignore the analog sticks and triggers
	uint8_t buf[6]; // holds the raw data from the i2c read

	printf("Reading buttons: printing when a new one is pressed.\n");
	printf("Press home to exit\n");
	while (!(buttons_pressed & BHO)) {
		if ((result = read_buttons(buf))) {
			printf("Error reading from controller\n");
			return result;
		}
		buttons_pressed_prev = buttons_pressed; // save previous state
		buttons_pressed = ~((buf[4] << 8) | buf[5]); // stuff buttons into a single variable and invert

		// figure out buttons that were just pressed
		uint16_t tmp = (buttons_pressed ^ buttons_pressed_prev) & buttons_pressed;
		if (tmp) {
			// print the pressed button
			if (tmp & BDR)
				printf("> ");
			if (tmp & BDD)
				printf("V ");
			if (tmp & BDL)
				printf("< ");
			if (tmp & BDU)
				printf("^ ");

			if (tmp & BA)
				printf("A ");
			if (tmp & BB)
				printf("B ");
			if (tmp & BX)
				printf("X ");
			if (tmp & BY)
				printf("Y ");

			if (tmp & BRT)
				printf("RT ");
			if (tmp & BLT)
				printf("LT ");
			if (tmp & BZR)
				printf("ZL ");
			if (tmp & BZL)
				printf("ZR ");

			if (tmp & BPL)
				printf("+ ");
			if (tmp & BMI)
				printf("- ");
			printf("\n", tmp);
		}

		usleep(2000);
	}

	return 0;
}
