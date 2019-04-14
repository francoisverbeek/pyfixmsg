#ifndef __fixmessageproxy_linear_h
#define __fixmessageproxy_linear_h

#include <Python.h>
#include "parse.h"
#include "msgTypesMap.h"

typedef struct {
    PyObject_HEAD;
    char *sourcestr;
    Py_ssize_t len;
    tags_t *tags;
    size_t numtags;
    size_t cachedtags;
    groupsForType_t *groups;
    PyObject *msgTypesRef; // 'groups' above is a pointer within here, need to keep a reference
} fixMessageProxyLinear_t;

typedef struct {
    PyObject_HEAD;
    fixMessageProxyLinear_t *msg;
    tagentry_t *next_tag; // pointer to the next tag
    PyObject *iterator_in_dict; // iterator inside the dict that contains added or non-int tags
} fixMessageIterator_t;

Py_ssize_t LproxyLen(PyObject *proxy);

PyObject *LproxyGet(PyObject *proxy, PyObject *key);

int LproxySet(PyObject *proxy, PyObject *key, PyObject *value);


// init
int create_linear(PyObject *obj, PyObject *args, PyObject *kwargs);

// del
void destroy_linear(PyObject *obj);

extern PyTypeObject PythonFixMsg;
extern PyTypeObject PythonFixMsgIterator;


#endif
