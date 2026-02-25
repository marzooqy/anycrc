/* model.c -- Generic CRC parameter model routines
 * Copyright (C) 2014, 2016, 2017, 2020 Mark Adler
 * For conditions of distribution and use, see copyright notice in crcany.c.
 */

/*
* Edited by Hussain Al Marzooq
*/

#include <stdlib.h>
#include "model.h"

// See model.h
model_t get_model(unsigned short width, word_t poly, word_t init, char refin, char refout, word_t xorout) {
    return (model_t) {width, refin, refout, poly, init, xorout, NULL, NULL};
}

// See model.h.
char init_model(model_t *model) {
    if (model->ref)
        model->poly = reverse(model->poly, model->width);
    if (model->rev)
        model->init = reverse(model->init, model->width);

    model->rev ^= model->ref;
    model->init ^= model->xorout;

    model->table = (word_t*) malloc(16 * 256 * sizeof(word_t));
    if (model->table == NULL)
        return 1;

    model->table_comb = (word_t*) malloc(64 * sizeof(word_t));
    if (model->table_comb == NULL)
        return 1;

    return 0;
}

// See model.h.
void free_model(model_t *model) {
    free(model->table);
    free(model->table_comb);
}

// See model.h
word_t reverse(word_t x, unsigned n) {
    x = ((x >> 32) & 0xffffffff) | ((x << 32) & 0xffffffff00000000);
    x = ((x >> 16) & 0xffff0000ffff) | ((x << 16) & 0xffff0000ffff0000);
    x = ((x >> 8) & 0xff00ff00ff00ff) | ((x << 8) & 0xff00ff00ff00ff00);
    x = ((x >> 4) & 0xf0f0f0f0f0f0f0f) | ((x << 4) & 0xf0f0f0f0f0f0f0f0);
    x = ((x >> 2) & 0x3333333333333333) | ((x << 2) & 0xcccccccccccccccc);
    x = ((x >> 1) & 0x5555555555555555) | ((x << 1) & 0xaaaaaaaaaaaaaaaa);

    return x >> (64 - n);
}