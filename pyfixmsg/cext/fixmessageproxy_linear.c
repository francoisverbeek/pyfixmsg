//
//  fixMessageProxyLinear.c
//  pyfixlib
//

#define PY_SSIZE_T_CLEAN
#ifndef __fixmsgproxy_h
#define __fixmsgproxy_h

#include <Python.h>
#include <stdbool.h>
#include "fixmsgproxy.h"
#include "msgTypesMap.h"
#include "groupdef.h"
#include "parse.h"


Py_ssize_t LproxyLen(PyObject *proxy) {
    fixMessageProxyLinear_t *self = (fixMessageProxyLinear_t *) proxy;
    Py_ssize_t tag_count = self->numtags - self->cachedtags;
    if (self->tags->other_tags != NULL) {
        tag_count += PyDict_Size(self->tags->other_tags);
    }
    return tag_count;
}

static tagentry_t *getTagFrom(fixMessageProxyLinear_t *self,
                              size_t offset,
                              size_t intkey,
                              size_t *found_offset) {
    tagentry_t *returned;
    while (offset < self->numtags * sizeof(tagentry_t)) {
        returned = self->tags->tags + offset;
        if (returned->tag == intkey) {
            *found_offset = offset;
            return returned;
        }
        offset += sizeof(tagentry_t);
    }
    return NULL;
}

static groupdef_t *getGroup(fixMessageProxyLinear_t *self, tagentry_t *tagentry) {
    groupdef_t *group = NULL;
    int inttag = (int) tagentry->tag;
    if (self->groups != NULL) {
        HASH_FIND_INT(self->groups->groups, &inttag, group);
    }
    return group;
};

static bool tagincomposition(int16_t tag, groupdef_t *group) {
    for (uint16_t i = 0; i < group->compositionlen; i++) {
        if (tag == group->composition[i]) {
            return true;
        }
    }
    return false;
}

static PyObject *addTagEntryToDict(PyObject *dict, tagentry_t *tagentry, fixMessageProxyLinear_t *self) {
    // TODO Error handling
    uint64_t start = tagentry->start_offset;
    size_t len = tagentry->len;
    PyObject *value = PyBytes_FromStringAndSize(self->sourcestr + start, len);
    PyObject *key = PyLong_FromLong(tagentry->tag);
    assert(0 < Py_REFCNT(value));
    assert(0 < Py_REFCNT(key));
    PyDict_SetItem(dict, key, value);
    self->cachedtags++; /* so we can still count the tags correctly: otherwise numtags + number of keys in
                                    dictionary > actual number of tags */
    return value;
}

static PyObject *createListFromGroup(fixMessageProxyLinear_t *self, size_t offset, groupdef_t *group) {
    PyObject *returned = PyList_New(0);
    PyObject *currentDict = NULL;
    uint16_t first_tag = 0;
    // using first tag strategy i.e. first tag of the group MUST be repeated
    while (offset < self->numtags * sizeof(tagentry_t)) {
        tagentry_t *tagentry = self->tags->tags + offset;
        if (!tagincomposition(tagentry->tag, group)) {
            return returned;
        }
        if (first_tag == 0 || tagentry->tag == first_tag) {
            first_tag = tagentry->tag;
            currentDict = PyDict_New();
            PyList_Append(returned, currentDict);
            assert(0 < Py_REFCNT(currentDict));
        }
        addTagEntryToDict(currentDict, tagentry, self);
        offset += sizeof(tagentry_t);
    }
    return returned;
}

PyObject *returnedTag(fixMessageProxyLinear_t *self, tagentry_t *tagentry) {
    PyObject *returned;
    // cache the result in the dictionary
    if (self->tags->other_tags == NULL) {
        self->tags->other_tags = PyDict_New();
        assert(1 == Py_REFCNT(self->tags->other_tags));
    }
    returned = addTagEntryToDict(self->tags->other_tags, tagentry, self);
    assert(Py_REFCNT(returned) > 0);
    return returned;
}

PyObject *LproxyGet(PyObject *proxy, PyObject *key) {
    fixMessageProxyLinear_t *self = (fixMessageProxyLinear_t *) proxy;
    PyObject *returned = NULL;
    if (self->tags->other_tags != NULL) {
        returned = PyDict_GetItem(self->tags->other_tags, key);
        if (returned != NULL) {
            Py_INCREF(returned);
            return returned;
        }
    }
    if (PyLong_Check(key)) {
        size_t intkey = PyLong_AsSize_t(key);
        if (intkey < 0xffff) {
            size_t foundat = 0;
            tagentry_t *tagentry = getTagFrom(self, 0, intkey, &foundat);
            if (tagentry != NULL) {
                groupdef_t *group = getGroup(self, tagentry);
                if (group) {
                    // the group starts on the next tag
                    returned = createListFromGroup(self, foundat + sizeof(tagentry_t), group);
                } else {
                    returned = returnedTag(self, tagentry);
                }
                memset(tagentry, 0, sizeof(tagentry_t));
                return returned;
            }
        }
    }
    PyErr_SetString(PyExc_KeyError, "Tag not found");
    return returned;
}


