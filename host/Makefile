# Builds renderer code on the Mac (no HAL, no board needed)
#   make -C host test

CC      = cc
CFLAGS  = -std=c11 -O2 -Wall -Wextra -I../Core/Inc
SRC_FX  = ../Core/Src/fixed.c ../Core/Src/fixed_sin_table.c

test_fixed: test_fixed.c $(SRC_FX) ../Core/Inc/fixed.h
	$(CC) $(CFLAGS) -o $@ test_fixed.c $(SRC_FX) -lm

test: test_fixed
	./test_fixed

clean:
	rm -f test_fixed

.PHONY: test clean
