
################################################################
#
# Simple Makefile for SdrLib
#
################################################################



RTL = ../rtlmingw/rtl-sdr

#CC = gcc -arch x86_64
CC = gcc



#CFLAGS = -g -O2 -Wall
CFLAGS = -g -O2 -Wall -mwindows

#INC = -I. -Isrc
INC = -I. -Isrc -I$(RTL)/include

#LDFLAGS = -L. -Lobj -lsdr -lportaudio -lfftw3 -lpthread -lm
LDFLAGS = -L. -Lobj -lsdr -lportaudio -lfftw3-3 -lpthread -lwinmm -lm

vpath %.c src

SRC = \
sdrlib.c  \
audio.c   \
demod.c   \
device.c  \
fft.c     \
filter.c  \
impl.c    \
private.c \
util.c

OBJ = $(patsubst %.c,obj/%.o,$(SRC))

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
	$(RM) -r obj device/*




################################################################
# DEVICES
################################################################


device/device-rtl.so:  devicesrc/device-rtl.c
	$(CC) -shared $(CFLAGS) $(INC) $< -o $@ -L$(RTL)/.. -lrtlsdr



