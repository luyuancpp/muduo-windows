#pragma once
// Windows shim for <cxxabi.h>
// On Linux this header provides abi::__cxa_demangle(); on Windows we provide
// a stub that returns NULL so boost::core::demangle falls back to raw names.

#include <cstdlib>
#include <cstddef>

namespace abi {
inline char* __cxa_demangle(const char* /*mangled*/, char* /*buf*/,
                            size_t* /*length*/, int* status) {
  if (status) *status = -2;  // not available
  return nullptr;
}
}  // namespace abi
