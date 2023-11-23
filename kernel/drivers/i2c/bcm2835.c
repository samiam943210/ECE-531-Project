/*
 * This code drives the BCM2835 BSC I2C.
 * As described in chapter 3 of the BCM2835 ARM Peripherals Manual
 */

#include <stdint.h>
#include <stddef.h>

#include "drivers/gpio/gpio.h" /* Needed? */
#include "drivers/bcm2835/bcm2835_io.h"
#include "drivers/bcm2835/bcm2835_periph.h"
#include "drivers/i2c/i2c.h"

/* Private function definitions */
static void bcm2835_i2c_enable_interrupts(void) {
	return; /* FIXME */
}

static int32_t bcm2835_i2c_interrupt_handler(void) {
	return 0; /* FIXME */
}

static int32_t bcm2835_i2c_gpio_setup(void) {
	return 0;
}

static int32_t bcm2835_i2c_read(struct i2c_client *client, uint8_t *buf, size_t count) {
	return 0;
}
static int32_t bcm2835_i2c_write(struct i2c_client *client, uint8_t *buf, size_t count) {
	return 0;
}

int32_t bcm2835_i2c_init(struct i2c_core *i2c) {
	/* Setup the function pointers */
	i2c->read = bcm2835_i2c_read;
	i2c->write = bcm2835_i2c_write;
	i2c->enable_interrupts = bcm2835_i2c_enable_interrupts;
	i2c->interrupt_handler = bcm2835_i2c_interrupt_handler;

	/* Disable the i2c */

	/* Configure the needed GPIO pins */
	bcm2835_i2c_gpio_setup();

	/* Configure i2c registers */

	/* Enable i2c */
	return 0;
}
