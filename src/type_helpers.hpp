#ifndef NPI_TYPE_HELPERS_HPP
#define NPI_TYPE_HELPERS_HPP

#include "type_helpers.h"
#include <napi.h>

namespace NPI
{
    bool IsNullLike(const Napi::Value&);

    Napi::Value ToNodeValue(const Napi::Env&, PyObject*);

    Napi::Value ToNodeArray(const Napi::Env&, PyObject*);

    PyObject* ToPythonObject(const Napi::Value&);

    PyObject* ToPythonList(const Napi::Env&, const Napi::Value&);
};

#endif
