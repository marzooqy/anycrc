#ifndef CRC_BITS_H
#define CRC_BITS_H

#include "../../lib/crc-clmul/crc.h"

uint64_t crc_calc_bits(params_t *params, uint64_t crc, unsigned char const *buf, uint64_t len);
uint64_t crc_combine_constant_bits(params_t *params, uint64_t len);

#endif