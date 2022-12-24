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
#include "void_dict.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

size_t 
vdict_hash(const struct void_dict* vdict, const void* key) 
{
    const unsigned char* data = (const unsigned char*)key;
    unsigned char byte;
    size_t hash = 5381;

    for (size_t i = 0; i < vdict->key_size; i++) {
        if ((byte = *(data + i))) {
            hash = ((hash << 5) + hash) ^ byte;
        }
    }

    return hash % vdict->hash_pool;
}

size_t* 
vdict_hash_bsearch(const struct void_dict* vdict, const size_t hash, size_t* idx) 
{
    size_t* needle = NULL;
    size_t needle_idx = 0;
    if (vdict->size) {
        size_t f_idx = 0, l_idx = vdict->size - 1;
        while (f_idx <= l_idx) {
            needle_idx = (f_idx + l_idx) >> 1;
            if (vdict->hashes[needle_idx] < hash && needle_idx != 0) {
                f_idx = needle_idx + 1;
            } else if (vdict->hashes[needle_idx] > hash && needle_idx != 0) {
                l_idx = needle_idx - 1;
            } else {
                break;
            }
        }

        if (f_idx <= l_idx && vdict->hashes[needle_idx] == hash) {
            needle = vdict->hashes + needle_idx;
        }
    }
    if (idx != NULL) {
        *idx = needle_idx;
    }
    return needle;
}

int 
vdict_add_pair(struct void_dict* vdict, void* key, void* value) 
{
    if (vdict->size == vdict->hash_pool) {
        return VDICT_ERROR ^ VDICT_FULL;
    } else if (memcmp(key, vdict->key_zero_field, vdict->key_size) == 0) {
        return VDICT_ERROR ^ VDICT_ZERO_KEY;
    }
    size_t hash = vdict_hash(vdict, key);
    size_t needle_idx;
    while (vdict_hash_bsearch(vdict, hash, &needle_idx)) {
        hash = (hash + 1) % vdict->hash_pool;
    }
    memcpy((unsigned char*)vdict->key_bytes + hash * vdict->key_size, key, vdict->key_size);
    memcpy((unsigned char*)vdict->value_bytes + hash * vdict->value_size, value, vdict->value_size);
    if (vdict->size && vdict->hashes[needle_idx] < hash) needle_idx = (needle_idx + 1) % vdict->hash_pool;
    memmove(vdict->hashes + needle_idx + 1, vdict->hashes + needle_idx,
            sizeof(size_t) * (vdict->size - needle_idx));
    vdict->hashes[needle_idx] = hash;
    vdict->size++;
    return VDICT_SUCCESS;
}

void* 
vdict_get_value(const struct void_dict* vdict, const void* key) 
{
    size_t hash = vdict_hash(vdict, key);
    if (vdict->size && vdict_hash_bsearch(vdict, hash, NULL)) {
        unsigned char found = 0;
        size_t checked = 0;
        while (!found && checked < vdict->size) {
            if (memcmp((unsigned char*)vdict->key_bytes + vdict->key_size * hash, vdict->key_zero_field, vdict->key_size) == 0) {
                break;
            } else if (memcmp((unsigned char*)vdict->key_bytes + vdict->key_size * hash, key, vdict->key_size) == 0) {
                found = 1;
                break;
            } else {
                hash = (hash + 1) % vdict->hash_pool;
                checked++;
            }
        }

        if (found) {
            return (unsigned char*)vdict->value_bytes + vdict->value_size * hash;
        }
    }
    return NULL;
}

void* 
vdict_get_keys(const struct void_dict* vdict, void* key_array) 
{
    for (size_t i = 0; i < vdict->size; i++) {
        memcpy((unsigned char*)key_array + (vdict->key_size * i), (unsigned char*)vdict->key_bytes + (vdict->key_size * vdict->hashes[i]), vdict->key_size);
    }
    return key_array;
}

int 
vdict_del_pair(struct void_dict* vdict, const void* key) 
{
    unsigned char* value_ptr = (unsigned char*)vdict_get_value(vdict, key);
    if (value_ptr) {
        size_t hash = (value_ptr + vdict->value_size - (unsigned char*)vdict->value_bytes) / vdict->key_size;
        memset((unsigned char*)vdict->key_bytes + vdict->key_size * hash, 0, vdict->key_size);
        if (vdict->value_free_fn) {
            vdict->value_free_fn((unsigned char*)vdict->value_bytes + vdict->value_size * hash);
        }
        memset((unsigned char*)vdict->value_bytes + vdict->value_size * hash, 0, vdict->value_size);
        size_t hash_idx;
        size_t* hash_ptr = vdict_hash_bsearch(vdict, hash, &hash_idx);
        memmove(hash_ptr, hash_ptr + 1,
                sizeof(size_t) * (vdict->size - hash_idx));
        memset(vdict->hashes + vdict->size, 0, sizeof(size_t));
        vdict->size--;
        return VDICT_SUCCESS;
    } else {
        return VDICT_ERROR ^ VDICT_KEY_404;
    }
}

int 
vdict_clear(struct void_dict* vdict) 
{
    if (vdict) {
        memset(vdict->key_bytes, 0, vdict->key_size * vdict->hash_pool);
        if (vdict->value_free_fn) {
            for (size_t hash_idx = 0; hash_idx < vdict->size; hash_idx++) {
                vdict->value_free_fn((unsigned char*)vdict->value_bytes + vdict->value_size * vdict->hashes[hash_idx]);
            }
        }
        memset(vdict->value_bytes, 0, vdict->value_size * vdict->hash_pool);
        memset(vdict->hashes, 0, sizeof(size_t) * vdict->size);
        vdict->size = 0;
        return VDICT_SUCCESS;
    }
    return VDICT_ERROR ^ VDICT_DICT_404;
}

int 
vdict_init(struct void_dict* vdict, size_t hash_pool, size_t key_size, size_t value_size, void(*value_free_fn)(void*)) 
{
    if (vdict) {
        vdict->key_bytes = calloc(key_size, hash_pool);
        vdict->key_zero_field = calloc(key_size, hash_pool);
        vdict->value_bytes = calloc(value_size, hash_pool);
        vdict->hashes = (size_t*)calloc(sizeof(size_t), hash_pool);
        vdict->size = 0;
        vdict->key_size = key_size;
        vdict->value_size = value_size;
        vdict->hash_pool = hash_pool;
        vdict->value_free_fn = value_free_fn;
        return VDICT_SUCCESS;
    }
    return VDICT_ERROR ^ VDICT_DICT_404;
}

void 
vdict_free(void* vdict_ptr) 
{
    struct void_dict* vdict = (struct void_dict*)vdict_ptr;
    if (vdict->value_free_fn) {
        for (size_t hash_idx = 0; hash_idx < vdict->size; hash_idx++) {
            vdict->value_free_fn((unsigned char*)vdict->value_bytes + vdict->value_size * vdict->hashes[hash_idx]);
        }
    }
    free(vdict->key_bytes);
    free((void*)vdict->key_zero_field);
    free(vdict->value_bytes);
    free(vdict->hashes);
    vdict->key_bytes = NULL;
    vdict->key_zero_field = NULL;
    vdict->value_bytes = NULL;
    vdict->hashes = NULL;
    vdict->hash_pool = 0;
    vdict->size = 0;
    vdict->value_free_fn = NULL;
    vdict->value_size = 0;
    vdict->key_size = 0;
}
