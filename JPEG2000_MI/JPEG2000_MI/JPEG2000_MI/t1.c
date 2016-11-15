
#include "mi_includes.h"
#include "t1_luts.h"

/** @defgroup T1 T1 - Implementation of the tier-1 coding */
/*@{*/

/** @name Local static functions */
/*@{*/

static INLINE mi_BYTE mi_t1_getctxno_zc(mi_UINT32 f, mi_UINT32 orient);
static mi_BYTE mi_t1_getctxno_sc(mi_UINT32 f);
static INLINE mi_UINT32 mi_t1_getctxno_mag(mi_UINT32 f);
static mi_BYTE mi_t1_getspb(mi_UINT32 f);
static mi_INT16 mi_t1_getnmsedec_sig(mi_UINT32 x, mi_UINT32 bitpos);
static mi_INT16 mi_t1_getnmsedec_ref(mi_UINT32 x, mi_UINT32 bitpos);
static void mi_t1_updateflags(mi_flag_t *flagsp, mi_UINT32 s, mi_UINT32 stride);
/**
Encode significant pass
*/
static void mi_t1_enc_sigpass_step(mi_t1_t *t1,
                                    mi_flag_t *flagsp,
                                    mi_INT32 *datap,
                                    mi_UINT32 orient,
                                    mi_INT32 bpno,
                                    mi_INT32 one,
                                    mi_INT32 *nmsedec,
                                    mi_BYTE type,
                                    mi_UINT32 vsc);

/**
Decode significant pass
*/
#if 0
static void mi_t1_dec_sigpass_step(mi_t1_t *t1,
                                    mi_flag_t *flagsp,
                                    mi_INT32 *datap,
                                    mi_UINT32 orient,
                                    mi_INT32 oneplushalf,
                                    mi_BYTE type,
                                    mi_UINT32 vsc);
#endif

static INLINE void mi_t1_dec_sigpass_step_raw(
                mi_t1_t *t1,
                mi_flag_t *flagsp,
                mi_INT32 *datap,
                mi_INT32 orient,
                mi_INT32 oneplushalf,
                mi_INT32 vsc);
static INLINE void mi_t1_dec_sigpass_step_mqc(
                mi_t1_t *t1,
                mi_flag_t *flagsp,
                mi_INT32 *datap,
                mi_INT32 orient,
                mi_INT32 oneplushalf);
static INLINE void mi_t1_dec_sigpass_step_mqc_vsc(
                mi_t1_t *t1,
                mi_flag_t *flagsp,
                mi_INT32 *datap,
                mi_INT32 orient,
                mi_INT32 oneplushalf,
                mi_INT32 vsc);


/**
Encode significant pass
*/
static void mi_t1_enc_sigpass( mi_t1_t *t1,
                                mi_INT32 bpno,
                                mi_UINT32 orient,
                                mi_INT32 *nmsedec,
                                mi_BYTE type,
                                mi_UINT32 cblksty);

/**
Decode significant pass
*/
static void mi_t1_dec_sigpass_raw(
                mi_t1_t *t1,
                mi_INT32 bpno,
                mi_INT32 orient,
                mi_INT32 cblksty);
static void mi_t1_dec_sigpass_mqc(
                mi_t1_t *t1,
                mi_INT32 bpno,
                mi_INT32 orient);
static void mi_t1_dec_sigpass_mqc_vsc(
                mi_t1_t *t1,
                mi_INT32 bpno,
                mi_INT32 orient);



/**
Encode refinement pass
*/
static void mi_t1_enc_refpass_step(mi_t1_t *t1,
                                    mi_flag_t *flagsp,
                                    mi_INT32 *datap,
                                    mi_INT32 bpno,
                                    mi_INT32 one,
                                    mi_INT32 *nmsedec,
                                    mi_BYTE type,
                                    mi_UINT32 vsc);


/**
Encode refinement pass
*/
static void mi_t1_enc_refpass( mi_t1_t *t1,
                                mi_INT32 bpno,
                                mi_INT32 *nmsedec,
                                mi_BYTE type,
                                mi_UINT32 cblksty);

/**
Decode refinement pass
*/
static void mi_t1_dec_refpass_raw(
                mi_t1_t *t1,
                mi_INT32 bpno,
                mi_INT32 cblksty);
static void mi_t1_dec_refpass_mqc(
                mi_t1_t *t1,
                mi_INT32 bpno);
static void mi_t1_dec_refpass_mqc_vsc(
                mi_t1_t *t1,
                mi_INT32 bpno);


/**
Decode refinement pass
*/

static INLINE void  mi_t1_dec_refpass_step_raw(
                mi_t1_t *t1,
                mi_flag_t *flagsp,
                mi_INT32 *datap,
                mi_INT32 poshalf,
                mi_INT32 neghalf,
                mi_INT32 vsc);
static INLINE void mi_t1_dec_refpass_step_mqc(
                mi_t1_t *t1,
                mi_flag_t *flagsp,
                mi_INT32 *datap,
                mi_INT32 poshalf,
                mi_INT32 neghalf);
static INLINE void mi_t1_dec_refpass_step_mqc_vsc(
                mi_t1_t *t1,
                mi_flag_t *flagsp,
                mi_INT32 *datap,
                mi_INT32 poshalf,
                mi_INT32 neghalf,
                mi_INT32 vsc);



/**
Encode clean-up pass
*/
static void mi_t1_enc_clnpass_step(
		mi_t1_t *t1,
		mi_flag_t *flagsp,
		mi_INT32 *datap,
		mi_UINT32 orient,
		mi_INT32 bpno,
		mi_INT32 one,
		mi_INT32 *nmsedec,
		mi_UINT32 partial,
		mi_UINT32 vsc);
/**
Decode clean-up pass
*/
static void mi_t1_dec_clnpass_step_partial(
		mi_t1_t *t1,
		mi_flag_t *flagsp,
		mi_INT32 *datap,
		mi_INT32 orient,
		mi_INT32 oneplushalf);
static void mi_t1_dec_clnpass_step(
		mi_t1_t *t1,
		mi_flag_t *flagsp,
		mi_INT32 *datap,
		mi_INT32 orient,
		mi_INT32 oneplushalf);
static void mi_t1_dec_clnpass_step_vsc(
		mi_t1_t *t1,
		mi_flag_t *flagsp,
		mi_INT32 *datap,
		mi_INT32 orient,
		mi_INT32 oneplushalf,
		mi_INT32 partial,
		mi_INT32 vsc);
/**
Encode clean-up pass
*/
static void mi_t1_enc_clnpass(
		mi_t1_t *t1,
		mi_INT32 bpno,
		mi_UINT32 orient,
		mi_INT32 *nmsedec,
		mi_UINT32 cblksty);
/**
Decode clean-up pass
*/
static void mi_t1_dec_clnpass(
		mi_t1_t *t1,
		mi_INT32 bpno,
		mi_INT32 orient,
		mi_INT32 cblksty);

static mi_FLOAT64 mi_t1_getwmsedec(
		mi_INT32 nmsedec,
		mi_UINT32 compno,
		mi_UINT32 level,
		mi_UINT32 orient,
		mi_INT32 bpno,
		mi_UINT32 qmfbid,
		mi_FLOAT64 stepsize,
		mi_UINT32 numcomps,
		const mi_FLOAT64 * mct_norms,
		mi_UINT32 mct_numcomps);

static void mi_t1_encode_cblk( mi_t1_t *t1,
                                mi_tcd_cblk_enc_t* cblk,
                                mi_UINT32 orient,
                                mi_UINT32 compno,
                                mi_UINT32 level,
                                mi_UINT32 qmfbid,
                                mi_FLOAT64 stepsize,
                                mi_UINT32 cblksty,
                                mi_UINT32 numcomps,
                                mi_tcd_tile_t * tile,
                                const mi_FLOAT64 * mct_norms,
                                mi_UINT32 mct_numcomps);

/**
Decode 1 code-block
@param t1 T1 handle
@param cblk Code-block coding parameters
@param orient
@param roishift Region of interest shifting value
@param cblksty Code-block style
*/
static mi_BOOL mi_t1_decode_cblk( mi_t1_t *t1,
                                    mi_tcd_cblk_dec_t* cblk,
                                    mi_UINT32 orient,
                                    mi_UINT32 roishift,
                                    mi_UINT32 cblksty);

static mi_BOOL mi_t1_allocate_buffers(   mi_t1_t *t1,
                                    mi_UINT32 w,
                                    mi_UINT32 h);

