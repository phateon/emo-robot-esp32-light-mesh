#ifndef MAP_STRING_H
#define MAP_STRING_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    char *key;
    void *value;
} map_entry_t;

typedef struct {
    map_entry_t *entries;
    size_t size;
    uint16_t capacity;
} map_t;

// Lookup value by key, returns NULL if not found
void *map_get(const map_t *m, const char *key);

// Add or update a key-value pair. Returns 0 on success, -1 if full.
int map_set(map_t *m, const char *key, void *value);

// Remove a key from the map. Returns 0 on success, -1 if not found.
int map_remove(map_t *m, const char *key);

// Free all values and reset the map
void map_clear(map_t *m);

#endif // MAP_STRING_H
