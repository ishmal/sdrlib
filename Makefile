
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
sdrlib.c \
audio.c  \
fft.c    \
filter.c \
private.c

OBJ = $(patsubst %.c,obj/%.o,$(SRC))

INC = -Isrc

all : testme

testme: test/testme.c obj/libsdr.a
	$(CC) $(CFLAGS) $(INC) test/testme.c $(LDFLAGS) -o $@

obj/libsdr.a : $(OBJ)
	$(AR) rv $@ $(OBJ)

obj/%.o : %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@


clean:
	$(RM) testme
	$(RM) -r obj





