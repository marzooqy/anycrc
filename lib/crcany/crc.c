/* crc.c -- Generic CRC calculations
 * Copyright (C) 2014, 2016, 2017, 2020, 2021 Mark Adler
 * For conditions of distribution and use, see copyright notice in crcany.c.
 */

/*
* Edited by Hussain Al Marzooq
*/

#include <stdlib.h>
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
    }
    else {
        word_t mask = (word_t)1 << (WORDBITS - 1);
        unsigned top = WORDBITS - model->width;
        unsigned shift = WORDBITS - 8;
        int k;

        poly <<= top;
        crc <<= top;
        while (len) {
            crc ^= (word_t)(*buf++) << shift;
            for (k = 0; k < (len > 8 ? 8 : len); k++)
                crc = crc & mask ? (crc << 1) ^ poly : crc << 1;
            len -= k;
        }
        crc >>= top;
    }

    // Post-process and return the CRC.
    if (model->rev)
        crc = reverse(crc, model->width);
    return crc ^ model->xorout;
}

int crc_table_bytewise(model_t *model) {
    if (model->table_byte == NULL) {
        model->table_byte = (word_t*) malloc(WORDCHARS * 256);

        if (model->table_byte == NULL) {
            return 1;
        }
    }

    unsigned char k = 0;
    do {
        word_t crc = crc_bitwise(model, 0, &k, 8);
        if (model->rev)
            crc = reverse(crc, model->width);
        if (!model->ref)
            crc <<= WORDBITS - model->width;
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
            while (len >= 8) {
                crc = (crc >> 8) ^ model->table_byte[(crc ^ *buf++) & 0xff];
                len -= 8;
            }
        }
        else {
            unsigned top = WORDBITS - model->width;
            unsigned shift = WORDBITS - 8;
            crc <<= top;
            while (len >= 8) {
                crc = (crc << 8) ^ model->table_byte[((crc >> shift) ^ *buf++) & 0xff];
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

int crc_table_slice16(model_t *model) {
    if (model->table_slice16 == NULL) {
        model->table_slice16 = (word_t*) malloc(WORDCHARS * 16 * 256);

        if (model->table_slice16 == NULL) {
            return 1;
        }
    }

    word_t xorout = model->xorout;
    if (!model->ref)
        xorout <<= WORDBITS - model->width;

    for (unsigned k = 0; k < 256; k++) {
        word_t crc = model->table_byte[k];
        model->table_slice16[k] = crc;

        for (unsigned n = 1; n < 16; n++) {
            crc ^= xorout;
            if (model->ref)
                crc = (crc >> 8) ^ model->table_byte[crc & 0xff];
            else
                crc = (crc << 8) ^ model->table_byte[(crc >> (WORDBITS - 8)) & 0xff];
            crc ^= xorout;
            model->table_slice16[(n << 8) | k] = crc;
        }
    }

    return 0;
}

#define SLICE_BYTE_REF(val, idx) model->table_slice16[((15 - (idx)) << 8) | (((val >> ((idx) << 3)) ^ buf[(idx)]) & 0xff)]
#define SLICE_WORD_REF(val, idx) SLICE_BYTE_REF(val, idx) ^ SLICE_BYTE_REF(val, idx + 1) \
                               ^ SLICE_BYTE_REF(val, idx + 2) ^ SLICE_BYTE_REF(val, idx + 3)

#define SLICE_BYTE(val, idx) model->table_slice16[((idx) << 8) | (((val >> (((idx) & 0x7) << 3)) ^ buf[15 - (idx)]) & 0xff)]
#define SLICE_WORD(val, idx) SLICE_BYTE(val, idx) ^ SLICE_BYTE(val, idx + 1) ^ SLICE_BYTE(val, idx + 2) ^ SLICE_BYTE(val, idx + 3)

word_t crc_slice16(model_t *model, word_t crc, void const *dat, size_t len) {
    unsigned char const *buf = dat;

    // Process as many 16 byte blocks as are available
    if (len >= 16 * 8) {
        // Pre-process the CRC
        if (model->rev)
            crc = reverse(crc, model->width);

        if (model->ref) {
            do {
                crc = SLICE_WORD_REF(crc, 0) ^ SLICE_WORD_REF(crc, 4)
                    ^ SLICE_WORD_REF((word_t) 0, 8) ^ SLICE_WORD_REF((word_t) 0, 12);
                buf += 16;
                len -= 16 * 8;
            } while (len >= 16 * 8);
        }
        else {
            unsigned top = WORDBITS - model->width;
            crc <<= top;
            do {
                crc = SLICE_WORD((word_t) 0, 0) ^ SLICE_WORD((word_t) 0, 4)
                    ^ SLICE_WORD(crc, 8) ^ SLICE_WORD(crc, 12);
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