#ifndef __groupdef_h
#define __groupdef_h

#include <stdint.h>
#include "uthash.h"

typedef struct {
    int countTag;
    uint16_t compositionlen;
    uint16_t *composition;
    size_t pointer_in_composition;
    UT_hash_handle hh;
} groupdef_t;

groupdef_t *alloc_groupdef(uint16_t tag, uint16_t len);

void free_groupdef(groupdef_t *groupdef);

#endif
