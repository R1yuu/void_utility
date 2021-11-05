#ifndef VA_VOID_ARRAY_H
#define VA_VOID_ARRAY_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
#define VARR_SUCCESS                0b0000
#define VARR_ERROR                  0b0001
#define VARR_ARRAY_404              0b0010
#define VARR_INDEX_OUT_OF_RANGE     0b0011

/**
 * Type which is used to index data
 */
#ifndef VARR_SIZE_TYPE
#define VARR_SIZE_TYPE size_t
#endif //VARR_SIZE_TYPE

/**
 * Void Array Structure
 */
struct void_array {
    /** byte array of all stored values */
    unsigned char* value_bytes;
    /** amount of currently stored values */
    VARR_SIZE_TYPE size; 
    /** allocated memory of the array */
    VARR_SIZE_TYPE capacity; 
    /** size of values in bytes */
    VARR_SIZE_TYPE value_size; 
    /** free function for recursive freeing on each value */
    void(*value_free_fn)(void*); 
};

/**
 * Doubles the memory size of a Void Array.
 * 
 * @param varray Void Array to expand
 */
void varr_expand(struct void_array* varray) {
    unsigned char* tmp = varray->value_bytes;
    if ((SIZE_MAX >> 1) > varray->capacity) {
        varray->capacity = (varray->capacity << 1) + 1;
    } else {
        varray->capacity = SIZE_MAX;
    }
    varray->value_bytes = (unsigned char*)calloc(varray->value_size, varray->capacity);
    memcpy(varray->value_bytes, tmp, varray->value_size * varray->size);
    free(tmp);
}

/**
 * Shrinks the memory size to the used size of a Void Array.
 * 
 * @param varray Void Array to shrink
 */
void varr_shrink(struct void_array* varray) {
    unsigned char* tmp = varray->value_bytes;
    varray->capacity = varray->size;
    varray->value_bytes = (unsigned char*)calloc(varray->value_size, varray->capacity);
    memcpy(varray->value_bytes, tmp, varray->value_size * varray->size);
    free(tmp);
}

/**
 * Copys given Value into Void Array.
 * 
 * @param varray Void Array to add element to
 * @param value Value to be copied into Void Array
 * @return Error Code
 */
int varr_add(struct void_array* varray, void* value) {
    if (varray) {
        if (varray->size == varray->capacity) {
            varr_expand(varray);
        }
        memcpy(varray->value_bytes + varray->value_size * varray->size, value, varray->value_size);
        varray->size++;
        return VARR_SUCCESS;
    }
    return VARR_ERROR ^ VARR_ARRAY_404;
}

/**
 * Gets Element from Void Array at given index.
 * 
 * @param varray Void Array to get element from
 * @param idx Index of the Element
 * @return Pointer to Element (NULL if idx out of range)
 */
void* varr_get(const struct void_array* varray, VARR_SIZE_TYPE idx) {
    if (idx < varray->size) {
        return varray->value_bytes + varray->value_size * idx;
    }
    return NULL;
}

/**
 * Removes Element from Void Array at given index.\n 
 * Calls varray->value_free_fn on value if available.
 * 
 * @param varray Void Array to delete element from
 * @param idx Index of Element to be removed
 * @return Error Code
 */
int varr_remove(struct void_array* varray, VARR_SIZE_TYPE idx) {
    if (varray) {
        if (idx < varray->size) {
            if (varray->value_free_fn) {
                varray->value_free_fn(varray->value_bytes + varray->value_size * idx);
            }
            // Moving Memory one to the right, thus deleting the given Element
            memmove(varray->value_bytes + varray->value_size * idx, varray->value_bytes + varray->value_size * idx + 1,
                    (varray->size - 1 - idx) * varray->value_size);
            memset(varray->value_bytes + (varray->value_size * varray->size), 0, varray->value_size);
            varray->size--;
            return VARR_SUCCESS;
        }
        return VARR_ERROR ^ VARR_INDEX_OUT_OF_RANGE;
        
    }
    return VARR_ERROR ^ VARR_ARRAY_404;
}

/**
 * Clears Void Array.\n 
 * Calls varray->value_free_fn on values if available.
 * 
 * @param varray Void Array to be cleared
 * @return Error Code
 */
int varr_clear(struct void_array* varray) {
    if (varray) {
        if (varray->value_free_fn) {
            for (VARR_SIZE_TYPE idx = 0; idx < varray->size; idx++) {
                varray->value_free_fn(varray->value_bytes + varray->value_size * idx);
            }
        }
        memset(varray->value_bytes, 0, varray->value_size * varray->size);
        varray->size = 0;
        return VARR_SUCCESS;
    }
    return VARR_ERROR ^ VARR_ARRAY_404;
}

/**
 * Initializes a Void Array.
 * 
 * @param varray Void Array to be initialized
 * @param init_capacity Initial Memory Capacity of the Void Array
 * @param value_size Size of Values to be saved (in Bytes)
 * @param value_free_fn Function to be called when freeing special datatypes (Nullable)
 * @return Error Code
 */
int varr_init(struct void_array* varray, VARR_SIZE_TYPE init_capacity, VARR_SIZE_TYPE value_size, void(*value_free_fn)(void*)) {
    if (varray) {
        varray->value_bytes = (unsigned char*)calloc(value_size, init_capacity);
        varray->capacity = init_capacity;
        varray->size = 0;
        varray->value_size = value_size;
        varray->value_free_fn = value_free_fn;
        return VARR_SUCCESS;
    }
    return VARR_ERROR ^ VARR_ARRAY_404;
}

/**
 * Frees Content of Void Arrays.\n 
 * Calls varray->value_free_fn on values if available.
 *
 * @param varray_ptr Void Array of which the content is to be freed
 */
void varr_free(void* varray_ptr) {
    struct void_array* varray = (struct void_array*)varray_ptr;
    if (varray->value_free_fn) {
        for (VARR_SIZE_TYPE idx = 0; idx < varray->size; idx++) {
            varray->value_free_fn(varray->value_bytes + varray->value_size * idx);
        }
    }
    free(varray->value_bytes);
}


#endif //VA_VOID_ARRAY_H
