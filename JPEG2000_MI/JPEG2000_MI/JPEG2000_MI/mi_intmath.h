
#ifndef __INT_H
#define __INT_H
#include"openjpeg.h"
#include"t1.h"
/**
@file mi_intmath.h
@brief Implementation of operations on integers (INT)

The functions in mi_INTMATH.H have for goal to realize operations on integers.
*/

/** @defgroup mi_INTMATH mi_INTMATH - Implementation of operations on integers */
/*@{*/

/** @name Exported functions (see also openjpeg.h) */
/*@{*/
/* ----------------------------------------------------------------------- */
/**
Get the minimum of two integers
@return Returns a if a < b else b
*/
static INLINE mi_INT32 mi_int_min(mi_INT32 a, mi_INT32 b) {
	return a < b ? a : b;
}

/**
Get the minimum of two integers
@return Returns a if a < b else b
*/
static INLINE mi_UINT32 mi_uint_min(mi_UINT32 a, mi_UINT32 b) {
	return a < b ? a : b;
}

/**
Get the maximum of two integers
@return Returns a if a > b else b
*/
static INLINE mi_INT32 mi_int_max(mi_INT32 a, mi_INT32 b) {
	return (a > b) ? a : b;
}

/**
Get the maximum of two integers
@return Returns a if a > b else b
*/
static INLINE mi_UINT32 mi_uint_max(mi_UINT32  a, mi_UINT32  b) {
	return (a > b) ? a : b;
}

/**
 Get the saturated sum of two unsigned integers
 @return Returns saturated sum of a+b
 */
static INLINE mi_UINT32 mi_uint_adds(mi_UINT32 a, mi_UINT32 b) {
	mi_UINT64 sum = (mi_UINT64)a + (mi_UINT64)b;
	return (mi_UINT32)(-(mi_INT32)(sum >> 32)) | (mi_UINT32)sum;
}

/**
Clamp an integer inside an interval
@return
<ul>
<li>Returns a if (min < a < max)
<li>Returns max if (a > max)
<li>Returns min if (a < min) 
</ul>
*/
static INLINE mi_INT32 mi_int_clamp(mi_INT32 a, mi_INT32 min, mi_INT32 max) {
	if (a < min)
		return min;
	if (a > max)
		return max;
	return a;
}
/**
@return Get absolute value of integer
*/
static INLINE mi_INT32 mi_int_abs(mi_INT32 a) {
	return a < 0 ? -a : a;
}
/**
Divide an integer and round upwards
@return Returns a divided by b
*/
static INLINE mi_INT32 mi_int_ceildiv(mi_INT32 a, mi_INT32 b) {
	assert(b);
	return (a + b - 1) / b;
}

/**
Divide an integer and round upwards
@return Returns a divided by b
*/
static INLINE mi_UINT32  mi_uint_ceildiv(mi_UINT32  a, mi_UINT32  b) {
	assert(b);
	return (a + b - 1) / b;
}

/**
Divide an integer by a power of 2 and round upwards
@return Returns a divided by 2^b
*/
static INLINE mi_INT32 mi_int_ceildivpow2(mi_INT32 a, mi_INT32 b) {
	return (mi_INT32)((a + ((mi_INT64)1 << b) - 1) >> b);
}

/**
 Divide a 64bits integer by a power of 2 and round upwards
 @return Returns a divided by 2^b
 */
static INLINE mi_INT32 mi_int64_ceildivpow2(mi_INT64 a, mi_INT32 b) {
	return (mi_INT32)((a + ((mi_INT64)1 << b) - 1) >> b);
}

/**
 Divide an integer by a power of 2 and round upwards
 @return Returns a divided by 2^b
 */
static INLINE mi_UINT32 mi_uint_ceildivpow2(mi_UINT32 a, mi_UINT32 b) {
	return (mi_UINT32)((a + ((mi_UINT64)1U << b) - 1U) >> b);
}

/**
Divide an integer by a power of 2 and round downwards
@return Returns a divided by 2^b
*/
static INLINE mi_INT32 mi_int_floordivpow2(mi_INT32 a, mi_INT32 b) {
	return a >> b;
}
/**
Get logarithm of an integer and round downwards
@return Returns log2(a)
*/
static INLINE mi_INT32 mi_int_floorlog2(mi_INT32 a) {
	mi_INT32 l;
	for (l = 0; a > 1; l++) {
		a >>= 1;
	}
	return l;
}
/**
Get logarithm of an integer and round downwards
@return Returns log2(a)
*/
static INLINE mi_UINT32  mi_uint_floorlog2(mi_UINT32  a) {
	mi_UINT32  l;
	for (l = 0; a > 1; ++l)
	{
		a >>= 1;
	}
	return l;
}

/**
Multiply two fixed-precision rational numbers.
@param a
@param b
@return Returns a * b
*/
static INLINE mi_INT32 mi_int_fix_mul(mi_INT32 a, mi_INT32 b) {
#if defined(_MSC_VER) && (_MSC_VER >= 1400) && !defined(__INTEL_COMPILER) && defined(_M_IX86)
	mi_INT64 temp = __emul(a, b);
#else
	mi_INT64 temp = (mi_INT64) a * (mi_INT64) b ;
#endif
	temp += 4096;
	assert((temp >> 13) <= (mi_INT64)0x7FFFFFFF);
	assert((temp >> 13) >= (-(mi_INT64)0x7FFFFFFF - (mi_INT64)1));
	return (mi_INT32) (temp >> 13);
}

static INLINE mi_INT32 mi_int_fix_mul_t1(mi_INT32 a, mi_INT32 b) {
#if defined(_MSC_VER) && (_MSC_VER >= 1400) && !defined(__INTEL_COMPILER) && defined(_M_IX86)
	mi_INT64 temp = __emul(a, b);
#else
	mi_INT64 temp = (mi_INT64) a * (mi_INT64) b ;
#endif
	temp += 4096;
	assert((temp >> (13 + 11 - T1_NMSEDEC_FRACBITS)) <= (mi_INT64)0x7FFFFFFF);
	assert((temp >> (13 + 11 - T1_NMSEDEC_FRACBITS)) >= (-(mi_INT64)0x7FFFFFFF - (mi_INT64)1));
	return (mi_INT32) (temp >> (13 + 11 - T1_NMSEDEC_FRACBITS)) ;
}

/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/

#endif
