#ifndef NPI_TYPE_HELPERS_H
#define NPI_TYPE_HELPERS_H

#include <node_api.h>
#include <Python.h>

#ifdef __cplusplus
extern "C"
{
#endif

napi_value NPI_PythonValueToNodeValue(napi_env node_env, PyObject* python_value);

PyObject* NPI_NodeValueToPythonValue(napi_env node_env, napi_value node_value);

#ifdef __cplusplus
}
#endif

#endif