int LproxySet(PyObject *proxy, PyObject *key, PyObject *value) {
    fixMessageProxyLinear_t *self = (fixMessageProxyLinear_t *) proxy;
    if (PyLong_Check(key)) {
        size_t intkey = PyLong_AsSize_t(key);
        size_t foundat = 0;
        tagentry_t *tagentry = getTagFrom(self, 0, intkey, &foundat);
        if (tagentry != NULL) {
            memset(tagentry, 0, sizeof(tagentry_t));
        }
        Py_INCREF(key);
        Py_INCREF(value);
        PyDict_SetItem(self->tags->other_tags, key, value);
    }
    return 0;
}


PyMappingMethods LproxyMapping = {LproxyLen, LproxyGet, LproxySet};

PyMethodDef methods[] = {

        {NULL, NULL, 0, NULL}
};

// init
int create_linear(PyObject *obj, PyObject *args, PyObject *kwargs) {
    fixMessageProxyLinear_t *self = (fixMessageProxyLinear_t *) obj;
    self->sourcestr = NULL;
    self->tags = NULL;
    self->numtags = 0;
    self->cachedtags = 0;
    self->msgTypesRef = NULL;
    if (kwargs != NULL) {
        PyErr_SetString(PyExc_AttributeError,
                        "Keyword arguments are not supported");
        return 0;
    }
    char *data;
    Py_ssize_t len;
    PyObject *PymsgTypes = NULL;
    char separator = 1;
    char delimiter = '=';
    if (!PyArg_ParseTuple(args, "y#|O!CC", &data, &len, &MsgTypeMap_Type, &PymsgTypes, &separator, &delimiter)) {
        return 0;
    }

    self->sourcestr = malloc(len);
    self->len = len;
    self->groups = NULL;
    memcpy(self->sourcestr, data, len);

    self->tags = findtags_linear(self->sourcestr, len, separator, delimiter, &self->numtags);
    groupsForType_t *groups = NULL;
    msgTypes_t *msgTypes = (msgTypes_t *) PymsgTypes;
    if (msgTypes != NULL && self->tags->msgType != 0) {
        HASH_FIND(hh, msgTypes->msgTypes, &(self->tags->msgType), sizeof(uint64_t), groups);
        self->groups = groups;
        self->msgTypesRef = PymsgTypes;
        Py_INCREF(self->msgTypesRef);
    }
    return 1;
}

// del
void destroy_linear(PyObject *obj) {
    fixMessageProxyLinear_t *self = (fixMessageProxyLinear_t *) obj;
    assert(Py_REFCNT(self) == 0);
    if (self->sourcestr != NULL) {
        free(self->sourcestr);
        self->sourcestr = NULL;
    };
    if (self->tags != NULL) {
        free_tags_linear(self->tags);
        self->tags = NULL;
    };
    Py_XDECREF(self->msgTypesRef);
    PyObject_Del(obj);
}

int alloc_iterator(PyObject *obj, PyObject *args, PyObject *kwargs) {
    fixMessageIterator_t *self = (fixMessageIterator_t *) obj;
    PyObject *fixMsg = (PyObject *) args;
    Py_INCREF(fixMsg);
    self->msg = (fixMessageProxyLinear_t *) fixMsg;
    self->next_tag = self->msg->tags->tags;
    if (self->msg->tags->other_tags != NULL) {
        self->iterator_in_dict = PyObject_GetIter(self->msg->tags->other_tags);
    } else { self->iterator_in_dict = NULL; }
    return 1;
}

PyObject *iterproxy(PyObject *args) {
    PyObject *fixMsg = args;
    // int res = PyArg_ParseTuple(args, "O",  &fixMsg);
    // if (!res) return NULL;
    fixMessageProxyLinear_t *iterator = PyObject_New(fixMessageProxyLinear_t, &PythonFixMsgIterator);
    alloc_iterator((PyObject *) iterator, fixMsg, NULL);
    return (PyObject *) iterator;
}

void destroy_iterator(PyObject *obj) {
    fixMessageIterator_t *self = (fixMessageIterator_t *) obj;
    Py_DECREF((PyObject *) self->msg);
    Py_XDECREF(self->iterator_in_dict);
    PyObject_Del(obj);
}

PyObject *iterator_self(PyObject *self) {
    Py_INCREF(self);
    return self;
}

// as tagentry_t are 64 bits we can cast
// to uint64_t for fast comparison to 0

typedef union {
    uint64_t i;
    tagentry_t t;
} tagentryAsInt;

