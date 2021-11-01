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
 * Void Dict Structure
 * Keys are not allowed to be 0
 */
struct void_dict {
    /** byte array of all stored keys */
    uint8_t* key_bytes;
    /** zeroed byte array with size key_size */
    const uint8_t* key_zero_field;
    /** byte array of all stored values */
    uint8_t* value_bytes;
    /** sorted array which holds all stored hashes */
    size_t* hashes;
    /** amount of currently stored key-value pairs */
    size_t size;
    /** size of keys in bytes */
    size_t key_size;
    /** size of values in bytes */
    size_t value_size;
    /** range of hashes */
    size_t hash_pool;
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
size_t vdict_hash(const struct void_dict* vdict, const void* key) {
    const uint8_t* data = (const uint8_t*)key;
    uint8_t byte;
    size_t hash = 5381;

    for (size_t i = 0; i < vdict->key_size; i++) {
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
size_t* vdict_hash_bsearch(const struct void_dict* vdict, const size_t hash, size_t* idx) {
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

/**
 * Adds given Key-Value Pair to Void Dict
 * 
 * @param vdict Void Dict to add Key-Value Pair to
 * @param key Key to Add
 * @param value Value to Add
 * @return Error Code
 */
int vdict_add_pair(struct void_dict* vdict, void* key, void* value) {
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
    memcpy(vdict->key_bytes + hash * vdict->key_size, key, vdict->key_size);
    memcpy(vdict->value_bytes + hash * vdict->value_size, value, vdict->value_size);
    if (vdict->size && vdict->hashes[needle_idx] < hash) needle_idx = (needle_idx + 1) % vdict->hash_pool;
    memmove(vdict->hashes + needle_idx + 1, vdict->hashes + needle_idx,
            sizeof(size_t) * (vdict->size - needle_idx));
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
void* vdict_get_value(const struct void_dict* vdict, const void* key) {
    size_t hash = vdict_hash(vdict, key);
    if (vdict->size && vdict_hash_bsearch(vdict, hash, NULL)) {
        uint8_t found = 0;
        size_t checked = 0;
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
void* vdict_get_keys(const struct void_dict* vdict, void* key_array) {
    uint8_t* byte_key_array = (uint8_t*)key_array;
    for (size_t i = 0; i < vdict->size; i++) {
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
int vdict_del_pair(struct void_dict* vdict, const void* key) {
    uint8_t* value_ptr = (uint8_t*)vdict_get_value(vdict, key);
    if (value_ptr) {
        size_t hash = (value_ptr + vdict->value_size - vdict->value_bytes) / vdict->key_size;
        memset(vdict->key_bytes + vdict->key_size * hash, 0, vdict->key_size);
        if (vdict->value_free_fn) {
            vdict->value_free_fn(vdict->value_bytes + vdict->value_size * hash);
        }
        memset(vdict->value_bytes + vdict->value_size * hash, 0, vdict->value_size);
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

/**
 * Clears Void Dict.\n 
 * Calls vdict->value_free_fn on values if available.
 * 
 * @param vdict Void Dict to be cleared
 * @return Error Code
 */
int vdict_clear(struct void_dict* vdict) {
    if (vdict) {
        memset(vdict->key_bytes, 0, vdict->key_size * vdict->hash_pool);
        if (vdict->value_free_fn) {
            for (size_t hash_idx = 0; hash_idx < vdict->size; hash_idx++) {
                vdict->value_free_fn(vdict->value_bytes + vdict->value_size * vdict->hashes[hash_idx]);
            }
        }
        memset(vdict->value_bytes, 0, vdict->value_size * vdict->hash_pool);
        memset(vdict->hashes, 0, sizeof(size_t) * vdict->size);
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
int vdict_init(struct void_dict* vdict, size_t hash_pool, size_t key_size, size_t value_size, void(*value_free_fn)(void*)) {
    if (vdict) {
        vdict->key_bytes = (uint8_t*)calloc(key_size, hash_pool);
        vdict->key_zero_field = (uint8_t*)calloc(key_size, hash_pool);
        vdict->value_bytes = (uint8_t*)calloc(value_size, hash_pool);
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

/**
 * Frees Content of Void Dict.\n 
 * Calls vdict->value_free_fn on values if available.
 *
 * @param vdict_ptr Void Dict of which the content is to be freed
 */
void vdict_free(void* vdict_ptr) {
    struct void_dict* vdict = (struct void_dict*)vdict_ptr;
    if (vdict->value_free_fn) {
        for (size_t hash_idx = 0; hash_idx < vdict->size; hash_idx++) {
            vdict->value_free_fn(vdict->value_bytes + vdict->value_size * vdict->hashes[hash_idx]);
        }
    }
    free(vdict->key_bytes);
    free((void*)vdict->key_zero_field);
    free(vdict->value_bytes);
    free(vdict->hashes);
}

#endif //VD_VOID_DICT_H
