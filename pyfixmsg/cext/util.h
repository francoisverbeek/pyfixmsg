#ifndef __fixlibutil_h

#define __fixlibutil_h

#include <stdint.h>
#include <stdlib.h>


uint64_t msgTypeToUint64(char *msg);

uint64_t msgTypeAndSizeToUint64(char *msg, size_t len);


#endif