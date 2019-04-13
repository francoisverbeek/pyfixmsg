//
//  module.c
//  pyfixlib
//


#define PY_SSIZE_T_CLEAN

#include "util.h"
#include "fixmsgproxy.h"
#include "rgroups.h"


static PyModuleDef fixlib = {
        PyModuleDef_HEAD_INIT,
        "libpyfix",
        NULL,
        -1,
        NULL,
};

PyMODINIT_FUNC PyInit_libpyfix(void) {
    PyObject *m;

    PythonFixMsg.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PythonFixMsg) < 0)
        return NULL;
    MsgTypeMap_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&MsgTypeMap_Type) < 0) {
        return NULL;
    }
    m = PyModule_Create(&fixlib);
    if (m == NULL) {
        return NULL;
    }

    Py_INCREF(&PythonFixMsg);
    Py_INCREF(&MsgTypeMap_Type);

    PyModule_AddObject(m, "FixMsg", (PyObject * ) & PythonFixMsg);
    PyModule_AddObject(m, "MsgTypes", (PyObject * ) & MsgTypeMap_Type);

    return m;
}
