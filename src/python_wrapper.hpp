#ifndef NPI_PYTHON_WRAPPER_HPP
#define NPI_PYTHON_WRAPPER_HPP

#include <napi.h>
#include <Python.h>

namespace NPI
{
    class WrappedPythonObject : public Napi::ObjectWrap<WrappedPythonObject>
    {
        public:
            // static Napi::FunctionReference constructor;

            static Napi::FunctionReference& Constructor() { return m_constructor; }

            static Napi::Object Init(Napi::Env env, Napi::Object exports);

            static Napi::Object New(Napi::Env env, PyObject* python_value);

            const PyObject* python_value() const { return m_python_value; }

            PyObject* python_value() { return m_python_value; }

            PyObject* Value() { return m_python_value; }

            WrappedPythonObject(const Napi::CallbackInfo& info);

            ~WrappedPythonObject();
        private:
            static Napi::FunctionReference m_constructor;

            PyObject* m_python_value;
    };
};

#endif
