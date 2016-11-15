
#ifndef mi_INCLUDES_H
#define mi_INCLUDES_H

/*
 * This must be included before any system headers,
 * since they can react to macro defined there
 */
//#include "mi_config_private.h"

/*
 ==========================================================
   Standard includes used by the library
 ==========================================================
*/
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

/*
  Use fseeko() and ftello() if they are available since they use
  'off_t' rather than 'long'.  It is wrong to use fseeko() and
  ftello() only on systems with special LFS support since some systems
  (e.g. FreeBSD) support a 64-bit off_t by default.
*/
#if defined(mi_HAVE_FSEEKO) && !defined(fseek)
#  define fseek  fseeko
#  define ftell  ftello
#endif


#if defined(WIN32) && !defined(Windows95) && !defined(__BORLANDC__) && \
  !(defined(_MSC_VER) && _MSC_VER < 1400) && \
  !(defined(__MINGW32__) && __MSVCRT_VERSION__ < 0x800)
  /*
    Windows '95 and Borland C do not support _lseeki64
    Visual Studio does not support _fseeki64 and _ftelli64 until the 2005 release.
    Without these interfaces, files over 2GB in size are not supported for Windows.
  */
#  define mi_FSEEK(stream,offset,whence) _fseeki64(stream,/* __int64 */ offset,whence)
#  define mi_FSTAT(fildes,stat_buff) _fstati64(fildes,/* struct _stati64 */ stat_buff)
#  define mi_FTELL(stream) /* __int64 */ _ftelli64(stream)
#  define mi_STAT_STRUCT_T struct _stati64
#  define mi_STAT(path,stat_buff) _stati64(path,/* struct _stati64 */ stat_buff)
#else
#  define mi_FSEEK(stream,offset,whence) fseek(stream,offset,whence)
#  define mi_FSTAT(fildes,stat_buff) fstat(fildes,stat_buff)
#  define mi_FTELL(stream) ftell(stream)
#  define mi_STAT_STRUCT_T struct stat
#  define mi_STAT(path,stat_buff) stat(path,stat_buff)
#endif


/*
 ==========================================================
   OpenJPEG interface
 ==========================================================
 */
#include "openjpeg.h"

/*
 ==========================================================
   OpenJPEG modules
 ==========================================================
*/

/* Are restricted pointers available? (C99) */
#if (__STDC_VERSION__ != 199901L)
	/* Not a C99 compiler */
	#ifdef __GNUC__
		#define restrict __restrict__
	#else
		#define restrict /* restrict */
	#endif
#endif

#ifdef __has_attribute
	#if __has_attribute(no_sanitize)
		#define mi_NOSANITIZE(kind) __attribute__((no_sanitize(kind)))
	#endif
#endif
#ifndef mi_NOSANITIZE
	#define mi_NOSANITIZE(kind)
#endif


/* MSVC before 2013 and Borland C do not have lrintf */
#if defined(_MSC_VER)
#include <intrin.h>
static INLINE long mi_lrintf(float f){
#ifdef _M_X64
	return _mm_cvt_ss2si(_mm_load_ss(&f));

	/* commented out line breaks many tests */
  /* return (long)((f>0.0f) ? (f + 0.5f):(f -0.5f)); */
#elif defined(_M_IX86)
    int i;
     _asm{
        fld f
        fistp i
    };
 
    return i;
#else 
	return (long)((f>0.0f) ? (f + 0.5f) : (f - 0.5f));
#endif
}
#elif defined(__BORLANDC__)
static INLINE long mi_lrintf(float f) {
#ifdef _M_X64
     return (long)((f>0.0f) ? (f + 0.5f):(f -0.5f));
#else
	int i;

	_asm {
		fld f
			fistp i
	};

	return i;
#endif
}
#else
static INLINE long mi_lrintf(float f) {
	return lrintf(f);
}
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1400)
	#define vsnprintf _vsnprintf
#endif

/* MSVC x86 is really bad at doing int64 = int32 * int32 on its own. Use intrinsic. */
#if defined(_MSC_VER) && (_MSC_VER >= 1400) && !defined(__INTEL_COMPILER) && defined(_M_IX86)
#	include <intrin.h>
#	pragma intrinsic(__emul)
#endif

#include "mi_inttypes.h"
#include "mi_clock.h"
#include "mi_malloc.h"
#include "event.h"
#include "function_list.h"
#include "bio.h"
#include "cio.h"

#include "image.h"
#include "invert.h"
#include "j2k.h"
#include "jp2.h"

#include "mqc.h"
#include "raw.h"
#include "bio.h"

#include "pi.h"
#include "tgt.h"
#include "tcd.h"
#include "t1.h"
#include "dwt.h"
#include "t2.h"
#include "mct.h"
#include "mi_intmath.h"

#ifdef USE_JPIP
#include "cidx_manager.h"
#include "indexbox_manager.h"
#endif

/* JPWL>> */
#ifdef USE_JPWL
#include "openjpwl/jpwl.h"
#endif /* USE_JPWL */
/* <<JPWL */

/* V2 */
#include "mi_codec.h"


#endif /* mi_INCLUDES_H */
