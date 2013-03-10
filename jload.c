/* Copyright 2012 Brian Swetland <swetland@frotz.net>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "jtag.h"

#define VIR_CTRL	0x0
#define VIR_ADDR	0x1
#define VIR_DATA	0x2

int main(int argc, char **argv) {
	unsigned bits;
	char buf[1024];
	FILE *fp;

	if (argc != 2) {
		fprintf(stderr,"usage: download hexfile\n");
		return -1;
	}

	if (jtag_open_virtual_device(0x00))
		return -1;

	fp = fopen(argv[1],"r");
	if (!fp) return -1;

	jtag_vir(VIR_CTRL);
	jtag_vdr(32, 1, 0);

	jtag_vir(VIR_ADDR);
	jtag_vdr(32, 0, 0);

	jtag_vir(VIR_DATA);
	while (fgets(buf, 1024, fp)) {
		if(buf[0] == '/') continue;
		if(isspace(buf[0])) continue;
		bits = strtoul(buf, 0, 16);
		jtag_vdr(32, bits, 0);
	}

	jtag_vir(VIR_CTRL);
	jtag_vdr(32, 0, 0);
	return 0;
}

