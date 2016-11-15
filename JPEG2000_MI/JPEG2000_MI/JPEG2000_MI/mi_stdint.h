
#ifndef mi_STDINT_H
#define mi_STDINT_H

//#include "mi_config.h"
#ifdef mi_HAVE_STDINT_H
#include <stdint.h>
#else
#if defined(_WIN32)
typedef   signed __int8   int8_t;
typedef unsigned __int8   uint8_t;
typedef   signed __int16  int16_t;
typedef unsigned __int16  uint16_t;
typedef   signed __int32  int32_t;
typedef unsigned __int32  uint32_t;
typedef   signed __int64  int64_t;
typedef unsigned __int64  uint64_t;
#else
#error unsupported platform
#endif
#endif

#endif /* mi_STDINT_H */
