#include "map.h"
#include <string.h>
#include <stdlib.h>

// Implementation for standard keys
int string_key_comparison(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

// Standard comparison uint8_t keys
 int uint8_key_comparison(const void *a, const void *b) {
    uint8_t va = *(const uint8_t *)a;
    uint8_t vb = *(const uint8_t *)b;
    return (va > vb) - (va < vb);
}

// Standard comparison uint32_t keys
int uint32_key_comparison(const void *a, const void *b) {
    uint32_t va = *(const uint32_t *)a;
    uint32_t vb = *(const uint32_t *)b;
    return (va > vb) - (va < vb);
}

// No-op cleanup for string, uint8_t, uint32_t, keys.
// This funciton can also be used for value clean up if no cleanup is necessary.
void no_cleanup(void *key_value) {
    // No-op: nothing to free for uint32_t keys
}

void clean_up(map_t *m, size_t i) {
    if (m->value_free) m->value_free(m->entries[i].value);
    if (m->key_free) m->key_free(m->entries[i].key);
}

void *map_get(const map_t *m, const void *key) {
    for (size_t i = 0; i < m->size; ++i) {
        if (m->key_cmp(m->entries[i].key, key) == 0) {
            return m->entries[i].value;
        }
    }
    return NULL;
}

int map_set(map_t *m, void *key, void *value) {
    for (size_t i = 0; i < m->size; ++i) {
        if (m->key_cmp(m->entries[i].key, key) == 0) {
            if (m->value_free) m->value_free(m->entries[i].value);
            m->entries[i].value = value;
            return 0;
        }
    }
    if (m->size < m->capacity) {
        m->entries[m->size].key = key;
        m->entries[m->size].value = value;
        m->size++;
        return 0;
    }
    return -1;
}

int map_remove(map_t *m, const void *key) {
    for (size_t i = 0; i < m->size; ++i) {
        if (m->key_cmp(m->entries[i].key, key) == 0) {
            clean_up(m, i);
            m->entries[i] = m->entries[m->size - 1];
            m->size--;
            return 0;
        }
    }
    return -1;
}

void map_clear(map_t *m) {
    if ((m->value_free || m->key_free) && m->size > 0) {
        for (size_t i = 0; i < m->size; ++i) {
            clean_up(m, i);
        }
    }
    m->size = 0;
}
