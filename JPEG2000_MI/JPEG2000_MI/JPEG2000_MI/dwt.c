
#ifdef __SSE__
#include <xmmintrin.h>
#endif

#include "mi_includes.h"

/** @defgroup DWT DWT - Implementation of a discrete wavelet transform */
/*@{*/

#define mi_WS(i) v->mem[(i)*2]
#define mi_WD(i) v->mem[(1+(i)*2)]

/** @name Local data structures */
/*@{*/

typedef struct dwt_local {
	mi_INT32* mem;
	mi_INT32 dn;
	mi_INT32 sn;
	mi_INT32 cas;
} mi_dwt_t;

typedef union {
	mi_FLOAT32	f[4];
} mi_v4_t;

typedef struct v4dwt_local {
	mi_v4_t*	wavelet ;
	mi_INT32		dn ;
	mi_INT32		sn ;
	mi_INT32		cas ;
} mi_v4dwt_t ;

static const mi_FLOAT32 mi_dwt_alpha =  1.586134342f; /*  12994 */
static const mi_FLOAT32 mi_dwt_beta  =  0.052980118f; /*    434 */
static const mi_FLOAT32 mi_dwt_gamma = -0.882911075f; /*  -7233 */
static const mi_FLOAT32 mi_dwt_delta = -0.443506852f; /*  -3633 */

static const mi_FLOAT32 mi_K      = 1.230174105f; /*  10078 */
static const mi_FLOAT32 mi_c13318 = 1.625732422f;

/*@}*/

/**
Virtual function type for wavelet transform in 1-D 
*/
typedef void (*DWT1DFN)(mi_dwt_t* v);

/** @name Local static functions */
/*@{*/

/**
Forward lazy transform (horizontal)
*/
static void mi_dwt_deinterleave_h(mi_INT32 *a, mi_INT32 *b, mi_INT32 dn, mi_INT32 sn, mi_INT32 cas);
/**
Forward lazy transform (vertical)
*/
static void mi_dwt_deinterleave_v(mi_INT32 *a, mi_INT32 *b, mi_INT32 dn, mi_INT32 sn, mi_INT32 x, mi_INT32 cas);
/**
Inverse lazy transform (horizontal)
*/
static void mi_dwt_interleave_h(mi_dwt_t* h, mi_INT32 *a);
/**
Inverse lazy transform (vertical)
*/
static void mi_dwt_interleave_v(mi_dwt_t* v, mi_INT32 *a, mi_INT32 x);
/**
Forward 5-3 wavelet transform in 1-D
*/
static void mi_dwt_encode_1(mi_INT32 *a, mi_INT32 dn, mi_INT32 sn, mi_INT32 cas);
/**
Inverse 5-3 wavelet transform in 1-D
*/
static void mi_dwt_decode_1(mi_dwt_t *v);
static void mi_dwt_decode_1_(mi_INT32 *a, mi_INT32 dn, mi_INT32 sn, mi_INT32 cas);
/**
Forward 9-7 wavelet transform in 1-D
*/
static void mi_dwt_encode_1_real(mi_INT32 *a, mi_INT32 dn, mi_INT32 sn, mi_INT32 cas);
/**
Explicit calculation of the Quantization Stepsizes 
*/
static void mi_dwt_encode_stepsize(mi_INT32 stepsize, mi_INT32 numbps, mi_stepsize_t *bandno_stepsize);
/**
Inverse wavelet transform in 2-D.
*/
static mi_BOOL mi_dwt_decode_tile(mi_tcd_tilecomp_t* tilec, mi_UINT32 i, DWT1DFN fn);

static mi_BOOL mi_dwt_encode_procedure(	mi_tcd_tilecomp_t * tilec,
										    void (*p_function)(mi_INT32 *, mi_INT32,mi_INT32,mi_INT32) );

static mi_UINT32 mi_dwt_max_resolution(mi_tcd_resolution_t* restrict r, mi_UINT32 i);

/* <summary>                             */
/* Inverse 9-7 wavelet transform in 1-D. */
/* </summary>                            */
static void mi_v4dwt_decode(mi_v4dwt_t* restrict dwt);

static void mi_v4dwt_interleave_h(mi_v4dwt_t* restrict w, mi_FLOAT32* restrict a, mi_INT32 x, mi_INT32 size);

static void mi_v4dwt_interleave_v(mi_v4dwt_t* restrict v , mi_FLOAT32* restrict a , mi_INT32 x, mi_INT32 nb_elts_read);

static void mi_v4dwt_decode_step1(mi_v4_t* w, mi_INT32 count, const mi_FLOAT32 c);

