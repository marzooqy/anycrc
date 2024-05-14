/* crcdbl.c -- Generic bit-wise CRC calculation for a double-wide CRC
 * Copyright (C) 2014, 2016, 2017 Mark Adler
 * For conditions of distribution and use, see copyright notice in crcany.c.
 */

#include <stdlib.h>

#include "crcdbl.h"
#include "crc.h"

// Shift left a double-word quantity by n bits: a <<= n, 0 <= n < WORDBITS.  ah
// and al must be word_t lvalues.  WORDBITS is the number of bits in a word_t,
// which must be an unsigned integer type.
#define SHL(ah, al, n) \
    do { \
        if ((n) == 0) \
            break; \
        ah <<= n; \
        ah |= al >> (WORDBITS - (n)); \
        al <<= n; \
    } while (0)

// Shift right a double-word quantity by n bits: a >>= n, 0 <= n < WORDBITS.
// ah and al must be word_t lvalues.  WORDBITS is the number of bits in a
// word_t, which must be an unsigned integer type.
#define SHR(ah, al, n) \
    do { \
        if ((n) == 0) \
            break; \
        al >>= n; \
        al |= ah << (WORDBITS - (n)); \
        ah >>= n; \
    } while (0)

// Shifting helpers
#define SHL_HI(ah, al, n) ((ah << n) | (al >> (WORDBITS - n)))
#define SHL_LO(ah, al, n) (al << n)
#define SHR_HI(ah, al, n) (ah >> n)
#define SHR_LO(ah, al, n) ((al >> n) | (ah << (WORDBITS - n)))

// Process one bit in a big reflected CRC in crc_bitwise_dbl().
#define BIGREF \
    do { \
        word_t tmp = lo & 1; \
        lo = (lo >> 1) | (hi << (WORDBITS - 1)); \
        hi >>= 1; \
        if (tmp) { \
            lo ^= poly_lo; \
            hi ^= poly_hi; \
        } \
    } while (0)

// Process one bit in a non-reflected CRC that has been shifted to put the high
// byte at the bottom of hi in crc_bitwise_dbl().
#define BIGCROSS \
    do { \
        word_t tmp = hi & 0x80; \
        hi = (hi << 1) | (lo >> (WORDBITS - 1)); \
        lo <<= 1; \
        if (tmp) { \
            lo ^= poly_lo; \
            hi ^= poly_hi; \
        } \
    } while (0)

// Process one bit in a big non-reflected CRC in crc_bitwise_dbl().
#define BIGNORM \
    do { \
        word_t tmp = hi & mask; \
        hi = (hi << 1) | (lo >> (WORDBITS - 1)); \
        lo <<= 1; \
        if (tmp) { \
            lo ^= poly_lo; \
            hi ^= poly_hi; \
        } \
    } while (0)

void crc_bitwise_dbl(model_t *model, word_t *crc_hi, word_t *crc_lo,
                     unsigned char const *buf, size_t len) {
    word_t poly_lo = model->poly;
    word_t poly_hi = model->poly_hi;

    // Use crc_bitwise() for CRCs that fit in a word_t.
    if (model->width <= WORDBITS) {
        *crc_lo = crc_bitwise(model, *crc_lo, buf, len);
        *crc_hi = 0;
        return;
    }

    // If requested, return the initial CRC.
    if (buf == NULL) {
        *crc_lo = model->init;
        *crc_hi = model->init_hi;
        return;
    }

    // Pre-process the CRC.
    word_t lo = *crc_lo ^ model->xorout;
    word_t hi = *crc_hi ^ model->xorout_hi;
    if (model->rev)
        reverse_dbl(&hi, &lo, model->width);

    // Process the input data a bit at a time.
    if (model->ref) {
        hi &= ONES(model->width - WORDBITS);
        while (len--) {
            lo ^= *buf++;
            BIGREF;  BIGREF;  BIGREF;  BIGREF;
            BIGREF;  BIGREF;  BIGREF;  BIGREF;
        }
    }
    else if (model->width - WORDBITS <= 8) {
        unsigned shift = 8 - (model->width - WORDBITS); // 0..7
        SHL(poly_hi, poly_lo, shift);
        SHL(hi, lo, shift);
        while (len--) {
            hi ^= *buf++;
            BIGCROSS;  BIGCROSS;  BIGCROSS;  BIGCROSS;
            BIGCROSS;  BIGCROSS;  BIGCROSS;  BIGCROSS;
        }
        SHR(hi, lo, shift);
        hi &= ONES(model->width - WORDBITS);
    }
    else {
        word_t mask = (word_t)1 << (model->width - WORDBITS - 1);
        unsigned shift = model->width - WORDBITS - 8;   // 1..WORDBITS-8
        while (len--) {
            hi ^= (word_t)(*buf++) << shift;
            BIGNORM;  BIGNORM;  BIGNORM;  BIGNORM;
            BIGNORM;  BIGNORM;  BIGNORM;  BIGNORM;
        }
        hi &= ONES(model->width - WORDBITS);
    }

    // Post-process and return the CRC.
    if (model->rev)
        reverse_dbl(&hi, &lo, model->width);
    lo ^= model->xorout;
    hi ^= model->xorout_hi;
    *crc_lo = lo;
    *crc_hi = hi;
}

