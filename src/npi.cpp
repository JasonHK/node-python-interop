#include "npi.hpp"

#include "internal_helpers.h"
#include "interop_helpers.hpp"
#include "python_helpers.hpp"
#include "python_wrapper.hpp"
#include "type_helpers.hpp"

#include <napi.h>
#ifndef _WIN32
    #include <dlfcn.h>
#endif

namespace NPI
{
    /**
     * Loads a dynamic library. Intended as a workaround for linking issue in some Linux distros. 
     * See <https://bugs.python.org/issue4434> for more information.
     * 
     * @see [more information on the issue](https://bugs.python.org/issue4434)
     */
    Napi::Value DlOpen(const Napi::CallbackInfo&);

    Napi::Value StartInterpreter(const Napi::CallbackInfo&);

    Napi::Value AppendSysPath(const Napi::CallbackInfo& info);

    Napi::Value Import(const Napi::CallbackInfo&);

    Napi::Value Eval(const Napi::CallbackInfo&);

    Napi::Value Dir(const Napi::CallbackInfo&);

    Napi::Value GetAttr(const Napi::CallbackInfo&);
}

Napi::Object NPI::Init(Napi::Env env, Napi::Object exports)
{
    using Napi::Function;

    exports.Set("dlOpen", Function::New(env, DlOpen, STRINGIFY(DlOpen)));
    exports.Set("startInterpreter", Function::New(env, StartInterpreter, STRINGIFY(StartInterpreter)));
    exports.Set("appendSysPath", Function::New(env, AppendSysPath, STRINGIFY(AppendSysPath)));
    exports.Set("import", Function::New(env, Import, STRINGIFY(Import)));
    exports.Set("eval", Function::New(env, Eval, STRINGIFY(Eval)));
    exports.Set("dir", Function::New(env, Dir, STRINGIFY(Dir)));
    exports.Set("getattr", Function::New(env, GetAttr, STRINGIFY(GetAttr)));

    WrappedPythonObject::Init(env, exports);

    return exports;
}

Napi::Value NPI::DlOpen(const Napi::CallbackInfo& info)
{
    auto env = info.Env();

#ifdef _WIN32
#ifndef NPI_CONFIGS_IGNORE_INCORRECT_DLOPEN_CALLS
    throw Napi::Error::New(env, STRINGIFY(NPI::DlOpen) " was not available in Windows.");
#else
    return env.Undefined();
#endif
#else
    auto file = info[0].As<Napi::String>().Utf8Value();
    dlopen(file.c_str(), RTLD_LAZY | RTLD_GLOBAL);

    return env.Undefined();
#endif
}

using NPI::PythonEnsureGil;

static wchar_t *program = NULL;

Napi::Value NPI::StartInterpreter(const Napi::CallbackInfo& info)
{
    auto env = info.Env();

    if ((program != NULL) && (info.Length() >= 1) && info[0].IsString())
    {
        auto path = info[0].As<Napi::String>().Utf8Value();

        program = Py_DecodeLocale(path.c_str(), NULL);
        Py_SetProgramName(program);
    }

    if (!Py_IsInitialized())
    {
        Py_Initialize();
    }

#if PY_VERSION_HEX < 0x03070000
    if (!PyEval_ThreadsInitialized())
    {
        PyEval_InitThreads();
    }
#else
    // No need to call PyEval_InitThreads() since Python 3.7.
    // See also: https://docs.python.org/3/c-api/init.html#c.PyEval_InitThreads
#endif

    PyEval_SaveThread();

    return env.Undefined();
}

Napi::Value NPI::AppendSysPath(const Napi::CallbackInfo& info)
{
    auto env = info.Env();
    EnsurePythonInitialized(env);

    auto path = info[0].As<Napi::String>().Utf8Value();

    {
        PythonEnsureGil _;

        auto p_path = PyUnicode_FromString(path.c_str());

        auto p_paths = PySys_GetObject("path");
        PyList_Append(p_paths, p_path);
        Py_DECREF(p_path);
    }

    return env.Undefined();
}

Napi::Value NPI::Import(const Napi::CallbackInfo& info)
{
    auto env = info.Env();
    EnsurePythonInitialized(env);

    auto name = info[0].As<Napi::String>().Utf8Value();

    {
        PythonEnsureGil _;

        auto python_name   = PyUnicode_FromString(name.c_str());
        auto python_module = PyImport_Import(python_name);
        Py_DECREF(python_name);

        if (python_module == NULL)
        {
            PyObject* error_type;
            PyObject* error_value;
            PyObject* error_trace;

            PyErr_Fetch(&error_type, &error_value, &error_trace);

            auto error = Napi::Error::New(env, PyUnicode_AsUTF8(PyObject_Str(error_value)));
            error.Set("name", ((PyTypeObject*) error_type)->tp_name);

            throw error;
        }

        return ToNodeValue(env, python_module);
    }
}

Napi::Value NPI::Eval(const Napi::CallbackInfo& info)
{
    auto env = info.Env();
    EnsurePythonInitialized(env);

    auto program = info[0].As<Napi::String>().Utf8Value();

    {
        PythonEnsureGil _;

        bool has_frame = (PyEval_GetFrame() != NULL);

        PyObject* globals;
        if ((info.Length() >= 1) && !IsNullLike(info[1]))
        {
            globals = ToPythonObject(info[1]);
        }
        else
        {
            globals = has_frame ? PyEval_GetGlobals() : PyModule_GetDict(PyImport_AddModule("__main__"));
        }

        PyObject* locals;
        if ((info.Length() >= 2) && !IsNullLike(info[2]))
        {
            locals = ToPythonObject(info[2]);
        }
        else
        {
            locals = has_frame ? PyEval_GetLocals() : globals;
        }

        PyObject* p_return = PyRun_String(program.c_str(), Py_eval_input, globals, locals);
        if (p_return == NULL)
        {
            // PyErr_Print();

            PyObject* error_type;
            PyObject* error_value;
            PyObject* error_trace;

            PyErr_Fetch(&error_type, &error_value, &error_trace);

            auto error = Napi::Error::New(env, PyUnicode_AsUTF8(PyObject_Str(error_value)));
            error.Set("name", ((PyTypeObject*) error_type)->tp_name);

            throw error;
        }

        auto n_return = ToNodeValue(env, p_return);
        Py_DECREF(p_return);

        return n_return;
    }
}

Napi::Value NPI::Dir(const Napi::CallbackInfo& info)
{
    auto env = info.Env();
    EnsurePythonInitialized(env);

    {
        PythonEnsureGil _;

        auto python_target = ToPythonObject(info[0]);
        auto python_keys   = PyObject_Dir(python_target);

        return ToNodeValue(env, python_keys);
    }
}

Napi::Value NPI::GetAttr(const Napi::CallbackInfo& info)
{
    auto env = info.Env();
    EnsurePythonInitialized(env);

    {
        PythonEnsureGil _;

        auto python_target = ToPythonObject(info[0]);
        auto python_name   = ToPythonObject(info[1]);
        auto python_value  = PyObject_GetAttr(python_target, python_name);

        return ToNodeValue(env, python_value);
    }
}

Napi::Value NPI::Symbols::Repr(Napi::Env env)
{
    return Napi::Symbol::WellKnown(env, "repr");
}
