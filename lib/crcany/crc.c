/* crc.c -- Generic CRC calculations
 * Copyright (C) 2014, 2016, 2017, 2020, 2021 Mark Adler
 * For conditions of distribution and use, see copyright notice in crcany.c.
 */

/*
* Edited by Hussain Al Marzooq
*/

#include <stdlib.h>
#include "crc.h"

// Shift to the left edge of word_t and back
#define SHIFT_L(value, width) ((word_t) (value) << (WORDBITS - (width)))
#define SHIFT_R(value, width) ((value) >> (WORDBITS - (width)))

word_t crc_preprocess(model_t* model, word_t crc) {
    crc ^= model->xorout;
    if (model->rev)
        crc = reverse(crc, model->width);
    if (!model->ref)
        crc = SHIFT_L(crc, model->width);
    return crc;
}

word_t crc_postprocess(model_t* model, word_t crc) {
    if (!model->ref)
        crc = SHIFT_R(crc, model->width);
    if (model->rev)
        crc = reverse(crc, model->width);
    return crc ^ model->xorout;
}

word_t crc_bitwise(model_t *model, word_t crc, void const *dat, size_t len) {
    unsigned char const *buf = dat;
    word_t poly = model->poly;

    crc = crc_preprocess(model, crc);

    // Process the input data a bit at a time.
    if (model->ref) {
        while (len >= 8) {
            crc ^= *buf++;
            for (int k = 0; k < 8; k++)
                crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
            len -= 8;
        }

        // Clear the remaining bits so that they won't affect the CRC calculation.
        if (len > 0) {
            unsigned char off = (unsigned char)-1 >> (8 - len);
            crc ^= *buf & off;
            while (len--)
                crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
        }
    }
    else {
        word_t mask = SHIFT_L(1, 1);
        poly = SHIFT_L(poly, model->width);

        while (len >= 8) {
            crc ^= SHIFT_L(*buf++, 8);
            for (int k = 0; k < 8; k++)
                crc = crc & mask ? (crc << 1) ^ poly : crc << 1;
            len -= 8;
        }

        // Clear the remaining bits so that they won't affect the CRC calculation.
        if (len > 0) {
            unsigned char off = (unsigned char)-1 << (8 - len);
            crc ^= SHIFT_L(*buf & off, 8);
            while (len--)
                crc = crc & mask ? (crc << 1) ^ poly : crc << 1;
        }
    }

    return crc_postprocess(model, crc);
}

void crc_table_bytewise(model_t *model) {
    word_t mask = SHIFT_L(1, 1);

    for (int i = 0; i < 256; i++) {
        word_t poly = model->poly;
        word_t crc = i;

        if (model->ref) {
            for (int k = 0; k < 8; k++)
                crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
        }
        else {
            poly = SHIFT_L(poly, model->width);
            crc = SHIFT_L(crc, 8);
            for (int k = 0; k < 8; k++)
                crc = crc & mask ? (crc << 1) ^ poly : crc << 1;
        }
        model->table[i] = crc;
    }
}

word_t crc_bytewise(model_t *model, word_t crc, void const *dat, size_t len) {
    unsigned char const *buf = dat;

    crc = crc_preprocess(model, crc);

    // Process the input data a byte at a time.
    if (model->ref) {
        while (len >= 8) {
            crc = (crc >> 8) ^ model->table[(crc ^ *buf++) & 0xff];
            len -= 8;
        }
    }
    else {
        word_t shift = WORDBITS - 8;
        while (len >= 8) {
            crc = (crc << 8) ^ model->table[((crc >> shift) ^ *buf++) & 0xff];
            len -= 8;
        }
    }

    crc = crc_postprocess(model, crc);

    // Process any remaining bits after the last byte
    if (len > 0)
        crc = crc_bitwise(model, crc, buf, len);
    return crc;
}

void crc_table_slice16(model_t *model) {
    for (unsigned k = 0; k < 256; k++) {
        word_t crc = model->table[k];

        for (unsigned n = 1; n < 16; n++) {
            if (model->ref)
                crc = (crc >> 8) ^ model->table[crc & 0xff];
            else
                crc = (crc << 8) ^ model->table[(crc >> (WORDBITS - 8)) & 0xff];

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

    crc = crc_preprocess(model, crc);

    // Process as many 16 byte blocks as are available
    if (model->ref) {
        while (len >= 16 * 8) {
            crc = SLICE_CRC_REF(0) ^ SLICE_CRC_REF(1) ^ SLICE_CRC_REF(2) ^ SLICE_CRC_REF(3)
                ^ SLICE_CRC_REF(4) ^ SLICE_CRC_REF(5) ^ SLICE_CRC_REF(6) ^ SLICE_CRC_REF(7)
                ^ SLICE_BYTE_REF(8) ^ SLICE_BYTE_REF(9) ^ SLICE_BYTE_REF(10) ^ SLICE_BYTE_REF(11)
                ^ SLICE_BYTE_REF(12) ^ SLICE_BYTE_REF(13) ^ SLICE_BYTE_REF(14) ^ SLICE_BYTE_REF(15);

            buf += 16;
            len -= 16 * 8;
        }
    }
    else {
        while (len >= 16 * 8) {
            crc = SLICE_BYTE(0) ^ SLICE_BYTE(1) ^ SLICE_BYTE(2) ^ SLICE_BYTE(3)
                ^ SLICE_BYTE(4) ^ SLICE_BYTE(5) ^ SLICE_BYTE(6) ^ SLICE_BYTE(7)
                ^ SLICE_CRC(8) ^ SLICE_CRC(9) ^ SLICE_CRC(10) ^ SLICE_CRC(11)
                ^ SLICE_CRC(12) ^ SLICE_CRC(13) ^ SLICE_CRC(14) ^ SLICE_CRC(15);

            buf += 16;
            len -= 16 * 8;
        }
    }

    crc = crc_postprocess(model, crc);

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

// Build table_comb[] for model.
void crc_table_combine(model_t *model) {
    // Keep squaring x^1 modulo p(x), where p(x) is the CRC polynomial, to
    // generate x^2^n modulo p(x).
    word_t sq = model->ref ? (word_t)1 << (model->width - 2) : 2;   // x^1
    model->table_comb[0] = sq;

    for (int n = 1; n < 64; n++) {
        sq = multmodp(model, sq, sq); // x^2^n
        model->table_comb[n] = sq;
    }
}

// Return x^n modulo p(x), where p(x) is the CRC polynomial. model->cycle
// and model->table_comb[] must first be initialized by crc_table_combine().
static word_t xnmodp(model_t *model, size_t n) {
    word_t xp = model->ref ? (word_t)1 << (model->width - 1) : 1;   // x^0

    int k = 0;
    while (n) {
        if (n & 1)
            xp = multmodp(model, model->table_comb[k], xp);
        n >>= 1;
        k++;
    }
    return xp;
}

word_t crc_combine(model_t *model, word_t crc1, word_t crc2, size_t len2) {
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