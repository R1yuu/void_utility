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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include "void_array.h"
#include "void_dict.h"

#define INIT_CAPACITY 5


static void 
void_array_test() 
{
    struct void_array varray;
    unsigned ret_value;

    unsigned add_values[INIT_CAPACITY + 1] = {0};
    srand(time(NULL));
    for (size_t i=0; i < INIT_CAPACITY + 1; i++) {
        add_values[i] = rand();
    }

    assert(varr_init(&varray, INIT_CAPACITY, sizeof(unsigned), NULL) == VARR_SUCCESS);
    assert(varray.capacity == INIT_CAPACITY);
    assert(varray.size == 0);
    assert(varray.value_size == sizeof(unsigned));
    assert(varray.value_bytes != NULL);
    assert(varray.value_free_fn == NULL);
    printf("%25s%15s\n", "varr_init", "success");

    assert(varr_add(&varray, add_values, 1) == VARR_SUCCESS);
    assert(varray.size == 1);
    assert(*(int*)varray.value_bytes == add_values[0]);
    assert(varray.capacity == INIT_CAPACITY);
    printf("%25s%15s\n", "varr_add - no expand", "success");

    assert(varr_add(&varray, add_values + 1, INIT_CAPACITY) == VARR_SUCCESS);
    assert(varray.size == INIT_CAPACITY + 1);
    assert(varray.capacity == INIT_CAPACITY << 1);
    assert(!memcmp(varray.value_bytes, add_values, INIT_CAPACITY + 1));
    printf("%25s%15s\n", "varr_add - 1 expand", "success");

    varr_expand(&varray, 2);
    assert(varray.size == INIT_CAPACITY + 1);
    assert(varray.capacity == INIT_CAPACITY << 3);
    printf("%25s%15s\n", "varr_expand - 2 expands", "success");

    varr_shrink(&varray);
    assert(varray.size == INIT_CAPACITY + 1);
    assert(varray.capacity == INIT_CAPACITY + 1);
    printf("%25s%15s\n", "varr_shrink", "success");

    ret_value = *(unsigned*)varr_get(&varray, INIT_CAPACITY >> 1);
    assert(ret_value == *(unsigned*)((unsigned char*)varray.value_bytes + sizeof(unsigned) * (INIT_CAPACITY >> 1)));
    assert(ret_value == add_values[INIT_CAPACITY >> 1]);
    printf("%25s%15s\n", "varr_get", "success");

    assert(varr_remove(&varray, INIT_CAPACITY >> 1, 1) == VARR_SUCCESS);
    assert(varray.size == INIT_CAPACITY);
    assert(varray.capacity == INIT_CAPACITY + 1);
    assert(ret_value != *(unsigned*)varr_get(&varray, INIT_CAPACITY >> 1));
    printf("%25s%15s\n", "varr_remove - 1", "success");

    unsigned fill = 69;
    unsigned* fill_arr = malloc(varray.value_size * varray.capacity);
    for (size_t i=0; i < varray.capacity; i++) {
        memcpy(fill_arr + i, &fill, varray.value_size);
    }
    assert(varr_fill(&varray, 0, &fill, varray.capacity) == VARR_SUCCESS);
    assert(!memcmp(varray.value_bytes, fill_arr, varray.value_size * varray.capacity));
    free(fill_arr);
    printf("%25s%15s\n", "varr_fill", "success");

    assert(varr_remove(&varray, INIT_CAPACITY >> 1, 2) == VARR_SUCCESS);
    assert(varray.size == varray.capacity - 2);
    assert(varray.capacity == INIT_CAPACITY + 1);
    printf("%25s%15s\n", "varr_remove - 2", "success");

    assert(varr_clear(&varray) == VARR_SUCCESS);
    assert(varray.size == 0);
    assert(varray.capacity == INIT_CAPACITY + 1);
    void* empty_mem = calloc(varray.capacity, varray.value_size);
    assert(!memcmp(varray.value_bytes, empty_mem, varray.value_size * varray.capacity));
    free(empty_mem);
    printf("%25s%15s\n", "varr_clear", "success");

	varr_free(&varray);
    assert(varray.value_free_fn == NULL);
    assert(varray.value_bytes == NULL);
    assert(varray.value_size == 0);
    assert(varray.size == 0);
    assert(varray.capacity == 0);
    printf("%25s%15s\n", "varr_free", "success");
}

static void
void_dict_test() {
    struct void_dict vdict;

    assert(vdict_init(&vdict, 7, sizeof(char) * 10, sizeof(double), NULL) == VDICT_SUCCESS);
    assert(vdict.hash_pool == 7);
    assert(vdict.key_size == sizeof(char) * 10);
    assert(vdict.value_size == sizeof(double));
    assert(vdict.value_free_fn == NULL);
    assert(vdict.size == 0);
    assert(vdict.value_bytes != NULL);
    assert(vdict.hashes != NULL);
    assert(vdict.key_zero_field != NULL);
    assert(vdict.key_bytes != NULL);
    printf("%25s%15s\n", "vdict_init", "success");

    double new_val = 3.1415926;
    assert(vdict_add_pair(&vdict, "test1", &new_val) == VDICT_SUCCESS);
    assert(vdict.size == 1);
    printf("%25s%15s\n", "vdict_add", "success");

    assert(*(double*)vdict_get_value(&vdict, "test1") == new_val);
    printf("%25s%15s\n", "vdict_get_value", "success");

    void* keys = malloc(vdict.key_size);
    assert(vdict_get_keys(&vdict, keys) != NULL);
    assert(!memcmp(keys, "test1\0\0\0\0", vdict.key_size));
    free(keys);
    printf("%25s%15s\n", "vdict_get_keys", "success");

    assert(vdict_del_pair(&vdict, "test1") == VDICT_SUCCESS);

    vdict_free(&vdict);
    assert(vdict.hash_pool == 0);
    assert(vdict.key_size == 0);
    assert(vdict.value_size == 0);
    assert(vdict.value_free_fn == NULL);
    assert(vdict.size == 0);
    assert(vdict.value_bytes == NULL);
    assert(vdict.hashes == NULL);
    assert(vdict.key_zero_field == NULL);
    assert(vdict.key_bytes == NULL);
    printf("%25s%15s\n", "vdict_free", "success");
}

int 
main() 
{
    printf("void_array_test:\n");
	void_array_test();
    printf("void_dict_test:\n");
	void_dict_test();

    return 0;
}