static void mi_v4dwt_decode_step2(mi_v4_t* l, mi_v4_t* w, mi_INT32 k, mi_INT32 m, mi_FLOAT32 c);


/*@}*/

/*@}*/

#define mi_S(i) a[(i)*2]
#define mi_D(i) a[(1+(i)*2)]
#define mi_S_(i) ((i)<0?mi_S(0):((i)>=sn?mi_S(sn-1):mi_S(i)))
#define mi_D_(i) ((i)<0?mi_D(0):((i)>=dn?mi_D(dn-1):mi_D(i)))
/* new */
#define mi_SS_(i) ((i)<0?mi_S(0):((i)>=dn?mi_S(dn-1):mi_S(i)))
#define mi_DD_(i) ((i)<0?mi_D(0):((i)>=sn?mi_D(sn-1):mi_D(i)))

/* <summary>                                                              */
/* This table contains the norms of the 5-3 wavelets for different bands. */
/* </summary>                                                             */
static const mi_FLOAT64 mi_dwt_norms[4][10] = {
	{1.000, 1.500, 2.750, 5.375, 10.68, 21.34, 42.67, 85.33, 170.7, 341.3},
	{1.038, 1.592, 2.919, 5.703, 11.33, 22.64, 45.25, 90.48, 180.9},
	{1.038, 1.592, 2.919, 5.703, 11.33, 22.64, 45.25, 90.48, 180.9},
	{.7186, .9218, 1.586, 3.043, 6.019, 12.01, 24.00, 47.97, 95.93}
};

/* <summary>                                                              */
/* This table contains the norms of the 9-7 wavelets for different bands. */
/* </summary>                                                             */
static const mi_FLOAT64 mi_dwt_norms_real[4][10] = {
	{1.000, 1.965, 4.177, 8.403, 16.90, 33.84, 67.69, 135.3, 270.6, 540.9},
	{2.022, 3.989, 8.355, 17.04, 34.27, 68.63, 137.3, 274.6, 549.0},
	{2.022, 3.989, 8.355, 17.04, 34.27, 68.63, 137.3, 274.6, 549.0},
	{2.080, 3.865, 8.307, 17.18, 34.71, 69.59, 139.3, 278.6, 557.2}
};

/* 
==========================================================
   local functions
==========================================================
*/

/* <summary>			                 */
/* Forward lazy transform (horizontal).  */
/* </summary>                            */ 
static void mi_dwt_deinterleave_h(mi_INT32 *a, mi_INT32 *b, mi_INT32 dn, mi_INT32 sn, mi_INT32 cas) {
	mi_INT32 i;
	mi_INT32 * l_dest = b;
	mi_INT32 * l_src = a+cas;

    for (i=0; i<sn; ++i) {
		*l_dest++ = *l_src;
		l_src += 2;
	}
	
    l_dest = b + sn;
	l_src = a + 1 - cas;

    for	(i=0; i<dn; ++i)  {
		*l_dest++=*l_src;
		l_src += 2;
	}
}

/* <summary>                             */  
/* Forward lazy transform (vertical).    */
/* </summary>                            */ 
static void mi_dwt_deinterleave_v(mi_INT32 *a, mi_INT32 *b, mi_INT32 dn, mi_INT32 sn, mi_INT32 x, mi_INT32 cas) {
    mi_INT32 i = sn;
	mi_INT32 * l_dest = b;
	mi_INT32 * l_src = a+cas;

    while (i--) {
		*l_dest = *l_src;
		l_dest += x;
		l_src += 2;
		} /* b[i*x]=a[2*i+cas]; */

	l_dest = b + sn * x;
	l_src = a + 1 - cas;
	
	i = dn;
    while (i--) {
		*l_dest = *l_src;
		l_dest += x;
		l_src += 2;
        } /*b[(sn+i)*x]=a[(2*i+1-cas)];*/
}

/* <summary>                             */
/* Inverse lazy transform (horizontal).  */
/* </summary>                            */
static void mi_dwt_interleave_h(mi_dwt_t* h, mi_INT32 *a) {
    mi_INT32 *ai = a;
    mi_INT32 *bi = h->mem + h->cas;
    mi_INT32  i	= h->sn;
    while( i-- ) {
      *bi = *(ai++);
	  bi += 2;
    }
    ai	= a + h->sn;
    bi	= h->mem + 1 - h->cas;
    i	= h->dn ;
    while( i-- ) {
      *bi = *(ai++);
	  bi += 2;
    }
}

