// Copyright 2012, Brian Swetland

#include <stdio.h>
#include "jtag.h"

int main(int argc, char **argv) {
	unsigned bits;

	if (jtag_open() < 0)
		return -1;

	if (jtag_reset() < 0)
		return -1;
	if (jtag_dr(32, 0, &bits) < 0)
		return -1;
	fprintf(stderr,"IDCODE: %08x\n", bits);

	if (jtag_open_virtual_device(0xffffffff))
		return -1;

	jtag_close();
	return 0;
}
