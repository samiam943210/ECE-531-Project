/*
 * This code attempts to be a generic i2c driver
 */

#include <stddef.h>
#include <stdint.h>

#include "lib/errors.h"
#include "drivers/i2c/i2c.h"
#include "drivers/i2c/bcm2835_i2c.h"

#include "lib/errors.h"

#define DEBUG

#ifdef DEBUG
#include "lib/delay.h"
#include "lib/printk.h"
#endif


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

	/* If debugging set, initialize the controller and read the ID */
	#ifdef DEBUG
	uint8_t buf[6];
	buf[0] = 0xF0;
	buf[1] = 0x55;
	result = i2c_write(0x52, buf, 2);
	printk("write 1 result is %d\n", result);
	// add a fat delay
	delay(3000000);

	buf[0] = 0xFB;
	buf[1] = 0x00;
	result = i2c_write(0x52, buf, 2);
	printk("write 2 result is %d\n", result);
	delay(3000000);

	// 0 out the buffer
	for (int i = 0; i < 6; ++i) {
		buf[i] = 0;
	}

	buf[0] = 0xFA;
	result = i2c_write(0x52, buf, 1);
	printk("write 3 result is %d\n", result);
	//
	int32_t x = i2c_read(0x52, buf, 6);

	printk("Read %d bytes\n", x);

	printk("Read bytes: %x %x %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	printk("Classic controller pro is 1 0 a4 20 1 1\n");
	#endif

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
		.address = fd,
		.flags = 0
	};

	return i2c.write(&client, buf, count);
}

int32_t i2c_ioctl(int fd, int32_t request, ...) {
	return 0;
}