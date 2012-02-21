
CFLAGS := -g -Wall
LIBS := -lusb-1.0

all: jload jinfo

jinfo.c: jtag.h
jload.c: jtag.h
jtag.c: jtag.h
jtag-virtual.c: jtag.h

JLOAD_OBJS := jload.o jtag-virtual.o jtag.o
jload: $(JLOAD_OBJS)
	$(CC) -o jload $(JLOAD_OBJS) $(LIBS)

JINFO_OBJS := jinfo.o jtag-virtual.o jtag.o
jinfo: $(JINFO_OBJS)
	$(CC) -o jinfo $(JINFO_OBJS) $(LIBS)

clean::
	rm -f jload jinfo *.o
