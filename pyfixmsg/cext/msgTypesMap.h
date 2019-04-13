#ifndef __msTypesMap_h
#define __msTypesMap_h

#include <stdint.h>
#include "uthash.h"
#include <Python.h>
#include "groupdef.h"

typedef struct {
    uint64_t msgType;    // 8 times 8 bits, key
    uint32_t numgroups;  // len of all groups count tags
    int *allgroups; // all count tags, iterate over this to find if count tag.
    // TODO check if faster than hash lookup
    groupdef_t *groups;
    UT_hash_handle hh;
} groupsForType_t;

typedef struct {
    PyObject_HEAD;
    groupsForType_t *msgTypes;
} msgTypes_t;

int allocMsgTypes(PyObject *obj, PyObject *args, PyObject *kwds);

groupsForType_t *alloc_groupsForType(void);

void freeMsgTypes(void *obj);

extern PyTypeObject MsgTypeMap_Type;

#endif
