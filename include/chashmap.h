#ifndef CHASHMAP_H
#define CHASHMAP_H

#include <stddef.h> // for size_t
#include <stdint.h> // for uint64_t, etc.
#include <stdlib.h> // for malloc, free
#include <string.h> // for memcpy
#include <stdio.h>  // for fprintf (debug), remove if not needed

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * A function pointer type for hashing `key_data`.
     *   @param key_data: Pointer to the raw key bytes.
     *   @param key_size: The size in bytes of the key.
     *   @return A 64-bit hash value for the key.
     */
    typedef uint64_t (*hash_func_t)(const void *key_data, size_t key_size);

    /**
     * A function pointer type for testing key equality.
     *   @param key_a, key_b: Pointers to the raw key bytes.
     *   @param key_size: The size in bytes of both keys.
     *   @return Non-zero if equal, 0 otherwise.
     */
    typedef int (*eq_func_t)(const void *key_a, const void *key_b, size_t key_size);

    /**
     * An entry in the hash map’s separate chaining list.
     */
    typedef struct HashMapEntry
    {
        void *key;
        size_t key_size;
        void *value;
        size_t value_size;
        struct HashMapEntry *next;
    } HashMapEntry;

    /**
     * The main HashMap structure.
     */
    typedef struct
    {
        HashMapEntry **buckets; // Array of pointers to entries
        size_t capacity;        // Number of buckets
        size_t size;            // Number of key-value pairs stored
        hash_func_t hash_func;  // Hash function
        eq_func_t eq_func;      // Equality function
        float load_factor;      // Max load factor before resizing
    } HashMap;

    /**
     * Initialize a new HashMap.
     *   @param map        Pointer to a HashMap to initialize.
     *   @param capacity   Initial capacity (number of buckets).
     *   @param hash_func  Hash function to use.
     *   @param eq_func    Equality function to use.
     *   @return 0 on success, non-zero on error.
     */
    int hashmap_init(HashMap *map,
                     size_t capacity,
                     hash_func_t hash_func,
                     eq_func_t eq_func,
                     float load_factor);

    /**
     * Free all resources used by the HashMap.
     */
    void hashmap_destroy(HashMap *map);

    /**
     * Insert or update a key-value pair.
     *   - If key already exists, update the value.
     *   - If key does not exist, insert a new entry.
     * @param map        Pointer to the HashMap.
     * @param key_data   Pointer to the key bytes.
     * @param key_size   Size of the key in bytes.
     * @param val_data   Pointer to the value bytes.
     * @param val_size   Size of the value in bytes.
     * @return 0 on success, non-zero on error.
     */
    int hashmap_insert(HashMap *map,
                       const void *key_data, size_t key_size,
                       const void *val_data, size_t val_size);

    /**
     * Retrieve a value associated with a key.
     *   @param map        Pointer to the HashMap.
     *   @param key_data   Pointer to the key bytes.
     *   @param key_size   Size of the key in bytes.
     *   @param out_val    Will be allocated and filled if found. Caller must free.
     *   @param out_size   Size of the returned value in bytes.
     *   @return 1 if found, 0 if not found, < 0 on error.
     */
    int hashmap_get(const HashMap *map,
                    const void *key_data, size_t key_size,
                    void **out_val, size_t *out_size);

    /**
     * Remove a key-value pair from the map.
     *   @param map        Pointer to the HashMap.
     *   @param key_data   Pointer to the key bytes.
     *   @param key_size   Size of the key in bytes.
     *   @return 1 if removed, 0 if not found, < 0 on error.
     */
    int hashmap_remove(HashMap *map, const void *key_data, size_t key_size);

#ifdef __cplusplus
}
#endif

#endif // CHASHMAP_H
