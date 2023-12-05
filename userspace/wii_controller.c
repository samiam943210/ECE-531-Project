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

	switch (id) {
		case 0x0000:
			printf("Nunchuk");
			break;
		case 0x0101:
			printf("Classic Controller");
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
	return 0;
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
 *
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

	int x = 5;
	int y = 2;
	int z = x % y;
	printf("%d %% %d = %d\n", x,y,z);

	for (int i = 0; i < 10; ++i) {
		printf("%d\n", rand());
	}

	return result;
}