/*@}*/

/*@}*/

/* ----------------------------------------------------------------------- */

static mi_BYTE mi_t1_getctxno_zc(mi_UINT32 f, mi_UINT32 orient) {
	return lut_ctxno_zc[(orient << 8) | (f & T1_SIG_OTH)];
}

static mi_BYTE mi_t1_getctxno_sc(mi_UINT32 f) {
	return lut_ctxno_sc[(f & (T1_SIG_PRIM | T1_SGN)) >> 4];
}

static mi_UINT32 mi_t1_getctxno_mag(mi_UINT32 f) {
	mi_UINT32 tmp1 = (f & T1_SIG_OTH) ? T1_CTXNO_MAG + 1 : T1_CTXNO_MAG;
	mi_UINT32 tmp2 = (f & T1_REFINE) ? T1_CTXNO_MAG + 2 : tmp1;
	return (tmp2);
}

static mi_BYTE mi_t1_getspb(mi_UINT32 f) {
	return lut_spb[(f & (T1_SIG_PRIM | T1_SGN)) >> 4];
}

static mi_INT16 mi_t1_getnmsedec_sig(mi_UINT32 x, mi_UINT32 bitpos) {
	if (bitpos > 0) {
		return lut_nmsedec_sig[(x >> (bitpos)) & ((1 << T1_NMSEDEC_BITS) - 1)];
	}
	
	return lut_nmsedec_sig0[x & ((1 << T1_NMSEDEC_BITS) - 1)];
}

static mi_INT16 mi_t1_getnmsedec_ref(mi_UINT32 x, mi_UINT32 bitpos) {
	if (bitpos > 0) {
		return lut_nmsedec_ref[(x >> (bitpos)) & ((1 << T1_NMSEDEC_BITS) - 1)];
	}

    return lut_nmsedec_ref0[x & ((1 << T1_NMSEDEC_BITS) - 1)];
}

static void mi_t1_updateflags(mi_flag_t *flagsp, mi_UINT32 s, mi_UINT32 stride) {
	mi_flag_t *np = flagsp - stride;
	mi_flag_t *sp = flagsp + stride;

	static const mi_flag_t mod[] = {
		T1_SIG_S, T1_SIG_S|T1_SGN_S,
		T1_SIG_E, T1_SIG_E|T1_SGN_E,
		T1_SIG_W, T1_SIG_W|T1_SGN_W,
		T1_SIG_N, T1_SIG_N|T1_SGN_N
	};

	np[-1] |= T1_SIG_SE;
	np[0]  |= mod[s];
	np[1]  |= T1_SIG_SW;

	flagsp[-1] |= mod[s+2];
	flagsp[0]  |= T1_SIG;
	flagsp[1]  |= mod[s+4];

	sp[-1] |= T1_SIG_NE;
	sp[0]  |= mod[s+6];
	sp[1]  |= T1_SIG_NW;
}
//重要性传播过程（Significance Propagation）
static void mi_t1_enc_sigpass_step(   mi_t1_t *t1,
                                mi_flag_t *flagsp,
                                mi_INT32 *datap,
                                mi_UINT32 orient,
                                mi_INT32 bpno,
                                mi_INT32 one,
                                mi_INT32 *nmsedec,
                                mi_BYTE type,
                                mi_UINT32 vsc
                                )
{
	mi_INT32 v;
    mi_UINT32 flag;
	
	mi_mqc_t *mqc = t1->mqc;	/* MQC component */
	
	flag = vsc ? (mi_UINT32)((*flagsp) & (~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW | T1_SGN_S))) : (mi_UINT32)(*flagsp);
	//下面开始重要性传播过程的重要性编码阶段
	if ((flag & T1_SIG_OTH) && !(flag & (T1_SIG | T1_VISIT))) {
		v = (mi_int_abs(*datap) & one) ? 1 : 0;//当前数据的绝对值在当前位平面的数值是否为1
		mi_mqc_setcurctx(mqc, mi_t1_getctxno_zc(flag, orient));	/* ESSAI *///保存当前计算出的上下文索引结果传到ctxs数组找到相应的MQ编码器状态然后再将状态存到到curctx数组中
		if (type == T1_TYPE_RAW) {	/* BYPASS/LAZY MODE */
			mi_mqc_bypass_enc(mqc, (mi_UINT32)v);
		} else {
			mi_mqc_encode(mqc, (mi_UINT32)v);
		}
		//下面开始重要性传播过程的符号编码阶段
		if (v) {//若之前编码的数据点的位是这个数据的第一个bit，则执行接下来的符号编码阶段
			v = *datap < 0 ? 1 : 0;//若若数据本就是是非负数，则将此数据在当前这个位平面的位恢复为0
			*nmsedec +=	mi_t1_getnmsedec_sig((mi_UINT32)mi_int_abs(*datap), (mi_UINT32)(bpno));
			mi_mqc_setcurctx(mqc, mi_t1_getctxno_sc(flag));	/* ESSAI */
			if (type == T1_TYPE_RAW) {	/* BYPASS/LAZY MODE *///lasy模式才用到，可暂不考虑
				mi_mqc_bypass_enc(mqc, (mi_UINT32)v);
			} else {
				mi_mqc_encode(mqc, (mi_UINT32)(v ^ mi_t1_getspb((mi_UINT32)flag)));//课本166页
			}
			mi_t1_updateflags(flagsp, (mi_UINT32)v, t1->flags_stride);
		}
		*flagsp |= T1_VISIT;
	}
}


static INLINE void mi_t1_dec_sigpass_step_raw(
                mi_t1_t *t1,
                mi_flag_t *flagsp,
                mi_INT32 *datap,
                mi_INT32 orient,
                mi_INT32 oneplushalf,
                mi_INT32 vsc)
{
        mi_INT32 v, flag;
        mi_raw_t *raw = t1->raw;       /* RAW component */
        mi_ARG_NOT_USED(orient);
       
        flag = vsc ? ((*flagsp) & (~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW | T1_SGN_S))) : (*flagsp);
        if ((flag & T1_SIG_OTH) && !(flag & (T1_SIG | T1_VISIT))) {
                        if (mi_raw_decode(raw)) {
                                v = (mi_INT32)mi_raw_decode(raw);    /* ESSAI */
                                *datap = v ? -oneplushalf : oneplushalf;
                                mi_t1_updateflags(flagsp, (mi_UINT32)v, t1->flags_stride);
                        }
                *flagsp |= T1_VISIT;
        }
}      

static INLINE void mi_t1_dec_sigpass_step_mqc(
                mi_t1_t *t1,
                mi_flag_t *flagsp,
                mi_INT32 *datap,
                mi_INT32 orient,
                mi_INT32 oneplushalf)
{
        mi_INT32 v, flag;
       
        mi_mqc_t *mqc = t1->mqc;       /* MQC component */
       
        flag = *flagsp;
        if ((flag & T1_SIG_OTH) && !(flag & (T1_SIG | T1_VISIT))) {
                        mi_mqc_setcurctx(mqc, mi_t1_getctxno_zc((mi_UINT32)flag, (mi_UINT32)orient));
                        if (mi_mqc_decode(mqc)) {
                                mi_mqc_setcurctx(mqc, mi_t1_getctxno_sc((mi_UINT32)flag));
                                v = mi_mqc_decode(mqc) ^ mi_t1_getspb((mi_UINT32)flag);
                                *datap = v ? -oneplushalf : oneplushalf;
                                mi_t1_updateflags(flagsp, (mi_UINT32)v, t1->flags_stride);
                        }
                *flagsp |= T1_VISIT;
        }
}                               /* VSC and  BYPASS by Antonin */

static INLINE void mi_t1_dec_sigpass_step_mqc_vsc(
                mi_t1_t *t1,
                mi_flag_t *flagsp,
                mi_INT32 *datap,
                mi_INT32 orient,
                mi_INT32 oneplushalf,
                mi_INT32 vsc)
{
        mi_INT32 v, flag;
       
        mi_mqc_t *mqc = t1->mqc;       /* MQC component */
       
        flag = vsc ? ((*flagsp) & (~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW | T1_SGN_S))) : (*flagsp);
        if ((flag & T1_SIG_OTH) && !(flag & (T1_SIG | T1_VISIT))) {
                mi_mqc_setcurctx(mqc, mi_t1_getctxno_zc((mi_UINT32)flag, (mi_UINT32)orient));
                if (mi_mqc_decode(mqc)) {
                        mi_mqc_setcurctx(mqc, mi_t1_getctxno_sc((mi_UINT32)flag));
                        v = mi_mqc_decode(mqc) ^ mi_t1_getspb((mi_UINT32)flag);
                        *datap = v ? -oneplushalf : oneplushalf;
                        mi_t1_updateflags(flagsp, (mi_UINT32)v, t1->flags_stride);
                }
                *flagsp |= T1_VISIT;
        }
}                               /* VSC and  BYPASS by Antonin */



