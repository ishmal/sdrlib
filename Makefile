
################################################################
#
# Simple Makefile for SdrLib
#
################################################################

CC = gcc

CFLAGS = -g -O2 -Wall


LDFLAGS = -lc

vpath %.c src

SRC = \
sdrlib.c \
filter.c

OBJ = $(patsubst %.c,obj/%.o,$(SRC))

INC = -Isrc

all : obj/sdrlib.a

OBJ : | obj
obj: 
	@mkdir -p $@

obj/sdrlib.a : $(OBJ)
	$(AR) crv $@ $<

obj/%.o : %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@


clean:
	$(RM) -r obj





