/*
 * This code attempts to be a generic i2c driver
 */

#include <stddef.h>
#include <stdint.h>

#include "lib/errors.h"
#include "drivers/i2c/i2c.h"
#include "drivers/i2c/bcm2835.h"

static struct i2c_core i2c;

int32_t i2c_init(int32_t type) {
	uint32_t result = 0;

	if (type == I2C_BCM2835) {
		result = bcm2835_i2c1_init(&i2c);
	}
	return 0;
}