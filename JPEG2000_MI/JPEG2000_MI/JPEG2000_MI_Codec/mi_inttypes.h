
#ifndef OPJ_INTTYPES_H
#define OPJ_INTTYPES_H

//#include "opj_config_private.h"
#ifdef OPJ_HAVE_INTTYPES_H
#include <inttypes.h>
#else
#if defined(_WIN32)
#define PRId64 "I64d"
#define PRIi64 "I64i"
#define PRIu64 "I64u"
#define PRIx64 "I64x"
#else
#error unsupported platform
#endif
#endif

#endif /* OPJ_INTTYPES_H */
