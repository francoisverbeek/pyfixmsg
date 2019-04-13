#ifndef __rgroups_h
#define __rgroups_h

#include <Python.h>
#include <stdint.h>
#include "msgTypesMap.h"

int addToMap(groupsForType_t *msgType, uint16_t countTag, PyObject *tagList);

PyObject *addGroupMaps(PyObject *self, PyObject *args);

#endif
