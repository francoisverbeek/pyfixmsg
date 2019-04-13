#include "msgTypesMap.h"
#include "uthash.h"
#include <Python.h>
#include "groupdef.h"
#include "util.h"
#include "rgroups.h"

int allocMsgTypes(PyObject *obj, PyObject *args, PyObject *kwds) {
    msgTypes_t *msgTypes = (msgTypes_t *) obj;
    (void) args;
    (void) kwds;
    msgTypes->msgTypes = NULL;
    return 0;
}

void freeMsgTypes(void *obj) {
    msgTypes_t *msgTypes = (msgTypes_t *) obj;
    groupdef_t *current, *tmp;
    if (msgTypes->msgTypes != NULL) {
        HASH_ITER(hh, msgTypes->msgTypes->groups, current, tmp) {
            HASH_DEL(msgTypes->msgTypes->groups, current);
            free_groupdef(current);
        }
        if (msgTypes->msgTypes->numgroups) {
            free(msgTypes->msgTypes->allgroups);
        }
        free(msgTypes->msgTypes);
    }
    PyObject_Free(obj);
}

groupsForType_t *alloc_groupsForType() {
    groupsForType_t *returned = malloc(sizeof(groupsForType_t));
    returned->msgType = 0;
    returned->numgroups = 0;
    returned->allgroups = NULL;
    returned->groups = NULL;
    return returned;
}

PyObject *addGroupMaps(PyObject *self, PyObject *args) {
    char *msgTypeStr;
    char *tupledef = "shO!";
    uint16_t countTag;
    PyObject *taglist;
    msgTypes_t *msgTypes = (msgTypes_t *) self;
    int res = PyArg_ParseTuple(args, tupledef, &msgTypeStr, &countTag, &PyList_Type, &taglist);
    if (!res) {
        return NULL;
    }
    groupsForType_t *msgType;
    uint64_t msgTypeInt = msgTypeToUint64(msgTypeStr);
    HASH_FIND(hh,
              msgTypes->msgTypes,
              &msgTypeInt,
              sizeof(uint64_t),
              msgType);
    if (msgType == NULL) {
        msgType = alloc_groupsForType();
        msgType->msgType = msgTypeInt;
        HASH_ADD(hh,
                 msgTypes->msgTypes,
                 msgType,
                 sizeof(uint64_t),
                 msgType);
    }
    if (!addToMap(msgType, countTag, taglist)) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyMethodDef objMethods[] = {
        {"addGroupMaps", addGroupMaps, METH_VARARGS, "add group to map"},
        {NULL, NULL,                   0, NULL}
};


PyTypeObject MsgTypeMap_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        "pyfixlib.MsgTypeMap", /* tp_name */
        sizeof(msgTypes_t),                          /* tp_basicsize */
        0,                                                   /* tp_itemsize */
        0,                                                   /* tp_dealloc */
        0,                                                   /* tp_print */
        0,                                                   /* tp_getattr */
        0,                                                   /* tp_setattr */
        0,                                                   /* tp_reserved */
        0,                                                   /* tp_repr */
        0,                                                   /* tp_as_number */
        0,                                                   /* tp_as_sequence */
        0,                                    /* tp_as_mapping */
        0,                                                   /* tp_hash  */
        0,                                                   /* tp_call */
        0,                                                   /* tp_str */
        0,                                                   /* tp_getattro */
        0,                                                   /* tp_setattro */
        0,                                                   /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                                  /* tp_flags */
        "An opaque type for message types",                    /* tp_doc */
        0,                                                   // tp_traverse
        0,                                                   // tp_clear
        0,                                                   // tp_richcompare
        0,                                                   // tp_wearklistoffset
        0,                                                   // tp_iter
        0,                                                   // tp_iternext
        objMethods,                                                   // tp_methods
        0,                                                   // tp_members
        0,                                                   // tp_getset
        0,                                                   // tp_base
        0,                                                   // tp_dict
        0,                                                   // tp_descr_get
        0,                                                   // tp_descr_set
        0,                                                   // tp_dictoffset
        allocMsgTypes,                                           // tp_init
        0,                                                   // tp_alloc,
        0,                                                   // tp_new
        freeMsgTypes,                                          // tp_free
        0,                                                   // tp_is_gc
        0,                                                   // tp_bases
        0,                                                   // tp_mro
        0,                                                   // tp_cache
        0,                                                   // tp_subclasses
        0,                                                   // tp_weaklist
        0                                                    // tp_del

};
