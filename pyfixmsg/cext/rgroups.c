#include <Python.h>
#include <stdint.h>
#include "uthash.h"
#include "msgTypesMap.h"
#include <assert.h>
#include "util.h"

int addToMap(groupsForType_t *msgType, uint16_t countTag, PyObject *tagList) {
    groupdef_t *group;
    HASH_FIND_INT(msgType->groups, &countTag, group);
    if (group != NULL) {
        PyErr_Format(PyExc_KeyError, "The tag %s already exists", countTag);
        return 0;
    }

    size_t len = PySequence_Length(tagList);
    assert(len < 65535);
    group = alloc_groupdef(countTag, len);
    group->countTag = countTag;
    PyObject *listCompo = PySequence_Fast(tagList, "the composition need to be a list");
    if (listCompo == NULL) {
        free_groupdef(group);
        return 0;
    }
    for (size_t i = 0; i < len; i++) {
        long tag = PyLong_AS_LONG(PySequence_Fast_GET_ITEM(listCompo, i));
        if (tag < 0 || tag > 65535) {
            PyErr_Format(PyExc_TypeError, "The %zu th item in the sequence is not valid", i);
            free_groupdef(group);
            return 0;
        }
        group->composition[i] = tag;
    }
    HASH_ADD_INT(msgType->groups, countTag, group);
    return 1;
}