static void mi_t1_enc_sigpass(mi_t1_t *t1,
                        mi_INT32 bpno,
                        mi_UINT32 orient,
                        mi_INT32 *nmsedec,
                        mi_BYTE type,
                        mi_UINT32 cblksty//这个可以看OPENJPEG官方说明，在命令行下是“-M”项，默认是0，即不开启mode
                        )
{
	mi_UINT32 i, j, k, vsc; 
    mi_INT32 one;

	*nmsedec = 0;
	one = 1 << (bpno + T1_NMSEDEC_FRACBITS);//代码块表示数据所需位数-1+6
	for (k = 0; k < t1->h; k += 4) {//条带扫描
		for (i = 0; i < t1->w; ++i) {
			for (j = k; j < k + 4 && j < t1->h; ++j) {
				vsc = ((cblksty & J2K_CCP_CBLKSTY_VSC) && (j == k + 3 || j == t1->h - 1)) ? 1 : 0;//与VSC模式有关，可以确认为关闭（0）
				mi_t1_enc_sigpass_step(
						t1,
						&t1->flags[((j+1) * t1->flags_stride) + i + 1],
						&t1->data[(j * t1->data_stride) + i],
						orient,
						bpno,
						one,
						nmsedec,
						type,
						vsc);
			}
		}
	}
}

static void mi_t1_dec_sigpass_raw(
                mi_t1_t *t1,
                mi_INT32 bpno,
                mi_INT32 orient,
                mi_INT32 cblksty)
{
        mi_INT32 one, half, oneplushalf, vsc;
        mi_UINT32 i, j, k; 
        one = 1 << bpno;
        half = one >> 1;
        oneplushalf = one | half;
        for (k = 0; k < t1->h; k += 4) {
                for (i = 0; i < t1->w; ++i) {
                        for (j = k; j < k + 4 && j < t1->h; ++j) {
                                vsc = ((cblksty & J2K_CCP_CBLKSTY_VSC) && (j == k + 3 || j == t1->h - 1)) ? 1 : 0;
                                mi_t1_dec_sigpass_step_raw(
                                                t1,
                                                &t1->flags[((j+1) * t1->flags_stride) + i + 1],
                                                &t1->data[(j * t1->w) + i],
                                                orient,
                                                oneplushalf,
                                                vsc);
                        }
                }
        }
}                               /* VSC and  BYPASS by Antonin */

static void mi_t1_dec_sigpass_mqc(
                mi_t1_t *t1,
                mi_INT32 bpno,
                mi_INT32 orient)
{
        mi_INT32 one, half, oneplushalf;
        mi_UINT32 i, j, k;
        mi_INT32 *data1 = t1->data;
        mi_flag_t *flags1 = &t1->flags[1];
        one = 1 << bpno;
        half = one >> 1;
        oneplushalf = one | half;
        for (k = 0; k < (t1->h & ~3u); k += 4) {
                for (i = 0; i < t1->w; ++i) {
                        mi_INT32 *data2 = data1 + i;
                        mi_flag_t *flags2 = flags1 + i;
                        flags2 += t1->flags_stride;
                        mi_t1_dec_sigpass_step_mqc(t1, flags2, data2, orient, oneplushalf);
                        data2 += t1->w;
                        flags2 += t1->flags_stride;
                        mi_t1_dec_sigpass_step_mqc(t1, flags2, data2, orient, oneplushalf);
                        data2 += t1->w;
                        flags2 += t1->flags_stride;
                        mi_t1_dec_sigpass_step_mqc(t1, flags2, data2, orient, oneplushalf);
                        data2 += t1->w;
                        flags2 += t1->flags_stride;
                        mi_t1_dec_sigpass_step_mqc(t1, flags2, data2, orient, oneplushalf);
                        data2 += t1->w;
                }
                data1 += t1->w << 2;
                flags1 += t1->flags_stride << 2;
        }
        for (i = 0; i < t1->w; ++i) {
                mi_INT32 *data2 = data1 + i;
                mi_flag_t *flags2 = flags1 + i;
                for (j = k; j < t1->h; ++j) {
                        flags2 += t1->flags_stride;
                        mi_t1_dec_sigpass_step_mqc(t1, flags2, data2, orient, oneplushalf);
                        data2 += t1->w;
                }
        }
}                               /* VSC and  BYPASS by Antonin */

static void mi_t1_dec_sigpass_mqc_vsc(
                mi_t1_t *t1,
                mi_INT32 bpno,
                mi_INT32 orient)
{
        mi_INT32 one, half, oneplushalf, vsc;
        mi_UINT32 i, j, k;
        one = 1 << bpno;
        half = one >> 1;
        oneplushalf = one | half;
        for (k = 0; k < t1->h; k += 4) {
                for (i = 0; i < t1->w; ++i) {
                        for (j = k; j < k + 4 && j < t1->h; ++j) {
                                vsc = (j == k + 3 || j == t1->h - 1) ? 1 : 0;
                                mi_t1_dec_sigpass_step_mqc_vsc(
                                                t1,
                                                &t1->flags[((j+1) * t1->flags_stride) + i + 1],
                                                &t1->data[(j * t1->w) + i],
                                                orient,
                                                oneplushalf,
                                                vsc);
                        }
                }
        }
}                               /* VSC and  BYPASS by Antonin */



static void mi_t1_enc_refpass_step(   mi_t1_t *t1,
                                mi_flag_t *flagsp,
                                mi_INT32 *datap,
                                mi_INT32 bpno,
                                mi_INT32 one,
                                mi_INT32 *nmsedec,
                                mi_BYTE type,
                                mi_UINT32 vsc)
{
	mi_INT32 v;
	mi_UINT32 flag;
	
	mi_mqc_t *mqc = t1->mqc;	/* MQC component */
	
	flag = vsc ? (mi_UINT32)((*flagsp) & (~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW | T1_SGN_S))) : (mi_UINT32)(*flagsp);
	if ((flag & (T1_SIG | T1_VISIT)) == T1_SIG) {
		*nmsedec += mi_t1_getnmsedec_ref((mi_UINT32)mi_int_abs(*datap), (mi_UINT32)(bpno));
		v = (mi_int_abs(*datap) & one) ? 1 : 0;
		mi_mqc_setcurctx(mqc, mi_t1_getctxno_mag(flag));	/* ESSAI */
		if (type == T1_TYPE_RAW) {	/* BYPASS/LAZY MODE */
			mi_mqc_bypass_enc(mqc, (mi_UINT32)v);
		} else {
			mi_mqc_encode(mqc, (mi_UINT32)v);
		}
		*flagsp |= T1_REFINE;
	}
}

static INLINE void mi_t1_dec_refpass_step_raw(
                mi_t1_t *t1,
                mi_flag_t *flagsp,
                mi_INT32 *datap,
                mi_INT32 poshalf,
                mi_INT32 neghalf,
                mi_INT32 vsc)
{
        mi_INT32 v, t, flag;
       
        mi_raw_t *raw = t1->raw;       /* RAW component */
       
        flag = vsc ? ((*flagsp) & (~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW | T1_SGN_S))) : (*flagsp);
        if ((flag & (T1_SIG | T1_VISIT)) == T1_SIG) {
                        v = (mi_INT32)mi_raw_decode(raw);
                t = v ? poshalf : neghalf;
                *datap += *datap < 0 ? -t : t;
                *flagsp |= T1_REFINE;
        }
}                               /* VSC and  BYPASS by Antonin  */

static INLINE void mi_t1_dec_refpass_step_mqc(
                mi_t1_t *t1,
                mi_flag_t *flagsp,
                mi_INT32 *datap,
                mi_INT32 poshalf,
                mi_INT32 neghalf)
{
        mi_INT32 v, t, flag;
       
        mi_mqc_t *mqc = t1->mqc;       /* MQC component */
       
        flag = *flagsp;
        if ((flag & (T1_SIG | T1_VISIT)) == T1_SIG) {
                mi_mqc_setcurctx(mqc, mi_t1_getctxno_mag((mi_UINT32)flag));      /* ESSAI */
                        v = mi_mqc_decode(mqc);
                t = v ? poshalf : neghalf;
                *datap += *datap < 0 ? -t : t;
                *flagsp |= T1_REFINE;
                }
}                               /* VSC and  BYPASS by Antonin  */

