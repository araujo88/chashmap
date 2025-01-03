#include <stdio.h>
#include "../include/chashmap.h"

int main(void)
{
    HashMap map;
    if (hashmap_init(&map, 0, NULL, NULL, 0.0f) != 0)
    {
        fprintf(stderr, "Failed to initialize HashMap.\n");
        return 1;
    }

    // Insert int -> double
    int key_int = 42;
    double val_double = 3.14159;
    hashmap_insert(&map, &key_int, sizeof(key_int), &val_double, sizeof(val_double));

    // Insert string -> string
    const char *key_str = "hello";
    const char *val_str = "world";
    hashmap_insert(&map, key_str, strlen(key_str) + 1, val_str, strlen(val_str) + 1);

    // Insert custom struct as key
    struct Point
    {
        int x, y;
    } p = {10, 20};
    const char *val_for_struct = "a point";
    hashmap_insert(&map, &p, sizeof(p), val_for_struct, strlen(val_for_struct) + 1);

    // Lookup int -> double
    void *out_val = NULL;
    size_t out_size = 0;
    int found = hashmap_get(&map, &key_int, sizeof(key_int), &out_val, &out_size);
    if (found == 1)
    {
        double retrieved = *(double *)out_val;
        printf("Retrieved value for key %d is %f\n", key_int, retrieved);
        free(out_val); // Must free after retrieval
    }

    // Lookup string -> string
    found = hashmap_get(&map, key_str, strlen(key_str) + 1, &out_val, &out_size);
    if (found == 1)
    {
        printf("Retrieved value for key \"%s\" is \"%s\"\n", key_str, (char *)out_val);
        free(out_val);
    }

    // Remove int -> double
    int removed = hashmap_remove(&map, &key_int, sizeof(key_int));
    printf("Removed key %d: %s\n", key_int, (removed == 1) ? "true" : "false");

    // Cleanup
    hashmap_destroy(&map);
    return 0;
}
