/* model.c -- Generic CRC parameter model routines
 * Copyright (C) 2014, 2016, 2017, 2020 Mark Adler
 * For conditions of distribution and use, see copyright notice in crcany.c.
 */

/*
* Edited by Hussain Al Marzooq
*/

#include <stdlib.h>
#include "model.h"

// See model.h.
model_t get_model(unsigned short width, word_t poly, word_t init, char refin, char refout, word_t xorout, word_t check) {
    model_t model;
    model.width = width;
    model.poly = poly;
    model.init = init;
    model.xorout = xorout;
    model.ref = refin;
    model.rev = refin ^ refout;
    model.check = check;

    if (refin)
        model.poly = reverse(model.poly, model.width);
    if (refout)
        model.init = reverse(model.init, model.width);

    model.init ^= model.xorout;
    model.table_byte = NULL;
    model.table_slice16 = NULL;
    model.table_byte_dbl = NULL;

    return model;
}

// See model.h.
model_t get_model_dbl(unsigned short width, word_t poly_hi, word_t poly, word_t init_hi, word_t init, char refin, char refout,
                      word_t xorout_hi, word_t xorout, word_t check_hi, word_t check) {
    model_t model;
    model.width = width;
    model.poly = poly;
    model.poly_hi = poly_hi;
    model.init = init;
    model.init_hi = init_hi;
    model.xorout = xorout;
    model.xorout_hi = xorout_hi;
    model.ref = refin;
    model.rev = refin ^ refout;
    model.check = check;
    model.check_hi = check_hi;

    if (refin) {
        dword_t dpoly = reverse_dbl((dword_t) {poly_hi, poly}, model.width);
        model.poly = dpoly.lo;
        model.poly_hi = dpoly.hi;
    }

    if (refout) {
        dword_t dinit = reverse_dbl((dword_t) {init_hi, init}, model.width);
        model.init = dinit.lo;
        model.init_hi = dinit.hi;
    }

    model.init ^= model.xorout;
    model.init_hi ^= model.xorout_hi;
    model.table_byte = NULL;
    model.table_slice16 = NULL;
    model.table_byte_dbl = NULL;

    return model;
}

// See model.h.
void free_model(model_t *model) {
    free(model->table_byte);
    free(model->table_slice16);
    free(model->table_byte_dbl);
}

// See model.h.
word_t reverse(word_t x, unsigned n) {
    if (n == 1)
        return x & 1;
    if (n == 2)
        return ((x >> 1) & 1) + ((x << 1) & 2);
    if (n <= 4) {
        x = ((x >> 2) & 3) + ((x << 2) & 0xc);
        x = ((x >> 1) & 5) + ((x << 1) & 0xa);
        return x >> (4 - n);
    }
    if (n <= 8) {
        x = ((x >> 4) & 0xf) + ((x << 4) & 0xf0);
        x = ((x >> 2) & 0x33) + ((x << 2) & 0xcc);
        x = ((x >> 1) & 0x55) + ((x << 1) & 0xaa);
        return x >> (8 - n);
    }
    if (n <= 16) {
        x = ((x >> 8) & 0xff) + ((x << 8) & 0xff00);
        x = ((x >> 4) & 0xf0f) + ((x << 4) & 0xf0f0);
        x = ((x >> 2) & 0x3333) + ((x << 2) & 0xcccc);
        x = ((x >> 1) & 0x5555) + ((x << 1) & 0xaaaa);
        return x >> (16 - n);
    }
#if WORDBITS >= 32
    if (n <= 32) {
        x = ((x >> 16) & 0xffff) + ((x << 16) & 0xffff0000);
        x = ((x >> 8) & 0xff00ff) + ((x << 8) & 0xff00ff00);
        x = ((x >> 4) & 0xf0f0f0f) + ((x << 4) & 0xf0f0f0f0);
        x = ((x >> 2) & 0x33333333) + ((x << 2) & 0xcccccccc);
        x = ((x >> 1) & 0x55555555) + ((x << 1) & 0xaaaaaaaa);
        return x >> (32 - n);
    }
#  if WORDBITS >= 64
    if (n <= 64) {
        x = ((x >> 32) & 0xffffffff) + ((x << 32) & 0xffffffff00000000);
        x = ((x >> 16) & 0xffff0000ffff) + ((x << 16) & 0xffff0000ffff0000);
        x = ((x >> 8) & 0xff00ff00ff00ff) + ((x << 8) & 0xff00ff00ff00ff00);
        x = ((x >> 4) & 0xf0f0f0f0f0f0f0f) + ((x << 4) & 0xf0f0f0f0f0f0f0f0);
        x = ((x >> 2) & 0x3333333333333333) + ((x << 2) & 0xcccccccccccccccc);
        x = ((x >> 1) & 0x5555555555555555) + ((x << 1) & 0xaaaaaaaaaaaaaaaa);
        return x >> (64 - n);
    }
#  endif
#endif
    return n < 2 * WORDBITS ? reverse(x, WORDBITS) << (n - WORDBITS) : 0;
}

// See model.h.
dword_t reverse_dbl(dword_t x, unsigned n) {
    if (n <= WORDBITS) {
        x.lo = reverse(x.lo, n);
        x.hi = 0;
    }
    else {
        word_t tmp = reverse(x.lo, WORDBITS);
        x.lo = reverse(x.hi, n - WORDBITS);
        if (n < WORDBITS * 2) {
            x.lo |= tmp << (n - WORDBITS);
            x.hi = tmp >> (WORDBITS * 2 - n);
        }
        else
            x.hi = tmp;
    }

    return x;
}