/* crc.h -- Generic CRC calculations
 * Copyright (C) 2014, 2016, 2017, 2020, 2021 Mark Adler
 * For conditions of distribution and use, see copyright notice in crcany.c.
 */

/*
* Edited by Hussain Al Marzooq
*/

#ifndef _CRC_H_
#define _CRC_H_

#include "model.h"

/* Apply the len bits at dat to crc using the CRC described in model. If dat
   is NULL, then return the initial CRC for this model.  This allows for the
   calculation of a CRC in pieces, where the first call must be with crc equal
   to the initial value for this CRC model. crc_bitwise() must only be used for
   model->width less than or equal to WORDBITS.  Otherwise use
   crc_bitwise_dbl().

   This routine and all of the crc routines here can process the data a chunk
   at a time. For example, this will calculate the CRC of the sequence chunk1,
   chunk2, chunk3:

      word_t crc;
      crc = crc_bitwise(model, 0, NULL, 0);
      crc = crc_bitwise(model, crc, chunk1, len1);
      crc = crc_bitwise(model, crc, chunk2, len2);
      crc = crc_bitwise(model, crc, chunk3, len3);

   The final value of crc is the CRC of the chunks in sequence.  The first call
   of crc_bitwise() gets the initial CRC value for this model.
 */
word_t crc_bitwise(model_t *model, word_t crc, void const *dat, size_t len);

/* Fill in the 256-entry table in model with the CRC of the bytes 0..255, for a
   byte-wise calculation of the given CRC model.  The table value is the
   internal CRC register contents after processing the byte.  If not reflected
   and the CRC width is less than 8, then the CRC is pre-shifted left to the
   high end of the low 8 bits so that the incoming byte can be exclusive-ored
   directly into a shifted CRC. */
int crc_table_bytewise(model_t *model);

/* Equivalent to crc_bitwise(), but use a faster byte-wise table-based
   approach. This assumes that model->table_byte has been initialized using
   crc_table_bytewise(). */
word_t crc_bytewise(model_t *model, word_t crc, void const *dat, size_t len);

/* Fill in the table in model to support a word-wise CRC calculation. This
   requires that the byte-wise table has already been initialized.

   The entry in table_word[n][k] is the CRC register contents after processing
   a byte with value k, followed by n zero bytes. For non-reflected CRCs, the
   CRC is shifted up to the top of the word. The CRC is byte-swapped as needed
   so that the first byte of the CRC to be shifted out is in the same place in
   the word_t as the first byte that comes from memory. */
int crc_table_slice16(model_t *model);

/* Equivalent to crc_bitwise(), but use an even faster word-wise table-based
   approach.  This assumes that model->table_byte and model->table_word have
   been initialized using crc_table_bytewise() and crc_table_slice16(). */
word_t crc_slice16(model_t *model, word_t crc, void const *dat, size_t len);

#endif
