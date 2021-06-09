#include "npi.hpp"
#include <napi.h>

/**
 * Initialize the addon.
 */
Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    NPI::Init(env, exports);
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init);
