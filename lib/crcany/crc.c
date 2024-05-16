/* crc.c -- Generic CRC calculations
 * Copyright (C) 2014, 2016, 2017, 2020, 2021 Mark Adler
 * For conditions of distribution and use, see copyright notice in crcany.c.
 */

/*
* Edited by Hussain Al Marzooq
*/
  
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "omp.h"
#include "crc.h"

word_t crc_bitwise(model_t *model, word_t crc, void const *dat, size_t len) {
    unsigned char const *buf = dat;
    word_t poly = model->poly;

    // If requested, return the initial CRC.
    if (buf == NULL)
        return model->init;

    // Pre-process the CRC.
    crc ^= model->xorout;
    if (model->rev)
        crc = reverse(crc, model->width);

    // Process the input data a bit at a time.
    if (model->ref) {
        crc &= ONES(model->width);
        while (len--) {
            crc ^= *buf++;
            for (int k = 0; k < 8; k++)
                crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
        }
    }
    else if (model->width <= 8) {
        unsigned shift = 8 - model->width;  // 0..7
        poly <<= shift;
        crc <<= shift;
        while (len--) {
            crc ^= *buf++;
            for (int k = 0; k < 8; k++)
                crc = crc & 0x80 ? (crc << 1) ^ poly : crc << 1;
        }
        crc >>= shift;
        crc &= ONES(model->width);
    }
    else {
        word_t mask = (word_t)1 << (model->width - 1);
        unsigned shift = model->width - 8;  // 1..WORDBITS-8
        while (len--) {
            crc ^= (word_t)(*buf++) << shift;
            for (int k = 0; k < 8; k++)
                crc = crc & mask ? (crc << 1) ^ poly : crc << 1;
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
        word_t crc = crc_bitwise(model, 0, &k, 1);
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

    // If requested, return the initial CRC.
    if (buf == NULL)
        return model->init;

    // Pre-process the CRC.
    if (model->rev)
        crc = reverse(crc, model->width);

    // Process the input data a byte at a time.
    if (model->ref) {
        crc &= ONES(model->width);
        while (len--)
            crc = (crc >> 8) ^ model->table_byte[(crc ^ *buf++) & 0xff];
    }
    else if (model->width <= 8) {
        unsigned shift = 8 - model->width;  // 0..7
        crc <<= shift;
        while (len--)
            crc = model->table_byte[crc ^ *buf++];
        crc >>= shift;
    }
    else {
        unsigned shift = model->width - 8;  // 1..WORDBITS-8
        while (len--)
            crc = (crc << 8) ^
                  model->table_byte[((crc >> shift) ^ *buf++) & 0xff];
        crc &= ONES(model->width);
    }

    // Post-process and return the CRC
    if (model->rev)
        crc = reverse(crc, model->width);
    return crc;
}

// Swap the low n bytes of x. Bytes above those are discarded.
static inline word_t swaplow(word_t x, unsigned n) {
    if (n == 0)
        return 0;
    word_t y = x & 0xff;
    while (--n) {
        x >>= 8;
        y <<= 8;
        y |= x & 0xff;
    }
    return y;
}

// Swap the bytes in a word_t. swap() is used at most twice per crc_wordwise()
// call, and then only on little-endian machines if the CRC is not reflected,
// or on big-endian machines if the CRC is reflected.
static inline word_t swap(word_t x) {
    return swaplow(x, WORDCHARS);
}

int crc_table_wordwise(model_t *model, unsigned little, unsigned word_bits) {
    unsigned word_bytes = word_bits >> 3;
    
    if(model->table_word == NULL) {
        model->table_word = (word_t*) malloc(WORDCHARS * word_bytes * 256);
        
        if(model->table_word == NULL) {
            return 1;
        }
    }
    
    unsigned opp = little ^ model->ref;
    unsigned top =
        model->ref ? 0 :
                     word_bits - (model->width > 8 ? model->width : 8);
    word_t xor = model->xorout;
    if (model->width < 8 && !model->ref)
        xor <<= 8 - model->width;
    
    for (unsigned k = 0; k < 256; k++) {
        word_t crc = model->table_byte[k];
        model->table_word[k] = opp ? swaplow(crc << top, word_bytes) :
                                        crc << top;
        for (unsigned n = 1; n < (word_bits >> 3); n++) {
            crc ^= xor;
            if (model->ref)
                crc = (crc >> 8) ^ model->table_byte[crc & 0xff];
            else if (model->width <= 8)
                crc = model->table_byte[crc];
            else {
                crc = (crc << 8) ^
                      model->table_byte[(crc >> (model->width - 8)) & 0xff];
                crc &= ONES(model->width);
            }
            crc ^= xor;
            model->table_word[n * 256 + k] = opp ? swaplow(crc << top, word_bytes) :
                                            crc << top;
        }
    }
    
    return 0;
}

word_t crc_wordwise(model_t *model, word_t crc, void const *dat, size_t len) {
    unsigned char const *buf = dat;

    // If requested, return the initial CRC.
    if (buf == NULL)
        return model->init;

    // Prepare common constants.
    unsigned little = 1;
    little = *((unsigned char *)(&little));
    unsigned top = model->ref ? 0 :
                   WORDBITS - (model->width > 8 ? model->width : 8);
    unsigned shift = model->width <= 8 ? 8 - model->width : model->width - 8;

    // Pre-process the CRC.
    if (model->rev)
        crc = reverse(crc, model->width);

    // Process the first few bytes up to a word_t boundary, if any.
    if (model->ref) {
        crc &= ONES(model->width);
        while (len && ((ptrdiff_t)buf & (WORDCHARS - 1))) {
            crc = (crc >> 8) ^ model->table_byte[(crc ^ *buf++) & 0xff];
            len--;
        }
    }
    else if (model->width <= 8) {
        crc <<= shift;
        while (len && ((ptrdiff_t)buf & (WORDCHARS - 1))) {
            crc = model->table_byte[(crc ^ *buf++) & 0xff];
            len--;
        }
    }
    else
        while (len && ((ptrdiff_t)buf & (WORDCHARS - 1))) {
            crc = (crc << 8) ^
                  model->table_byte[((crc >> shift) ^ *buf++) & 0xff];
            len--;
        }

    // Process as many word_t's as are available.
    if (len >= WORDCHARS) {
        crc <<= top;
        if (little) {
            if (!model->ref)
                crc = swap(crc);
            do {
                crc ^= *(word_t const *)buf;
                crc = model->table_word[(WORDCHARS - 1) * 256 + (crc & 0xff)]
                    ^ model->table_word[(WORDCHARS - 2) * 256 + ((crc >> 8) & 0xff)
#if WORDCHARS > 2
                                                                  ]
                    ^ model->table_word[(WORDCHARS - 3) * 256 + ((crc >> 16) & 0xff)]
                    ^ model->table_word[(WORDCHARS - 4) * 256 + ((crc >> 24) & 0xff)
#if WORDCHARS > 4
                                                                   ]
                    ^ model->table_word[(WORDCHARS - 5) * 256 + ((crc >> 32) & 0xff)]
                    ^ model->table_word[(WORDCHARS - 6) * 256 + ((crc >> 40) & 0xff)]
                    ^ model->table_word[(WORDCHARS - 7) * 256 + ((crc >> 48) & 0xff)]
                    ^ model->table_word[(WORDCHARS - 8) * 256 + ((crc >> 56) & 0xff)
#if WORDCHARS > 8
                                                                   ]
                    ^ model->table_word[(WORDCHARS - 9) * 256 + ((crc >> 64) & 0xff)]
                    ^ model->table_word[(WORDCHARS - 10) * 256 + ((crc >> 72) & 0xff)]
                    ^ model->table_word[(WORDCHARS - 11) * 256 + ((crc >> 80) & 0xff)]
                    ^ model->table_word[(WORDCHARS - 12) * 256 + ((crc >> 88) & 0xff)]
                    ^ model->table_word[(WORDCHARS - 13) * 256 + ((crc >> 96) & 0xff)]
                    ^ model->table_word[(WORDCHARS - 14) * 256 + ((crc >> 104) & 0xff)]
                    ^ model->table_word[(WORDCHARS - 15) * 256 + ((crc >> 112) & 0xff)]
                    ^ model->table_word[(WORDCHARS - 16) * 256 + (crc >> 120)
#endif
#endif
#endif
                                                                    ];
                buf += WORDCHARS;
                len -= WORDCHARS;
            } while (len >= WORDCHARS);
            if (!model->ref)
                crc = swap(crc);
        }
        else {
            if (model->ref)
                crc = swap(crc);
            do {
                crc ^= *(word_t const *)buf;
                crc = model->table_word[crc & 0xff]
                    ^ model->table_word[256 + ((crc >> 8) & 0xff)
#if WORDCHARS > 2
                                                      ]
                    ^ model->table_word[2 * 256 + ((crc >> 16) & 0xff)]
                    ^ model->table_word[3 * 256 + ((crc >> 24) & 0xff)
#if WORDCHARS > 4
                                                       ]
                    ^ model->table_word[4 * 256 + ((crc >> 32) & 0xff)]
                    ^ model->table_word[5 * 256 + ((crc >> 40) & 0xff)]
                    ^ model->table_word[6 * 256 + ((crc >> 48) & 0xff)]
                    ^ model->table_word[7 * 256 + ((crc >> 56) & 0xff)
#if WORDCHARS > 8
                                                       ]
                    ^ model->table_word[8 * 256 + ((crc >> 64) & 0xff)]
                    ^ model->table_word[9 * 256 + ((crc >> 72) & 0xff)]
                    ^ model->table_word[10 * 256 + ((crc >> 80) & 0xff)]
                    ^ model->table_word[11 * 256 + ((crc >> 88) & 0xff)]
                    ^ model->table_word[12 * 256 + ((crc >> 96) & 0xff)]
                    ^ model->table_word[13 * 256 + ((crc >> 104) & 0xff)]
                    ^ model->table_word[14 * 256 + ((crc >> 112) & 0xff)]
                    ^ model->table_word[15 * 256 + (crc >> 120)
#endif
#endif
#endif
                                                        ];
                buf += WORDCHARS;
                len -= WORDCHARS;
            } while (len >= WORDCHARS);
            if (model->ref)
                crc = swap(crc);
        }
        crc >>= top;
    }

    // Process any remaining bytes after the last word_t.
    if (model->ref)
        while (len--)
            crc = (crc >> 8) ^ model->table_byte[(crc ^ *buf++) & 0xff];
    else if (model->width <= 8) {
        while (len--)
            crc = model->table_byte[(crc ^ *buf++) & 0xff];
        crc >>= shift;
    }
    else {
        while (len--)
            crc = (crc << 8) ^
                  model->table_byte[((crc >> shift) ^ *buf++) & 0xff];
        crc &= ONES(model->width);
    }

    // Post-process and return the CRC.
    if (model->rev)
        crc = reverse(crc, model->width);
    return crc;
}

int crc_table_slice16(model_t *model, unsigned little, unsigned word_bits) {
    if(model->table_slice16 == NULL) {
        model->table_slice16 = (word_t*) malloc(WORDCHARS * 16 * 256);
        
        if(model->table_slice16 == NULL) {
            return 1;
        }
    }
    
    unsigned opp = little ^ model->ref;
    unsigned top =
        model->ref ? 0 :
                     word_bits - (model->width > 8 ? model->width : 8);
    word_t xor = model->xorout;
    if (model->width < 8 && !model->ref)
        xor <<= 8 - model->width;
    unsigned word_bytes = word_bits >> 3;
    for (unsigned k = 0; k < 256; k++) {
        word_t crc = model->table_byte[k];
        model->table_slice16[k] = opp ? swaplow(crc << top, word_bytes) :
                                        crc << top;
        for (unsigned n = 1; n < 16; n++) {
            crc ^= xor;
            if (model->ref)
                crc = (crc >> 8) ^ model->table_byte[crc & 0xff];
            else if (model->width <= 8)
                crc = model->table_byte[crc];
            else {
                crc = (crc << 8) ^
                      model->table_byte[(crc >> (model->width - 8)) & 0xff];
                crc &= ONES(model->width);
            }
            crc ^= xor;
            model->table_slice16[n * 256 + k] = opp ? swaplow(crc << top, word_bytes) :
                                            crc << top;
        }
    }
    
    return 0;
}

word_t crc_slice16(model_t *model, word_t crc, void const *dat, size_t len) {
    unsigned char const *buf = dat;

    // If requested, return the initial CRC.
    if (buf == NULL)
        return model->init;

    // Prepare common constants.
    unsigned little = 1;
    little = *((unsigned char *)(&little));
    unsigned top = model->ref ? 0 :
                   WORDBITS - (model->width > 8 ? model->width : 8);
    unsigned shift = model->width <= 8 ? 8 - model->width : model->width - 8;

    // Pre-process the CRC.
    if (model->rev)
        crc = reverse(crc, model->width);

    // Process the first few bytes up to a word_t boundary, if any.
    if (model->ref) {
        crc &= ONES(model->width);
        while (len && ((ptrdiff_t)buf & (WORDCHARS - 1))) {
            crc = (crc >> 8) ^ model->table_byte[(crc ^ *buf++) & 0xff];
            len--;
        }
    }
    else if (model->width <= 8) {
        crc <<= shift;
        while (len && ((ptrdiff_t)buf & (WORDCHARS - 1))) {
            crc = model->table_byte[(crc ^ *buf++) & 0xff];
            len--;
        }
    }
    else
        while (len && ((ptrdiff_t)buf & (WORDCHARS - 1))) {
            crc = (crc << 8) ^
                  model->table_byte[((crc >> shift) ^ *buf++) & 0xff];
            len--;
        }

    // Process as many 16 byte block as are available.
    if (len >= 16) {
        crc <<= top;
        if (little) {
            if (!model->ref)
                crc = swap(crc);
            do {
                uint32_t crc_lo = (crc & 0xffffffff) ^ *(uint32_t const *)buf;
                uint32_t crc_hi = (crc >> 32) ^ *(uint32_t const *)(buf + 4);
                uint32_t i3 = *(uint32_t const *)(buf + 8);
                uint32_t i4 = *(uint32_t const *)(buf + 12);
                
                crc = model->table_slice16[15 * 256 + (crc_lo & 0xff)]
                    ^ model->table_slice16[14 * 256 + ((crc_lo >> 8) & 0xff)]
                    ^ model->table_slice16[13 * 256 + ((crc_lo >> 16) & 0xff)]
                    ^ model->table_slice16[12 * 256 + (crc_lo >> 24)]
                    ^ model->table_slice16[11 * 256 + (crc_hi & 0xff)]
                    ^ model->table_slice16[10 * 256 + ((crc_hi >> 8) & 0xff)]
                    ^ model->table_slice16[9 * 256 + ((crc_hi >> 16) & 0xff)]
                    ^ model->table_slice16[8 * 256 + (crc_hi >> 24)]
                    ^ model->table_slice16[7 * 256 + (i3 & 0xff)]
                    ^ model->table_slice16[6 * 256 + ((i3 >> 8) & 0xff)]
                    ^ model->table_slice16[5 * 256 + ((i3 >> 16) & 0xff)]
                    ^ model->table_slice16[4 * 256 + (i3 >> 24)]
                    ^ model->table_slice16[3 * 256 + (i4 & 0xff)]
                    ^ model->table_slice16[2 * 256 + ((i4 >> 8) & 0xff)]
                    ^ model->table_slice16[256 + ((i4 >> 16) & 0xff)]
                    ^ model->table_slice16[i4 >> 24];
                    
                buf += 16;
                len -= 16;
            } while (len >= 16);
            if (!model->ref)
                crc = swap(crc);
        }
        else {
            if (model->ref)
                crc = swap(crc);
            do {
                uint32_t crc_hi = (crc >> 32) ^ *(uint32_t const *)buf;
                uint32_t crc_lo = (crc & 0xffffffff) ^ *(uint32_t const *)(buf + 4);
                uint32_t i3 = *(uint32_t const *)(buf + 8);
                uint32_t i4 = *(uint32_t const *)(buf + 12);
                
                crc = model->table_slice16[i4 & 0xff]
                    ^ model->table_slice16[256 + ((i4 >> 8) & 0xff)]
                    ^ model->table_slice16[2 * 256 + ((i4 >> 16) & 0xff)]
                    ^ model->table_slice16[3 * 256 + (i4 >> 24)]
                    ^ model->table_slice16[4 * 256 + (i3 & 0xff)]
                    ^ model->table_slice16[5 * 256 + ((i3 >> 8) & 0xff)]
                    ^ model->table_slice16[6 * 256 + ((i3 >> 16) & 0xff)]
                    ^ model->table_slice16[7 * 256 + (i3 >> 24)]
                    ^ model->table_slice16[8 * 256 + (crc_lo & 0xff)]
                    ^ model->table_slice16[9 * 256 + ((crc_lo >> 8) & 0xff)]
                    ^ model->table_slice16[10 * 256 + ((crc_lo >> 16) & 0xff)]
                    ^ model->table_slice16[11 * 256 + (crc_lo >> 24)]
                    ^ model->table_slice16[12 * 256 + (crc_hi & 0xff)]
                    ^ model->table_slice16[13 * 256 + ((crc_hi >> 8) & 0xff)]
                    ^ model->table_slice16[14 * 256 + ((crc_hi >> 16) & 0xff)]
                    ^ model->table_slice16[15 * 256 + (crc_hi >> 24)];
                    
                buf += 16;
                len -= 16;
            } while (len >= 16);
            if (model->ref)
                crc = swap(crc);
        }
        crc >>= top;
    }

    // Process any remaining bytes after the last 16 byte block.
    if (model->ref)
        while (len--)
            crc = (crc >> 8) ^ model->table_byte[(crc ^ *buf++) & 0xff];
    else if (model->width <= 8) {
        while (len--)
            crc = model->table_byte[(crc ^ *buf++) & 0xff];
        crc >>= shift;
    }
    else {
        while (len--)
            crc = (crc << 8) ^
                  model->table_byte[((crc >> shift) ^ *buf++) & 0xff];
        crc &= ONES(model->width);
    }

    // Post-process and return the CRC.
    if (model->rev)
        crc = reverse(crc, model->width);
    return crc;
}

word_t crc_parallel(model_t *model, word_t crc, void const *dat, size_t len, int *error) {
    short nthreads = omp_get_max_threads();
    
    word_t* crc_p = (word_t*)malloc((nthreads - 1) * WORDCHARS);
    
    if(crc_p == NULL) {
        *error = 1;
        return crc;
    }
    
    size_t block_len = len / nthreads;
    
    // This way all of the later blocks will have the same size
    size_t first_block_len = block_len + (len - nthreads * block_len);
    unsigned char* offset = (unsigned char*)dat + first_block_len;
    
    // Use signed variable to handle OpenMP compiler error (MSVC)
    short i;
    
    // Split the data into a number of blocks equal to the system's number of threads
    // then compute the CRC for each block in parallel
    #pragma omp parallel for
    for(i = -1; i < nthreads - 1; i++) {
        // First block goes directly into the initial CRC
        if(i == -1) {
            crc = crc_slice16(model, crc, (unsigned char*)dat, first_block_len);
        } else {
            // Cast index to unsigned to handle compiler error (GCC)
            crc_p[(unsigned short)i] = crc_slice16(model, model->init, offset + i * block_len, block_len);
        }
    }
    
    // Combine the CRCs
    // Not much could be done to parallelize this, so just do it serially
    for(i = 0; i < nthreads - 1; i++) {
        crc = crc_combine(model, crc, crc_p[i], block_len);
    }
    
    free(crc_p);
    
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
int crc_table_combine(model_t *model) {
    if(model->table_comb == NULL) {
        model->table_comb = (word_t*) malloc(WORDCHARS * 67);
        
        if(model->table_comb == NULL) {
            return 1;
        }
    }
    
    // Keep squaring x^1 modulo p(x), where p(x) is the CRC polynomial, to
    // generate x^2^n modulo p(x).
    word_t sq = model->ref ? (word_t)1 << (model->width - 2) : 2;   // x^1
    model->table_comb[0] = sq;
    int n = 1;
    while ((unsigned)n < 67) {
        sq = multmodp(model, sq, sq);       // x^2^n

        // If this value has already appeared, then done.
        for (int j = 0; j < n; j++)
            if (model->table_comb[j] == sq) {
                model->cycle = n;
                model->back = j;
                return 0;
            }

        // New value -- append to table.
        model->table_comb[n++] = sq;
    }

    // No cycle was found, up to the size of the table.
    model->cycle = n;
    model->back = -1;

#ifdef FIND_CYCLE
#   define GIVEUP 10000
    // Just out of curiosity, see when x^2^n cycles for this CRC.
    word_t comb[GIVEUP];
    for (int k = 0; k < n; k++)
        comb[k] = model->table_comb[k];
    while (n < GIVEUP) {
        sq = multmodp(model, sq, sq);
        for (int j = 0; j < n; j++)
            if (comb[j] == sq) {
                fprintf(stderr, "%s cycled at %u to %u\n",
                        model->name, n, j);
                return 0;
            }
        comb[n++] = sq;
    }
    fprintf(stderr, "%s never cycled?\n", model->name);
#endif

    return 0;
}

word_t crc_zeros(model_t *model, word_t crc, uintmax_t n) {
    // Pre-process the CRC.
    crc ^= model->xorout;
    if (model->rev)
        crc = reverse(crc, model->width);

    // Apply n zero bits to crc.
    if (n < 128) {
        word_t poly = model->poly;
        if (model->ref) {
            crc &= ONES(model->width);
            while (n--)
                crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
        }
        else {
            word_t mask = (word_t)1 << (model->width - 1);
            while (n--)
                crc = crc & mask ? (crc << 1) ^ poly : crc << 1;
            crc &= ONES(model->width);
        }
    }
    else {
        crc &= ONES(model->width);
        int k = 0;
        for (;;) {
            if (n & 1)
                crc = multmodp(model, model->table_comb[k], crc);
            n >>= 1;
            if (n == 0)
                break;
            if (++k == model->cycle) {
                assert(model->back != -1);
                k = model->back;
            }
        }
    }

    // Post-process and return the CRC.
    if (model->rev)
        crc = reverse(crc, model->width);
    return crc ^ model->xorout;
}

// Return x^(8n) modulo p(x), where p(x) is the CRC polynomial. model->cycle
// and model->table_comb[] must first be initialized by crc_table_combine().
static word_t x8nmodp(model_t *model, uintmax_t n) {
    word_t xp = model->ref ? (word_t)1 << (model->width - 1) : 1;   // x^0
    int k = model->cycle > 3 ? 3 :
            model->cycle == 3 ? model->back :
            model->cycle - 1;
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

word_t crc_combine(model_t *model, word_t crc1, word_t crc2,
                   uintmax_t len2) {
    crc1 ^= model->init;
    if (model->rev) {
        crc1 = reverse(crc1, model->width);
        crc2 = reverse(crc2, model->width);
    }
    word_t crc = multmodp(model, x8nmodp(model, len2), crc1) ^ crc2;
    if (model->rev)
        crc = reverse(crc, model->width);
    return crc;
}