static INLINE void mi_t1_dec_refpass_step_mqc_vsc(
                mi_t1_t *t1,
                mi_flag_t *flagsp,
                mi_INT32 *datap,
                mi_INT32 poshalf,
                mi_INT32 neghalf,
                mi_INT32 vsc)
{
        mi_INT32 v, t, flag;
       
        mi_mqc_t *mqc = t1->mqc;       /* MQC component */
       
        flag = vsc ? ((*flagsp) & (~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW | T1_SGN_S))) : (*flagsp);
        if ((flag & (T1_SIG | T1_VISIT)) == T1_SIG) {
                mi_mqc_setcurctx(mqc, mi_t1_getctxno_mag((mi_UINT32)flag));      /* ESSAI */
                v = mi_mqc_decode(mqc);
                t = v ? poshalf : neghalf;
                *datap += *datap < 0 ? -t : t;
                *flagsp |= T1_REFINE;
        }
}                               /* VSC and  BYPASS by Antonin  */


static void mi_t1_enc_refpass(
		mi_t1_t *t1,
		mi_INT32 bpno,
		mi_INT32 *nmsedec,
		mi_BYTE type,
		mi_UINT32 cblksty)
{
	mi_UINT32 i, j, k, vsc;
    mi_INT32 one;

	*nmsedec = 0;
	one = 1 << (bpno + T1_NMSEDEC_FRACBITS);
	for (k = 0; k < t1->h; k += 4) {
		for (i = 0; i < t1->w; ++i) {
			for (j = k; j < k + 4 && j < t1->h; ++j) {
				vsc = ((cblksty & J2K_CCP_CBLKSTY_VSC) && (j == k + 3 || j == t1->h - 1)) ? 1 : 0;
				mi_t1_enc_refpass_step(
						t1,
						&t1->flags[((j+1) * t1->flags_stride) + i + 1],
						&t1->data[(j * t1->data_stride) + i],
						bpno,
						one,
						nmsedec,
						type,
						vsc);
			}
		}
	}
}

static void mi_t1_dec_refpass_raw(
                mi_t1_t *t1,
                mi_INT32 bpno,
                mi_INT32 cblksty)
{
        mi_INT32 one, poshalf, neghalf;
        mi_UINT32 i, j, k;
        mi_INT32 vsc;
        one = 1 << bpno;
        poshalf = one >> 1;
        neghalf = bpno > 0 ? -poshalf : -1;
        for (k = 0; k < t1->h; k += 4) {
                for (i = 0; i < t1->w; ++i) {
                        for (j = k; j < k + 4 && j < t1->h; ++j) {
                                vsc = ((cblksty & J2K_CCP_CBLKSTY_VSC) && (j == k + 3 || j == t1->h - 1)) ? 1 : 0;
                                mi_t1_dec_refpass_step_raw(
                                                t1,
                                                &t1->flags[((j+1) * t1->flags_stride) + i + 1],
                                                &t1->data[(j * t1->w) + i],
                                                poshalf,
                                                neghalf,
                                                vsc);
                        }
                }
        }
}                               /* VSC and  BYPASS by Antonin */

static void mi_t1_dec_refpass_mqc(
                mi_t1_t *t1,
                mi_INT32 bpno)
{
        mi_INT32 one, poshalf, neghalf;
        mi_UINT32 i, j, k;
        mi_INT32 *data1 = t1->data;
        mi_flag_t *flags1 = &t1->flags[1];
        one = 1 << bpno;
        poshalf = one >> 1;
        neghalf = bpno > 0 ? -poshalf : -1;
        for (k = 0; k < (t1->h & ~3u); k += 4) {
                for (i = 0; i < t1->w; ++i) {
                        mi_INT32 *data2 = data1 + i;
                        mi_flag_t *flags2 = flags1 + i;
                        flags2 += t1->flags_stride;
                        mi_t1_dec_refpass_step_mqc(t1, flags2, data2, poshalf, neghalf);
                        data2 += t1->w;
                        flags2 += t1->flags_stride;
                        mi_t1_dec_refpass_step_mqc(t1, flags2, data2, poshalf, neghalf);
                        data2 += t1->w;
                        flags2 += t1->flags_stride;
                        mi_t1_dec_refpass_step_mqc(t1, flags2, data2, poshalf, neghalf);
                        data2 += t1->w;
                        flags2 += t1->flags_stride;
                        mi_t1_dec_refpass_step_mqc(t1, flags2, data2, poshalf, neghalf);
                        data2 += t1->w;
                }
                data1 += t1->w << 2;
                flags1 += t1->flags_stride << 2;
        }
        for (i = 0; i < t1->w; ++i) {
                mi_INT32 *data2 = data1 + i;
                mi_flag_t *flags2 = flags1 + i;
                for (j = k; j < t1->h; ++j) {
                        flags2 += t1->flags_stride;
                        mi_t1_dec_refpass_step_mqc(t1, flags2, data2, poshalf, neghalf);
                        data2 += t1->w;
                }
        }
}                               /* VSC and  BYPASS by Antonin */

static void mi_t1_dec_refpass_mqc_vsc(
                mi_t1_t *t1,
                mi_INT32 bpno)
{
        mi_INT32 one, poshalf, neghalf;
        mi_UINT32 i, j, k;
        mi_INT32 vsc;
        one = 1 << bpno;
        poshalf = one >> 1;
        neghalf = bpno > 0 ? -poshalf : -1;
        for (k = 0; k < t1->h; k += 4) {
                for (i = 0; i < t1->w; ++i) {
                        for (j = k; j < k + 4 && j < t1->h; ++j) {
                                vsc = ((j == k + 3 || j == t1->h - 1)) ? 1 : 0;
                                mi_t1_dec_refpass_step_mqc_vsc(
                                                t1,
                                                &t1->flags[((j+1) * t1->flags_stride) + i + 1],
                                                &t1->data[(j * t1->w) + i],
                                                poshalf,
                                                neghalf,
                                                vsc);
                        }
                }
        }
}                               /* VSC and  BYPASS by Antonin */


static void mi_t1_enc_clnpass_step(
		mi_t1_t *t1,
		mi_flag_t *flagsp,
		mi_INT32 *datap,
		mi_UINT32 orient,
		mi_INT32 bpno,
		mi_INT32 one,
		mi_INT32 *nmsedec,
		mi_UINT32 partial,
		mi_UINT32 vsc)
{
	mi_INT32 v;
	mi_UINT32 flag;
	
	mi_mqc_t *mqc = t1->mqc;	/* MQC component */
	
	flag = vsc ? (mi_UINT32)((*flagsp) & (~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW | T1_SGN_S))) : (mi_UINT32)(*flagsp);
	if (partial) {
		goto LABEL_PARTIAL;
	}
	if (!(*flagsp & (T1_SIG | T1_VISIT))) {
		mi_mqc_setcurctx(mqc, mi_t1_getctxno_zc(flag, orient));
		v = (mi_int_abs(*datap) & one) ? 1 : 0;
		mi_mqc_encode(mqc, (mi_UINT32)v);
		if (v) {
LABEL_PARTIAL:
			*nmsedec += mi_t1_getnmsedec_sig((mi_UINT32)mi_int_abs(*datap), (mi_UINT32)(bpno));
			mi_mqc_setcurctx(mqc, mi_t1_getctxno_sc(flag));
			v = *datap < 0 ? 1 : 0;
			mi_mqc_encode(mqc, (mi_UINT32)(v ^ mi_t1_getspb((mi_UINT32)flag)));
			mi_t1_updateflags(flagsp, (mi_UINT32)v, t1->flags_stride);
		}
	}
	*flagsp &= ~T1_VISIT;
}

static void mi_t1_dec_clnpass_step_partial(
		mi_t1_t *t1,
		mi_flag_t *flagsp,
		mi_INT32 *datap,
		mi_INT32 orient,
		mi_INT32 oneplushalf)
{
	mi_INT32 v, flag;
	mi_mqc_t *mqc = t1->mqc;	/* MQC component */
	
	mi_ARG_NOT_USED(orient);
	
	flag = *flagsp;
	mi_mqc_setcurctx(mqc, mi_t1_getctxno_sc((mi_UINT32)flag));
	v = mi_mqc_decode(mqc) ^ mi_t1_getspb((mi_UINT32)flag);
	*datap = v ? -oneplushalf : oneplushalf;
	mi_t1_updateflags(flagsp, (mi_UINT32)v, t1->flags_stride);
	*flagsp &= ~T1_VISIT;
}				/* VSC and  BYPASS by Antonin */

