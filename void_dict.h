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
#define VDICT_SUCCESS              0
#define VDICT_ERROR                1
#define VDICT_FULL                 2
#define VDICT_ZERO_KEY             3
#define VDICT_KEY_404              4
#define VDICT_DICT_404             5


/**
 * Void Dict Structure
 * Keys are not allowed to be 0
 */
struct void_dict {
    /** byte array of all stored keys */
    void* key_bytes;
    /** zeroed byte array with size key_size */
    const void* key_zero_field;
    /** byte array of all stored values */
    void* value_bytes;
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
size_t 
vdict_hash(const struct void_dict* vdict, const void* key);

/**
 * Searches for given hash in hash-array of Void Dict
 * 
 * @param vdict Void Dict in which to search for hash
 * @param hash Hash to be searched for
 * @param idx Index of found Hash or last searched index
 * @return Pointer to found hash or NULL
 */
size_t* 
vdict_hash_bsearch(const struct void_dict* vdict, const size_t hash, size_t* idx);

/**
 * Adds given Key-Value Pair to Void Dict
 * 
 * @param vdict Void Dict to add Key-Value Pair to
 * @param key Key to Add
 * @param value Value to Add
 * @return Error Code
 */
int 
vdict_add_pair(struct void_dict* vdict, void* key, void* value);

/**
 * Returns Pointer to Value of given Key
 * 
 * @param vdict Void Dict to get Value from
 * @param key Key to get Value with
 * @return Pointer to Value (NULL if idx out of range)
 */
void* 
vdict_get_value(const struct void_dict* vdict, const void* key);

/**
 * Stores all keys into given array
 * 
 * @param vdict Void Dict to get the keys of
 * @param key_array Array to store keys into
 * @return key_array
 */
void* 
vdict_get_keys(const struct void_dict* vdict, void* key_array);

/**
 * Deletes key-value Pair of given key
 * 
 * @param vdict Void Dict to delete from
 * @param key Key of key-value pair to be deleted
 * @return Error Code
 */
int 
vdict_del_pair(struct void_dict* vdict, const void* key);

/**
 * Clears Void Dict.\n 
 * Calls vdict->value_free_fn on values if available.
 * 
 * @param vdict Void Dict to be cleared
 * @return Error Code
 */
int 
vdict_clear(struct void_dict* vdict);

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
int 
vdict_init(struct void_dict* vdict, size_t hash_pool, size_t key_size, size_t value_size, void(*value_free_fn)(void*));

/**
 * Frees Content of Void Dict.\n 
 * Calls vdict->value_free_fn on values if available.
 *
 * @param vdict_ptr Void Dict of which the content is to be freed
 */
void 
vdict_free(void* vdict_ptr);

#endif /* VD_VOID_DICT_H */