/* <summary>                             */  
/* Inverse lazy transform (vertical).    */
/* </summary>                            */ 
static void mi_dwt_interleave_v(mi_dwt_t* v, mi_INT32 *a, mi_INT32 x) {
    mi_INT32 *ai = a;
    mi_INT32 *bi = v->mem + v->cas;
    mi_INT32  i = v->sn;
    while( i-- ) {
      *bi = *ai;
	  bi += 2;
	  ai += x;
    }
    ai = a + (v->sn * x);
    bi = v->mem + 1 - v->cas;
    i = v->dn ;
    while( i-- ) {
      *bi = *ai;
	  bi += 2;  
	  ai += x;
    }
}


/* <summary>                            */
/* Forward 5-3 wavelet transform in 1-D. */
/* </summary>                           */
static void mi_dwt_encode_1(mi_INT32 *a, mi_INT32 dn, mi_INT32 sn, mi_INT32 cas) {
	mi_INT32 i;
	
	if (!cas) {
		if ((dn > 0) || (sn > 1)) {	/* NEW :  CASE ONE ELEMENT */
			for (i = 0; i < dn; i++) mi_D(i) -= (mi_S_(i) + mi_S_(i + 1)) >> 1;
			for (i = 0; i < sn; i++) mi_S(i) += (mi_D_(i - 1) + mi_D_(i) + 2) >> 2;
		}
	} else {
		if (!sn && dn == 1)		    /* NEW :  CASE ONE ELEMENT */
			mi_S(0) *= 2;
		else {
			for (i = 0; i < dn; i++) mi_S(i) -= (mi_DD_(i) + mi_DD_(i - 1)) >> 1;
			for (i = 0; i < sn; i++) mi_D(i) += (mi_SS_(i) + mi_SS_(i + 1) + 2) >> 2;
		}
	}
}

/* <summary>                            */
/* Inverse 5-3 wavelet transform in 1-D. */
/* </summary>                           */ 
static void mi_dwt_decode_1_(mi_INT32 *a, mi_INT32 dn, mi_INT32 sn, mi_INT32 cas) {
	mi_INT32 i;
	
	if (!cas) {
		if ((dn > 0) || (sn > 1)) { /* NEW :  CASE ONE ELEMENT */
			for (i = 0; i < sn; i++) mi_S(i) -= (mi_D_(i - 1) + mi_D_(i) + 2) >> 2;
			for (i = 0; i < dn; i++) mi_D(i) += (mi_S_(i) + mi_S_(i + 1)) >> 1;
		}
	} else {
		if (!sn  && dn == 1)          /* NEW :  CASE ONE ELEMENT */
			mi_S(0) /= 2;
		else {
			for (i = 0; i < sn; i++) mi_D(i) -= (mi_SS_(i) + mi_SS_(i + 1) + 2) >> 2;
			for (i = 0; i < dn; i++) mi_S(i) += (mi_DD_(i) + mi_DD_(i - 1)) >> 1;
		}
	}
}

/* <summary>                            */
/* Inverse 5-3 wavelet transform in 1-D. */
/* </summary>                           */ 
static void mi_dwt_decode_1(mi_dwt_t *v) {
	mi_dwt_decode_1_(v->mem, v->dn, v->sn, v->cas);
}

/* <summary>                             */
/* Forward 9-7 wavelet transform in 1-D. */
/* </summary>                            */
static void mi_dwt_encode_1_real(mi_INT32 *a, mi_INT32 dn, mi_INT32 sn, mi_INT32 cas) {
	mi_INT32 i;
	if (!cas) {
		if ((dn > 0) || (sn > 1)) {	/* NEW :  CASE ONE ELEMENT */
			for (i = 0; i < dn; i++)
				mi_D(i) -= mi_int_fix_mul(mi_S_(i) + mi_S_(i + 1), 12993);
			for (i = 0; i < sn; i++)
				mi_S(i) -= mi_int_fix_mul(mi_D_(i - 1) + mi_D_(i), 434);
			for (i = 0; i < dn; i++)
				mi_D(i) += mi_int_fix_mul(mi_S_(i) + mi_S_(i + 1), 7233);
			for (i = 0; i < sn; i++)
				mi_S(i) += mi_int_fix_mul(mi_D_(i - 1) + mi_D_(i), 3633);
			for (i = 0; i < dn; i++)
				mi_D(i) = mi_int_fix_mul(mi_D(i), 5038);	/*5038 */
			for (i = 0; i < sn; i++)
				mi_S(i) = mi_int_fix_mul(mi_S(i), 6659);	/*6660 */
		}
	} else {
		if ((sn > 0) || (dn > 1)) {	/* NEW :  CASE ONE ELEMENT */
			for (i = 0; i < dn; i++)
				mi_S(i) -= mi_int_fix_mul(mi_DD_(i) + mi_DD_(i - 1), 12993);
			for (i = 0; i < sn; i++)
				mi_D(i) -= mi_int_fix_mul(mi_SS_(i) + mi_SS_(i + 1), 434);
			for (i = 0; i < dn; i++)
				mi_S(i) += mi_int_fix_mul(mi_DD_(i) + mi_DD_(i - 1), 7233);
			for (i = 0; i < sn; i++)
				mi_D(i) += mi_int_fix_mul(mi_SS_(i) + mi_SS_(i + 1), 3633);
			for (i = 0; i < dn; i++)
				mi_S(i) = mi_int_fix_mul(mi_S(i), 5038);	/*5038 */
			for (i = 0; i < sn; i++)
				mi_D(i) = mi_int_fix_mul(mi_D(i), 6659);	/*6660 */
		}
	}
}

