/* crcdbl.c -- Generic bit-wise CRC calculation for a double-wide CRC
 * Copyright (C) 2014, 2016, 2017 Mark Adler
 * For conditions of distribution and use, see copyright notice in crcany.c.
 */

/*
* Edited by Hussain Al Marzooq
*/

#include <stdlib.h>

#include "crcdbl.h"
#include "crc.h"

dword_t shl(dword_t dw, word_t shft) {
    return (dword_t) {(dw.hi << shft) | (dw.lo >> (WORDBITS - shft)), dw.lo << shft};
}

dword_t shr(dword_t dw, word_t shft) {
    return (dword_t) {dw.hi >> shft, (dw.hi << (WORDBITS - shft)) | (dw.lo >> shft)};
}

dword_t xor(dword_t dw, dword_t dw2) {
    return (dword_t) {dw.hi ^ dw2.hi, dw.lo ^ dw2.lo};
}

dword_t crc_bitwise_dbl(model_t *model, dword_t crc, unsigned char const *buf, size_t len) {
    dword_t poly = {model->poly_hi, model->poly};
    dword_t xorout = {model->xorout_hi, model->xorout};

    // Use crc_bitwise() for CRCs that fit in a word_t.
    if (model->width <= WORDBITS) {
        crc.lo = crc_bitwise(model, crc.lo, buf, len);
        crc.hi = 0;
        return crc;
    }

    // Pre-process the CRC.
    crc = xor(crc, xorout);
    if (model->rev)
        crc = reverse_dbl(crc, model->width);

    // Process the input data a bit at a time.
    if (model->ref) {
        int k = 0;
        crc.hi &= ONES(model->width - WORDBITS);
        while (len >= 8) {
            crc.lo ^= *buf++;
            for (k = 0; k < 8; k++)
                crc = crc.lo & 1 ? xor(shr(crc, 1), poly) : shr(crc, 1);
            len -= 8;
        }
    }
    else {
        word_t mask = (word_t)1 << (model->width - WORDBITS - 1);
        unsigned shift = model->width - WORDBITS - 8;   // 1..WORDBITS-8
        int k = 0;
        while (len) {
            crc = xor(crc, shl((dword_t){0, *buf++}, shift));
            for (k = 0; k < (len > 8 ? 8 : len); k++)
                crc = crc.hi & mask ? xor(shl(crc, 1), poly) : shl(crc, 1);
            len -= k;
        }
        crc.hi &= ONES(model->width - WORDBITS);
    }

    // Post-process and return the CRC.
    if (model->rev)
        crc = reverse_dbl(crc, model->width);

    return xor(crc, xorout);
}

int crc_table_bytewise_dbl(model_t *model) {
    if(model->table_byte_dbl == NULL) {
        model->table_byte_dbl = (dword_t*) malloc(WORDCHARS * 256 * 2);

        if(model->table_byte_dbl == NULL) {
            return 1;
        }
    }

    unsigned char k = 0;
    do {
        dword_t crc = {0, 0};
        crc = crc_bitwise_dbl(model, crc, &k, 8);

        if (model->rev)
            crc = reverse_dbl(crc, model->width);

        model->table_byte_dbl[k] = crc;
    } while (++k);

    return 0;
}

dword_t crc_bytewise_dbl(model_t *model, dword_t crc, unsigned char const *buf, size_t len) {
    if (len >= 8) {
        // Use crc_bytewise() for CRCs that fit in a word_t.
        if (model->width <= WORDBITS) {
            crc.lo = crc_bytewise(model, crc.lo, buf, len);
            crc.hi = 0;
            return crc;
        }

        // Pre-process the CRC.
        if (model->rev)
            crc = reverse_dbl(crc, model->width);

        // Process the input data a byte at a time.
        if (model->ref) {
            crc.hi &= ONES(model->width - WORDBITS);
            while (len >= 8) {
                crc = xor(shr(crc, 8), model->table_byte_dbl[(crc.lo ^ *buf++) & 0xff]);
                len -= 8;
            }
        }
        else {
            unsigned shift = model->width - 8;  // 1..WORDBITS-8
            while (len >= 8) {
                crc = xor(shl(crc, 8), model->table_byte_dbl[(shr(crc, shift).lo ^ *buf++) & 0xff]);
                len -= 8;
            }
            crc.hi &= ONES(model->width - WORDBITS);
        }

        // Post-process and return the CRC
        if (model->rev)
            crc = reverse_dbl(crc, model->width);
    }

    // Process any remaining bits after the last byte
    if (len > 0)
        crc = crc_bitwise_dbl(model, crc, buf, len);

    return crc;
}