#ifndef _I2C_BCM283_H
#define _I2C_BCM283_H

#include <stdint.h>
#include "drivers/i2c/i2c.h"

int32_t bcm2835_i2c1_init(struct i2c_core *i2c);

#endif /* _I2C_BCM283_H */