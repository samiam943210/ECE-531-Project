/*
 * Syscall numbers and interface for the i2c driver
 */

#ifndef _I2C_DEV_H
#define _I2C_DEV_H

#include <stdint.h>
#include <stddef.h>

#define __NR_i2c_read	8202
#define __NR_i2c_write	8203

int32_t i2c_read(uint16_t address, void *buf, size_t count);
int32_t i2c_write(uint16_t address, const void *buf, size_t count);

#endif /* _I2C_DEV_H */