PyObject *iterator_next(PyObject *obj) {
    fixMessageIterator_t *self = (fixMessageIterator_t *) obj;
    fixMessageProxyLinear_t *msg = self->msg;

    tagentry_t *last_tagentry = msg->tags->tags + ((msg->numtags - 1) * sizeof(tagentry_t));
    PyObject *returned = NULL;
    while (self->next_tag <= last_tagentry) {
        tagentryAsInt *tagentry = (tagentryAsInt *) self->next_tag;
        if (tagentry->i == 0) {
            self->next_tag += 1;
            continue;
        }
        returned = PyLong_FromLong(self->next_tag->tag);
        Py_INCREF(returned);
        self->next_tag += 1;
        return returned;
    }
    if (self->iterator_in_dict != NULL) {
        returned = PyIter_Next(self->iterator_in_dict);
        if (returned != NULL) {
            return returned;
        };
        Py_DECREF(self->iterator_in_dict);
    };
    // reset the loop
    self->next_tag = msg->tags->tags;
    return NULL;
}

PyTypeObject PythonFixMsg = {
        PyVarObject_HEAD_INIT(NULL, 0) "pyfixlib.FixMsg",  /* tp_name */
        sizeof(fixMessageProxyLinear_t),                   /* tp_basicsize */
        0,                                                 /* tp_itemsize */
        destroy_linear,                                    /* tp_dealloc */
        0,                                                 /* tp_print */
        0,                                                 /* tp_getattr */
        0,                                                 /* tp_setattr */
        0,                                                 /* tp_reserved */
        0,                                                 /* tp_repr */
        0,                                                 /* tp_as_number */
        0,                                                 /* tp_as_sequence */
        &LproxyMapping,                                    /* tp_as_mapping */
        0,                                                 /* tp_hash  */
        0,                                                 /* tp_call */
        0,                                                 /* tp_str */
        0,                                                 /* tp_getattro */
        0,                                                 /* tp_setattro */
        0,                                                 /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                                /* tp_flags */
        "A fast Fix message access type",                  /* tp_doc */
        0,                                                 // tp_traverse
        0,                                                 // tp_clear
        0,                                                 // tp_richcompare
        0,                                                 // tp_wearklistoffset
        iterproxy,                                         // tp_iter
        0,                                                 // tp_iternext
        methods,                                          // tp_methods
        0,                                                 // tp_members
        0,                                                 // tp_getset
        0,                                                 // tp_base
        0,                                                 // tp_dict
        0,                                                 // tp_descr_get
        0,                                                 // tp_descr_set
        0,                                                 // tp_dictoffset
        create_linear,                                     // tp_init
        0,                                                 // tp_alloc,
        0,                                                 // tp_new
        0,                                                 // tp_free
        0,                                                 // tp_is_gc
        0,                                                 // tp_bases
        0,                                                 // tp_mro
        0,                                                 // tp_cache
        0,                                                 // tp_subclasses
        0,                                                 // tp_weaklist
        0                                                  // tp_del

};
PyTypeObject PythonFixMsgIterator = {
        PyVarObject_HEAD_INIT(NULL, 0)
        "pyfixlib._FixMsgIterator",  /* tp_name */
        sizeof(fixMessageIterator_t),                   /* tp_basicsize */
        0,                                                 /* tp_itemsize */
        destroy_iterator,                                                 /* tp_dealloc */
        0,                                                 /* tp_print */
        0,                                                 /* tp_getattr */
        0,                                                 /* tp_setattr */
        0,                                                 /* tp_reserved */
        0,                                                 /* tp_repr */
        0,                                                 /* tp_as_number */
        0,                                                 /* tp_as_sequence */
        0,                                                 /* tp_as_mapping */
        0,                                                 /* tp_hash  */
        0,                                                 /* tp_call */
        0,                                                 /* tp_str */
        0,                                                 /* tp_getattro */
        0,                                                 /* tp_setattro */
        0,                                                 /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                                /* tp_flags */
        "an iterator inside a fast fixmsg",                /* tp_doc */
        0,                                                 // tp_traverse
        0,                                                 // tp_clear
        0,                                                 // tp_richcompare
        0,                                                 // tp_wearklistoffset
        iterator_self,                                     // tp_iter
        iterator_next,                                     // tp_iternext
        0,                                                 // tp_methods
        0,                                                 // tp_members
        0,                                                 // tp_getset
        0,                                                 // tp_base
        0,                                                 // tp_dict
        0,                                                 // tp_descr_get
        0,                                                 // tp_descr_set
        0,                                                 // tp_dictoffset
        alloc_iterator,                                    // tp_init
        0,                                                 // tp_alloc,
        0,                                                 // tp_new
        0,                                  // tp_free
        0,                                                 // tp_is_gc
        0,                                                 // tp_bases
        0,                                                 // tp_mro
        0,                                                 // tp_cache
        0,                                                 // tp_subclasses
        0,                                                 // tp_weaklist
        0                                                  // tp_del

};
#endif
