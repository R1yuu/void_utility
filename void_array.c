/*
    Copyright (C) 2022  Andre Schneider

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License Version 2.1 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License Version 2.1 for more details.

    You should have received a copy of the GNU Lesser General Public
    License Version 2.1 along with this library; if not, 
    write to <andre.schneider@outlook.at>.
*/
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "void_array.h"

static const unsigned long mbi = sizeof(unsigned long) * CHAR_BIT;
static const unsigned long xbi = mbi >> 4;

static const unsigned long mb = -1;
static const unsigned long hb = 1ul << (mbi >> 1);
static const unsigned long qb = 1ul << (mbi >> 2);
static const unsigned long ob = 1ul << (mbi >> 3);
static const unsigned long xb = 1ul << (mbi >> 4);

static inline unsigned long 
ceiled_log2(register const unsigned long val) 
{
    register unsigned long sb = (xbi * (xb <= val)) << (
                                (ob <= val) +
                                (qb <= val) +
                                (hb <= val) +
                                (mb == val)
                         );

    if (sb != mbi) {
        for (register unsigned long cbm=mbi >> 5; cbm >= 2; cbm >>= 1) {
            while (1ul << (sb + cbm) < val) sb += cbm;
        }
        
        while (1ul << sb < val) sb++;
    }
    return sb;
}

static inline unsigned long 
needed_doublings(const struct void_array* const varray, const unsigned long n) 
{
    return ceiled_log2((varray->size + n) / varray->capacity + 1);
}

void 
varr_expand(struct void_array* const varray, const unsigned long doublings) 
{
    if (doublings > 0) {
        void* tmp = varray->value_bytes;
        if (((1ul - 1) >> 1) > varray->capacity) {
            varray->capacity = varray->capacity << doublings;
        } else {
            varray->capacity = -1;
        }
        varray->value_bytes = calloc(varray->value_size, varray->capacity);
        memcpy(varray->value_bytes, tmp, varray->value_size * varray->size);
        free(tmp);
    }
}

void 
varr_shrink(struct void_array* const varray) 
{
    void* tmp = varray->value_bytes;
    varray->capacity = varray->size;
    varray->value_bytes = calloc(varray->value_size, varray->capacity);
    memcpy(varray->value_bytes, tmp, varray->value_size * varray->size);
    free(tmp);
}

int 
varr_add(struct void_array* const varray, const void* const data, const unsigned long n) 
{
    if (varray) {
        varr_expand(varray, needed_doublings(varray, n));
        memcpy((unsigned char*)varray->value_bytes + varray->value_size * varray->size, data, varray->value_size * n);
        varray->size += n;
        return VARR_SUCCESS;
    }
    return VARR_ERROR ^ VARR_ARRAY_404;
}

int
varr_insert(struct void_array* const varray, const unsigned long idx, const void* const data, const unsigned long n)
{
    if (varray) {
        if (idx < varray->size) {
            varr_expand(varray, needed_doublings(varray, n));
            if (idx != varray->size - 1) {
                memmove((unsigned char*)varray->value_bytes + varray->value_size * (idx + n), (unsigned char*)varray->value_bytes + varray->value_size * idx, varray->value_size * n);
            }
            memcpy((unsigned char*)varray->value_bytes + varray->value_size * idx, data, varray->value_size * n);
            varray->size += n;
            return VARR_SUCCESS;
        }
        return VARR_ERROR ^ VARR_INDEX_OUT_OF_RANGE;
    }
    return VARR_ERROR ^ VARR_ARRAY_404;
}

