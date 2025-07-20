#ifndef MAP_GENRIC_H
#define MAP_GENRIC_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Function pointer types for key comparison and value cleanup
typedef int (*map_key_cmp_fn)(const void *a, const void *b);
typedef void (*map_free_fn)(void *value);

typedef struct {
    void *key;
    void *value;
} map_entry_t;

typedef struct {
    map_entry_t *entries;
    size_t size;
    size_t capacity;
    map_key_cmp_fn key_cmp;
    map_free_fn key_free;
    map_free_fn value_free;
} map_t;

// compare functions for standard types
int string_key_comparison(const void *a, const void *b);
int uint8_key_comparison(const void *a, const void *b);
int uint32_key_comparison(const void *a, const void *b);
// No-op cleanup function for keys and values that do not require freeing
void no_cleanup(void *key_value);


// Lookup value by key, returns NULL if not found
void *map_get(const map_t *m, const void *key);
// Add or update a key-value pair. Returns 0 on success, -1 if full.
int map_set(map_t *m, void *key, void *value);
// Remove a key from the map. Returns 0 on success, -1 if not found.
int map_remove(map_t *m, const void *key);
// Free all values and reset the map
void map_clear(map_t *m);

#endif // MAP_H