static void mi_dwt_encode_stepsize(mi_INT32 stepsize, mi_INT32 numbps, mi_stepsize_t *bandno_stepsize) {
	mi_INT32 p, n;
	p = mi_int_floorlog2(stepsize) - 13;
	n = 11 - mi_int_floorlog2(stepsize);
	bandno_stepsize->mant = (n < 0 ? stepsize >> -n : stepsize << n) & 0x7ff;
	bandno_stepsize->expn = numbps - p;
}

/* 
==========================================================
   DWT interface
==========================================================
*/


/* <summary>                            */
/* Forward wavelet transform in 2-D. 二维小波变换函数（带谓词，有你来说用5/3还是9/7） */
/* </summary>                           */
static INLINE mi_BOOL mi_dwt_encode_procedure(mi_tcd_tilecomp_t * tilec,void (*p_function)(mi_INT32 *, mi_INT32,mi_INT32,mi_INT32) )
{
	mi_INT32 i, j, k;
	mi_INT32 *a = 00;//切片的原始分量数据
	mi_INT32 *aj = 00;
	mi_INT32 *bj = 00;//当前级的分辨率的一纵列的数据
	mi_INT32 w;//切片宽
	mi_INT32 l;//一共的小波分解次数

	mi_INT32 rw;			/* width of the resolution level computed  （当前级的那个分辨率的宽） */
	mi_INT32 rh;			/* height of the resolution level computed （当前级的那个分辨率的高） */
	mi_UINT32 l_data_size;//最高分辨率的尺寸

	mi_tcd_resolution_t * l_cur_res = 0;//与l对应的那个分辨率的信息（即当前分辨率图像）
	mi_tcd_resolution_t * l_last_res = 0;//上一个分辨率级别图像的信息（即底一级分辨率）

	w = tilec->x1-tilec->x0;
	l = (mi_INT32)tilec->numresolutions-1;
	a = tilec->data;

	l_cur_res = tilec->resolutions + l;
	l_last_res = l_cur_res - 1;

	l_data_size = mi_dwt_max_resolution( tilec->resolutions,tilec->numresolutions) * (mi_UINT32)sizeof(mi_INT32);
	bj = (mi_INT32*)mi_malloc((size_t)l_data_size);
	/* l_data_size is equal to 0 when numresolutions == 1 but bj is not used */
	/* in that case, so do not error out */
	if (l_data_size != 0 && ! bj) {
		return mi_FALSE;
	}
	i = l;

	while (i--) {
		mi_INT32 rw1;		/* width of the resolution level once lower than computed one（底一级的那个分辨率的宽）                                       */
		mi_INT32 rh1;		/* height of the resolution level once lower than computed one    （底一级的那个分辨率的高）                                     */
		mi_INT32 cas_col;	/* 0 = non inversion on horizontal filtering 1 = inversion between low-pass and high-pass filtering */
		mi_INT32 cas_row;	/* 0 = non inversion on vertical filtering 1 = inversion between low-pass and high-pass filtering   */
		mi_INT32 dn; //当前级分辨率的高/宽，减去底一级分辨率的高/宽
		mi_INT32 sn;//底一级分辨率的高/宽

		rw  = l_cur_res->x1 - l_cur_res->x0;
		rh  = l_cur_res->y1 - l_cur_res->y0;
		rw1 = l_last_res->x1 - l_last_res->x0;
		rh1 = l_last_res->y1 - l_last_res->y0;

		cas_row = l_cur_res->x0 & 1;
		cas_col = l_cur_res->y0 & 1;

		sn = rh1;
		dn = rh - rh1;
		for (j = 0; j < rw; ++j) {
			aj = a + j;
			for (k = 0; k < rh; ++k) {
				bj[k] = aj[k*w];
			}

			(*p_function) (bj, dn, sn, cas_col);

			mi_dwt_deinterleave_v(bj, aj, dn, sn, w, cas_col);
		}

		sn = rw1;
		dn = rw - rw1;

		for (j = 0; j < rh; j++) {
			aj = a + j * w;
			for (k = 0; k < rw; k++)  bj[k] = aj[k];
			(*p_function) (bj, dn, sn, cas_row);
			mi_dwt_deinterleave_h(bj, aj, dn, sn, cas_row);
		}

		l_cur_res = l_last_res;

		--l_last_res;
	}

	mi_free(bj);
	return mi_TRUE;
}

