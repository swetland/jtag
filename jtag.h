// Copyright 2012, Brian Swetland

#ifndef _JTAG_H_
#define _JTAG_H_

int jtag_open(void);
int jtag_close(void);

/* move into RESET state */
int jtag_reset(void);

/* clock count times, TDI=0, TMS=bits[0], bits >>= 1 */
int jtag_move(int count, unsigned bits);

/* clock count-1 times, TMS=0, TDI=bits[0], bits >>= 1
 * clock 1 time, TMS=1, TDI=bits[0]
 * if out, capture TDO into out
 */
int jtag_shift(int count, unsigned bits, unsigned *out);


/* load sz bits into IR */
int jtag_ir(unsigned sz, unsigned bits);

/* load sz bits into DR, capture sz bits into out if non-null */
int jtag_dr(unsigned sz, unsigned bits, unsigned *out);


/* altera virtual jtag support */
int jtag_open_virtual_device(unsigned iid);
int jtag_vir(unsigned vir);
int jtag_vdr(unsigned sz, unsigned bits, unsigned *out);

#endif
