#ifndef PY_ALPHA_UTIL_H
#define PY_ALPHA_UTIL_H

#include <alpha/alpha-common.h>

template<typename T, alpha_return_t (*Free)(T)>
struct AlphaWrapper {

    T handle;

    AlphaWrapper(T h)
    : handle{h} {}

    AlphaWrapper(AlphaWrapper&&) = delete;
    AlphaWrapper(const AlphaWrapper&) = delete;

    operator T() const {
        return handle;
    }

    ~AlphaWrapper() {
        Free(handle);
    }
};

#endif