static void mi_t1_dec_clnpass_step(
		mi_t1_t *t1,
		mi_flag_t *flagsp,
		mi_INT32 *datap,
		mi_INT32 orient,
		mi_INT32 oneplushalf)
{
	mi_INT32 v, flag;
	
	mi_mqc_t *mqc = t1->mqc;	/* MQC component */
	
	flag = *flagsp;
	if (!(flag & (T1_SIG | T1_VISIT))) {
		mi_mqc_setcurctx(mqc, mi_t1_getctxno_zc((mi_UINT32)flag, (mi_UINT32)orient));
		if (mi_mqc_decode(mqc)) {
			mi_mqc_setcurctx(mqc, mi_t1_getctxno_sc((mi_UINT32)flag));
			v = mi_mqc_decode(mqc) ^ mi_t1_getspb((mi_UINT32)flag);
			*datap = v ? -oneplushalf : oneplushalf;
			mi_t1_updateflags(flagsp, (mi_UINT32)v, t1->flags_stride);
		}
	}
	*flagsp &= ~T1_VISIT;
}				/* VSC and  BYPASS by Antonin */

static void mi_t1_dec_clnpass_step_vsc(
		mi_t1_t *t1,
		mi_flag_t *flagsp,
		mi_INT32 *datap,
		mi_INT32 orient,
		mi_INT32 oneplushalf,
		mi_INT32 partial,
		mi_INT32 vsc)
{
	mi_INT32 v, flag;
	
	mi_mqc_t *mqc = t1->mqc;	/* MQC component */
	
	flag = vsc ? ((*flagsp) & (~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW | T1_SGN_S))) : (*flagsp);
	if (partial) {
		goto LABEL_PARTIAL;
	}
	if (!(flag & (T1_SIG | T1_VISIT))) {
		mi_mqc_setcurctx(mqc, mi_t1_getctxno_zc((mi_UINT32)flag, (mi_UINT32)orient));
		if (mi_mqc_decode(mqc)) {
LABEL_PARTIAL:
			mi_mqc_setcurctx(mqc, mi_t1_getctxno_sc((mi_UINT32)flag));
			v = mi_mqc_decode(mqc) ^ mi_t1_getspb((mi_UINT32)flag);
			*datap = v ? -oneplushalf : oneplushalf;
			mi_t1_updateflags(flagsp, (mi_UINT32)v, t1->flags_stride);
		}
	}
	*flagsp &= ~T1_VISIT;
}

static void mi_t1_enc_clnpass(
		mi_t1_t *t1,
		mi_INT32 bpno,
		mi_UINT32 orient,
		mi_INT32 *nmsedec,
		mi_UINT32 cblksty)
{
	mi_UINT32 i, j, k;
	mi_INT32 one;
	mi_UINT32 agg, runlen, vsc;
	
	mi_mqc_t *mqc = t1->mqc;	/* MQC component */
	
	*nmsedec = 0;
	one = 1 << (bpno + T1_NMSEDEC_FRACBITS);
	for (k = 0; k < t1->h; k += 4) {
		for (i = 0; i < t1->w; ++i) {
			if (k + 3 < t1->h) {
				if (cblksty & J2K_CCP_CBLKSTY_VSC) {
					agg = !(MACRO_t1_flags(1 + k,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH)
						|| MACRO_t1_flags(1 + k + 1,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH)
						|| MACRO_t1_flags(1 + k + 2,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH)
						|| (MACRO_t1_flags(1 + k + 3,1 + i) 
						& (~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW |	T1_SGN_S))) & (T1_SIG | T1_VISIT | T1_SIG_OTH));
				} else {
					agg = !((MACRO_t1_flags(1 + k,1 + i) | 
						    MACRO_t1_flags(1 + k + 1,1 + i) |
						    MACRO_t1_flags(1 + k + 2,1 + i) | 
						     MACRO_t1_flags(1 + k + 3,1 + i)) & (T1_SIG | T1_VISIT | T1_SIG_OTH));
				}
			} else {
				agg = 0;
			}
			if (agg) {//开启行程模式
				for (runlen = 0; runlen < 4; ++runlen) {
					if (mi_int_abs(t1->data[((k + runlen)*t1->data_stride) + i]) & one)
						break;//一旦发现有变为重要的位，停止这个for
				}
				mi_mqc_setcurctx(mqc, T1_CTXNO_AGG);
				mi_mqc_encode(mqc, runlen != 4);//不管是不是出现了行程中断，都进行MO编码
				if (runlen == 4) {
					continue;//若当前条带根本没有变为重要的位，直接进行下一条带的读取分析，不考虑后面的内容了
				}
				//这三个步骤是进行游长编码
				mi_mqc_setcurctx(mqc, T1_CTXNO_UNI);
				mi_mqc_encode(mqc, runlen >> 1);
				mi_mqc_encode(mqc, runlen & 1);
			} else {
				runlen = 0;
			}
			for (j = k + runlen; j < k + 4 && j < t1->h; ++j) {
				vsc = ((cblksty & J2K_CCP_CBLKSTY_VSC) && (j == k + 3 || j == t1->h - 1)) ? 1 : 0;
				mi_t1_enc_clnpass_step(
						t1,
						&t1->flags[((j+1) * t1->flags_stride) + i + 1],
						&t1->data[(j * t1->data_stride) + i],
						orient,
						bpno,
						one,
						nmsedec,
						agg && (j == k + runlen),
						vsc);
			}
		}
	}
}

