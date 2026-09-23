#include "bits.h"

#define and_mask(c) (-(uint64_t)(c))

static uint64_t reflect(uint64_t x, uint8_t w) {
    x = ((x >> 32) & 0xffffffff) | ((x << 32) & 0xffffffff00000000);
    x = ((x >> 16) & 0xffff0000ffff) | ((x << 16) & 0xffff0000ffff0000);
    x = ((x >> 8) & 0xff00ff00ff00ff) | ((x << 8) & 0xff00ff00ff00ff00);
    x = ((x >> 4) & 0xf0f0f0f0f0f0f0f) | ((x << 4) & 0xf0f0f0f0f0f0f0f0);
    x = ((x >> 2) & 0x3333333333333333) | ((x << 2) & 0xcccccccccccccccc);
    x = ((x >> 1) & 0x5555555555555555) | ((x << 1) & 0xaaaaaaaaaaaaaaaa);
    return x >> (64 - w);
}

static uint64_t crc_initial(params_t *params, uint64_t crc) {
    crc ^= params->xorout;
    if(params->refin ^ params->refout) {
        crc = reflect(crc, params->width);
    }
    if(!params->refin) {
        crc <<= 64 - params->width;
    }
    return crc;
}

static uint64_t crc_final(params_t *params, uint64_t crc) {
    if(!params->refin) {
        crc >>= 64 - params->width;
    }
    if(params->refin ^ params->refout) {
        crc = reflect(crc, params->width);
    }
    return crc ^ params->xorout;
}

#include <stdio.h>

uint64_t crc_calc_bits(params_t *params, uint64_t crc, unsigned char const *buf, uint64_t len) {
    crc = crc_initial(params, crc);

    if(params->refin) {
        while(len >= 8) {
            crc = (crc >> 8) ^ params->table[(crc ^ *buf++) & 0xff];
            len -= 8;
        }

        if(len > 0) {
            uint8_t mask = (uint8_t)(0 - 1) >> (8 - len);
            crc ^= *buf & mask;
            while(len--) {
                crc = (crc >> 1) ^ (params->poly & and_mask(crc & 1));
            }
        }

    } else {
        while(len >= 8) {
            crc = (crc << 8) ^ params->table[(crc >> 56) ^ *buf++];
            len -= 8;
        }

        if(len) {
            uint8_t mask = (uint8_t)(0 - 1) << (8 - len);
            crc ^= (uint64_t)(*buf & mask) << 56;
            while(len--) {
                crc = (crc << 1) ^ (params->poly & and_mask(crc >> 63));
            }
        }
    }

    return crc_final(params, crc);
}

uint64_t crc_combine_constant_bits(params_t *params, uint64_t len) {
    uint64_t xp = params->refin ? (uint64_t)1 << 63 : 1;
    return crc_zeros(params, xp, len);
}