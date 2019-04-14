#ifndef __fixlib_linear_h
#define __fixlib_linear_h

#include <stdint.h>
#include <Python.h>

typedef struct {
    uint32_t start_offset;
    uint16_t len;
    uint16_t tag;
} tagentry_t;

typedef struct {
    void *tags;
    PyObject *other_tags;
    uint64_t msgType;
} tags_t;

tags_t *alloc_tags_linear(size_t numtags);

void free_tags_linear(tags_t *tags);

tags_t *findtags_linear(char *rawfix, size_t len, char sep, char delim,
                        size_t *tagslen);

#endif




