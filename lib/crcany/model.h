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

/* Type to use for CRC calculations.  This should be the largest unsigned
   integer type available, to maximize the cases that can be computed.
   All of the algorithms here can process CRCs up to the size of a word_t. */
typedef uint64_t word_t;

/* The number of bytes in a word_t. */
#define WORDCHARS 8

/* The number of bits in a word_t. */
#define WORDBITS (WORDCHARS * 8)

/* CRC description and tables

   The description is based on Ross William's parameters, but with some changes
   to the parameters as described below.

   ref and rev are derived from refin and refout.  ref and rev must be 0 or 1.
   ref is the same as refin.  rev is true only if refin and refout are
   different.  rev true is very uncommon, and occurs in only one of the CRCs
   in the RevEng catalogue.  When rev is false, the common case, ref true means
   that both the input and output are reflected. Reflected CRCs are quite
   common.

   init is different here as well, representing the CRC of a zero-length
   sequence, instead of the initial contents of the CRC register.

   poly is reflected for refin true.  init is reflected for refout true.

   The structure includes space for pre-computed tables used to speed up CRC
   and CRC combination calculations. table_byte[] and table_word[] are filled
   in by the crc_table_wordwise() routine, using the CRC parameters already
   defined in the structure.
*/
typedef struct {
    unsigned short width;     /* number of bits in the CRC (the degree of the polynomial) */
    short cycle;              /* length of the table_comb[] cycle */
    short back;               /* index of table_comb[] to cycle back to */
    char ref;                 /* if true, reflect input and output */
    char rev;                 /* if true, reverse output */
    word_t poly;              /* polynomial representation (sans x^width) */
    word_t init;              /* CRC of a zero-length sequence */
    word_t xorout;            /* final CRC is exclusive-or'ed with this */
    word_t *table;            /* table for the CRC calculation */
    word_t *table_comb;       /* table for CRC combination */
} model_t;

/*
   Returns a model initialized with the specified parameters

   The parameters are "width", "poly", "init", "refin", "refout", "xorout".
   "width", "poly", "init", "xorout" are non-negative integers,
   refin and refout must be "1" or "0".

   "width" is the number of bits in the CRC, referred to herein as n.  "poly"
   is the binary representation of the CRC polynomial, sans the x^n term.
   "poly" is never provided in a reflected form.  So x^16 + x^12 + x^5 + 1 is
   "width=16 poly=0x1021".

   "init" is the initial contents of the CRC register.  If "refin" is true,
   then the input bytes are reflected.  If "refout" is true, then the CRC
   register is reflected on output.  "xorout" is exclusive-ored with the CRC,
   after reflection if any.

   "width" can be as much as WORDBITS bits.

   "poly", "init", and "xorout" must all be less than 2^n.
   The least significant bit of "poly" must be one.

   Example (from the RevEng catalogue at
   http://reveng.sourceforge.net/crc-catalogue/all.htm):

   width=16 poly=0x1021 init=0x0000 refin=true refout=true xorout=0x0000
*/
model_t get_model(unsigned short width, word_t poly, word_t init, char refin, char refout, word_t xorout);

/* Processs values for use in crc routines -- note that this reflects the
   polynomial and init values for ready use in the crc routines if necessary,
   changes the meaning of init, and replaces refin and refout with the
   different meanings reflect and reverse (reverse is very rarely used).
   Returns 1 if the allocation of the model's table fails. */
char init_model(model_t *model);

/* Deallocate the model's tables */
void free_model(model_t *model);

/* Return the reversal of the low n-bits of x. 1 <= n <= WORDBITS. The high
   WORDBITS - n bits in x are ignored, and are set to zero in the returned
   result. reverse() is only used if refin and refout are different. */
word_t reverse(word_t x, unsigned n);

#endif