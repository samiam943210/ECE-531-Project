#include <stdint.h>
#include <stddef.h>

#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"
#include "kernel/i2c-dev.h"

void init_controller(void) {
	int32_t result;
	uint8_t buf[6];
	buf[0] = 0xF0;
	buf[1] = 0x55;
	result = i2c_write(0x52, buf, 2);
	printf("write 1 result is %d\n", result);
	// add a fat delay
	usleep(2000);

	buf[0] = 0xFB;
	buf[1] = 0x00;
	result = i2c_write(0x52, buf, 2);
	printf("write 2 result is %d\n", result);
	usleep(2000);

	// 0 out the buffer
	for (int i = 0; i < 6; ++i) {
		buf[i] = 0;
	}

	buf[0] = 0xFA;
	result = i2c_write(0x52, buf, 1);
	printf("write 3 result is %d\n", result);
	//
	int32_t x = i2c_read(0x52, buf, 6);

	printf("Read %d bytes\n", x);

	printf("Read bytes: %x %x %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	printf("Classic controller pro is 1 0 a4 20 1 1\n");
}

int main() {
	printf("Initializing wii classic controller pro\n");
	init_controller();
	return 0;
}