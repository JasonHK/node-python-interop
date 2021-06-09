#ifndef NPI_INTEROP_HELPERS_HPP
#define NPI_INTEROP_HELPERS_HPP

#include <napi.h>
#include <Python.h>

namespace NPI
{
    void EnsurePythonInitialized(const Napi::Env& env);
}

#endif
