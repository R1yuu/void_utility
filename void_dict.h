#ifndef VD_VOID_DICT_H
#define VD_VOID_DICT_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/**
 * Every Return Code != 0 is an Error
 * The Error Codes are Xor'ed
 * If you want to check if a specific Error Code is included you 
 * can check be Xor'ing with VDICT_ERROR
 * 
 * Examples:
 * (VDICT_ERROR ^ VDICT_DICT_404) ^ VDICT_ERROR == VDICT_DICT_404
 * (VDICT_ERROR ^ VDICT_DICT_404 ^ VDICT_ZERO_KEY) ^ VDICT_ERROR ^ VDICT_ZERO_KEY == VDICT_DICT_404
 */
#define VDICT_SUCCESS              0b0000
#define VDICT_ERROR                0b0001
#define VDICT_FULL                 0b0010
#define VDICT_ZERO_KEY             0b0011
#define VDICT_KEY_404              0b0100
#define VDICT_DICT_404             0b0101

/**
 * Type which is used to index data
 */
#ifndef VDICT_SIZE_TYPE
#define VDICT_SIZE_TYPE size_t
#endif //VDICT_SIZE_TYPE

/**
 * Void Dict Structure
 * Keys are not allowed to be 0
 */
struct void_dict {
    /** byte array of all stored keys */
    unsigned char* key_bytes;
    /** zeroed byte array with size key_size */
    const unsigned char* key_zero_field;
    /** byte array of all stored values */
    unsigned char* value_bytes;
    /** sorted array which holds all stored hashes */
    VDICT_SIZE_TYPE* hashes;
    /** amount of currently stored key-value pairs */
    VDICT_SIZE_TYPE size;
    /** size of keys in bytes */
    VDICT_SIZE_TYPE key_size;
    /** size of values in bytes */
    VDICT_SIZE_TYPE value_size;
    /** range of hashes */
    VDICT_SIZE_TYPE hash_pool;
    /** free function for recursive freeing on each value */
    void(*value_free_fn)(void*);
};

/**
 * Generates Hash of key for given Void Dict
 * 
 * @param vdict Void Dict to build hash for
 * @param key Key to build hash of
 * @return Hash in Hash-Pool
 */
inline VDICT_SIZE_TYPE vdict_hash(const struct void_dict* vdict, const void* key) {
    const unsigned char* data = (const unsigned char*)key;
    unsigned char byte;
    VDICT_SIZE_TYPE hash = 5381;

    for (VDICT_SIZE_TYPE i = 0; i < vdict->key_size; i++) {
        if ((byte = *(data + i))) {
            hash = ((hash << 5) + hash) ^ byte;
        }
    }

    return hash % vdict->hash_pool;
}

/**
 * Searches for given hash in hash-array of Void Dict
 * 
 * @param vdict Void Dict in which to search for hash
 * @param hash Hash to be searched for
 * @param idx Index of found Hash or last searched index
 * @return Pointer to found hash or NULL
 */
