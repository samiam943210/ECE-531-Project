#ifndef _KERNEL_I2C_H
#define _KERNEL_I2C_H

#include <stdint.h>
#include <stddef.h>

#define I2C_BCM2835 0

struct i2c_client {
	uint32_t address; /* Stored in lower bits */
	uint32_t flags; /* Holds things like 10 bit address, etc. */
};

struct i2c_core {
	uint32_t initialized;
	void (*enable_interrupts)(void); /* May not be used */
	int32_t (*interrupt_handler)(void); /* May not be used */
	int32_t (*read)(const struct i2c_client *client, uint8_t *buf, size_t count);
	int32_t (*write)(const struct i2c_client *client, const uint8_t *buf, size_t count);
};

/* Not sure if these function definitions should be here or in i2c-dev.h */

int32_t i2c_init(int32_t type);
void i2c_enable_interrupts(void);
int32_t i2c_read(int fd, uint8_t *buf, size_t count); /* fd will probably be changed */
int32_t i2c_write(int fd, const uint8_t *buf, size_t count); /* fd will probably be changed */
int32_t i2c_ioctl(int fd, int32_t request, ...); /* fd will probably be changed */

#endif /* _KERNEL_I2C_H */