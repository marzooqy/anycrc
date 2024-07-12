/* crcdbl.h -- Generic bit-wise CRC calculation for a double-wide CRC
 * Copyright (C) 2014, 2016, 2017 Mark Adler
 * For conditions of distribution and use, see copyright notice in crcany.c.
 */

/*
* Edited by Hussain Al Marzooq
*/

#ifndef _CRCDBL_H_
#define _CRCDBL_H_

#include "model.h"

/* Similar to crc_bitwise(), but works for CRCs up to twice as long as a
   word_t. This processes long CRCs stored in two word_t values, *crc_hi and
   *crc_lo. The final CRC is returned in *crc_hi and *crc_lo.  If buf is NULL,
   then return the initial CRC for this model.  This allows for the calculation
   of a CRC in pieces, but the first call must be with the initial value for
   this CRC model.  This calls crc_bitwise() for short CRC models.  For long
   CRC models, this does the same thing crc_bitwise() does, but with the shift
   and exclusive-or operations extended across two word_t's.

   An example to compute the CRC of three chunks in sequence:

     word_t hi, lo;
     crc_bitwise_dbl(model, &hi, &lo, NULL, 0);
     crc_bitwise_dbl(model, &hi, &lo, chunk1, len1);
     crc_bitwise_dbl(model, &hi, &lo, chunk2, len2);
     crc_bitwise_dbl(model, &hi, &lo, chunk3, len3);

   The CRC of the sequence is left in hi, lo.
 */
dword_t crc_bitwise_dbl(model_t *, dword_t, unsigned char const *, size_t);

/* Fill in the 256-entry tables in model with the CRC of the bytes 0..255, for a
   byte-wise calculation of the given CRC model.  The table value is the
   internal CRC register contents after processing the byte.  If not reflected
   and the CRC width is less than 8, then the CRC is pre-shifted left to the
   high end of the low 8 bits so that the incoming byte can be exclusive-ored
   directly into a shifted CRC. */
int crc_table_bytewise_dbl(model_t *);

/* Equivalent to crc_bitwise(), but use a faster byte-wise table-based
   approach. This assumes that model->table_byte and model->table_byte_hi
   has been initialized using crc_table_bytewise_dbl(). */
dword_t crc_bytewise_dbl(model_t *, dword_t, unsigned char const *, size_t);

#endif
