#include <stdint.h>
#include <stdlib.h>
#include "groupdef.h"

groupdef_t *alloc_groupdef(uint16_t tag, uint16_t len) {
    groupdef_t *returned = malloc(sizeof(groupdef_t));
    returned->composition = calloc(len, sizeof(uint16_t));
    returned->compositionlen = len;
    returned->countTag = tag;
    returned->pointer_in_composition = 0;
    return returned;
}

void free_groupdef(groupdef_t *groupdef) {
    free(groupdef->composition);
    free(groupdef);
}
