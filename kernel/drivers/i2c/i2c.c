/*
 * This code attempts to be a generic i2c driver
 */

#include <stddef.h>
#include <stdint.h>

#include "lib/errors.h"
#include "drivers/i2c/i2c.h"
#include "drivers/i2c/bcm2835_i2c.h"

#include "lib/errors.h"

static struct i2c_core i2c;

int32_t i2c_init(int32_t type) {
	uint32_t result = 1;

	if (type == I2C_BCM2835) {
		result = bcm2835_i2c1_init(&i2c);
	}

	if (result != 0) { // some error occurred
		return -ENODEV;
	}

	i2c.initialized = 1;


	return 0;
}

void i2c_enable_interrupts(void) {
	return;
}

int32_t i2c_read(uint16_t address, uint8_t *buf, size_t count) {
	struct i2c_client client = {
		.address = address,
		.flags = 0
	};

	return i2c.read(&client, buf, count);
}

int32_t i2c_write(uint16_t address, const uint8_t *buf, size_t count) {
	struct i2c_client client = {
		.address = address,
		.flags = 0
	};

	return i2c.write(&client, buf, count);
}

/* TODO: Make a /dev/ entry for this and be a driver like in Linux */
//int32_t i2c_ioctl(int fd, int32_t request, ...); /* fd will probably be changed */