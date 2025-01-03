#include "../include/chashmap.h"
#include <assert.h>
#include <string.h>

#define DEFAULT_INITIAL_CAPACITY 16
#define DEFAULT_LOAD_FACTOR 0.75f

// Forward declarations
static uint64_t default_hash(const void *data, size_t size);
static int default_eq(const void *data1, const void *data2, size_t size);
static int hashmap_resize(HashMap *map, size_t new_capacity);
static HashMapEntry *hashmap_create_entry(const void *key, size_t key_size,
                                          const void *val, size_t val_size);
static void hashmap_free_entry(HashMapEntry *entry);

/**
 * Jenkins' one-at-a-time hash (an example).
 */
static uint64_t default_hash(const void *data, size_t size)
{
    const unsigned char *key = (const unsigned char *)data;
    uint64_t hash = 0;
    for (size_t i = 0; i < size; i++)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

/**
 * Default equality function: byte-wise comparison.
 */
static int default_eq(const void *data1, const void *data2, size_t size)
{
    return memcmp(data1, data2, size) == 0;
}

int hashmap_init(HashMap *map,
                 size_t capacity,
                 hash_func_t hash_func,
                 eq_func_t eq_func,
                 float load_factor)
{
    if (!map)
        return -1;

    if (capacity == 0)
    {
        capacity = DEFAULT_INITIAL_CAPACITY;
    }
    if (load_factor <= 0.0f)
    {
        load_factor = DEFAULT_LOAD_FACTOR;
    }

    map->capacity = capacity;
    map->size = 0;
    map->hash_func = (hash_func != NULL) ? hash_func : default_hash;
    map->eq_func = (eq_func != NULL) ? eq_func : default_eq;
    map->load_factor = load_factor;

    map->buckets = (HashMapEntry **)calloc(map->capacity, sizeof(HashMapEntry *));
    if (!map->buckets)
    {
        return -1;
    }
    return 0;
}

void hashmap_destroy(HashMap *map)
{
    if (!map || !map->buckets)
        return;

    for (size_t i = 0; i < map->capacity; i++)
    {
        HashMapEntry *entry = map->buckets[i];
        while (entry)
        {
            HashMapEntry *next = entry->next;
            hashmap_free_entry(entry);
            entry = next;
        }
    }
    free(map->buckets);
    map->buckets = NULL;
    map->capacity = 0;
    map->size = 0;
    map->hash_func = NULL;
    map->eq_func = NULL;
    map->load_factor = 0;
}

int hashmap_insert(HashMap *map,
                   const void *key_data, size_t key_size,
                   const void *val_data, size_t val_size)
{
    if (!map || !key_data || key_size == 0)
        return -1;

    // Resize if load factor exceeded
    float current_load = (float)map->size / (float)map->capacity;
    if (current_load >= map->load_factor)
    {
        int resize_status = hashmap_resize(map, map->capacity * 2);
        if (resize_status != 0)
        {
            // Could not resize; continue anyway but with reduced performance.
            fprintf(stderr, "Warning: hashmap resizing failed.\n");
        }
    }

    uint64_t hash_val = map->hash_func(key_data, key_size);
    size_t index = hash_val % map->capacity;

    // Check for existing key in the chain
    HashMapEntry *entry = map->buckets[index];
    while (entry)
    {
        if (entry->key_size == key_size &&
            map->eq_func(entry->key, key_data, key_size))
        {
            // Key found, update value
            free(entry->value);
            entry->value = malloc(val_size);
            if (!entry->value)
                return -1;
            memcpy(entry->value, val_data, val_size);
            entry->value_size = val_size;
            return 0;
        }
        entry = entry->next;
    }

    // Not found; insert new entry at head of the chain
    HashMapEntry *new_entry = hashmap_create_entry(key_data, key_size, val_data, val_size);
    if (!new_entry)
        return -1;

    new_entry->next = map->buckets[index];
    map->buckets[index] = new_entry;
    map->size++;

    return 0;
}

int hashmap_get(const HashMap *map,
                const void *key_data, size_t key_size,
                void **out_val, size_t *out_size)
{
    if (!map || !key_data || key_size == 0)
        return -1;

    uint64_t hash_val = map->hash_func(key_data, key_size);
    size_t index = hash_val % map->capacity;

    HashMapEntry *entry = map->buckets[index];
    while (entry)
    {
        if (entry->key_size == key_size &&
            map->eq_func(entry->key, key_data, key_size))
        {
            // Found the key
            if (out_val && out_size)
            {
                *out_val = malloc(entry->value_size);
                if (!(*out_val))
                {
                    return -1; // memory error
                }
                memcpy(*out_val, entry->value, entry->value_size);
                *out_size = entry->value_size;
            }
            return 1; // found
        }
        entry = entry->next;
    }

    return 0; // not found
}

int hashmap_remove(HashMap *map, const void *key_data, size_t key_size)
{
    if (!map || !key_data || key_size == 0)
        return -1;

    uint64_t hash_val = map->hash_func(key_data, key_size);
    size_t index = hash_val % map->capacity;

    HashMapEntry *entry = map->buckets[index];
    HashMapEntry *prev = NULL;

    while (entry)
    {
        if (entry->key_size == key_size &&
            map->eq_func(entry->key, key_data, key_size))
        {
            // Remove this entry
            if (prev)
            {
                prev->next = entry->next;
            }
            else
            {
                map->buckets[index] = entry->next;
            }
            hashmap_free_entry(entry);
            map->size--;
            return 1; // removed
        }
        prev = entry;
        entry = entry->next;
    }
    return 0; // not found
}

/**
 * Resize (rehash) the hash map to a new capacity.
 */
static int hashmap_resize(HashMap *map, size_t new_capacity)
{
    if (new_capacity < 1)
    {
        return -1;
    }

    // Allocate new buckets
    HashMapEntry **new_buckets = (HashMapEntry **)calloc(new_capacity, sizeof(HashMapEntry *));
    if (!new_buckets)
    {
        return -1;
    }

    // Rehash all entries
    for (size_t i = 0; i < map->capacity; i++)
    {
        HashMapEntry *entry = map->buckets[i];
        while (entry)
        {
            HashMapEntry *next = entry->next;
            uint64_t hash_val = map->hash_func(entry->key, entry->key_size);
            size_t new_index = hash_val % new_capacity;

            // Insert into new bucket chain
            entry->next = new_buckets[new_index];
            new_buckets[new_index] = entry;

            entry = next;
        }
    }

    // Free old bucket array (but not entries!)
    free(map->buckets);

    map->buckets = new_buckets;
    map->capacity = new_capacity;
    return 0;
}

/**
 * Helper to create a new entry object.
 */
static HashMapEntry *hashmap_create_entry(const void *key, size_t key_size,
                                          const void *val, size_t val_size)
{
    HashMapEntry *entry = (HashMapEntry *)malloc(sizeof(HashMapEntry));
    if (!entry)
        return NULL;
    entry->next = NULL;

    // Allocate and copy the key
    entry->key = malloc(key_size);
    if (!entry->key)
    {
        free(entry);
        return NULL;
    }
    memcpy(entry->key, key, key_size);
    entry->key_size = key_size;

    // Allocate and copy the value
    entry->value = malloc(val_size);
    if (!entry->value)
    {
        free(entry->key);
        free(entry);
        return NULL;
    }
    memcpy(entry->value, val, val_size);
    entry->value_size = val_size;

    return entry;
}

/**
 * Helper to free an entry (and all memory it owns).
 */
static void hashmap_free_entry(HashMapEntry *entry)
{
    if (entry)
    {
        free(entry->key);
        free(entry->value);
        free(entry);
    }
}
