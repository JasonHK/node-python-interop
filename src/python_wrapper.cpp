#include "python_wrapper.hpp"
#include "internal_helpers.h"

Napi::FunctionReference NPI::WrappedPythonObject::m_constructor;

Napi::Object NPI::WrappedPythonObject::New(Napi::Env env, PyObject* python_value)
{
    auto python_value_ref = Napi::External<PyObject>::New(env, python_value);
    return Constructor().New({ python_value_ref });
}

Napi::Object NPI::WrappedPythonObject::Init(Napi::Env env, Napi::Object exports)
{
    auto function = DefineClass(env, STRINGIFY(WrappedPythonObject),
        {

        });

    m_constructor = Napi::Persistent(function);
    m_constructor.SuppressDestruct();

    exports.Set("WrappedPythonObject", function);
    return exports;
}

NPI::WrappedPythonObject::WrappedPythonObject(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<WrappedPythonObject>(info)
{
    m_python_value = info[0].As<Napi::External<PyObject>>().Data();
    Py_INCREF(m_python_value);
}

NPI::WrappedPythonObject::~WrappedPythonObject()
{
    Py_DECREF(m_python_value);
}