/* Forward 5-3 wavelet transform in 2-D. */
/* </summary>                           */
mi_BOOL mi_dwt_encode(mi_tcd_tilecomp_t * tilec)
{
	return mi_dwt_encode_procedure(tilec,mi_dwt_encode_1);
}

/* <summary>                            */
/* Inverse 5-3 wavelet transform in 2-D. */
/* </summary>                           */
mi_BOOL mi_dwt_decode(mi_tcd_tilecomp_t* tilec, mi_UINT32 numres) {
	return mi_dwt_decode_tile(tilec, numres, &mi_dwt_decode_1);
}


/* <summary>                          */
/* Get gain of 5-3 wavelet transform. */
/* </summary>                         */
mi_UINT32 mi_dwt_getgain(mi_UINT32 orient) {
	if (orient == 0)
		return 0;
	if (orient == 1 || orient == 2)
		return 1;
	return 2;
}

/* <summary>                */
/* Get norm of 5-3 wavelet. */
/* </summary>               */
mi_FLOAT64 mi_dwt_getnorm(mi_UINT32 level, mi_UINT32 orient) {
	return mi_dwt_norms[orient][level];
}

/* <summary>                             */
/* Forward 9-7 wavelet transform in 2-D. */
/* </summary>                            */
mi_BOOL mi_dwt_encode_real(mi_tcd_tilecomp_t * tilec)
{
	return mi_dwt_encode_procedure(tilec,mi_dwt_encode_1_real);
}

/* <summary>                          */
/* Get gain of 9-7 wavelet transform. */
/* </summary>                         */
mi_UINT32 mi_dwt_getgain_real(mi_UINT32 orient) {
	(void)orient;
	return 0;
}

/* <summary>                */
/* Get norm of 9-7 wavelet. */
/* </summary>               */
mi_FLOAT64 mi_dwt_getnorm_real(mi_UINT32 level, mi_UINT32 orient) {
	return mi_dwt_norms_real[orient][level];
}
//显式量化（9/7小波）
void mi_dwt_calc_explicit_stepsizes(mi_tccp_t * tccp, mi_UINT32 prec) {
	mi_UINT32 numbands, bandno;
	numbands = 3 * tccp->numresolutions - 2;
	for (bandno = 0; bandno < numbands; bandno++) {
		mi_FLOAT64 stepsize;
		mi_UINT32 resno, level, orient, gain;

		resno = (bandno == 0) ? 0 : ((bandno - 1) / 3 + 1);
		orient = (bandno == 0) ? 0 : ((bandno - 1) % 3 + 1);
		level = tccp->numresolutions - 1 - resno;
		gain = (tccp->qmfbid == 0) ? 0 : ((orient == 0) ? 0 : (((orient == 1) || (orient == 2)) ? 1 : 2));//子带增益，可见课本P160
		if (tccp->qntsty == J2K_CCP_QNTSTY_NOQNT) {
			stepsize = 1.0;
		} else {
			mi_FLOAT64 norm = mi_dwt_norms_real[orient][level];
			stepsize = (1 << (gain)) / norm;
		}
		mi_dwt_encode_stepsize((mi_INT32) floor(stepsize * 8192.0), (mi_INT32)(prec + gain), &tccp->stepsizes[bandno]);//与课本公式5.6-2一致
	}
}

/* <summary>                             */
/* Determine maximum computed resolution level for inverse wavelet transform */
/* </summary>                            */
static mi_UINT32 mi_dwt_max_resolution(mi_tcd_resolution_t* restrict r, mi_UINT32 i) {
	mi_UINT32 mr	= 0;
	mi_UINT32 w;
	while( --i ) {
		++r;
		if( mr < ( w = (mi_UINT32)(r->x1 - r->x0) ) )
			mr = w ;
		if( mr < ( w = (mi_UINT32)(r->y1 - r->y0) ) )
			mr = w ;
	}
	return mr ;
}

