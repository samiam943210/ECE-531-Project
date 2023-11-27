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

int32_t i2c_read(int fd, uint8_t *buf, size_t count) {
	/* Let's cheat a little bit and have fd simply be the address */
	struct i2c_client client = {
		.address = (short)fd,
		.flags = 0
	};

	return i2c.read(&client, buf, count);
}

int32_t i2c_write(int fd, const uint8_t *buf, size_t count) {
		/* Let's cheat a little bit and have fd simply be the address */
	struct i2c_client client = {
		.address = (short)fd,
		.flags = 0
	};

	return i2c.write(&client, buf, count);
}

int32_t i2c_ioctl(int fd, int32_t request, ...) {
	return 0;
}