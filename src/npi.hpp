#ifndef NPI_HPP
#define NPI_HPP

#include <napi.h>

/**
 * Interop between Node and Python.
 */
namespace NPI
{
    namespace Symbols
    {
        Napi::Value Repr(Napi::Env env);
    }

    /**
     * Initialize the addon.
     */
    Napi::Object Init(Napi::Env env, Napi::Object exports);
}

#endif
