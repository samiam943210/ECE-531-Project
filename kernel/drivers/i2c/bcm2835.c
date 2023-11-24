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

#include "lib/delay.h"

/* Private function definitions */
static void bcm2835_i2c_enable_interrupts(void) {
	return; /* FIXME */
}

static int32_t bcm2835_i2c_interrupt_handler(void) {
	return 0; /* FIXME */
}

static int32_t bcm2835_i2c_gpio_setup(void) {
	uint32_t old;
	
	/* Setup GPIO pins 2 and 3 */
	gpio_request(2, "i2c1_sda");
	gpio_request(3, "i2c1_scl");

	/* Set GPIO2 and 3 to be I2C1 SDA and SCL (ALT0) */
	old = bcm2835_read(GPIO_GPFSEL0);
	old &= ~(077 << 6);
	old |= (GPIO_GPFSEL_ALT0 << 6) | (GPIO_GPFSEL_ALT0 << 9);
	bcm2835_write(GPIO_GPFSEL0, old);

	/* Enable the pull up on GPIO pins 2 and 3 */
	/* See the Peripheral Manual for more info */
	/* Configure pull up enable and delay for 150 cycles */
	bcm2835_write(GPIO_GPPUD, GPIO_GPPUD_PULLUP);
	delay(150);

	/* Pass the enable clock to GPIO pins 14 and delay */
	bcm2835_write(GPIO_GPPUDCLK0, (1 << 2) | (1 << 3));
	delay(150);

	/* Remove the control signal */
	bcm2835_write(GPIO_GPPUD, 0);

	/* Remove the clock */
	bcm2835_write(GPIO_GPPUDCLK0, 0);
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
