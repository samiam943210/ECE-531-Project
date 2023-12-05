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
	if (id == 0)
		return 1;
	return id != 0x0101;
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

int32_t wii_controller_prompt(const char *msg, int32_t starting_val, int32_t delta, int32_t minimum) {
	int32_t cash_money = starting_val;
	int32_t col = strlen(msg)+1;
	int result;
	printf("\033[?25l%s%d", msg, cash_money);

	uint16_t buttons_pressed = 0; // just ignore the analog sticks and triggers
	uint8_t buf[6]; // holds the raw data from the i2c read

	uint8_t consecutive = 0;

	while (!(buttons_pressed & BA)) { // loop until A pressed
		if ((result = read_buttons(buf))) {
			printf("\rError reading from controller\033[0\n\033[?25h");
			return INT32_MIN;
		}

		buttons_pressed = ~((buf[4] << 8) | buf[5]); // stuff buttons into a single variable and invert

		// figure out buttons that were just pressed
		if (buttons_pressed & (BDU | BDD)) { // only look at up and down
			if (!consecutive && (buttons_pressed & BDD) && (cash_money - delta) >= minimum) {
				cash_money -= delta;
			}
			if (!consecutive && (buttons_pressed & BDU)) {
				cash_money += delta;
			}
			consecutive = (consecutive + 1) & 0x07;
			printf("\033[%dG%d\033[0K", col, cash_money);
		} else {
			consecutive = 0; // reset the counter
		}

		usleep(2000); // a delay to slow down the program a bit
	}
	printf("\n\033[?25h");
	return cash_money;
}

int main() {
	int result = init_controller();
	if (result < 0) {
		printf("Error configuring controller: %d\n", result);
		return -1;
	} else if (result) {
		printf("A controller was connected, but not a classic controller, exiting.\n");
		return -2;
	}

	printf("Press up or down to change the value or press A to confirm.\n");
	int32_t cash_money = wii_controller_prompt("Choose bet amount: $", 10,10,0);

	printf("Player betted $%d\n", cash_money);

	return 0;
}