int
varr_fill(struct void_array* const varray, const unsigned long idx, void* const data, const unsigned long n) 
{
    if (varray) {
        if (idx <= varray->size && idx + n <= varray->capacity) {
            for (unsigned long i=0; i < n; i++) {
                if (varray->value_free_fn) {
                    varray->value_free_fn((unsigned char*)varray->value_bytes + varray->value_size * (idx + i));
                }
                memcpy((unsigned char*)varray->value_bytes + varray->value_size * (idx + i), data, varray->value_size);
            }
            varray->size = idx + n;
            return VARR_SUCCESS;
        }
        return VARR_ERROR ^ VARR_INDEX_OUT_OF_RANGE;
    }
    return VARR_ERROR ^ VARR_ARRAY_404;
}

int
varr_replace(struct void_array* const varray, const unsigned long idx, void* const data, const unsigned long n)
{
    if (varray) {
        if (idx + n <= varray->size) {
            if (varray->value_free_fn) {
                for (unsigned long i=0; i < n; i++) {
                    varray->value_free_fn((unsigned char*)varray->value_bytes + varray->value_size * (idx + i));
                }
            }
            memcpy((unsigned char*)varray->value_bytes + varray->value_size * idx, data, varray->value_size * n);
            return VARR_SUCCESS;
        }
        return VARR_ERROR ^ VARR_INDEX_OUT_OF_RANGE;
    }
    return VARR_ERROR ^ VARR_ARRAY_404;
}

const void*
varr_get(const struct void_array* const varray, const unsigned long idx) 
{
    if (idx < varray->size) {
        return (unsigned char*)varray->value_bytes + varray->value_size * idx;
    }
    return NULL;
}

int 
varr_remove(struct void_array* const varray, const unsigned long idx, const unsigned long n) 
{
    if (varray) {
        if (idx + n <= varray->size) {
            if (varray->value_free_fn) {
                for (unsigned long i=0; i < n; i++) {
                    varray->value_free_fn((unsigned char*)varray->value_bytes + varray->value_size * (idx + i));
                }
            }

            if (idx != varray->size - 1) {
                /* Moving Memory one to the left, thus deleting the given Element */
                memmove((unsigned char*)varray->value_bytes + varray->value_size * idx, 
                    (unsigned char*)varray->value_bytes + varray->value_size * (idx + n),
                    varray->value_size * (varray->size - idx - n));
            }
            varray->size -= n;
            memset((unsigned char*)varray->value_bytes + (varray->value_size * varray->size), 0, varray->value_size * n);
            return VARR_SUCCESS;
        }
        return VARR_ERROR ^ VARR_INDEX_OUT_OF_RANGE;
        
    }
    return VARR_ERROR ^ VARR_ARRAY_404;
}

int 
varr_clear(struct void_array* const varray) 
{
    if (varray) {
        if (varray->value_free_fn) {
            for (unsigned long idx = 0; idx < varray->size; idx++) {
                varray->value_free_fn((unsigned char*)varray->value_bytes + varray->value_size * idx);
            }
        }
        memset(varray->value_bytes, 0, varray->value_size * varray->size);
        varray->size = 0;
        return VARR_SUCCESS;
    }
    return VARR_ERROR ^ VARR_ARRAY_404;
}

int 
varr_init(struct void_array* const varray, const unsigned long init_capacity, const unsigned long value_size, void(* const value_free_fn)(void*)) 
{
    if (varray) {
        varray->value_bytes = calloc(value_size, init_capacity);
        varray->size = 0;
        varray->capacity = init_capacity;
        varray->value_size = value_size;
        varray->value_free_fn = value_free_fn;
        return VARR_SUCCESS;
    }
    return VARR_ERROR ^ VARR_ARRAY_404;
}

void 
varr_free(void* varray_ptr) 
{
    struct void_array* varray = (struct void_array*)varray_ptr;
    if (varray->value_free_fn) {
        for (unsigned long idx = 0; idx < varray->size; idx++) {
            varray->value_free_fn((unsigned char*)varray->value_bytes + varray->value_size * idx);
        }
    }
    free(varray->value_bytes);
    varray->value_bytes = NULL;
    varray->capacity = 0;
    varray->size = 0;
    varray->value_size = 0;
    varray->value_free_fn = NULL;
}
