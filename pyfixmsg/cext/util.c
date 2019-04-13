#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void decodeOffsets(uint64_t encoded, uint32_t *start, uint32_t *end) {
    *start = (uint32_t) (encoded >> 32);
    *end = (uint32_t) (encoded & 0x00000000ffffffff);
}

uint64_t encodeOffset(uint32_t start, uint32_t end) {
    uint64_t returned = ((uint64_t) start) << 32;
    returned = returned | ((uint64_t) end);
    return returned;
}

void rawToStrAndLen(char *rawFix, uint64_t offsets, char **outstr,
                    size_t *outlen) {
    uint32_t start, end;
    decodeOffsets(offsets, &start, &end);
    *outstr = rawFix + (size_t) start;
    *outlen = end - start;
}


uint64_t msgTypeToUint64(char *msg) {
    uint64_t returned = 0;
    size_t len =
            (sizeof(uint64_t) < strlen(msg)) ? sizeof(uint64_t) : strlen(msg);
    memcpy(&returned, msg, len);
    return returned;
}

void uint64ToMsgType(uint64_t encoded, char *out) {
    size_t maxlen = sizeof(uint64_t) + 1;
    memset(out, 0, maxlen);
    memcpy(out, &encoded, sizeof(uint64_t));
}

uint64_t msgTypeAndSizeToUint64(char *msg, size_t len) {
    uint64_t returned = 0;
    len = (sizeof(uint64_t) < len) ? sizeof(uint64_t) : len;
    memcpy(&returned, msg, len);
    return returned;
}