/* <summary>                            */
/* Inverse wavelet transform in 2-D.     */
/* </summary>                           */
static mi_BOOL mi_dwt_decode_tile(mi_tcd_tilecomp_t* tilec, mi_UINT32 numres, DWT1DFN dwt_1D) {
	mi_dwt_t h;
	mi_dwt_t v;

	mi_tcd_resolution_t* tr = tilec->resolutions;

	mi_UINT32 rw = (mi_UINT32)(tr->x1 - tr->x0);	/* width of the resolution level computed */
	mi_UINT32 rh = (mi_UINT32)(tr->y1 - tr->y0);	/* height of the resolution level computed */

	mi_UINT32 w = (mi_UINT32)(tilec->x1 - tilec->x0);
	
	if (numres == 1U) {
		return mi_TRUE;
	}
	h.mem = (mi_INT32*)mi_aligned_malloc(mi_dwt_max_resolution(tr, numres) * sizeof(mi_INT32));
	if (! h.mem){
		/* FIXME event manager error callback */
		return mi_FALSE;
	}

	v.mem = h.mem;

	while( --numres) {
		mi_INT32 * restrict tiledp = tilec->data;
		mi_UINT32 j;

		++tr;
		h.sn = (mi_INT32)rw;
		v.sn = (mi_INT32)rh;

		rw = (mi_UINT32)(tr->x1 - tr->x0);
		rh = (mi_UINT32)(tr->y1 - tr->y0);

		h.dn = (mi_INT32)(rw - (mi_UINT32)h.sn);
		h.cas = tr->x0 % 2;

		for(j = 0; j < rh; ++j) {
			mi_dwt_interleave_h(&h, &tiledp[j*w]);
			(dwt_1D)(&h);
			memcpy(&tiledp[j*w], h.mem, rw * sizeof(mi_INT32));
		}

		v.dn = (mi_INT32)(rh - (mi_UINT32)v.sn);
		v.cas = tr->y0 % 2;

		for(j = 0; j < rw; ++j){
			mi_UINT32 k;
			mi_dwt_interleave_v(&v, &tiledp[j], (mi_INT32)w);
			(dwt_1D)(&v);
			for(k = 0; k < rh; ++k) {
				tiledp[k * w + j] = v.mem[k];
			}
		}
	}
	mi_aligned_free(h.mem);
	return mi_TRUE;
}

static void mi_v4dwt_interleave_h(mi_v4dwt_t* restrict w, mi_FLOAT32* restrict a, mi_INT32 x, mi_INT32 size){
	mi_FLOAT32* restrict bi = (mi_FLOAT32*) (w->wavelet + w->cas);
	mi_INT32 count = w->sn;
	mi_INT32 i, k;

	for(k = 0; k < 2; ++k){
		if ( count + 3 * x < size && ((size_t) a & 0x0f) == 0 && ((size_t) bi & 0x0f) == 0 && (x & 0x0f) == 0 ) {
			/* Fast code path */
			for(i = 0; i < count; ++i){
				mi_INT32 j = i;
				bi[i*8    ] = a[j];
				j += x;
				bi[i*8 + 1] = a[j];
				j += x;
				bi[i*8 + 2] = a[j];
				j += x;
				bi[i*8 + 3] = a[j];
			}
		}
		else {
			/* Slow code path */
			for(i = 0; i < count; ++i){
				mi_INT32 j = i;
				bi[i*8    ] = a[j];
				j += x;
				if(j >= size) continue;
				bi[i*8 + 1] = a[j];
				j += x;
				if(j >= size) continue;
				bi[i*8 + 2] = a[j];
				j += x;
				if(j >= size) continue;
				bi[i*8 + 3] = a[j]; /* This one*/
			}
		}

		bi = (mi_FLOAT32*) (w->wavelet + 1 - w->cas);
		a += w->sn;
		size -= w->sn;
		count = w->dn;
	}
}

static void mi_v4dwt_interleave_v(mi_v4dwt_t* restrict v , mi_FLOAT32* restrict a , mi_INT32 x, mi_INT32 nb_elts_read){
	mi_v4_t* restrict bi = v->wavelet + v->cas;
	mi_INT32 i;

	for(i = 0; i < v->sn; ++i){
		memcpy(&bi[i*2], &a[i*x], (size_t)nb_elts_read * sizeof(mi_FLOAT32));
	}

	a += v->sn * x;
	bi = v->wavelet + 1 - v->cas;

	for(i = 0; i < v->dn; ++i){
		memcpy(&bi[i*2], &a[i*x], (size_t)nb_elts_read * sizeof(mi_FLOAT32));
	}
}


