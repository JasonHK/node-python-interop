#include "type_helpers.h"
#include "type_helpers.hpp"
#include "python_wrapper.hpp"

namespace NPI
{
    bool IsInteger(const Napi::Env& env, const Napi::Value& payload);

    bool IsWrappedPythonObject(const Napi::Object& payload);
}

PyObject* NPI::ToPythonValue(const Napi::Value& node_value)
{
    auto node_env = node_value.Env();

    if (node_value.IsUndefined() || node_value.IsNull())
    {
        Py_RETURN_NONE;
    }
    else if (node_value.IsBoolean())
    {
        auto value = node_value.As<Napi::Boolean>().Value();
        return PyBool_FromLong(value);
    }
    else if (node_value.IsNumber())
    {
    }
    else if (node_value.IsString())
    {
        auto value = node_value.As<Napi::String>().Utf8Value();
        return PyUnicode_FromString(value.c_str());
    }
    else if (node_value.IsArray())
    {
        return ToPythonList(node_env, node_value);
    }
    else if (node_value.IsObject())
    {
        if (IsWrappedPythonObject(node_value.ToObject()))
        {
            auto object = WrappedPythonObject::Unwrap(node_value.ToObject());
            return object->Value();
        }
        else
        {

        }
    }

    // throw Napi::Error::New
}

PyObject* NPI::ToPythonList(const Napi::Env& node_env, const Napi::Value& node_value)
{
    auto node_array = node_value.As<Napi::Array>();

    auto length      = node_array.Length();
    auto python_list = PyList_New(length);

    for (size_t i = 0; i < length; i++)
    {
        if (!node_array.Has(i)) { continue; }

        auto node_element   = node_array.Get(i);
        auto python_element = ToPythonValue(node_element);

        PyList_SetItem(python_list, i, python_element);
    }

    return python_list;
}

Napi::Value NPI::ToNodeValue(const Napi::Env& node_env, PyObject* python_value)
{
    if (python_value == Py_None)
    {
        return node_env.Null();
    }
    else if (PyBool_Check(python_value))
    {
        return Napi::Boolean::New(node_env, PyObject_IsTrue(python_value));
    }
    else if (PyLong_Check(python_value))
    {
        
    }
    else if (PyFloat_Check(python_value))
    {
        auto value = PyFloat_AsDouble(python_value);
        return Napi::Number::New(node_env, value);
    }
    else if (PyUnicode_Check(python_value))
    {
        auto value = PyUnicode_AsUTF8(python_value);
        return Napi::String::New(node_env, value);
    }
    else if (PyList_Check(python_value))
    {
        return ToNodeArray(node_env, python_value);
    }
    else
    {
        // auto python_value_ref = Napi::External<PyObject>::New(node_env, python_value);
        // return WrappedPythonObject::constructor.New({ python_value_ref });
        return WrappedPythonObject::New(node_env, python_value);
    }
}

Napi::Value NPI::ToNodeArray(const Napi::Env& node_env, PyObject* python_value)
{
    auto length     = PySequence_Fast_GET_SIZE(python_value);
    auto node_array = Napi::Array::New(node_env, length);

    for (Py_ssize_t i = 0; i < length; i++)
    {
        auto python_element = PySequence_Fast_GET_ITEM(python_value, i);
        auto node_element   = ToNodeValue(node_env, python_element);

        node_array.Set(i, node_element);
    }

    return node_array;
}

bool NPI::IsInteger(const Napi::Env& env, const Napi::Value& payload)
{
    return env.Global()
              .Get("Number")
              .ToObject()
              .Get("isInteger")
              .As<Napi::Function>()
              .Call({ payload })
              .ToBoolean()
              .Value();
}

bool NPI::IsWrappedPythonObject(const Napi::Object& payload)
{
    return payload.InstanceOf(WrappedPythonObject::Constructor().Value());
    return false;
}