void crc_zeros_dbl(model_t *model, word_t *crc_hi, word_t *crc_lo,
                   size_t count) {
    word_t poly_lo = model->poly;
    word_t poly_hi = model->poly_hi;

    // Use crc_zeros() for CRCs that fit in a word_t.
    if (model->width <= WORDBITS) {
        *crc_lo = crc_zeros(model, *crc_lo, count);
        *crc_hi = 0;
        return;
    }

    // Pre-process the CRC.
    word_t lo = *crc_lo ^ model->xorout;
    word_t hi = *crc_hi ^ model->xorout_hi;
    if (model->rev)
        reverse_dbl(&hi, &lo, model->width);

    // Process the input data a bit at a time.
    if (model->ref) {
        hi &= ONES(model->width - WORDBITS);
        while (count--)
            BIGREF;
    }
    else {
        word_t mask = (word_t)1 << (model->width - WORDBITS - 1);
        while (count--)
            BIGNORM;
        hi &= ONES(model->width - WORDBITS);
    }

    // Post-process and return the CRC.
    if (model->rev)
        reverse_dbl(&hi, &lo, model->width);
    lo ^= model->xorout;
    hi ^= model->xorout_hi;
    *crc_lo = lo;
    *crc_hi = hi;
}

int crc_table_bytewise_dbl(model_t *model) {
    if(model->table_byte == NULL) {
        model->table_byte = (word_t*) malloc(WORDCHARS * 256 * 2);
        
        if(model->table_byte == NULL) {
            return 1;
        }
    }
    
    unsigned char k = 0;
    do {
        word_t crc_lo = 0;
        word_t crc_hi = 0;
        crc_bitwise_dbl(model, &crc_hi, &crc_lo, &k, 1);
        if (model->rev)
            reverse_dbl(&crc_hi, &crc_lo, model->width);
        model->table_byte[k] = crc_lo;
        model->table_byte[256 + k] = crc_hi;
    } while (++k);
    
    return 0;
}

void crc_bytewise_dbl(model_t *model, word_t *crc_hi, word_t *crc_lo, unsigned char const *buf, size_t len) {
    // Pre-process the CRC.
    if (model->rev)
        reverse_dbl(crc_hi, crc_lo, model->width);
        
    word_t lo = *crc_lo;
    word_t hi = *crc_hi;
    unsigned short idx;
    
    // Process the input data a byte at a time.
    if (model->ref) {
        if(model->width > WORDBITS) {
            hi &= ONES(model->width - WORDBITS);
        } else {
            lo &= ONES(model->width);
            hi = 0;
        }
        while (len--) {
            idx = (lo ^ *buf++) & 0xff;
            lo = SHR_LO(hi, lo, 8) ^ model->table_byte[idx];
            hi = SHR_HI(hi, lo, 8) ^ model->table_byte[256 + idx];
        }
    }
    else {
        unsigned shift = model->width - 8;  // 1..WORDBITS-8
        while (len--) {
            idx = (SHR_LO(hi, lo, shift) ^ *buf++) & 0xff;
            lo = SHL_LO(hi, lo, 8) ^ model->table_byte[idx];
            hi = SHL_HI(hi, lo, 8) ^ model->table_byte[256 + idx];
        }
        if(model->width > WORDBITS) {
            hi &= ONES(model->width - WORDBITS);
        } else {
            lo &= ONES(model->width);
            hi = 0;
        }
    }
    
    *crc_lo = lo;
    *crc_hi = hi;
    
    // Post-process and return the CRC
    if (model->rev)
        reverse_dbl(crc_hi, crc_lo, model->width);
}