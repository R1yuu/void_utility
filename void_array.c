#include "void_array.h"

#include <stdint.h>
#include <string.h>

void 
varr_expand(struct void_array* varray, size_t doublings) 
{
    if (doublings > 0) {
        void* tmp = varray->value_bytes;
        if ((SIZE_MAX >> 1) > varray->capacity) {
            varray->capacity = varray->capacity << doublings;
        } else {
            varray->capacity = SIZE_MAX;
        }
        varray->value_bytes = calloc(varray->value_size, varray->capacity);
        memcpy(varray->value_bytes, tmp, varray->value_size * varray->size);
        free(tmp);
    }
}

void 
varr_shrink(struct void_array* varray) 
{
    void* tmp = varray->value_bytes;
    varray->capacity = varray->size;
    varray->value_bytes = calloc(varray->value_size, varray->capacity);
    memcpy(varray->value_bytes, tmp, varray->value_size * varray->size);
    free(tmp);
}

int 
varr_add(struct void_array* varray, void* data, size_t n) 
{
    if (varray) {
        varr_expand(varray, (size_t)((varray->size + n) / varray->capacity));
        memcpy((unsigned char*)varray->value_bytes + varray->value_size * varray->size, data, varray->value_size * n);
        varray->size += n;
        return VARR_SUCCESS;
    }
    return VARR_ERROR ^ VARR_ARRAY_404;
}

void*
varr_get(const struct void_array* varray, size_t idx) 
{
    if (idx < varray->size) {
        return (unsigned char*)varray->value_bytes + varray->value_size * idx;
    }
    return NULL;
}

int 
varr_remove(struct void_array* varray, size_t idx, size_t n) 
{
    if (varray) {
        if (idx < varray->size) {
            if (varray->value_free_fn) {
                for (size_t i=0; i < n; i++) {
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
varr_clear(struct void_array* varray) 
{
    if (varray) {
        if (varray->value_free_fn) {
            for (size_t idx = 0; idx < varray->size; idx++) {
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
varr_init(struct void_array* varray, size_t init_capacity, size_t value_size, void(*value_free_fn)(void*)) 
{
    if (varray) {
        varray->value_bytes = calloc(value_size, init_capacity);
        varray->capacity = init_capacity;
        varray->size = 0;
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
        for (size_t idx = 0; idx < varray->size; idx++) {
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
