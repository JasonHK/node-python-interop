#include "type_helpers.h"
#include "type_helpers.hpp"
#include "python_wrapper.hpp"

#define UINT64_SIZE sizeof(uint64_t)

#if LONG_WIDTH == INT64_WIDTH
    #define PyLong_AsInt64(object) PyLong_AsLong(object);
#elif LLONG_WIDTH == INT64_WIDTH
    #define PyLong_AsInt64(object) PyLong_AsLongLong(object);
#endif

#if ULONG_WIDTH == UINT64_WIDTH
    #define PyLong_AsUint64(object) PyLong_AsUnsignedLong(object);
#elif ULLONG_WIDTH == UINT64_WIDTH
    #define PyLong_AsUint64(object) PyLong_AsUnsignedLongLong(object);
#endif

namespace NPI
{
    bool IsSafeInteger(const Napi::Env& env, const Napi::Value& payload);

    bool IsWrappedPythonObject(const Napi::Object& payload);

    /**
     * Convert a PyLongObject into a Napi::BigInt.
     * 
     * @param n_env  The current Node environment.
     * @param p_long The PyLongObject to convert.
     */
    Napi::BigInt ToNodeBigInt(const Napi::Env &n_env, PyObject *p_long);

    /**
     * Convert a Napi::BigInt into a PyLongObject.
     * 
     * @param n_env    The current Node environment.
     * @param n_bigint The Napi::BigInt to convert.
     */
    PyObject* ToPythonLong(const Napi::Env &n_env, Napi::BigInt n_bigint);
}

bool NPI::IsNullLike(const Napi::Value& payload)
{
    return (payload.IsNull() || payload.IsUndefined());
}

Napi::Value NPI::ToNodeValue(const Napi::Env &n_env, PyObject *p_object)
{
    if ((p_object == NULL))
    {
        return n_env.Null();
    }
    if (p_object == Py_None)
    {
        return n_env.Undefined();
    }
    else if (PyBool_Check(p_object))
    {
        return Napi::Boolean::New(n_env, PyObject_IsTrue(p_object));
    }
    else if (PyLong_Check(p_object))
    {
        return ToNodeBigInt(n_env, p_object);
    }
    else if (PyFloat_Check(p_object))
    {
        auto number = PyFloat_AsDouble(p_object);
        return Napi::Number::New(n_env, number);
    }
    else if (PyUnicode_Check(p_object))
    {
        auto string = PyUnicode_AsUTF8(p_object);
        return Napi::String::New(n_env, string);
    }
    else if (PyList_Check(p_object))
    {
        return ToNodeArray(n_env, p_object);
    }
    else
    {
        // auto python_value_ref = Napi::External<PyObject>::New(node_env, python_value);
        // return WrappedPythonObject::constructor.New({ python_value_ref });
        return WrappedPythonObject::New(n_env, p_object);
    }
}

PyObject* NPI::ToPythonObject(const Napi::Value &n_value)
{
    auto n_env = n_value.Env();

    if (n_value.IsUndefined() || n_value.IsNull())
    {
        Py_RETURN_NONE;
    }
    else if (n_value.IsBoolean())
    {
        auto value = n_value.As<Napi::Boolean>().Value();
        return PyBool_FromLong(value);
    }
    else if (n_value.IsNumber())
    {
        auto value = n_value.As<Napi::Number>().DoubleValue();
        return PyFloat_FromDouble(value);
    }
    else if (n_value.IsBigInt())
    {
        return ToPythonLong(n_env, n_value.As<Napi::BigInt>());
    }
    else if (n_value.IsString())
    {
        auto value = n_value.As<Napi::String>().Utf8Value();
        return PyUnicode_FromString(value.c_str());
    }
    else if (n_value.IsArray())
    {
        return ToPythonList(n_env, n_value);
    }
    else if (n_value.IsObject())
    {
        if (IsWrappedPythonObject(n_value.ToObject()))
        {
            auto object = WrappedPythonObject::Unwrap(n_value.ToObject());
            return object->Value();
        }
        else
        {

        }
    }

    // throw Napi::Error::New
}