static void mi_t1_dec_clnpass(
		mi_t1_t *t1,
		mi_INT32 bpno,
		mi_INT32 orient,
		mi_INT32 cblksty)
{
	mi_INT32 one, half, oneplushalf, agg, runlen, vsc;
    mi_UINT32 i, j, k;
	mi_INT32 segsym = cblksty & J2K_CCP_CBLKSTY_SEGSYM;
	
	mi_mqc_t *mqc = t1->mqc;	/* MQC component */
	
	one = 1 << bpno;
	half = one >> 1;
	oneplushalf = one | half;
	if (cblksty & J2K_CCP_CBLKSTY_VSC) {
	for (k = 0; k < t1->h; k += 4) {
		for (i = 0; i < t1->w; ++i) {
			if (k + 3 < t1->h) {
					agg = !(MACRO_t1_flags(1 + k,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH)
						|| MACRO_t1_flags(1 + k + 1,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH)
						|| MACRO_t1_flags(1 + k + 2,1 + i) & (T1_SIG | T1_VISIT | T1_SIG_OTH)
						|| (MACRO_t1_flags(1 + k + 3,1 + i) 
						& (~(T1_SIG_S | T1_SIG_SE | T1_SIG_SW |	T1_SGN_S))) & (T1_SIG | T1_VISIT | T1_SIG_OTH));
				} else {
				agg = 0;
			}
			if (agg) {
				mi_mqc_setcurctx(mqc, T1_CTXNO_AGG);
				if (!mi_mqc_decode(mqc)) {
					continue;
				}
				mi_mqc_setcurctx(mqc, T1_CTXNO_UNI);
				runlen = mi_mqc_decode(mqc);
				runlen = (runlen << 1) | mi_mqc_decode(mqc);
			} else {
				runlen = 0;
			}
			for (j = k + (mi_UINT32)runlen; j < k + 4 && j < t1->h; ++j) {
					vsc = (j == k + 3 || j == t1->h - 1) ? 1 : 0;
					mi_t1_dec_clnpass_step_vsc(
						t1,
						&t1->flags[((j+1) * t1->flags_stride) + i + 1],
						&t1->data[(j * t1->w) + i],
						orient,
						oneplushalf,
						agg && (j == k + (mi_UINT32)runlen),
						vsc);
			}
		}
	}
	} else {
		mi_INT32 *data1 = t1->data;
		mi_flag_t *flags1 = &t1->flags[1];
		for (k = 0; k < (t1->h & ~3u); k += 4) {
			for (i = 0; i < t1->w; ++i) {
				mi_INT32 *data2 = data1 + i;
				mi_flag_t *flags2 = flags1 + i;
				agg = !((MACRO_t1_flags(1 + k, 1 + i) |
							MACRO_t1_flags(1 + k + 1, 1 + i) |
							MACRO_t1_flags(1 + k + 2, 1 + i) |
							MACRO_t1_flags(1 + k + 3, 1 + i)) & (T1_SIG | T1_VISIT | T1_SIG_OTH));
				if (agg) {
					mi_mqc_setcurctx(mqc, T1_CTXNO_AGG);
					if (!mi_mqc_decode(mqc)) {
						continue;
					}
					mi_mqc_setcurctx(mqc, T1_CTXNO_UNI);
					runlen = mi_mqc_decode(mqc);
					runlen = (runlen << 1) | mi_mqc_decode(mqc);
					flags2 += (mi_UINT32)runlen * t1->flags_stride;
					data2 += (mi_UINT32)runlen * t1->w;
					for (j = (mi_UINT32)runlen; j < 4 && j < t1->h; ++j) {
						flags2 += t1->flags_stride;
						if (agg && (j == (mi_UINT32)runlen)) {
							mi_t1_dec_clnpass_step_partial(t1, flags2, data2, orient, oneplushalf);
						} else {
							mi_t1_dec_clnpass_step(t1, flags2, data2, orient, oneplushalf);
						}
						data2 += t1->w;
					}
				} else {
					flags2 += t1->flags_stride;
					mi_t1_dec_clnpass_step(t1, flags2, data2, orient, oneplushalf);
					data2 += t1->w;
					flags2 += t1->flags_stride;
					mi_t1_dec_clnpass_step(t1, flags2, data2, orient, oneplushalf);
					data2 += t1->w;
					flags2 += t1->flags_stride;
					mi_t1_dec_clnpass_step(t1, flags2, data2, orient, oneplushalf);
					data2 += t1->w;
					flags2 += t1->flags_stride;
					mi_t1_dec_clnpass_step(t1, flags2, data2, orient, oneplushalf);
					data2 += t1->w;
				}
			}
			data1 += t1->w << 2;
			flags1 += t1->flags_stride << 2;
		}
		for (i = 0; i < t1->w; ++i) {
			mi_INT32 *data2 = data1 + i;
			mi_flag_t *flags2 = flags1 + i;
			for (j = k; j < t1->h; ++j) {
				flags2 += t1->flags_stride;
				mi_t1_dec_clnpass_step(t1, flags2, data2, orient, oneplushalf);
				data2 += t1->w;
			}
		}
	}

	if (segsym) {
		mi_INT32 v = 0;
		mi_mqc_setcurctx(mqc, T1_CTXNO_UNI);
		v = mi_mqc_decode(mqc);
		v = (v << 1) | mi_mqc_decode(mqc);
		v = (v << 1) | mi_mqc_decode(mqc);
		v = (v << 1) | mi_mqc_decode(mqc);
		/*
		if (v!=0xa) {
			mi_event_msg(t1->cinfo, EVT_WARNING, "Bad segmentation symbol %x\n", v);
		} 
		*/
	}
}				/* VSC and  BYPASS by Antonin */


/** mod fixed_quality */
static mi_FLOAT64 mi_t1_getwmsedec(
		mi_INT32 nmsedec,
		mi_UINT32 compno,
		mi_UINT32 level,
		mi_UINT32 orient,
		mi_INT32 bpno,
		mi_UINT32 qmfbid,
		mi_FLOAT64 stepsize,
		mi_UINT32 numcomps,
		const mi_FLOAT64 * mct_norms,
		mi_UINT32 mct_numcomps)
{
	mi_FLOAT64 w1 = 1, w2, wmsedec;
	mi_ARG_NOT_USED(numcomps);

	if (mct_norms && (compno < mct_numcomps)) {
		w1 = mct_norms[compno];
	}

	if (qmfbid == 1) {
		w2 = mi_dwt_getnorm(level, orient);
	} else {	/* if (qmfbid == 0) */
		w2 = mi_dwt_getnorm_real(level, orient);
	}

	wmsedec = w1 * w2 * stepsize * (1 << bpno);
	wmsedec *= wmsedec * nmsedec / 8192.0;

	return wmsedec;
}

static mi_BOOL mi_t1_allocate_buffers(
		mi_t1_t *t1,
		mi_UINT32 w,
		mi_UINT32 h)
{
	mi_UINT32 datasize=w * h;
	mi_UINT32 flagssize;

	/* encoder uses tile buffer, so no need to allocate */
	if (!t1->encoder) {
		if(datasize > t1->datasize){
			mi_aligned_free(t1->data);
			t1->data = (mi_INT32*) mi_aligned_malloc(datasize * sizeof(mi_INT32));
			if(!t1->data){
				/* FIXME event manager error callback */
				return mi_FALSE;
			}
			t1->datasize=datasize;
		}
		/* memset first arg is declared to never be null by gcc */
		if (t1->data != NULL) {
			memset(t1->data,0,datasize * sizeof(mi_INT32));
		}
	}
	t1->flags_stride=w+2;
	flagssize=t1->flags_stride * (h+2);

	if(flagssize > t1->flagssize){
		mi_aligned_free(t1->flags);
		t1->flags = (mi_flag_t*) mi_aligned_malloc(flagssize * sizeof(mi_flag_t));
		if(!t1->flags){
			/* FIXME event manager error callback */
			return mi_FALSE;
		}
		t1->flagssize=flagssize;
	}
	memset(t1->flags,0,flagssize * sizeof(mi_flag_t));

	t1->w=w;
	t1->h=h;

	return mi_TRUE;
}

/* ----------------------------------------------------------------------- */

/* ----------------------------------------------------------------------- */
/**
 * Creates a new Tier 1 handle
 * and initializes the look-up tables of the Tier-1 coder/decoder
 * @return a new T1 handle if successful, returns NULL otherwise
*/
mi_t1_t* mi_t1_create(mi_BOOL isEncoder)
{
	mi_t1_t *l_t1 = 00;

	l_t1 = (mi_t1_t*) mi_calloc(1,sizeof(mi_t1_t));
	if (!l_t1) {
		return 00;
	}

	/* create MQC and RAW handles */
	l_t1->mqc = mi_mqc_create();
	if (! l_t1->mqc) {
		mi_t1_destroy(l_t1);
		return 00;
	}

	l_t1->raw = mi_raw_create();
	if (! l_t1->raw) {
		mi_t1_destroy(l_t1);
		return 00;
	}
	l_t1->encoder = isEncoder;

	return l_t1;
}


/**
 * Destroys a previously created T1 handle
 *
 * @param p_t1 Tier 1 handle to destroy
*/
void mi_t1_destroy(mi_t1_t *p_t1)
{
	if (! p_t1) {
		return;
	}

	/* destroy MQC and RAW handles */
	mi_mqc_destroy(p_t1->mqc);
	p_t1->mqc = 00;
	mi_raw_destroy(p_t1->raw);
	p_t1->raw = 00;
	
	/* encoder uses tile buffer, so no need to free */
	if (!p_t1->encoder && p_t1->data) {
		mi_aligned_free(p_t1->data);
		p_t1->data = 00;
	}

	if (p_t1->flags) {
		mi_aligned_free(p_t1->flags);
		p_t1->flags = 00;
	}

	mi_free(p_t1);
}

