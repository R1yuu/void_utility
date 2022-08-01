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
#ifndef VA_VOID_ARRAY_H
#define VA_VOID_ARRAY_H

#include <stdlib.h>

/**
 * Error Codes
 */
#define VARR_SUCCESS                0
#define VARR_ERROR                  1
#define VARR_ARRAY_404              2
#define VARR_INDEX_OUT_OF_RANGE     3

/**
 * Void Array Structure
 */
struct void_array {
    void* value_bytes; /** byte array of all stored values */
    unsigned long size; /** amount of currently stored values */
    unsigned long capacity; /** allocated memory of the array */
    unsigned long value_size; /** size of values in bytes */
    void(*value_free_fn)(void*); /** free function for recursive freeing on each value */
};


/**
 * Doubles allocated memory of `varray->value_bytes` for `doublings` times
 * and increases `varray->capacity` accordingly.
 * 
 * @param varray Void Array to expand
 * @param doublings How often the capacity should be doubled
 */
void 
varr_expand(struct void_array* const varray, const unsigned long doublings);

/**
 * Shrinks allocated memory of `varray->value_bytes` to `varray->size`
 * and sets `varray->capacity` to `varray->size` accordingly.
 *
 * @param varray Void Array to shrink
 */
void 
varr_shrink(struct void_array* const varray);

/**
 * Copies `data` of length `n` onto the end of `varray->value_bytes`
 * and increases `varray->size` and `varray->capacity` accordingly.
 * 
 * \param varray Void Array to add element to
 * \param data Data to be copied into Void Array
 * \param n Amount of data to be added
 * \return Error Code
 */
int 
varr_add(struct void_array* const varray, const void* const data, const unsigned long n);

/**
 * Copies `data` of length `n` to the index `idx` of `varray->value_bytes`,
 * pushes potentially overlapping data of `varray->value_bytes` to
 * the right and increases `varray->size` and `varray->capacity` accordingly.
 * 
 * @param varray Void Array to add element to
 * @param idx Index to insert data at
 * @param data Data to be copied into Void Array
 * @param n Amount of data to be added
 * @return Error Code
 */
int
varr_insert(struct void_array* const varray, const unsigned long idx, const void* const data, const unsigned long n);

/**
 * Fills data in `varray->value_bytes` from `idx` to `idx + n` with 
 * copies of `data` assumed to be of length `varray->value_size`.
 * Overridden data is freed using `value_free_fn` if defined.
 *
 * \param varray Void Array to fill data
 * \param idx Starting index to fill data at
 * \param data Data copied to fill
 * \param n Amount of data to replace
 * \return Error Code
 */
int
varr_fill(struct void_array* const varray, const unsigned long idx, void* const data, const unsigned long n);

/**
 * Replaces data in `varray->value_bytes` from `idx` to `idx + n` 
 * with `data` of length `n`.
 * Overridden data is freed using `value_free_fn` if defined.
 *
 * @param varray Void Array to fill data
 * @param idx Starting index to fill data at
 * @param data Data copied to fill
 * @param n Amount of data to replace
 * @return Error Code
 */
int 
varr_replace(struct void_array* const varray, const unsigned long idx, void* const data, const unsigned long n);

/**
 * Returns pointer to data of internal array on location `idx`.
 * 
 * @param varray Void Array to get element from
 * @param idx Index of the Element
 * @return Pointer to Element (NULL if idx out of range)
 */
const void* 
varr_get(const struct void_array* const varray, const unsigned long idx);

/**
 * Deletes data from `idx` to `idx + n` from `varray->value_bytes`
 * and decreases `varray->size` accordingly.
 * Deleted data is freed using `value_free_fn` if defined.
 * 
 * @param varray Void Array to delete element from
 * @param idx Index of Element to be removed
 * @param n Amount of data to be removed
 * @return Error Code
 */
int 
varr_remove(struct void_array* const varray, const unsigned long idx, const size_t n);

/**
 * Deletes all data from `varray->value_bytes` and sets
 * `varray->size` to 0.
 * Deleted data is freed using `value_free_fn` if defined.
 * 
 * @param varray Void Array to be cleared
 * @return Error Code
 */
int 
varr_clear(struct void_array* const varray);

/**
 * Initializes a `void_array` structure.
 * If stored data doesn't need to be freed in any specific
 * manner (ie. non-pointers), `value_free_fn` is to be `NULL`.
 * 
 * @param varray Void Array to be initialized
 * @param init_capacity Initial Memory Capacity of the Void Array
 * @param value_size Size of Values to be saved (in Bytes)
 * @param value_free_fn Function to be called when freeing special datatypes (Nullable)
 * @return Error Code
 */
int 
varr_init(struct void_array* const varray, const unsigned long init_capacity, const size_t value_size, void(* const value_free_fn)(void*));

/**
 * Assumes `varray_ptr` is of type `struct void_array*`.
 * Frees all allocated data and sets all members of the `void_array`
 * to 0.
 * Deleted data is freed using `value_free_fn` if defined.
 *
 * @param varray_ptr Void Array of which the content is to be freed
 */
void 
varr_free(void* const varray_ptr);


#endif /* VA_VOID_ARRAY_H */
