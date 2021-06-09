#include "interop_helpers.hpp"

void NPI::EnsurePythonInitialized(const Napi::Env& env)
{
    if (!Py_IsInitialized())
    {
        throw Napi::Error::New(env, "The Python interpreter was not initialized.");
    }
}
