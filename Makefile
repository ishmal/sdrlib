
################################################################
#
# Simple Makefile for SdrLib
#
################################################################





CC = gcc -arch x86_64



CFLAGS = -g -O2 -Wall


LDFLAGS = -Lobj -lsdr -lportaudio -lfftw3 -lm -lc

vpath %.c src

SRC = \
sdrlib.c  \
audio.c   \
device.c  \
fft.c     \
filter.c  \
plugin.c  \
private.c \
util.c

OBJ = $(patsubst %.c,obj/%.o,$(SRC))

INC = -Isrc

DEVICES = device/device-rtl.so

all : testme $(DEVICES)

testme: test/testme.c obj/libsdr.a
	$(CC) $(CFLAGS) $(INC) test/testme.c $(LDFLAGS) -o $@

$(OBJ) : | obj

obj :
	@mkdir -p $@

obj/libsdr.a : $(OBJ)
	$(AR) rv $@ $(OBJ)

obj/%.o : %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@








clean:
	$(RM) testme
	$(RM) -r obj plugin/*




################################################################
# DEVICES
################################################################


device/device-rtl.so:  devicesrc/device-rtl.c
	$(CC) -dynamiclib $(CFLAGS) $(INC) $< -o $@ -lrtlsdr



