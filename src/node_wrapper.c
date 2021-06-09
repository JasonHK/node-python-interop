#define PY_SSIZE_T_CLEAN

#include "node_wrapper.h"
#include "internal_helpers.h"
#include "type_helpers.h"
#include <structmember.h>

typedef struct
{
    PyObject_HEAD

    napi_ref node_ref;
    napi_env node_env;
    napi_value node_bound;
} NPI_WrappedNodeObject;

static void NPI_WrappedNodeObject_dealloc(NPI_WrappedNodeObject* self);

static PyObject* NPI_WrappedNodeObject_call(PyObject* self, PyObject* args, PyObject* kwargs);

static int NPI_WrappedNodeObject_init(NPI_WrappedNodeObject* self, PyObject* args, PyObject* kwargs);

static PyObject* NPI_WrappedNodeObject_new(PyTypeObject* type, PyObject* args, PyObject* kwargs);

static PyTypeObject NPI_WrappedNodeObject_Type =
{
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "npi.WrappedNodeObject",
    .tp_doc       = "",
    .tp_basicsize = sizeof(NPI_WrappedNodeObject),
    .tp_itemsize  = 0,
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_dealloc   = NPI_WrappedNodeObject_dealloc,
    .tp_call      = NPI_WrappedNodeObject_call,
    .tp_init      = NPI_WrappedNodeObject_init,
    .tp_new       = NPI_WrappedNodeObject_new,
};

static void NPI_WrappedNodeObject_dealloc(NPI_WrappedNodeObject* self)
{
    if (self->node_ref != NULL)
    {
        napi_delete_reference(self->node_env, self->node_ref);
        self->node_ref = NULL;
    }

    Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyObject* NPI_WrappedNodeObject_call(PyObject* self, PyObject* args, PyObject* kwargs)
{
    NPI_WrappedNodeObject* casted_self = (NPI_WrappedNodeObject*) self;
    PyObject* python_return;

    PyObject*  sequence = PySequence_Fast(args, "");
    Py_ssize_t length   = PySequence_Size(args);

    napi_value* node_args = calloc(length, sizeof(napi_value));
    if (node_args == NULL)
    {
        PyErr_SetString(PyExc_MemoryError, "Out of memory while allocating arguments array for Node.");
        goto finally;
    }

    napi_env node_env = &(casted_self->node_env);

    for (Py_ssize_t i = 0; i < length; i++)
    {
        PyObject*  python_arg = PySequence_Fast_GET_ITEM(sequence, i);
        napi_value node_arg   = NPI_PythonValueToNodeValue(node_env, python_arg);

        node_args[i] = node_arg;
    }

    Py_DECREF(sequence);

    napi_value node_function;
    napi_get_reference_value(node_env, casted_self->node_ref, &node_function);

    napi_value node_receiver;
    if (casted_self->node_bound != NULL)
    {
        node_receiver = casted_self->node_bound;
    }
    else
    {
        if (napi_get_global(node_env, &node_receiver))
        {
            PyErr_SetString(PyExc_RuntimeError, "Failed to retrieve the global object of Node.");
            goto finally;
        }
    }

    napi_value node_return;
    if (napi_call_function(node_env, node_receiver, node_function, length, node_args, &node_return))
    {
        goto finally;
    }

    python_return = NPI_NodeValueToPythonValue(node_env, node_return);
    if (python_return == NULL)
    {
        goto finally;
    }

finally:
    if (node_args != NULL)
    {
        free(node_args);
        node_args = NULL;
    }

    return python_return;
}

static int NPI_WrappedNodeObject_init(NPI_WrappedNodeObject* self, PyObject* args, PyObject* kwargs)
{
    if (!PyArg_ParseTuple(args, STRINGIFY(args) " must be a sequence."))
    {
        return -1;
    }

    return 0;
}

static PyObject* NPI_WrappedNodeObject_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    NPI_WrappedNodeObject* self = (NPI_WrappedNodeObject*) type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->node_ref = NULL;
        self->node_env = NULL;
        self->node_bound = NULL;
    }

    return (PyObject*) self;
}

static void NPI_WrappedNodeObject_AssignNodeValue(NPI_WrappedNodeObject* target, napi_env node_env, napi_value node_value)
{
    target->node_env = node_env;

    if (target->node_ref != NULL)
    {
        napi_delete_reference(node_env, target->node_ref);
        target->node_ref = NULL;
    }

    napi_create_reference(node_env, node_value, 1, &(target->node_ref));
}

static PyMemberDef NPI_WrappedNodeObject_Members[] =
{
    {NULL},
};

static PyObject* NPI_WrappedNodeObject_getattro(PyObject* self, PyObject* attr)
{
    NPI_WrappedNodeObject* c_self = (NPI_WrappedNodeObject*) self;

    napi_value node_value;

    bool has_attr;

    const char* node_property_name = PyUnicode_AsUTF8(attr);
    napi_value node_property_value;
    napi_valuetype node_property_type;

    napi_get_reference_value(c_self->node_env, c_self->node_ref, &node_value);
    napi_has_named_property(c_self->node_env, node_value, node_property_name, &has_attr);

    if (has_attr)
    {
        bool is_function;

        napi_get_named_property(c_self->node_env, node_value, node_property_name, &node_property_value);
        napi_typeof(c_self->node_env, node_property_value, &node_property_type);

        is_function = (node_property_type == napi_function);
    }
}

napi_value NPI_WrappedNodeObject_GetNodeValue(PyObject* target)
{
    NPI_WrappedNodeObject* c_target = (NPI_WrappedNodeObject*) target;

    napi_value value;
    napi_get_reference_value(c_target->node_env, c_target->node_ref, &value);

    return value;
}
