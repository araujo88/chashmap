# CHashMap

A generic and **memory-safe** C implementation of a hash map (sometimes called a dictionary or associative array) with **separate chaining** for collision handling and **automatic resizing** when a specified load factor is exceeded. This implementation is inspired by Go’s `map` in the sense that it supports **arbitrary key** and **value types** by storing them as raw bytes (`void *` plus a size).

## Table of Contents

- [Features](#features)
- [How It Works](#how-it-works)
  - [Load Factor](#load-factor)
  - [Separate Chaining](#separate-chaining)
  - [Automatic Resizing](#automatic-resizing)
- [Usage](#usage)
  - [Cloning & Building](#cloning--building)
  - [Including in Your Project](#including-in-your-project)
- [API Reference](#api-reference)
  - [Initialization](#initialization)
  - [Insertion](#insertion)
  - [Lookup](#lookup)
  - [Removal](#removal)
  - [Destruction](#destruction)
- [Default Hash & Equality](#default-hash--equality)
- [Custom Hash & Equality](#custom-hash--equality)
  - [Example: Custom Struct Key](#example-custom-struct-key)
- [Example Program](#example-program)
- [License](#license)

---

## Features

1. **Generic Key/Value**

   - Store any type of data as the key or value by passing a pointer and size.

2. **Separate Chaining**

   - Uses a linked list (chain) to handle collisions in each bucket.

3. **Automatic Resizing**

   - When the number of stored pairs exceeds `capacity * load_factor`, the map capacity is doubled to maintain efficient lookups.

4. **Default or Custom Hash/Equality**

   - A default hashing function and equality check (byte-wise comparison) are provided.
   - You can also pass your own for specialized data types.

5. **Shallow Copy**
   - The map copies the raw bytes of your key/value. If you store pointers, you must manage deeper memory yourself.

---

## How It Works

### Load Factor

The **load factor** is a ratio that defines how **“full”** the map is allowed to become before a resize is triggered. It is calculated as:

```
current_size / capacity
```

- **Default**: `0.75f` (or 75%)
- When the size-to-capacity ratio exceeds `0.75`, the map **doubles** its capacity and re-hashes existing entries into the new buckets.

### Separate Chaining

Each bucket in the hash map array points to a **linked list** (chain) of entries. When two keys hash to the same bucket index:

1. The new entry is placed at the **head** of the linked list.
2. During a lookup or insertion, we traverse this linked list and compare keys to find the correct entry.

### Automatic Resizing

When the load factor is exceeded:

1. The map doubles its capacity.
2. All existing entries are **re-hashed** into the new buckets, preserving key-value associations.
3. This usually brings the load factor back below the threshold to maintain **O(1)** amortized performance.

---

## Usage

### Cloning & Building

1. **Clone** this repository:

   ```bash
   git clone https://github.com/araujo88/chashmap.git
   cd chashmap
   ```

2. **Build** with `make`:

   ```bash
   make
   ```

   - By default, this creates an example binary named `chashmap_example` in the project root.

3. **Run** the example:

   ```bash
   ./chashmap_example
   ```

   You should see output demonstrating inserts, lookups, and removals.

### Including in Your Project

- Copy the `include/chashmap.h` header and `src/chashmap.c` file into your project, or simply add this repo as a submodule.
- Ensure you include `chashmap.h` in any source file that calls the hash map functions.
- Link or compile `chashmap.c` alongside your code.

---

## API Reference

Below is a brief summary of the major functions. For detailed usage, see `chashmap.h` or the [Example Program](#example-program).

### Initialization

```c
int hashmap_init(HashMap *map,
                 size_t capacity,
                 hash_func_t hash_func,
                 eq_func_t eq_func,
                 float load_factor);
```

- **`map`**: Your `HashMap` struct (will be initialized).
- **`capacity`**: Initial number of buckets (use `0` to let the library choose a default).
- **`hash_func`**: Custom hash function (pass `NULL` to use default).
- **`eq_func`**: Custom equality function (pass `NULL` to use default).
- **`load_factor`**: Trigger resize threshold (`<= 0` => default of `0.75f`).
- **Returns** `0` on success, non-zero on error.

### Insertion

```c
int hashmap_insert(HashMap *map,
                   const void *key_data, size_t key_size,
                   const void *val_data, size_t val_size);
```

- If the key already exists, the **value is updated**.
- If the key does not exist, a **new entry** is created.
- **Memory**: The map allocates and stores its own copies of `key_data` and `val_data`.

### Lookup

```c
int hashmap_get(const HashMap *map,
                const void *key_data, size_t key_size,
                void **out_val, size_t *out_size);
```

- Retrieves the value for a given key, if it exists.
- If `out_val` and `out_size` are provided, a copy of the value is allocated and returned.
- **Caller** must `free(*out_val)` once done reading it.
- **Returns**:
  - `1` if found
  - `0` if not found
  - `< 0` on error (e.g., invalid arguments)

### Removal

```c
int hashmap_remove(HashMap *map, const void *key_data, size_t key_size);
```

- Removes the key/value if found.
- **Returns**:
  - `1` if removed
  - `0` if not found
  - `< 0` on error

### Destruction

```c
void hashmap_destroy(HashMap *map);
```

- Frees all buckets and entries, as well as keys and values stored within those entries.
- After calling, the `map` can be reused only after calling `hashmap_init` again.

---

## Default Hash & Equality

1. **Default Hash**: A variant of **Jenkins' one-at-a-time** hash.
2. **Default Equality**: A **byte-wise** comparison via `memcmp`.

These defaults are appropriate for most **primitive** and **plain-old-data** (POD) structures where a simple memory comparison is valid.

---

## Custom Hash & Equality

If you have keys that aren’t trivially comparable by bytes, or you need a more specific notion of equality, you can define **custom** functions:

```c
typedef uint64_t (*hash_func_t)(const void *key_data, size_t key_size);
typedef int (*eq_func_t)(const void *key_a, const void *key_b, size_t key_size);
```

- **Hash Function**: Must produce a 64-bit integer for any given key data.
- **Equality Function**: Must return non-zero if keys are **equal**, `0` otherwise.

### Example: Custom Struct Key

Suppose you have:

```c
struct Point {
    int x, y;
};
```

You might define:

```c
uint64_t custom_point_hash(const void *data, size_t size) {
    // For demonstration, a simple polynomial combination:
    // hash = x + 31 * y
    if (size < sizeof(int) * 2) return 0;  // safety check
    const int *xy = (const int *)data;
    return (uint64_t)xy[0] + 31ULL * (uint64_t)xy[1];
}

int custom_point_eq(const void *data1, const void *data2, size_t size) {
    // Compare x and y
    const int *xy1 = (const int *)data1;
    const int *xy2 = (const int *)data2;
    return (xy1[0] == xy2[0]) && (xy1[1] == xy2[1]);
}
```

Then initialize your map with these functions:

```c
HashMap map;
hashmap_init(&map, 16, custom_point_hash, custom_point_eq, 0.75f);

// usage...
```

---

## Example Program

Below is a simple example that demonstrates inserting and retrieving **different types** of keys and values:

```c
#include <stdio.h>
#include "chashmap.h"

// Example usage
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
```

Compile and run the example to see output indicating successful storage, retrieval, and removal.

---

## License

This project is provided under the **MIT License**, which permits reuse within proprietary projects. See [LICENSE](LICENSE) for details.

Feel free to tailor this hash map to suit your specific performance, memory, or customization requirements. If you encounter any issues or want to suggest improvements, please open an issue or submit a pull request. Happy coding!