mi_BOOL mi_t1_decode_cblks(   mi_t1_t* t1,
                            mi_tcd_tilecomp_t* tilec,
                            mi_tccp_t* tccp
                            )
{
	mi_UINT32 resno, bandno, precno, cblkno;
	mi_UINT32 tile_w = (mi_UINT32)(tilec->x1 - tilec->x0);

	for (resno = 0; resno < tilec->minimum_num_resolutions; ++resno) {
		mi_tcd_resolution_t* res = &tilec->resolutions[resno];

		for (bandno = 0; bandno < res->numbands; ++bandno) {
			mi_tcd_band_t* restrict band = &res->bands[bandno];

			for (precno = 0; precno < res->pw * res->ph; ++precno) {
				mi_tcd_precinct_t* precinct = &band->precincts[precno];

				for (cblkno = 0; cblkno < precinct->cw * precinct->ch; ++cblkno) {
					mi_tcd_cblk_dec_t* cblk = &precinct->cblks.dec[cblkno];
					mi_INT32* restrict datap;
					mi_UINT32 cblk_w, cblk_h;
					mi_INT32 x, y;
					mi_UINT32 i, j;

                    if (mi_FALSE == mi_t1_decode_cblk(
                                            t1,
                                            cblk,
                                            band->bandno,
                                            (mi_UINT32)tccp->roishift,
                                            tccp->cblksty)) {
                            return mi_FALSE;
                    }

					x = cblk->x0 - band->x0;
					y = cblk->y0 - band->y0;
					if (band->bandno & 1) {
						mi_tcd_resolution_t* pres = &tilec->resolutions[resno - 1];
						x += pres->x1 - pres->x0;
					}
					if (band->bandno & 2) {
						mi_tcd_resolution_t* pres = &tilec->resolutions[resno - 1];
						y += pres->y1 - pres->y0;
					}

					datap=t1->data;
					cblk_w = t1->w;
					cblk_h = t1->h;

					if (tccp->roishift) {
						mi_INT32 thresh = 1 << tccp->roishift;
						for (j = 0; j < cblk_h; ++j) {
							for (i = 0; i < cblk_w; ++i) {
								mi_INT32 val = datap[(j * cblk_w) + i];
								mi_INT32 mag = abs(val);
								if (mag >= thresh) {
									mag >>= tccp->roishift;
									datap[(j * cblk_w) + i] = val < 0 ? -mag : mag;
								}
							}
						}
					}
					if (tccp->qmfbid == 1) {
                        mi_INT32* restrict tiledp = &tilec->data[(mi_UINT32)y * tile_w + (mi_UINT32)x];
						for (j = 0; j < cblk_h; ++j) {
							for (i = 0; i < cblk_w; ++i) {
								mi_INT32 tmp = datap[(j * cblk_w) + i];
								((mi_INT32*)tiledp)[(j * tile_w) + i] = tmp/2;
							}
						}
					} else {		/* if (tccp->qmfbid == 0) */
                        mi_FLOAT32* restrict tiledp = (mi_FLOAT32*) &tilec->data[(mi_UINT32)y * tile_w + (mi_UINT32)x];
						for (j = 0; j < cblk_h; ++j) {
                            mi_FLOAT32* restrict tiledp2 = tiledp;
							for (i = 0; i < cblk_w; ++i) {
                                mi_FLOAT32 tmp = (mi_FLOAT32)*datap * band->stepsize;
                                *tiledp2 = tmp;
                                datap++;
                                tiledp2++;
							}
                            tiledp += tile_w;
						}
					}
				} /* cblkno */
			} /* precno */
		} /* bandno */
	} /* resno */
        return mi_TRUE;
}


static mi_BOOL mi_t1_decode_cblk(mi_t1_t *t1,
                            mi_tcd_cblk_dec_t* cblk,
                            mi_UINT32 orient,
                            mi_UINT32 roishift,
                            mi_UINT32 cblksty)
{
	mi_raw_t *raw = t1->raw;	/* RAW component */
	mi_mqc_t *mqc = t1->mqc;	/* MQC component */

	mi_INT32 bpno_plus_one;
	mi_UINT32 passtype;
	mi_UINT32 segno, passno;
	mi_BYTE type = T1_TYPE_MQ; /* BYPASS mode */

	if(!mi_t1_allocate_buffers(
				t1,
				(mi_UINT32)(cblk->x1 - cblk->x0),
				(mi_UINT32)(cblk->y1 - cblk->y0)))
	{
		return mi_FALSE;
	}

	bpno_plus_one = (mi_INT32)(roishift + cblk->numbps);
	passtype = 2;

	mi_mqc_resetstates(mqc);
	mi_mqc_setstate(mqc, T1_CTXNO_UNI, 0, 46);
	mi_mqc_setstate(mqc, T1_CTXNO_AGG, 0, 3);
	mi_mqc_setstate(mqc, T1_CTXNO_ZC, 0, 4);

	for (segno = 0; segno < cblk->real_num_segs; ++segno) {
		mi_tcd_seg_t *seg = &cblk->segs[segno];

		/* BYPASS mode */
		type = ((bpno_plus_one <= ((mi_INT32) (cblk->numbps)) - 4) && (passtype < 2) && (cblksty & J2K_CCP_CBLKSTY_LAZY)) ? T1_TYPE_RAW : T1_TYPE_MQ;
		/* FIXME: slviewer gets here with a null pointer. Why? Partially downloaded and/or corrupt textures? */
		if(seg->data == 00){
			continue;
		}
		if (type == T1_TYPE_RAW) {
			mi_raw_init_dec(raw, (*seg->data) + seg->dataindex, seg->len);
		} else {
            if (mi_FALSE == mi_mqc_init_dec(mqc, (*seg->data) + seg->dataindex, seg->len)) {
                    return mi_FALSE;
            }
		}

		for (passno = 0; (passno < seg->real_num_passes) && (bpno_plus_one >= 1); ++passno) {
            switch (passtype) {
                case 0:
                    if (type == T1_TYPE_RAW) {
                        mi_t1_dec_sigpass_raw(t1, bpno_plus_one, (mi_INT32)orient, (mi_INT32)cblksty);
                    } else {
                        if (cblksty & J2K_CCP_CBLKSTY_VSC) {
                            mi_t1_dec_sigpass_mqc_vsc(t1, bpno_plus_one, (mi_INT32)orient);
                        } else {
                            mi_t1_dec_sigpass_mqc(t1, bpno_plus_one, (mi_INT32)orient);
                        }
                    }
                    break;
                case 1:
                    if (type == T1_TYPE_RAW) {
                            mi_t1_dec_refpass_raw(t1, bpno_plus_one, (mi_INT32)cblksty);
                    } else {
                        if (cblksty & J2K_CCP_CBLKSTY_VSC) {
                            mi_t1_dec_refpass_mqc_vsc(t1, bpno_plus_one);
                        } else {
                            mi_t1_dec_refpass_mqc(t1, bpno_plus_one);
                        }
                    }
                    break;
                case 2:
                    mi_t1_dec_clnpass(t1, bpno_plus_one, (mi_INT32)orient, (mi_INT32)cblksty);
                    break;
            }

			if ((cblksty & J2K_CCP_CBLKSTY_RESET) && type == T1_TYPE_MQ) {
				mi_mqc_resetstates(mqc);
				mi_mqc_setstate(mqc, T1_CTXNO_UNI, 0, 46);
				mi_mqc_setstate(mqc, T1_CTXNO_AGG, 0, 3);
				mi_mqc_setstate(mqc, T1_CTXNO_ZC, 0, 4);
			}
			if (++passtype == 3) {
				passtype = 0;
				bpno_plus_one--;
			}
		}
	}
    return mi_TRUE;
}




mi_BOOL mi_t1_encode_cblks(   mi_t1_t *t1,
                                mi_tcd_tile_t *tile,
                                mi_tcp_t *tcp,
                                const mi_FLOAT64 * mct_norms,
                                mi_UINT32 mct_numcomps
                                )
{
	mi_UINT32 compno, resno, bandno, precno, cblkno;

	tile->distotile = 0;		/* fixed_quality */

	for (compno = 0; compno < tile->numcomps; ++compno) {
		mi_tcd_tilecomp_t* tilec = &tile->comps[compno];//切片相关的分量的信息
		mi_tccp_t* tccp = &tcp->tccps[compno];//切片编码参数
		mi_UINT32 tile_w = (mi_UINT32)(tilec->x1 - tilec->x0);//图片分量宽

		for (resno = 0; resno < tilec->numresolutions; ++resno) {
			mi_tcd_resolution_t *res = &tilec->resolutions[resno];//当前分辨率的分辨率信息

			for (bandno = 0; bandno < res->numbands; ++bandno) {
				mi_tcd_band_t* restrict band = &res->bands[bandno];//当前子带的子带信息
                mi_INT32 bandconst = 8192 * 8192 / ((mi_INT32) floor(band->stepsize * 8192));//可逆压缩是用不到这个的

				for (precno = 0; precno < res->pw * res->ph; ++precno) {
					mi_tcd_precinct_t *prc = &band->precincts[precno];//当前区的信息

					for (cblkno = 0; cblkno < prc->cw * prc->ch; ++cblkno) {
						mi_tcd_cblk_enc_t* cblk = &prc->cblks.enc[cblkno];//当前块的信息
						mi_INT32* restrict tiledp;
						mi_UINT32 cblk_w;
						mi_UINT32 cblk_h;
						mi_UINT32 i, j, tileIndex=0, tileLineAdvance;

						mi_INT32 x = cblk->x0 - band->x0;//代码块头距子带头的偏移量（x）
						mi_INT32 y = cblk->y0 - band->y0;//代码块头距子带头的偏移量（y）
						if (band->bandno & 1) {
							mi_tcd_resolution_t *pres = &tilec->resolutions[resno - 1];
							x += pres->x1 - pres->x0;
						}
						if (band->bandno & 2) {
							mi_tcd_resolution_t *pres = &tilec->resolutions[resno - 1];
							y += pres->y1 - pres->y0;
						}

						if(!mi_t1_allocate_buffers(
									t1,
									(mi_UINT32)(cblk->x1 - cblk->x0),
									(mi_UINT32)(cblk->y1 - cblk->y0)))
						{
							return mi_FALSE;
						}

						cblk_w = t1->w;
						cblk_h = t1->h;
						tileLineAdvance = tile_w - cblk_w;

						tiledp=&tilec->data[(mi_UINT32)y * tile_w + (mi_UINT32)x];
						t1->data = tiledp;
						t1->data_stride = tile_w;
						if (tccp->qmfbid == 1) {
							for (j = 0; j < cblk_h; ++j) {
								for (i = 0; i < cblk_w; ++i) {
									tiledp[tileIndex] *= (1 << T1_NMSEDEC_FRACBITS);//？？？
									tileIndex++;
								}
								tileIndex += tileLineAdvance;
							}
						} else {		/* if (tccp->qmfbid == 0) */
							for (j = 0; j < cblk_h; ++j) {
								for (i = 0; i < cblk_w; ++i) {
									mi_INT32 tmp = tiledp[tileIndex];
									tiledp[tileIndex] =
										mi_int_fix_mul_t1(
										tmp,
										bandconst);
									tileIndex++;
								}
								tileIndex += tileLineAdvance;
							}
						}

						mi_t1_encode_cblk(
								t1,
								cblk,
								band->bandno,
								compno,
								tilec->numresolutions - 1 - resno,
								tccp->qmfbid,
								band->stepsize,
								tccp->cblksty,
								tile->numcomps,
								tile,
								mct_norms,
								mct_numcomps);

					} /* cblkno */
				} /* precno */
			} /* bandno */
		} /* resno  */
	} /* compno  */
	return mi_TRUE;
}

