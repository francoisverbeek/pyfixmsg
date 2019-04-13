//
//  pyfixlib
//
//  Created by Francois Verbeek on 02/02/2017.
//  Copyright © 2017 Francois Verbeek. All rights reserved.
//

#include <Python.h>
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include "fixlib_linear.h"
#include "util.h"

tags_t *alloc_tags_linear(size_t numtags) {
    tags_t *returned = malloc(sizeof(tags_t));
    returned->tags = calloc(numtags, sizeof(tagentry_t));
    returned->other_tags = NULL;
    returned->msgType = 0;
    return returned;
}

void free_tags_linear(tags_t *tags) {
    free(tags->tags);
    Py_XDECREF(tags->other_tags);
    free(tags);
}

tags_t *findtags_linear(char *rawfix, size_t len, char sep, char delim,
                        size_t *tagslen) {
    size_t num_delims = 0;
    size_t tagnums = 0;

    assert(len < UINT32_MAX); // max fix message size 4GBytes
    void *next_sep = NULL;
    void *nextbuf = rawfix;
    size_t nextlen = len;
    do {
        // FIXME : what if the first char is delim
        next_sep = memchr(nextbuf, sep, nextlen);
        if (next_sep == NULL) break;
        num_delims += 1;
        size_t thislen = next_sep - nextbuf;
        nextlen = nextlen - thislen - 1; // skip the sep
        nextbuf = next_sep + 1;
    } while (next_sep != NULL);

    tags_t *tags = alloc_tags_linear(num_delims);
    tagnums = (size_t) -1;
    // Tags
    // 32 bits offset from start of message
    // 16 bits tag number
    // 16 bits value length.  If any of these restrictions are invalid
    // the tag and value are stored in the py dict
    void *tagstart = 0;
    void *tagend = 0;
    void *valuestart = 0;
    void *valueend = 0;
    nextbuf = rawfix;
    nextlen = len;
    size_t taglen;
    char *endofint;
    do {
        tagnums++;
        tagstart = nextbuf;
        tagend = memchr(nextbuf, delim, nextlen);
        if (tagend == NULL) { break; };
        taglen = tagend - nextbuf;
        nextbuf = tagend + 1;
        nextlen = nextlen - taglen - 1;
        valuestart = nextbuf;
        valueend = memchr(nextbuf, sep, nextlen);
        if (valueend == NULL) { break; };
        nextbuf = valueend + 1;
        nextlen = nextlen - (valueend - valuestart) - 1;

        // convert tag to int, create uint64_t from valuestart and valueend
        // if works and < TAGARRAYSIZE, store in spec_tags
        // if works and 10K < tag < 10K+TAGARRAYSIZE, store in tenK_tags
        //  else store in pyDict
        // reset to reading tag
        uint64_t tagint = strtol(tagstart, &endofint, 10);
        if ((tagint == 0 && errno == EINVAL) || (tagint > 0xffff) ||
            ((size_t) (valueend - valuestart) > 0xffff)) {
            // Non-numerical tags take a slow path in a pydict
            if (tags->other_tags == NULL) {
                tags->other_tags = PyDict_New();
                assert(Py_REFCNT(tags->other_tags) == 1);
            }
            PyDict_SetItem(
                    tags->other_tags,
                    PyBytes_FromStringAndSize(tagstart, tagend - tagstart),
                    PyBytes_FromStringAndSize(valuestart,
                                              valueend - valuestart));
        } else {
            assert(endofint == tagend);
            tagentry_t *tag = tags->tags + tagnums * sizeof(tagentry_t);
            tag->tag = (uint16_t) tagint;
            if (tagint == 35) {
                tags->msgType = msgTypeAndSizeToUint64(valuestart, valueend - valuestart);
            }
            tag->len = (uint16_t) (valueend - valuestart);
            tag->start_offset = (uint32_t) (valuestart - (void *) rawfix);
        }
    } while (nextbuf != NULL);
    *tagslen = tagnums;
    if (tagnums < num_delims - 1) { // There is one more delimiter than there are tags as tag 10 ends with delim
        // there were delims in values
        memset(tags->tags + tagnums * sizeof(tagentry_t), 0,
               (num_delims - tagnums) * sizeof(tagentry_t));
    }
    return tags;
}