inline VDICT_SIZE_TYPE* vdict_hash_bsearch(const struct void_dict* vdict, const VDICT_SIZE_TYPE hash, VDICT_SIZE_TYPE* idx) {
    VDICT_SIZE_TYPE* needle = NULL;
    VDICT_SIZE_TYPE needle_idx = 0;
    if (vdict->size) {
        VDICT_SIZE_TYPE f_idx = 0, l_idx = vdict->size - 1;
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

/**
 * Adds given Key-Value Pair to Void Dict
 * 
 * @param vdict Void Dict to add Key-Value Pair to
 * @param key Key to Add
 * @param value Value to Add
 * @return Error Code
 */
inline int vdict_add_pair(struct void_dict* vdict, void* key, void* value) {
    if (vdict->size == vdict->hash_pool) {
        return VDICT_ERROR ^ VDICT_FULL;
    } else if (memcmp(key, vdict->key_zero_field, vdict->key_size) == 0) {
        return VDICT_ERROR ^ VDICT_ZERO_KEY;
    }
    VDICT_SIZE_TYPE hash = vdict_hash(vdict, key);
    VDICT_SIZE_TYPE needle_idx;
    while (vdict_hash_bsearch(vdict, hash, &needle_idx)) {
        hash = (hash + 1) % vdict->hash_pool;
    }
    memcpy(vdict->key_bytes + hash * vdict->key_size, key, vdict->key_size);
    memcpy(vdict->value_bytes + hash * vdict->value_size, value, vdict->value_size);
    if (vdict->size && vdict->hashes[needle_idx] < hash) needle_idx = (needle_idx + 1) % vdict->hash_pool;
    memmove(vdict->hashes + needle_idx + 1, vdict->hashes + needle_idx,
            sizeof(VDICT_SIZE_TYPE) * (vdict->size - needle_idx));
    vdict->hashes[needle_idx] = hash;
    vdict->size++;
    return VDICT_SUCCESS;
}

/**
 * Returns Pointer to Value of given Key
 * 
 * @param vdict Void Dict to get Value from
 * @param key Key to get Value with
 * @return Pointer to Value (NULL if idx out of range)
 */
inline void* vdict_get_value(const struct void_dict* vdict, const void* key) {
    VDICT_SIZE_TYPE hash = vdict_hash(vdict, key);
    if (vdict->size && vdict_hash_bsearch(vdict, hash, NULL)) {
        unsigned char found = 0;
        VDICT_SIZE_TYPE checked = 0;
        while (!found && checked < vdict->size) {
            if (memcmp(vdict->key_bytes + vdict->key_size * hash, vdict->key_zero_field, vdict->key_size) == 0) {
                break;
            } else if (memcmp(vdict->key_bytes + vdict->key_size * hash, key, vdict->key_size) == 0) {
                found = 1;
                break;
            } else {
                hash = (hash + 1) % vdict->hash_pool;
                checked++;
            }
        }

        if (found) {
            return vdict->value_bytes + vdict->value_size * hash;
        }
    }
    return NULL;
}

/**
 * Stores all keys into given array
 * 
 * @param vdict Void Dict to get the keys of
 * @param key_array Array to store keys into
 * @return key_array
 */
inline void* vdict_get_keys(const struct void_dict* vdict, void* key_array) {
    unsigned char* byte_key_array = (unsigned char*)key_array;
    for (VDICT_SIZE_TYPE i = 0; i < vdict->size; i++) {
        memcpy(byte_key_array + (vdict->key_size * i), vdict->key_bytes + (vdict->key_size * vdict->hashes[i]), vdict->key_size);
    }
    return key_array;
}

/**
 * Deletes key-value Pair of given key
 * 
 * @param vdict Void Dict to delete from
 * @param key Key of key-value pair to be deleted
 * @return Error Code
 */
inline int vdict_del_pair(struct void_dict* vdict, const void* key) {
    unsigned char* value_ptr = (unsigned char*)vdict_get_value(vdict, key);
    if (value_ptr) {
        VDICT_SIZE_TYPE hash = (value_ptr + vdict->value_size - vdict->value_bytes) / vdict->key_size;
        memset(vdict->key_bytes + vdict->key_size * hash, 0, vdict->key_size);
        if (vdict->value_free_fn) {
            vdict->value_free_fn(vdict->value_bytes + vdict->value_size * hash);
        }
        memset(vdict->value_bytes + vdict->value_size * hash, 0, vdict->value_size);
        VDICT_SIZE_TYPE hash_idx;
        VDICT_SIZE_TYPE* hash_ptr = vdict_hash_bsearch(vdict, hash, &hash_idx);
        memmove(hash_ptr, hash_ptr + 1,
                sizeof(VDICT_SIZE_TYPE) * (vdict->size - hash_idx));
        memset(vdict->hashes + vdict->size, 0, sizeof(VDICT_SIZE_TYPE));
        vdict->size--;
        return VDICT_SUCCESS;
    } else {
        return VDICT_ERROR ^ VDICT_KEY_404;
    }
}

/**
 * Clears Void Dict.\n 
 * Calls vdict->value_free_fn on values if available.
 * 
 * @param vdict Void Dict to be cleared
 * @return Error Code
 */
inline int vdict_clear(struct void_dict* vdict) {
    if (vdict) {
        memset(vdict->key_bytes, 0, vdict->key_size * vdict->hash_pool);
        if (vdict->value_free_fn) {
            for (VDICT_SIZE_TYPE hash_idx = 0; hash_idx < vdict->size; hash_idx++) {
                vdict->value_free_fn(vdict->value_bytes + vdict->value_size * vdict->hashes[hash_idx]);
            }
        }
        memset(vdict->value_bytes, 0, vdict->value_size * vdict->hash_pool);
        memset(vdict->hashes, 0, sizeof(VDICT_SIZE_TYPE) * vdict->size);
        vdict->size = 0;
        return VDICT_SUCCESS;
    }
    return VDICT_ERROR ^ VDICT_DICT_404;
}

/**
 * Initializes a Void Dictionary.
 *
 * @param dict Void Dictionary to be initialized
 * @param hash_pool Amount of Key-Value Pairs that can be stored
 * @param key_size Size of Keys in Bytes
 * @param value_size Size of Values in Bytes
 * @param value_free_fn Function to be called when freeing special datatypes (Nullable)
 * @return Error Code
 */
inline int vdict_init(struct void_dict* vdict, VDICT_SIZE_TYPE hash_pool, VDICT_SIZE_TYPE key_size, VDICT_SIZE_TYPE value_size, void(*value_free_fn)(void*)) {
    if (vdict) {
        vdict->key_bytes = (unsigned char*)calloc(key_size, hash_pool);
        vdict->key_zero_field = (unsigned char*)calloc(key_size, hash_pool);
        vdict->value_bytes = (unsigned char*)calloc(value_size, hash_pool);
        vdict->hashes = (VDICT_SIZE_TYPE*)calloc(sizeof(VDICT_SIZE_TYPE), hash_pool);
        vdict->size = 0;
        vdict->key_size = key_size;
        vdict->value_size = value_size;
        vdict->hash_pool = hash_pool;
        vdict->value_free_fn = value_free_fn;
        return VDICT_SUCCESS;
    }
    return VDICT_ERROR ^ VDICT_DICT_404;
}

/**
 * Frees Content of Void Dict.\n 
 * Calls vdict->value_free_fn on values if available.
 *
 * @param vdict_ptr Void Dict of which the content is to be freed
 */
inline void vdict_free(void* vdict_ptr) {
    struct void_dict* vdict = (struct void_dict*)vdict_ptr;
    if (vdict->value_free_fn) {
        for (VDICT_SIZE_TYPE hash_idx = 0; hash_idx < vdict->size; hash_idx++) {
            vdict->value_free_fn(vdict->value_bytes + vdict->value_size * vdict->hashes[hash_idx]);
        }
    }
    free(vdict->key_bytes);
    free((void*)vdict->key_zero_field);
    free(vdict->value_bytes);
    free(vdict->hashes);
}

#endif //VD_VOID_DICT_H