static void mi_v4dwt_decode_step1(mi_v4_t* w, mi_INT32 count, const mi_FLOAT32 c)
{
	mi_FLOAT32* restrict fw = (mi_FLOAT32*) w;
	mi_INT32 i;
	for(i = 0; i < count; ++i){
		mi_FLOAT32 tmp1 = fw[i*8    ];
		mi_FLOAT32 tmp2 = fw[i*8 + 1];
		mi_FLOAT32 tmp3 = fw[i*8 + 2];
		mi_FLOAT32 tmp4 = fw[i*8 + 3];
		fw[i*8    ] = tmp1 * c;
		fw[i*8 + 1] = tmp2 * c;
		fw[i*8 + 2] = tmp3 * c;
		fw[i*8 + 3] = tmp4 * c;
	}
}

static void mi_v4dwt_decode_step2(mi_v4_t* l, mi_v4_t* w, mi_INT32 k, mi_INT32 m, mi_FLOAT32 c)
{
	mi_FLOAT32* fl = (mi_FLOAT32*) l;
	mi_FLOAT32* fw = (mi_FLOAT32*) w;
	mi_INT32 i;
	for(i = 0; i < m; ++i){
		mi_FLOAT32 tmp1_1 = fl[0];
		mi_FLOAT32 tmp1_2 = fl[1];
		mi_FLOAT32 tmp1_3 = fl[2];
		mi_FLOAT32 tmp1_4 = fl[3];
		mi_FLOAT32 tmp2_1 = fw[-4];
		mi_FLOAT32 tmp2_2 = fw[-3];
		mi_FLOAT32 tmp2_3 = fw[-2];
		mi_FLOAT32 tmp2_4 = fw[-1];
		mi_FLOAT32 tmp3_1 = fw[0];
		mi_FLOAT32 tmp3_2 = fw[1];
		mi_FLOAT32 tmp3_3 = fw[2];
		mi_FLOAT32 tmp3_4 = fw[3];
		fw[-4] = tmp2_1 + ((tmp1_1 + tmp3_1) * c);
		fw[-3] = tmp2_2 + ((tmp1_2 + tmp3_2) * c);
		fw[-2] = tmp2_3 + ((tmp1_3 + tmp3_3) * c);
		fw[-1] = tmp2_4 + ((tmp1_4 + tmp3_4) * c);
		fl = fw;
		fw += 8;
	}
	if(m < k){
		mi_FLOAT32 c1;
		mi_FLOAT32 c2;
		mi_FLOAT32 c3;
		mi_FLOAT32 c4;
		c += c;
		c1 = fl[0] * c;
		c2 = fl[1] * c;
		c3 = fl[2] * c;
		c4 = fl[3] * c;
		for(; m < k; ++m){
			mi_FLOAT32 tmp1 = fw[-4];
			mi_FLOAT32 tmp2 = fw[-3];
			mi_FLOAT32 tmp3 = fw[-2];
			mi_FLOAT32 tmp4 = fw[-1];
			fw[-4] = tmp1 + c1;
			fw[-3] = tmp2 + c2;
			fw[-2] = tmp3 + c3;
			fw[-1] = tmp4 + c4;
			fw += 8;
		}
	}
}


/* <summary>                             */
/* Inverse 9-7 wavelet transform in 1-D. */
/* </summary>                            */
static void mi_v4dwt_decode(mi_v4dwt_t* restrict dwt)
{
	mi_INT32 a, b;
	if(dwt->cas == 0) {
		if(!((dwt->dn > 0) || (dwt->sn > 1))){
			return;
		}
		a = 0;
		b = 1;
	}else{
		if(!((dwt->sn > 0) || (dwt->dn > 1))) {
			return;
		}
		a = 1;
		b = 0;
	}
	mi_v4dwt_decode_step1(dwt->wavelet+a, dwt->sn, mi_K);
	mi_v4dwt_decode_step1(dwt->wavelet+b, dwt->dn, mi_c13318);
	mi_v4dwt_decode_step2(dwt->wavelet+b, dwt->wavelet+a+1, dwt->sn, mi_int_min(dwt->sn, dwt->dn-a), mi_dwt_delta);
	mi_v4dwt_decode_step2(dwt->wavelet+a, dwt->wavelet+b+1, dwt->dn, mi_int_min(dwt->dn, dwt->sn-b), mi_dwt_gamma);
	mi_v4dwt_decode_step2(dwt->wavelet+b, dwt->wavelet+a+1, dwt->sn, mi_int_min(dwt->sn, dwt->dn-a), mi_dwt_beta);
	mi_v4dwt_decode_step2(dwt->wavelet+a, dwt->wavelet+b+1, dwt->dn, mi_int_min(dwt->dn, dwt->sn-b), mi_dwt_alpha);
}


