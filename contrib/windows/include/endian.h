#ifndef MUDUO_CONTRIB_WINDOWS_ENDIAN_H
#define MUDUO_CONTRIB_WINDOWS_ENDIAN_H

#if defined(_WIN32) || defined(WIN32)

#include <stdint.h>
#include "muduo/base/CrossPlatformAdapterFunction.h"

#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1234
#endif

#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN 4321
#endif

#ifndef __BYTE_ORDER
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif

#else

#if defined(__GNUC__) || defined(__clang__)
#include_next <endian.h>
#endif

#endif

#endif