Napi::BigInt NPI::ToNodeBigInt(const Napi::Env &n_env, PyObject *p_long)
{
    auto is_negative = (_PyLong_Sign(p_long) == -1);
    auto bits_length = _PyLong_NumBits(p_long);

    // Use a faster approach when the integer is small enough to fit in an int64_t or uint64_t.
    if (bits_length < INT64_WIDTH)
    {
        auto value = PyLong_AsInt64(p_long);
        return Napi::BigInt::New(n_env, value);
    }
    else if ((bits_length == UINT64_WIDTH) && !is_negative)
    {
        auto value = PyLong_AsUint64(p_long);
        return Napi::BigInt::New(n_env, value);
    }

    // Strip the sign when the PyLongObject is a negative integer.
    if (is_negative) { p_long = PyNumber_Negative(p_long); }

    auto words_length = (bits_length / UINT64_WIDTH) + (bits_length % UINT64_WIDTH);
    auto bytes_length = words_length * UINT64_SIZE;

    auto words = new uint64_t[words_length];
    auto bytes = new uint8_t[bytes_length];

    _PyLong_AsByteArray((PyLongObject*) p_long, bytes, bytes_length, true, false);

    // Decrement the reference count for the PyLongObject if it's originally a negative integer,
    // since it's owned by this function.
    if (is_negative) { Py_DECREF(p_long); }

    std::memcpy(words, bytes, bytes_length);
    std::free(bytes);
    
    auto n_bigint = Napi::BigInt::New(n_env, is_negative, words_length, words);
    std::free(words);

    return n_bigint;
}

PyObject* NPI::ToPythonLong(const Napi::Env &n_env, Napi::BigInt n_bigint)
{
    // if (n_bigint.WordCount() == 1)
    // {
    //     int is_negative;
    //     n_bigint.ToWords(&is_negative, &words_length, words);
    // }

    auto words_length = n_bigint.WordCount();
    auto bytes_length = words_length * sizeof(uint64_t);

    auto words = new uint64_t[words_length];
    auto bytes = new uint8_t[bytes_length];

    int is_negative;
    n_bigint.ToWords(&is_negative, &words_length, words);

    std::memcpy(bytes, words, bytes_length);
    std::free(words);

    auto p_long = _PyLong_FromByteArray(bytes, bytes_length, true, false);
    std::free(bytes);

    // Add the sign if the Napi::BigInt instance is a negative integer.
    if (is_negative)
    {
        auto p_long_signed = PyNumber_Negative(p_long);
        Py_DECREF(p_long);

        p_long = p_long_signed;
    }

    return p_long;
}

Napi::Value NPI::ToNodeArray(const Napi::Env &n_env, PyObject *p_sequence)
{
    auto length  = PySequence_Size(p_sequence);
    auto n_array = Napi::Array::New(n_env, length);

    for (Py_ssize_t i = 0; i < length; i++)
    {
        auto p_element = PySequence_GetItem(p_sequence, i);
        auto n_element = ToNodeValue(n_env, p_element);

        n_array.Set(i, n_element);
    }

    return n_array;
}

PyObject* NPI::ToPythonList(const Napi::Env &n_env, const Napi::Value &node_value)
{
    auto node_array = node_value.As<Napi::Array>();

    auto length      = node_array.Length();
    auto python_list = PyList_New(length);

    for (size_t i = 0; i < length; i++)
    {
        if (!node_array.Has(i)) { continue; }

        auto node_element   = node_array.Get(i);
        auto python_element = ToPythonObject(node_element);

        PyList_SetItem(python_list, i, python_element);
    }

    return python_list;
}

bool NPI::IsSafeInteger(const Napi::Env& env, const Napi::Value& payload)
{
    return env.Global()
              .Get("Number")
              .ToObject()
              .Get("isSafeInteger")
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