/** mod fixed_quality 代码块EBCOT函数*/
static void mi_t1_encode_cblk(mi_t1_t *t1,
                        mi_tcd_cblk_enc_t* cblk,
                        mi_UINT32 orient,
                        mi_UINT32 compno,
                        mi_UINT32 level,
                        mi_UINT32 qmfbid,
                        mi_FLOAT64 stepsize,
                        mi_UINT32 cblksty,
                        mi_UINT32 numcomps,
                        mi_tcd_tile_t * tile,
                        const mi_FLOAT64 * mct_norms,
                        mi_UINT32 mct_numcomps)
{
	mi_FLOAT64 cumwmsedec = 0.0;

	mi_mqc_t *mqc = t1->mqc;	/* MQC component */

	mi_UINT32 passno;//位平面编码过程标识符
	mi_INT32 bpno;//代码块表示数据所需位数-1
	mi_UINT32 passtype;
	mi_INT32 nmsedec = 0;
	mi_INT32 max;
	mi_UINT32 i, j;
	mi_BYTE type = T1_TYPE_MQ;
	mi_FLOAT64 tempwmsedec;
	//找代码块内数据的绝对值最大的值
	max = 0;
	for (i = 0; i < t1->w; ++i) {
		for (j = 0; j < t1->h; ++j) {
			mi_INT32 tmp = abs(t1->data[i + j*t1->data_stride]);//一行一行读数据的绝对值
			max = mi_int_max(max, tmp);
		}
	}
	//确定代码块内表示所有数据所需要的位数（最大值的位数+1-6）
	cblk->numbps = max ? (mi_UINT32)((mi_int_floorlog2(max) + 1) - T1_NMSEDEC_FRACBITS) : 0;

	bpno = (mi_INT32)(cblk->numbps - 1);
	passtype = 2;

	mi_mqc_resetstates(mqc);
	mi_mqc_setstate(mqc, T1_CTXNO_UNI, 0, 46);//定义CX=18的操作为UNIFORM
	mi_mqc_setstate(mqc, T1_CTXNO_AGG, 0, 3);//定义CX=17的操作为Run-Length Coding
	mi_mqc_setstate(mqc, T1_CTXNO_ZC, 0, 4);//定义CX=0的操作为Zero Coding
	mi_mqc_init_enc(mqc, cblk->data);
	
	for (passno = 0; bpno >= 0; ++passno) {
		mi_tcd_pass_t *pass = &cblk->passes[passno];//位平面编码过程信息
		mi_UINT32 correction = 3;
		type = ((bpno < ((mi_INT32) (cblk->numbps) - 4)) && (passtype < 2) && (cblksty & J2K_CCP_CBLKSTY_LAZY)) ? T1_TYPE_RAW : T1_TYPE_MQ;//这句对于使用MQ编码来说估计没什么用

		switch (passtype) {
			case 0:
				mi_t1_enc_sigpass(t1, bpno, orient, &nmsedec, type, cblksty);
				break;
			case 1:
				mi_t1_enc_refpass(t1, bpno, &nmsedec, type, cblksty);
				break;
			case 2:
				mi_t1_enc_clnpass(t1, bpno, orient, &nmsedec, cblksty);
				/* code switch SEGMARK (i.e. SEGSYM) */
				if (cblksty & J2K_CCP_CBLKSTY_SEGSYM)
					mi_mqc_segmark_enc(mqc);
				break;
		}

		/* fixed_quality */
		tempwmsedec = mi_t1_getwmsedec(nmsedec, compno, level, orient, bpno, qmfbid, stepsize, numcomps,mct_norms, mct_numcomps) ;
		cumwmsedec += tempwmsedec;
		tile->distotile += tempwmsedec;

		/* Code switch "RESTART" (i.e. TERMALL) */
		if ((cblksty & J2K_CCP_CBLKSTY_TERMALL)	&& !((passtype == 2) && (bpno - 1 < 0))) {
			if (type == T1_TYPE_RAW) {
				mi_mqc_flush(mqc);
				correction = 1;
				/* correction = mqc_bypass_flush_enc(); */
			} else {			/* correction = mqc_restart_enc(); */
				mi_mqc_flush(mqc);
				correction = 1;
			}
			pass->term = 1;
		} else {
			if (((bpno < ((mi_INT32) (cblk->numbps) - 4) && (passtype > 0))
				|| ((bpno == ((mi_INT32)cblk->numbps - 4)) && (passtype == 2))) && (cblksty & J2K_CCP_CBLKSTY_LAZY)) {
				if (type == T1_TYPE_RAW) {
					mi_mqc_flush(mqc);
					correction = 1;
					/* correction = mqc_bypass_flush_enc(); */
				} else {		/* correction = mqc_restart_enc(); */
					mi_mqc_flush(mqc);
					correction = 1;
				}
				pass->term = 1;
			} else {
				pass->term = 0;
			}

		}

		if (++passtype == 3) {
			passtype = 0;
			bpno--;
		}

		if (pass->term && bpno > 0) {
			type = ((bpno < ((mi_INT32) (cblk->numbps) - 4)) && (passtype < 2) && (cblksty & J2K_CCP_CBLKSTY_LAZY)) ? T1_TYPE_RAW : T1_TYPE_MQ;
			if (type == T1_TYPE_RAW)
				mi_mqc_bypass_init_enc(mqc);
			else
				mi_mqc_restart_init_enc(mqc);
		}

		pass->distortiondec = cumwmsedec;
		pass->rate = mi_mqc_numbytes(mqc) + correction;	/* FIXME */

		/* Code-switch "RESET" */
		if (cblksty & J2K_CCP_CBLKSTY_RESET)
			mi_mqc_reset_enc(mqc);
	}

	/* Code switch "ERTERM" (i.e. PTERM) */
	if (cblksty & J2K_CCP_CBLKSTY_PTERM)
		mi_mqc_erterm_enc(mqc);
	else /* Default coding */ if (!(cblksty & J2K_CCP_CBLKSTY_LAZY))
		mi_mqc_flush(mqc);

	cblk->totalpasses = passno;

	for (passno = 0; passno<cblk->totalpasses; passno++) {
		mi_tcd_pass_t *pass = &cblk->passes[passno];
		if (pass->rate > mi_mqc_numbytes(mqc))
			pass->rate = mi_mqc_numbytes(mqc);
		/*Preventing generation of FF as last data byte of a pass*/
		if((pass->rate>1) && (cblk->data[pass->rate - 1] == 0xFF)){
			pass->rate--;
		}
		pass->len = pass->rate - (passno == 0 ? 0 : cblk->passes[passno - 1].rate);
	}
}

