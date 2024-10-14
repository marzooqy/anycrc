/* crc.c -- Generic CRC calculations
 * Copyright (C) 2014, 2016, 2017, 2020, 2021 Mark Adler
 * For conditions of distribution and use, see copyright notice in crcany.c.
 */

/*
* Edited by Hussain Al Marzooq
*/

#include <stdlib.h>
#include <assert.h>
#include "crc.h"

word_t crc_bitwise(model_t *model, word_t crc, void const *dat, size_t len) {
    unsigned char const *buf = dat;
    word_t poly = model->poly;

    // Pre-process the CRC.
    crc ^= model->xorout;
    if (model->rev)
        crc = reverse(crc, model->width);

    // Process the input data a bit at a time.
    if (model->ref) {
        while (len >= 8) {
            crc ^= *buf++;
            for (int k = 0; k < 8; k++)
                crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
            len -= 8;
        }

        if (len > 0) {
            unsigned char off = (unsigned char)-1 >> (8 - len);
            crc ^= *buf & off;
            while (len--)
                crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
        }
    }
    else {
        word_t mask = (word_t) 1 << (WORDBITS - 1);
        unsigned top = WORDBITS - model->width;
        unsigned shift = WORDBITS - 8;
        poly <<= top;
        crc <<= top;

        while (len >= 8) {
            crc ^= (word_t)(*buf++) << shift;
            for (int k = 0; k < 8; k++)
                crc = crc & mask ? (crc << 1) ^ poly : crc << 1;
            len -= 8;
        }

        // Clear the remaining bits so that they won't affect the CRC calculation.
        if (len > 0) {
            unsigned char off = (unsigned char)-1 << (8 - len);
            crc ^= (word_t)(*buf & off) << shift;
            while (len--)
                crc = crc & mask ? (crc << 1) ^ poly : crc << 1;
        }

        crc >>= top;
    }

    // Post-process and return the CRC.
    if (model->rev)
        crc = reverse(crc, model->width);
    return crc ^ model->xorout;
}

void crc_table_bytewise(model_t *model) {
    unsigned char k = 0;
    do {
        word_t crc = crc_bitwise(model, 0, &k, 8);
        if (model->rev)
            crc = reverse(crc, model->width);
        if (!model->ref)
            crc <<= WORDBITS - model->width;
        model->table[k] = crc;
    } while (++k);
}

word_t crc_bytewise(model_t *model, word_t crc, void const *dat, size_t len) {
    unsigned char const *buf = dat;

    if (len >= 8) {
        // Pre-process the CRC.
        if (model->rev)
            crc = reverse(crc, model->width);

        // Process the input data a byte at a time.
        if (model->ref) {
            while (len >= 8) {
                crc = (crc >> 8) ^ model->table[(crc ^ *buf++) & 0xff];
                len -= 8;
            }
        }
        else {
            unsigned top = WORDBITS - model->width;
            unsigned shift = WORDBITS - 8;
            crc <<= top;
            while (len >= 8) {
                crc = (crc << 8) ^ model->table[((crc >> shift) ^ *buf++) & 0xff];
                len -= 8;
            }
            crc >>= top;
        }

        // Post-process and return the CRC
        if (model->rev)
            crc = reverse(crc, model->width);
    }

    // Process any remaining bits after the last byte
    if (len > 0)
        crc = crc_bitwise(model, crc, buf, len);
    return crc;
}

void crc_table_slice16(model_t *model) {
    word_t xorout = model->xorout;
    if (!model->ref)
        xorout <<= WORDBITS - model->width;

    for (unsigned k = 0; k < 256; k++) {
        word_t crc = model->table[k];

        for (unsigned n = 1; n < 16; n++) {
            crc ^= xorout;
            if (model->ref)
                crc = (crc >> 8) ^ model->table[crc & 0xff];
            else
                crc = (crc << 8) ^ model->table[(crc >> (WORDBITS - 8)) & 0xff];
            crc ^= xorout;
            model->table[(n << 8) | k] = crc;
        }
    }
}

#define SLICE_BYTE_REF(idx) model->table[((15 - idx) * 256) | buf[idx]]
#define SLICE_CRC_REF(idx) model->table[((15 - idx) * 256) | (((crc >> (idx * 8)) ^ buf[idx]) & 0xff)]
#define SLICE_BYTE(idx) model->table[(idx * 256) | buf[15 - idx]]
#define SLICE_CRC(idx) model->table[(idx * 256) | (((crc >> ((idx & 0x7) * 8)) ^ buf[15 - idx]) & 0xff)]

