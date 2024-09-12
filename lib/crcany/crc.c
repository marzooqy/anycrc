/* crc.c -- Generic CRC calculations
 * Copyright (C) 2014, 2016, 2017, 2020, 2021 Mark Adler
 * For conditions of distribution and use, see copyright notice in crcany.c.
 */

/*
* Edited by Hussain Al Marzooq
*/

#include <stdlib.h>
#include "crc.h"

// Swap the bytes in a word_t. swap() is used at most twice per crc_slice16() call.
word_t swap(word_t x) {
    return ((x & 0xff) << 56) | ((x >> 56) & 0xff)
         | ((x & 0xff00) << 40) | ((x >> 40) & 0xff00)
         | ((x & 0xff0000) << 24) | ((x >> 24) & 0xff0000)
         | ((x & 0xff000000) << 8) | ((x >> 8) & 0xff000000);
}

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

#define SLICE_BYTE(idx) model->table[((15 - idx) << 8) | buf[idx]]
#define SLICE_CRC(idx) model->table[((15 - idx) << 8) | ((crc >> (idx * 8)) & 0xff)]

word_t crc_slice16(model_t *model, word_t crc, void const *dat, size_t len) {
    unsigned char const *buf = dat;

    // Process as many 16 byte blocks as are available
    if (len >= 16 * 8) {
        // Pre-process the CRC
        if (model->rev)
            crc = reverse(crc, model->width);

        if (!model->ref)
            crc = swap(crc);

        if (model->ref) {
            do {
                crc ^= *(word_t const*)buf;
                crc = SLICE_CRC(0) ^ SLICE_CRC(1) ^ SLICE_CRC(2) ^ SLICE_CRC(3)
                    ^ SLICE_CRC(4) ^ SLICE_CRC(5) ^ SLICE_CRC(6) ^ SLICE_CRC(7)
                    ^ SLICE_BYTE(8) ^ SLICE_BYTE(9) ^ SLICE_BYTE(10) ^ SLICE_BYTE(11)
                    ^ SLICE_BYTE(12) ^ SLICE_BYTE(13) ^ SLICE_BYTE(14) ^ SLICE_BYTE(15);

                buf += 16;
                len -= 16 * 8;
            } while (len >= 16 * 8);
        }

        if (!model->ref)
            crc = swap(crc);

        // Post-process
        if (model->rev)
            crc = reverse(crc, model->width);
    }

    // Process any remaining bytes after the last 16 byte block
    if (len > 0)
        crc = crc_bytewise(model, crc, buf, len);
    return crc;
}