/* <summary>                             */
/* Inverse 9-7 wavelet transform in 2-D. */
/* </summary>                            */
mi_BOOL mi_dwt_decode_real(mi_tcd_tilecomp_t* restrict tilec, mi_UINT32 numres)
{
	mi_v4dwt_t h;
	mi_v4dwt_t v;

	mi_tcd_resolution_t* res = tilec->resolutions;

	mi_UINT32 rw = (mi_UINT32)(res->x1 - res->x0);	/* width of the resolution level computed */
	mi_UINT32 rh = (mi_UINT32)(res->y1 - res->y0);	/* height of the resolution level computed */

	mi_UINT32 w = (mi_UINT32)(tilec->x1 - tilec->x0);

	h.wavelet = (mi_v4_t*) mi_aligned_malloc((mi_dwt_max_resolution(res, numres)+5) * sizeof(mi_v4_t));
	if (!h.wavelet) {
		/* FIXME event manager error callback */
		return mi_FALSE;
	}
	v.wavelet = h.wavelet;

	while( --numres) {
		mi_FLOAT32 * restrict aj = (mi_FLOAT32*) tilec->data;
		mi_UINT32 bufsize = (mi_UINT32)((tilec->x1 - tilec->x0) * (tilec->y1 - tilec->y0));
		mi_INT32 j;

		h.sn = (mi_INT32)rw;
		v.sn = (mi_INT32)rh;

		++res;

		rw = (mi_UINT32)(res->x1 - res->x0);	/* width of the resolution level computed */
		rh = (mi_UINT32)(res->y1 - res->y0);	/* height of the resolution level computed */

		h.dn = (mi_INT32)(rw - (mi_UINT32)h.sn);
		h.cas = res->x0 % 2;

		for(j = (mi_INT32)rh; j > 3; j -= 4) {
			mi_INT32 k;
			mi_v4dwt_interleave_h(&h, aj, (mi_INT32)w, (mi_INT32)bufsize);
			mi_v4dwt_decode(&h);

			for(k = (mi_INT32)rw; --k >= 0;){
				aj[k               ] = h.wavelet[k].f[0];
				aj[k+(mi_INT32)w  ] = h.wavelet[k].f[1];
				aj[k+(mi_INT32)w*2] = h.wavelet[k].f[2];
				aj[k+(mi_INT32)w*3] = h.wavelet[k].f[3];
			}

			aj += w*4;
			bufsize -= w*4;
		}

		if (rh & 0x03) {
			mi_INT32 k;
			j = rh & 0x03;
			mi_v4dwt_interleave_h(&h, aj, (mi_INT32)w, (mi_INT32)bufsize);
			mi_v4dwt_decode(&h);
			for(k = (mi_INT32)rw; --k >= 0;){
				switch(j) {
					case 3: aj[k+(mi_INT32)w*2] = h.wavelet[k].f[2];
					case 2: aj[k+(mi_INT32)w  ] = h.wavelet[k].f[1];
					case 1: aj[k               ] = h.wavelet[k].f[0];
				}
			}
		}

		v.dn = (mi_INT32)(rh - (mi_UINT32)v.sn);
		v.cas = res->y0 % 2;

		aj = (mi_FLOAT32*) tilec->data;
		for(j = (mi_INT32)rw; j > 3; j -= 4){
			mi_UINT32 k;

			mi_v4dwt_interleave_v(&v, aj, (mi_INT32)w, 4);
			mi_v4dwt_decode(&v);

			for(k = 0; k < rh; ++k){
				memcpy(&aj[k*w], &v.wavelet[k], 4 * sizeof(mi_FLOAT32));
			}
			aj += 4;
		}

		if (rw & 0x03){
			mi_UINT32 k;

			j = rw & 0x03;

			mi_v4dwt_interleave_v(&v, aj, (mi_INT32)w, j);
			mi_v4dwt_decode(&v);

			for(k = 0; k < rh; ++k){
				memcpy(&aj[k*w], &v.wavelet[k], (size_t)j * sizeof(mi_FLOAT32));
			}
		}
	}

	mi_aligned_free(h.wavelet);
	return mi_TRUE;
}
