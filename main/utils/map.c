#include "map.h"
#include <string.h>
#include <stdlib.h>

void *map_get(const map_t *m, const char *key) {
    for (size_t i = 0; i < m->size; ++i) {
        if (strcmp(m->entries[i].key, key) == 0) {
            return m->entries[i].value;
        }
    }
    return NULL;
}

int map_set(map_t *m, const char *key, void *value) {
    for (size_t i = 0; i < m->size; ++i) {
        if (strcmp(m->entries[i].key, key) == 0) {
            m->entries[i].value = value;
            return 0;
        }
    }
    if (m->size < m->capacity) {
        m->entries[m->size].key = strdup(key);
        m->entries[m->size].value = value;
        m->size++;
        return 0;
    }
    return -1;
}

int map_remove(map_t *m, const char *key) {
    for (size_t i = 0; i < m->size; ++i) {
        if (strcmp(m->entries[i].key, key) == 0) {
            free(m->entries[i].key);
            m->entries[i] = m->entries[m->size - 1];
            m->size--;
            return 0;
        }
    }
    return -1;
}

void map_clear(map_t *m) {
    for (size_t i = 0; i < m->size; ++i) {
        free(m->entries[i].key);
    }
    m->size = 0;
}
