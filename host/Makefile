# Builds renderer code on the Mac (no HAL, no board needed)
#   make -C host test

CC         = cc
CFLAGS     = -std=c11 -O2 -Wall -Wextra -I../Core/Inc
SRC_FX     = ../Core/Src/fixed.c ../Core/Src/fixed_sin_table.c
SRC_VEC    = $(SRC_FX) ../Core/Src/vec.c ../Core/Src/mat.c
SRC_RASTER = $(SRC_FX) ../Core/Src/gfx.c ../Core/Src/raster.c
HDR        = $(wildcard ../Core/Inc/*.h)

test_fixed: test_fixed.c $(SRC_FX) $(HDR)
	$(CC) $(CFLAGS) -o $@ test_fixed.c $(SRC_FX) -lm

test_vec: test_vec.c $(SRC_VEC) $(HDR)
	$(CC) $(CFLAGS) -o $@ test_vec.c $(SRC_VEC) -lm

test_raster: test_raster.c $(SRC_RASTER) $(HDR)
	$(CC) $(CFLAGS) -o $@ test_raster.c $(SRC_RASTER) -lm

test: test_fixed test_vec test_raster
	./test_fixed
	./test_vec
	./test_raster

clean:
	rm -f test_fixed test_vec test_raster

.PHONY: test clean
