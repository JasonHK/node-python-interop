#ifndef NPI_TYPE_HELPERS_HPP
#define NPI_TYPE_HELPERS_HPP

#include "type_helpers.h"
#include <napi.h>

namespace NPI
{
    PyObject* ToPythonValue(const Napi::Value&);

    PyObject* ToPythonList(const Napi::Env&, const Napi::Value&);

    Napi::Value ToNodeValue(const Napi::Env&, PyObject*);

    Napi::Value ToNodeArray(const Napi::Env&, PyObject*);
};

#endif
