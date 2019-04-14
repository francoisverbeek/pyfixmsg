#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint64_t msgTypeToUint64(char *msg) {
    uint64_t returned = 0;
    size_t len =
            (sizeof(uint64_t) < strlen(msg)) ? sizeof(uint64_t) : strlen(msg);
    memcpy(&returned, msg, len);
    return returned;
}

uint64_t msgTypeAndSizeToUint64(char *msg, size_t len) {
    uint64_t returned = 0;
    len = (sizeof(uint64_t) < len) ? sizeof(uint64_t) : len;
    memcpy(&returned, msg, len);
    return returned;
}
