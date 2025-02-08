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
   model->width less than or equal to WORDBITS.

   This routine and all of the crc routines here can process the data a chunk
   at a time. For example, this will calculate the CRC of the sequence chunk1,
   chunk2, chunk3:

      word_t crc = model.init;
      crc = crc_bitwise(model, crc, chunk1, len1);
      crc = crc_bitwise(model, crc, chunk2, len2);
      crc = crc_bitwise(model, crc, chunk3, len3);

   The final value of crc is the CRC of the chunks in sequence.  The first call
   of crc_bitwise() gets the initial CRC value for this model. */
EXPORT word_t crc_bitwise(model_t *model, word_t crc, void const *dat, size_t len);

/* Fill in the 256-entry table in model with the CRC of the bytes 0..255, for a
   byte-wise calculation of the given CRC model.  The table value is the
   internal CRC register contents after processing the byte.  If not reflected,
   then the CRC is pre-shifted left to the high end of WORDBITS so that the
   incoming byte can be exclusive-ored directly into a shifted CRC. */
EXPORT void crc_table_bytewise(model_t *model);

/* Equivalent to crc_bitwise(), but use a faster byte-wise table-based
   approach. This assumes that the first 256 entries in model->table
   have been initialized using crc_table_bytewise(). */
EXPORT word_t crc_bytewise(model_t *model, word_t crc, void const *dat, size_t len);

/* Fill in the table in model to support a slice-by-16 CRC calculation. This
   requires that the byte-wise table has already been initialized.

   The entry in table_word[(n << 8) | k] is the CRC register contents after processing
   a byte with value k, followed by n zero bytes. For non-reflected CRCs, the
   CRC is shifted up to the top of the word. */
EXPORT void crc_table_slice16(model_t *model);

/* Equivalent to crc_bitwise(), but use an even faster table-based approach.
   This assumes that model->table has been initialized using
   crc_table_bytewise() and crc_table_slice16(). */
EXPORT word_t crc_slice16(model_t *model, word_t crc, void const *dat, size_t len);

/* Fill in model->table_comb[n] for combining CRCs. Each entry is x raised to
   the 2 to the n power, modulo the CRC polynomial. Set model->cycle to the
   cycle length and model->back to the index to cycle back to. If the CRC did
   not cycle, then model->back is -1. In either case, model->cycle entries of
   model->table_comb[] are filled in. */
EXPORT void crc_table_combine(model_t *model);

/* Combine the CRC of the first portion of a message, crc1, with the CRC of the
   second portion, crc2, returning the CRC of the two concatenated. len2 is the
   length of the second portion of the message in bits. (The length of the
   first portion is not needed.) This assumes that model->table_comb has been
   initialized by crc_table_combine(). */
EXPORT word_t crc_combine(model_t *model, word_t crc1, word_t crc2, uintmax_t len2);

#endif