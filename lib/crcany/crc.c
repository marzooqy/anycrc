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
    word_t y = x & 0xff;
    for (int i = 0; i < WORDCHARS - 1; i++) {
        x >>= 8;
        y = (y << 8) | (x & 0xff);
    }
    return y;
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
        crc &= ONES(model->width);
        while (len >= 8) {
            crc ^= *buf++;
            for (int k = 0; k < 8; k++)
                crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
            len -= 8;
        }
    }
    else if (model->width <= 8) {
        unsigned shift = 8 - model->width;  // 0..7
        int k;
        poly <<= shift;
        crc <<= shift;
        while (len) {
            crc ^= *buf++;
            for (k = 0; k < (len > 8 ? 8 : len); k++)
                crc = crc & 0x80 ? (crc << 1) ^ poly : crc << 1;
            len -= k;
        }
        crc >>= shift;
        crc &= ONES(model->width);
    }
    else {
        word_t mask = (word_t)1 << (model->width - 1);
        int k;
        unsigned shift = model->width - 8;  // 1..WORDBITS-8
        while (len) {
            crc ^= (word_t)(*buf++) << shift;
            for (k = 0; k < (len > 8 ? 8 : len); k++)
                crc = crc & mask ? (crc << 1) ^ poly : crc << 1;
            len -= k;
        }
        crc &= ONES(model->width);
    }

    // Post-process and return the CRC.
    if (model->rev)
        crc = reverse(crc, model->width);
    return crc ^ model->xorout;
}

int crc_table_bytewise(model_t *model) {
    if(model->table_byte == NULL) {
        model->table_byte = (word_t*) malloc(WORDCHARS * 256);

        if(model->table_byte == NULL) {
            return 1;
        }
    }

    unsigned char k = 0;
    do {
        word_t crc = crc_bitwise(model, 0, &k, 8);
        if (model->rev)
            crc = reverse(crc, model->width);
        if (model->width < 8 && !model->ref)
            crc <<= 8 - model->width;
        model->table_byte[k] = crc;
    } while (++k);

    return 0;
}

word_t crc_bytewise(model_t *model, word_t crc, void const *dat, size_t len) {
    unsigned char const *buf = dat;

    if (len >= 8) {
        // Pre-process the CRC.
        if (model->rev)
            crc = reverse(crc, model->width);

        // Process the input data a byte at a time.
        if (model->ref) {
            crc &= ONES(model->width);
            while (len >= 8) {
                crc = (crc >> 8) ^ model->table_byte[(crc ^ *buf++) & 0xff];
                len -= 8;
            }
        }
        else if (model->width <= 8) {
            unsigned shift = 8 - model->width;  // 0..7
            crc <<= shift;
            while (len >= 8) {
                crc = model->table_byte[crc ^ *buf++];
                len -= 8;
            }
            crc >>= shift;
        }
        else {
            unsigned shift = model->width - 8;  // 1..WORDBITS-8
            while (len >= 8) {
                crc = (crc << 8) ^ model->table_byte[((crc >> shift) ^ *buf++) & 0xff];
                len -= 8;
            }
            crc &= ONES(model->width);
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

int crc_table_slice16(model_t *model) {
    if(model->table_slice16 == NULL) {
        model->table_slice16 = (word_t*) malloc(WORDCHARS * 16 * 256);

        if(model->table_slice16 == NULL) {
            return 1;
        }
    }

    unsigned top = model->ref ? 0 : WORDBITS - (model->width > 8 ? model->width : 8);
    word_t xorout = model->xorout;

    if (model->width < 8 && !model->ref)
        xorout <<= 8 - model->width;

    for (unsigned k = 0; k < 256; k++) {
        word_t crc = model->table_byte[k];
        model->table_slice16[k] = model->ref ? crc << top : swap(crc << top);

        for (unsigned n = 1; n < 16; n++) {
            crc ^= xorout;
            if (model->ref)
                crc = (crc >> 8) ^ model->table_byte[crc & 0xff];
            else if (model->width <= 8)
                crc = model->table_byte[crc];
            else {
                crc = (crc << 8) ^ model->table_byte[(crc >> (model->width - 8)) & 0xff];
                crc &= ONES(model->width);
            }
            crc ^= xorout;
            model->table_slice16[(n << 8) | k] = model->ref ? crc << top : swap(crc << top);
        }
    }

    return 0;
}

#define BYTE_SLICE(idx, val) model->table_slice16[((idx) << 8) | ((val >> ((~(idx) & 0x3) << 3)) & 0xff)]
#define WORD_SLICE(idx, val) (BYTE_SLICE(idx + 3, val) ^ BYTE_SLICE(idx + 2, val) ^ BYTE_SLICE(idx + 1, val) ^ BYTE_SLICE(idx, val))

word_t crc_slice16(model_t *model, word_t crc, void const *dat, size_t len) {
    unsigned char const *buf = dat;

    // Process as many 16 byte blocks as are available
    if (len >= 16 * 8) {
        // Prepare common constants
        uint32_t crc_hi, crc_lo, i3, i4;
        unsigned top = model->ref ? 0 : WORDBITS - (model->width > 8 ? model->width : 8);
        unsigned shift = model->width <= 8 ? 8 - model->width : model->width - 8;

        // Pre-process the CRC
        if (model->rev)
            crc = reverse(crc, model->width);

        if (model->ref)
            crc &= ONES(model->width);
        else if (model->width <= 8)
            crc <<= shift;

        crc <<= top;

        if (!model->ref)
            crc = swap(crc);

        do {
            crc_lo = (crc & 0xffffffff) ^ *(uint32_t const *)buf;
            crc_hi = (crc >> 32) ^ *(uint32_t const *)(buf + 4);
            i3 = *(uint32_t const *)(buf + 8);
            i4 = *(uint32_t const *)(buf + 12);
            crc = WORD_SLICE(12, crc_lo) ^ WORD_SLICE(8, crc_hi) ^ WORD_SLICE(4, i3) ^ WORD_SLICE(0, i4);

            buf += 16;
            len -= 16 * 8;
        } while (len >= 16 * 8);

        if (!model->ref)
            crc = swap(crc);

        crc >>= top;

        if (!model->ref) {
            if (model->width <= 8)
                crc >>= shift;
            else
                crc &= ONES(model->width);
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