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
#define VIR_UART	0x3

int main(int argc, char **argv) {
	unsigned bits;

	if (jtag_open_virtual_device(0x00))
		return -1;

	jtag_vir(VIR_UART);

	for (;;) {
		jtag_vdr(9, 0, &bits);
		if (bits & 0x100) {
			bits &= 0xFF;
			if ((bits < ' ') || (bits > 127))
				fputc('.', stderr);
			else
				fputc(bits, stderr);
		}
	}

	return 0;
}

