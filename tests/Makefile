# SPDX-License-Identifier: GPL-2.0
#
# Makefile for test01 - a test for monitoring thread starvation
#
CC	:= gcc
CFLAGS	:= -g -Wall -pthread
LIBS	:= -lpthread

all:  test01

test01:  test01.c
	$(CC) $(CFLAGS) -o test01 test01.c $(LIBS)

clean:
	@rm -f *.o *~ test01
