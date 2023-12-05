#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "syscalls.h"
#include "vmwos.h"
#include "vlibc.h"

static uint32_t s = 0xDA1CE2A9; // seed

void srand(uint16_t seed)
{
	// set the lower 16 bits of s to seed (and make sure it's odd)
	s = (s & 0xffff0000) | seed | 1;
	rand(); // generate a few random numbers
	rand();
	rand();
	rand();
}

uint16_t rand(void)
{
	static uint32_t x; // Previous random number
	static uint32_t w; // Weyl Sequence
	x *= x; // square number
	x += w += s; // apply the Weyl sequence to x
	return (x = (x >> 16) | (x << 16)); // rotate x
}