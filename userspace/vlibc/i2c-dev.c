#include <stdint.h>
#include <stddef.h>

#include "kernel/i2c-dev.h"

int32_t i2c_read(uint16_t address, void *buf, size_t count) {

	register long r7 __asm__("r7") = __NR_i2c_read;
	register long r0 __asm__("r0") = address;
	register long r1 __asm__("r1") = (long)buf;
	register long r2 __asm__("r2") = count;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2)
		: "memory");

	return r0;
}

int32_t i2c_write(uint16_t address, const void *buf, size_t count) {

	register long r7 __asm__("r7") = __NR_i2c_write;
	register long r0 __asm__("r0") = address;
	register long r1 __asm__("r1") = (long)buf;
	register long r2 __asm__("r2") = count;

	asm volatile(
		"svc #0\n"
		: "=r"(r0)
		: "r"(r7), "0"(r0), "r"(r1), "r"(r2)
		: "memory");

	return r0;
}