/* model.h -- Generic CRC parameter model routines
 * Copyright (C) 2014, 2016, 2017, 2020, 2021 Mark Adler
 * For conditions of distribution and use, see copyright notice in crcany.c.
 */

/*
* Edited by Hussain Al Marzooq
*/

/*
  Define a generic model for a CRC and interpret a description of CRC model
  using the standard set of parameters.
 */

#ifndef _MODEL_H_
#define _MODEL_H_

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

/* Verify that the number of bits in a char is eight, using limits.h. */
#if CHAR_BIT != 8
#  error The number of bits in a char must be 8 for this code.
#endif

/* Type to use for CRC calculations.  This should be the largest unsigned
   integer type available, to maximize the cases that can be computed.  word_t
   can be any unsigned integer type, except for unsigned char.  All of the
   algorithms here can process CRCs up to the size of a word_t.  The bit-wise
   algorithm here can process CRCs up to twice the size of a word_t. */
typedef uintmax_t word_t;

/* Determine the size of uintmax_t at pre-processor time.  (sizeof is not
   evaluated at pre-processor time.)  If word_t is instead set to an explicit
   size above, e.g. uint64_t, then #define WORDCHARS appropriately, e.g. as 8.
 */
#if UINTMAX_MAX == UINT16_MAX
#  define WORDCHARS 2
#elif UINTMAX_MAX == UINT32_MAX
#  define WORDCHARS 4
#elif UINTMAX_MAX == UINT64_MAX
#  define WORDCHARS 8
#else
#  error uintmax_t must be 2, 4, or 8 bytes for this code.
#endif

/* The number of bits in a word_t (assumes CHAR_BIT is 8). */
#define WORDBITS (WORDCHARS<<3)

/* Mask for the low n bits of a word_t (n must be greater than zero). */
#define ONES(n) (((word_t)0 - 1) >> (WORDBITS - (n)))

/* CRC description and tables, allowing for double-word CRCs.

   The description is based on Ross William's parameters, but with some changes
   to the parameters as described below.

   ref and rev are derived from refin and refout.  ref and rev must be 0 or 1.
   ref is the same as refin.  rev is true only if refin and refout are
   different.  rev true is very uncommon, and occurs in only one of the 72 CRCs
   in the RevEng catalogue.  When rev is false, the common case, ref true means
   that both the input and output are reflected. Reflected CRCs are quite
   common.

   init is different here as well, representing the CRC of a zero-length
   sequence, instead of the initial contents of the CRC register.

   poly is reflected for refin true.  xorout is reflected for refout true.

   The structure includes space for pre-computed tables used to speed up CRC
   and CRC combination calculations. table_byte[] and table_word[] are filled
   in by the crc_table_wordwise() routine, using the CRC parameters already
   defined in the structure. table_comb[] is filled in by crc_table_combine().
 */
typedef struct {
    unsigned short width;       /* number of bits in the CRC (the degree of the polynomial) */
    short cycle;                /* length of the table_comb[] cycle */
    short back;                 /* index of table_comb[] to cycle back to */
    char ref;                   /* if true, reflect input and output */
    char rev;                   /* if true, reverse output */
    word_t poly, poly_hi;       /* polynomial representation (sans x^width) */
    word_t init, init_hi;       /* CRC of a zero-length sequence */
    word_t xorout, xorout_hi;   /* final CRC is exclusive-or'ed with this */
    word_t check, check_hi;     /* CRC of the nine ASCII bytes "123456789" */
    word_t res, res_hi;         /* Residue of the CRC */
    word_t *table_comb;         /* table for CRC combination */
    word_t *table_byte;         /* table for byte-wise calculation */
    word_t *table_word;         /* tables for word-wise calculation */
    word_t *table_slice16;      /* tables for the slice16 calculation */
} model_t;

/*
   The parameters are "width", "poly", "init", "refin", "refout", "xorout",
   "check", "residue", "width", "poly", "init", "xorout", "check", and "residue"
   are non-negative integers, refin and refout must be "1" or "0".

   "width" is the number of bits in the CRC, referred to herein as n.  "poly"
   is the binary representation of the CRC polynomial, sans the x^n term.
   "poly" is never provided in a reflected form.  So x^16 + x^12 + x^5 + 1 is
   "width=16 poly=0x1021".

   "init" is the initial contents of the CRC register.  If "refin" is true,
   then the input bytes are reflected.  If "refout" is true, then the CRC
   register is reflected on output.  "xorout" is exclusive-ored with the CRC,
   after reflection if any. "check" is the CRC of the nine bytes "123456789"
   encoded in ASCII.  "name" is the name of the CRC.

   "width" can be as much as twice the number of bits in the word_t type, set
   here to the largest integer type available to the compiler (uintmax_t).  On
   most modern systems, this permits up to 128-bit CRCs.

   "poly", "init", "xorout", "check", and "residue" must all be less than 2^n.
   The least significant bit of "poly" must be one.

   Example (from the RevEng catalogue at
   http://reveng.sourceforge.net/crc-catalogue/all.htm ):

      width=16 poly=0x1021 init=0x0000 refin=true refout=true xorout=0x0000
          check=0x2189 residue=0x0000

   Processs values for use in crc routines -- note that this reflects the
   polynomial and init values for ready use in the crc routines if necessary,
   changes the meaning of init, and replaces refin and refout with the
   different meanings reflect and reverse (reverse is very rarely used)

   Returns the model. */
model_t get_model(unsigned short width, word_t poly, word_t init, char refin, char refout, word_t xorout, word_t check, word_t res);

/* Same as get_model but allows the width to be higher than 64 bits */
model_t get_model_dbl(unsigned short width, word_t poly_hi, word_t poly, word_t init_hi, word_t init,
                      char refin, char refout, word_t xorout_hi, word_t xorout, word_t check_hi, word_t check,
                      word_t res_hi, word_t res);

/* Deallocate the model's tables */
void free_model(model_t *model);

/* Return the reversal of the low n-bits of x.  1 <= n <= WORDBITS.  The high
   WORDBITS - n bits in x are ignored, and are set to zero in the returned
   result.  A table-driven implementation would be faster, but the speed of
   reverse() is of no consequence since it is used at most twice per crc()
   call.  Even then, it is only used in the rare case that refin and refout are
   different. */
word_t reverse(word_t x, unsigned n);

/* Return the reversal of the low n-bits of hi/lo in hi/lo.
   1 <= n <= WORDBITS*2. */
void reverse_dbl(word_t *hi, word_t *lo, unsigned n);

#endif