word_t crc_slice16(model_t *model, word_t crc, void const *dat, size_t len) {
    unsigned char const *buf = dat;

    // Process as many 16 byte blocks as are available
    if (len >= 16 * 8) {
        // Pre-process the CRC
        if (model->rev)
            crc = reverse(crc, model->width);

        if (model->ref) {
            do {
                crc = SLICE_CRC_REF(0) ^ SLICE_CRC_REF(1) ^ SLICE_CRC_REF(2) ^ SLICE_CRC_REF(3)
                    ^ SLICE_CRC_REF(4) ^ SLICE_CRC_REF(5) ^ SLICE_CRC_REF(6) ^ SLICE_CRC_REF(7)
                    ^ SLICE_BYTE_REF(8) ^ SLICE_BYTE_REF(9) ^ SLICE_BYTE_REF(10) ^ SLICE_BYTE_REF(11)
                    ^ SLICE_BYTE_REF(12) ^ SLICE_BYTE_REF(13) ^ SLICE_BYTE_REF(14) ^ SLICE_BYTE_REF(15);

                buf += 16;
                len -= 16 * 8;
            } while (len >= 16 * 8);
        }
        else {
            unsigned top = WORDBITS - model->width;
            crc <<= top;
            do {
                crc = SLICE_BYTE(0) ^ SLICE_BYTE(1) ^ SLICE_BYTE(2) ^ SLICE_BYTE(3)
                    ^ SLICE_BYTE(4) ^ SLICE_BYTE(5) ^ SLICE_BYTE(6) ^ SLICE_BYTE(7)
                    ^ SLICE_CRC(8) ^ SLICE_CRC(9) ^ SLICE_CRC(10) ^ SLICE_CRC(11)
                    ^ SLICE_CRC(12) ^ SLICE_CRC(13) ^ SLICE_CRC(14) ^ SLICE_CRC(15);

                buf += 16;
                len -= 16 * 8;
            } while (len >= 16 * 8);
            crc >>= top;
        }

        // Post-process
        if (model->rev)
            crc = reverse(crc, model->width);
    }

    // Process any remaining bytes after the last 16 byte block
    if (len > 0)
        crc = crc_bytewise(model, crc, buf, len);
    return crc;
}

// Return a(x) multiplied by b(x) modulo p(x), where p(x) is the CRC
// polynomial. For speed, this requires that a not be zero.
static word_t multmodp(model_t *model, word_t a, word_t b) {
    word_t top = (word_t)1 << (model->width - 1);
    word_t prod = 0;
    if (model->ref) {
        // Reflected polynomial.
        for (;;) {
            if (a & top) {
                prod ^= b;
                if ((a & (top - 1)) == 0)
                    break;
            }
            a <<= 1;
            b = b & 1 ? (b >> 1) ^ model->poly : b >> 1;
        }
    }
    else {
        // Normal polynomial.
        for (;;) {
            if (a & 1) {
                prod ^= b;
                if (a == 1)
                    break;
            }
            a >>= 1;
            b = b & top ? (b << 1) ^ model->poly : b << 1;
        }
        prod &= ((top << 1) - 1);
    }
    return prod;
}

// Build table_comb[] for model. Stop when a cycle is detected, or the table is
// full. On return, model->cycle is the number of entries in the table, which
// is the index at which to cycle. model->back is the index to go to when
// model->cycle is reached. If no cycle was detected, then model->back is -1.
void crc_table_combine(model_t *model) {
    // Keep squaring x^1 modulo p(x), where p(x) is the CRC polynomial, to
    // generate x^2^n modulo p(x).
    word_t sq = model->ref ? (word_t)1 << (model->width - 2) : 2;   // x^1
    model->table_comb[0] = sq;
    int n = 1;
    while (n < 67) {
        sq = multmodp(model, sq, sq); // x^2^n

        // If this value has already appeared, then done.
        for (int j = 0; j < n; j++)
            if (model->table_comb[j] == sq) {
                model->cycle = n;
                model->back = j;
                return;
            }

        // New value -- append to table.
        model->table_comb[n++] = sq;
    }

    // No cycle was found, up to the size of the table.
    model->cycle = n;
    model->back = -1;
}

// Return x^n modulo p(x), where p(x) is the CRC polynomial. model->cycle
// and model->table_comb[] must first be initialized by crc_table_combine().
static word_t xnmodp(model_t *model, uintmax_t n) {
    word_t xp = model->ref ? (word_t)1 << (model->width - 1) : 1;   // x^0
    int k = 0;
    for (;;) {
        if (n & 1)
            xp = multmodp(model, model->table_comb[k], xp);
        n >>= 1;
        if (n == 0)
            break;
        if (++k == model->cycle) {
            assert(model->back != -1);
            k = model->back;
        }
    }
    return xp;
}

word_t crc_combine(model_t *model, word_t crc1, word_t crc2, uintmax_t len2) {
    crc1 ^= model->init;
    if (model->rev) {
        crc1 = reverse(crc1, model->width);
        crc2 = reverse(crc2, model->width);
    }
    word_t crc = multmodp(model, xnmodp(model, len2), crc1) ^ crc2;
    if (model->rev)
        crc = reverse(crc, model->width);
    return crc;
}