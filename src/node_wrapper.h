#ifndef NPI_NODE_WRAPPER_H
#define NPI_NODE_WRAPPER_H

#include <node_api.h>
#include <Python.h>

#ifdef __cplusplus
extern "C"
{
#endif

PyMODINIT_FUNC PyInit_node_wrapper(void);

PyObject* NPI_WrappedNodeObject_FromNode(napi_env, napi_value);

napi_value NPI_WrappedNodeObject_GetNodeValue(PyObject*);

#ifdef __cplusplus
}
#endif

#endif
