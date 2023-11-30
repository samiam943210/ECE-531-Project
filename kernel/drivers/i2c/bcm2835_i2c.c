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
#include "drivers/i2c/bcm2835_i2c.h"

#include "lib/delay.h"
#include "lib/printk.h"

#define XFER_DONE(I2C) ((bcm2835_read(I2C##_S) & I2C_S_DONE))

/* Private function definitions */
static void bcm2835_i2c_enable_interrupts(void) {
	return; /* FIXME */
}
static int32_t bcm2835_i2c_interrupt_handler(void) {
	return 0; /* FIXME */
}

static void bcm2835_i2c1_gpio_setup(void) {
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

static void i2c1_setup_xfer(uint32_t addr, size_t count) {
	/*
	 * Transfers start with the following process:
	 * 1. Set address register
	 * 2. Clear flags from previous transfer
	 * 2. Set data length
	 * 3. Reset the FIFO
	 * 4. Set transfer direction
	 */
	uint32_t old;
	bcm2835_write(I2C1_A, addr & I2C_A_ADDR); /* Write the address */
	bcm2835_write(I2C1_S, I2C_S_ERR | I2C_S_CLKT | I2C_S_DONE); /* Clear previous flags */
	bcm2835_write(I2C1_DLEN, count); /* Write the data length */

	/* Clear the FIFO and previous direction */
	old = bcm2835_read(I2C1_C);
	old &= ~I2C_C_READ;
	old |= I2C_C_CLEAR;
	bcm2835_write(I2C1_C, old);
}

/* FIXME: this fails if count > 65535 */
static int32_t bcm2835_i2c1_read(const struct i2c_client *client, uint8_t *buf, size_t count) {
	uint32_t old;
	int32_t bytes_received = 0;

	i2c1_setup_xfer(client->address, count);

	/* Set direction and start the transfer */
	old = bcm2835_read(I2C1_C);
	old |= I2C_C_ST | I2C_C_READ;
	bcm2835_write(I2C1_C, old);
	delay(150);

	/* TODO: Use interrupts instead of this polling method */
	while (!XFER_DONE(I2C1)) { /* Block until done*/
		while(bytes_received < count && (bcm2835_read(I2C1_S) & I2C_S_RXD)) {
			*buf++ = bcm2835_read(I2C1_FIFO) & 0xff;
			bytes_received++;
		}
	}

	/* grab the remaining bytes from the fifo (if needed) */
	while (bytes_received < count && (bcm2835_read(I2C1_S) & I2C_S_RXD)) {
		*buf++ = bcm2835_read(I2C1_FIFO) & 0xff;
		bytes_received++;
	}

	/* Set the DONE flag */
	old = bcm2835_read(I2C1_S);
	old |= I2C_S_DONE;
	bcm2835_write(I2C1_S, old);

	/* Check for errors */
	if (old & I2C_S_ERR) {
		return -1; /* FIXME: put correct value here */
	} else if (old & I2C_S_CLKT) {
		return -2; /* FIXME: put correct value here */
	}

	return bytes_received;
}

/* FIXME: this fails if count > 65535 */
static int32_t bcm2835_i2c1_write(const struct i2c_client *client, const uint8_t *buf, size_t count) {
	uint32_t old;
	int32_t bytes_sent = 0;

	i2c1_setup_xfer(client->address, count);

	/* Set direction and start the transfer */
	old = bcm2835_read(I2C1_C);
	old |= I2C_C_ST;
	bcm2835_write(I2C1_C, old);
	delay(150);

	/* TODO: Use interrupts instead of this polling method */
	while (!XFER_DONE(I2C1)) { /* Loop while transfer is ongoing */
		while(bcm2835_read(I2C1_S) & I2C_S_TXW) { /* Loop while there is data to write */
			bcm2835_write(I2C1_FIFO, *buf++);
			++bytes_sent;
		}
	}

	/* Set the DONE flag */
	old = bcm2835_read(I2C1_S);
	old |= I2C_S_DONE;
	bcm2835_write(I2C1_S, old);

	/* Check for errors */
	if (old & I2C_S_ERR) {
		return -1; /* FIXME: put correct value here */
	} else if (old & I2C_S_CLKT) {
		return -1; /* FIXME: put correct value here */
	}

	return bytes_sent;
}

int32_t bcm2835_i2c1_init(struct i2c_core *i2c) {
	uint32_t old;

	/* Set up the function pointers */
	i2c->read = bcm2835_i2c1_read;
	i2c->write = bcm2835_i2c1_write;
	i2c->enable_interrupts = bcm2835_i2c_enable_interrupts;
	i2c->interrupt_handler = bcm2835_i2c_interrupt_handler;

	/* Disable the i2c */
	old = bcm2835_read(I2C1_C);
	old &= ~(I2C_C_I2CEN);
	bcm2835_write(I2C1_C, old);
	//bcm2835_write(I2C1_C, old & ~I2C_C_I2CEN);

	/* Configure i2c registers */
	/* For now, just disable interrupts */
	old &= ~(I2C_C_INTD | I2C_C_INTR | I2C_C_INTT);
	bcm2835_write(I2C1_C, old);

	/* Disable clock stretch timeout */
	bcm2835_write(I2C1_CLKT, 0);

	/* Enable i2c */
	old |= I2C_C_I2CEN;
	bcm2835_write(I2C1_C, old);

	/* Configure the GPIO pins */
	bcm2835_i2c1_gpio_setup();

	return 0;
}
