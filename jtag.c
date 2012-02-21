// Copyright 2012, Brian Swetland

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <libusb-1.0/libusb.h>

#define TRACE_USB	0
#define TRACE_JTAG	0

static struct libusb_device_handle *udev;
static int usb_open(unsigned vid, unsigned pid) {
	if (libusb_init(NULL) < 0)
		return -1;

	if (!(udev = libusb_open_device_with_vid_pid(NULL, vid, pid))) {
		fprintf(stderr,"cannot find device\n");
		return -1;
	}

	if (libusb_claim_interface(udev, 0) < 0) {
		fprintf(stderr,"cannot claim interface\n");
		return -1;
	}
	return 0;
}
static void usb_close(void) {
	libusb_exit(NULL);
}
#if TRACE_USB
static void dump(char *prefix, void *data, int len) {
	unsigned char *x = data;
	fprintf(stderr,"%s: (%d)", prefix, len);
	while (len > 0) {
		fprintf(stderr," %02x", *x++);
		len--;
	}
	fprintf(stderr,"\n");
}
#endif
static int usb_bulk(unsigned char ep, void *data, int len, unsigned timeout) {
	int r, xfer;
#if TRACE_USB
	if (!(ep & 0x80))
		dump("xmit", data, len);
#endif
	r = libusb_bulk_transfer(udev, ep, data, len, &xfer, timeout);
	if (r < 0) {
		fprintf(stderr,"bulk: error: %d\n", r);
		return r;
	}
#if TRACE_USB
	if (ep & 0x80)
		dump("recv", data, xfer);
#endif
	return xfer;
}

#define EP1_IN	0x81
#define EP2_OUT	0x02

#define UB_BYTEMODE	0x80
#define UB_BITMODE	0x00
#define UB_READBACK	0x40

/* bits in bit mode */
#define UB_OE		0x20
#define UB_TDI		0x10
#define UB_nCS		0x08
#define UB_nCE		0x04
#define UB_TMS		0x02
#define UB_TCK		0x01

/* bytecount for data bytes that follow in byte mode */
#define UB_COUNT(n)	((n) & 0x3F)

int jtag_move(int count, unsigned bits){
	unsigned char buf[64];
	int n = 0;
#if TRACE_JTAG
	fprintf(stderr,"move: %08x (%d)\n", bits, count);
#endif
	while (count-- > 0) {
		if (bits & 1) {
			buf[n++] = UB_TMS;
			buf[n++] = UB_TMS | UB_TCK;
		} else {
			buf[n++] = 0;
			buf[n++] = UB_TCK;
		}
		bits >>= 1;
	}
	return usb_bulk(EP2_OUT, buf, n, 1000);
}

int jtag_shift(int count, unsigned bits, unsigned *out) {
	unsigned char buf[64];
	unsigned RB = out ? UB_READBACK : 0;
	int n = 0;
	int readcount = count;
	int r,bit;
#if TRACE_JTAG
	fprintf(stderr,"xfer: %08x (%d)\n", bits, count);
#endif
	while (count-- > 0) {
		if (bits & 1) {
			buf[n++] = UB_TDI;
			buf[n++] = UB_TDI | UB_TCK | RB;
		} else {
			buf[n++] = 0;
			buf[n++] = UB_TCK | RB;
		}
		bits >>= 1;
	}
	buf[n-1] |= UB_TMS;
	buf[n-2] |= UB_TMS;
	r = usb_bulk(EP2_OUT, buf, n, 1000);
	if (r < 0)
		return r;
	if (!out)
		return 0;
	bits = 0;
	bit = 1;
	while (readcount > 0) {
		r = usb_bulk(EP1_IN, buf, 64, 1000);
		if (r < 0)
			return r;
		if (r < 3)
			continue;
		for (n = 2; n < r; n++) {
			if (buf[n] & 1)
				bits |= bit;
			bit <<= 1;
			readcount--;
			if (readcount == 0) {
#if TRACE_JTAG
				fprintf(stderr,"    : %08x\n", bits);
#endif
				*out = bits;
				return 0;
			}
		}
	}
	return -1;
}

/* JTAG notes
 *
 * TMS is sampled on +TCK
 * Capture-XR state loads shift register on +TCK as state is exited
 * Shift-XR state TDO goes active (containing shiftr[0]) on the first -TCK
 *          after entry, shifts occur on each +TCK, *including* the +TCK
 *          that will exist Shift-XR when TMS=1 again
 * Update-XR update occurs on the -TCK after entry to state
 * 
 * Any -> Reset: 11111
 * Any -> Reset -> RTI: 111110
 * RTI -> ShiftDR: 100
 * ShiftDR shifting: 0 x N
 * ShiftDR -> UpdateDR -> RTI: 110
 * ShiftDR -> UpdateDR -> ShiftDR: 11100
 * RTI -> ShiftIR: 1100
 * ShiftIR shifting: 0 x N
 * ShiftIR -> UpdateIR -> RTI: 110
 */

#define RESET	8,0b01111111
#define SHIFTDR	3,0b001
#define SHIFTIR	4,0b0011
#define DONE	2,0b01
#define AGAIN	4,0b0011

int jtag_ir(unsigned sz, unsigned bits) {
	int r;
	if ((r = jtag_move(SHIFTIR)) < 0) return r;
	if ((r = jtag_shift(sz, bits, 0)) < 0) return r;
	if ((r = jtag_move(DONE)) < 0) return r;
	return 0;
}

int jtag_dr(unsigned sz, unsigned bits, unsigned *out) {
	int r;
	if ((r = jtag_move(SHIFTDR)) < 0) return r;
	if ((r = jtag_shift(sz, bits, out)) < 0) return r;
	if ((r = jtag_move(DONE)) < 0) return r;
	return 0;
}

static int jtag_is_open = 0;

int jtag_open(void) {
	int r;
	if (!jtag_is_open) {
		r = usb_open(0x09fb, 0x6001);
		if (r < 0)
			return r;
		jtag_is_open = 1;
	}
	return 0;
}
int jtag_close(void) {
	if (jtag_is_open) {
		usb_close();
		jtag_is_open = 0;
	}
	return 0;
}
int jtag_reset(void) {
	return jtag_move(RESET);
}

