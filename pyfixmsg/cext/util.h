#ifndef __fixlibutil_h

#define __fixlibutil_h

#include <stdint.h>
#include <stdlib.h>

// used by sparse array approach 
void decodeOffsets(uint64_t encoded, uint32_t *start, uint32_t *end);

uint64_t encodeOffset(uint32_t start, uint32_t end);

// extract value from offsets
void rawToStrAndLen(char *rawFix, uint64_t offsets, char **outstr, size_t *outlen);

uint64_t msgTypeToUint64(char *msg);

uint64_t msgTypeAndSizeToUint64(char *msg, size_t len);

void uint64ToMsgType(uint64_t encoded, char *out);

#endif