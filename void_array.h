#ifndef VA_VOID_ARRAY_H
#define VA_VOID_ARRAY_H

#include <stdlib.h>

/**
 * Every Return Code != 0 is an Error
 * The Error Codes are Xor'ed
 * If you want to check if a specific Error Code is included you 
 * can check be Xor'ing with VARR_ERROR
 * 
 * Examples:
 * (VARR_ERROR ^ VARR_ARRAY_404) ^ VARR_ERROR == VARR_ARRAY_404
 * (VARR_ERROR ^ VARR_ARRAY_404 ^ VARR_INDEX_OUT_OF_RANGE) ^ VARR_ERROR ^ VARR_INDEX_OUT_OF_RANGE == VARR_ARRAY_404
 */
#define VARR_SUCCESS                0
#define VARR_ERROR                  1
#define VARR_ARRAY_404              2
#define VARR_INDEX_OUT_OF_RANGE     3

/**
 * Void Array Structure
 */
struct void_array {
    /** byte array of all stored values */
    void* value_bytes;
    /** amount of currently stored values */
    size_t size; 
    /** allocated memory of the array */
    size_t capacity; 
    /** size of values in bytes */
    size_t value_size; 
    /** free function for recursive freeing on each value */
    void(*value_free_fn)(void*); 
};


/**
 * Doubles the memory size of a Void Array.
 * 
 * @param varray Void Array to expand
 * @param doublings How often the capacity should be doubled
 */
void 
varr_expand(struct void_array* varray, size_t doublings);

/**
 * Shrinks the memory size to the used size of a Void Array.
 * 
 * @param varray Void Array to shrink
 */
void 
varr_shrink(struct void_array* varray);

/**
 * Copys given Data into Void Array.
 * 
 * @param varray Void Array to add element to
 * @param data Data to be copied into Void Array
 * @param n Amount of data to be added
 * @return Error Code
 */
int 
varr_add(struct void_array* varray, void* data, size_t n);

/**
 * Gets Element from Void Array at given index.
 * 
 * @param varray Void Array to get element from
 * @param idx Index of the Element
 * @return Pointer to Element (NULL if idx out of range)
 */
void* 
varr_get(const struct void_array* varray, size_t idx);

/**
 * Removes Element from Void Array at given index.\n 
 * Calls varray->value_free_fn on value if available.
 * 
 * @param varray Void Array to delete element from
 * @param idx Index of Element to be removed
 * @param n Amount of data to be removed
 * @return Error Code
 */
int 
varr_remove(struct void_array* varray, size_t idx, size_t n);

/**
 * Clears Void Array.\n 
 * Calls varray->value_free_fn on values if available.
 * 
 * @param varray Void Array to be cleared
 * @return Error Code
 */
int 
varr_clear(struct void_array* varray);

/**
 * Initializes a Void Array.
 * 
 * @param varray Void Array to be initialized
 * @param init_capacity Initial Memory Capacity of the Void Array
 * @param value_size Size of Values to be saved (in Bytes)
 * @param value_free_fn Function to be called when freeing special datatypes (Nullable)
 * @return Error Code
 */
int 
varr_init(struct void_array* varray, size_t init_capacity, size_t value_size, void(*value_free_fn)(void*));

/**
 * Frees Content of Void Arrays.\n 
 * Calls varray->value_free_fn on values if available.
 *
 * @param varray_ptr Void Array of which the content is to be freed
 */
void 
varr_free(void* varray_ptr);


#endif /* VA_VOID_ARRAY_H */
