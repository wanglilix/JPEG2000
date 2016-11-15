
#include "mi_includes.h"

/* ----------------------------------------------------------------------- */

/**
 * Initializes tile coding/decoding
 */
static INLINE mi_BOOL mi_tcd_init_tile(mi_tcd_t *p_tcd, mi_UINT32 p_tile_no, mi_BOOL isEncoder, mi_FLOAT32 fraction, mi_SIZE_T sizeof_block, mi_event_mgr_t* manager);

/**
* Allocates memory for a decoding code block.
*/
static mi_BOOL mi_tcd_code_block_dec_allocate (mi_tcd_cblk_dec_t * p_code_block);

/**
 * Deallocates the decoding data of the given precinct.
 */
static void mi_tcd_code_block_dec_deallocate (mi_tcd_precinct_t * p_precinct);

/**
 * Allocates memory for an encoding code block (but not data).
 */
static mi_BOOL mi_tcd_code_block_enc_allocate (mi_tcd_cblk_enc_t * p_code_block);

/**
 * Allocates data for an encoding code block
 */
static mi_BOOL mi_tcd_code_block_enc_allocate_data (mi_tcd_cblk_enc_t * p_code_block);

/**
 * Deallocates the encoding data of the given precinct.
 */
static void mi_tcd_code_block_enc_deallocate (mi_tcd_precinct_t * p_precinct);


/**
Free the memory allocated for encoding
@param tcd TCD handle
*/
static void mi_tcd_free_tile(mi_tcd_t *tcd);


static mi_BOOL mi_tcd_t2_decode ( mi_tcd_t *p_tcd,
                                    mi_BYTE * p_src_data,
                                    mi_UINT32 * p_data_read,
                                    mi_UINT32 p_max_src_size,
                                    mi_codestream_index_t *p_cstr_index,
                                    mi_event_mgr_t *p_manager);

static mi_BOOL mi_tcd_t1_decode (mi_tcd_t *p_tcd);

static mi_BOOL mi_tcd_dwt_decode (mi_tcd_t *p_tcd);

static mi_BOOL mi_tcd_mct_decode (mi_tcd_t *p_tcd, mi_event_mgr_t *p_manager);

static mi_BOOL mi_tcd_dc_level_shift_decode (mi_tcd_t *p_tcd);


static mi_BOOL mi_tcd_dc_level_shift_encode ( mi_tcd_t *p_tcd );

static mi_BOOL mi_tcd_mct_encode ( mi_tcd_t *p_tcd );

static mi_BOOL mi_tcd_dwt_encode ( mi_tcd_t *p_tcd );

static mi_BOOL mi_tcd_t1_encode ( mi_tcd_t *p_tcd );

static mi_BOOL mi_tcd_t2_encode (     mi_tcd_t *p_tcd,
                                                                    mi_BYTE * p_dest_data,
                                                                    mi_UINT32 * p_data_written,
                                                                    mi_UINT32 p_max_dest_size,
                                                                    mi_codestream_info_t *p_cstr_info );

static mi_BOOL mi_tcd_rate_allocate_encode(   mi_tcd_t *p_tcd,
                                                                                        mi_BYTE * p_dest_data,
                                                                                        mi_UINT32 p_max_dest_size,
                                                                                        mi_codestream_info_t *p_cstr_info );

/* ----------------------------------------------------------------------- */

/**
Create a new TCD handle
*/
mi_tcd_t* mi_tcd_create(mi_BOOL p_is_decoder)
{
        mi_tcd_t *l_tcd = 00;

        /* create the tcd structure */
        l_tcd = (mi_tcd_t*) mi_calloc(1,sizeof(mi_tcd_t));
        if (!l_tcd) {
                return 00;
        }

        l_tcd->m_is_decoder = p_is_decoder ? 1 : 0;

        l_tcd->tcd_image = (mi_tcd_image_t*)mi_calloc(1,sizeof(mi_tcd_image_t));
        if (!l_tcd->tcd_image) {
                mi_free(l_tcd);
                return 00;
        }

        return l_tcd;
}


/* ----------------------------------------------------------------------- */

void mi_tcd_rateallocate_fixed(mi_tcd_t *tcd) {
        mi_UINT32 layno;

        for (layno = 0; layno < tcd->tcp->numlayers; layno++) {
                mi_tcd_makelayer_fixed(tcd, layno, 1);
        }
}


void mi_tcd_makelayer( mi_tcd_t *tcd,
                                                mi_UINT32 layno,
                                                mi_FLOAT64 thresh,
                                                mi_UINT32 final)
{
        mi_UINT32 compno, resno, bandno, precno, cblkno;
        mi_UINT32 passno;

        mi_tcd_tile_t *tcd_tile = tcd->tcd_image->tiles;

        tcd_tile->distolayer[layno] = 0;        /* fixed_quality */

        for (compno = 0; compno < tcd_tile->numcomps; compno++) {
                mi_tcd_tilecomp_t *tilec = &tcd_tile->comps[compno];

                for (resno = 0; resno < tilec->numresolutions; resno++) {
                        mi_tcd_resolution_t *res = &tilec->resolutions[resno];

                        for (bandno = 0; bandno < res->numbands; bandno++) {
                                mi_tcd_band_t *band = &res->bands[bandno];

                                for (precno = 0; precno < res->pw * res->ph; precno++) {
                                        mi_tcd_precinct_t *prc = &band->precincts[precno];

                                        for (cblkno = 0; cblkno < prc->cw * prc->ch; cblkno++) {
                                                mi_tcd_cblk_enc_t *cblk = &prc->cblks.enc[cblkno];
                                                mi_tcd_layer_t *layer = &cblk->layers[layno];
                                                mi_UINT32 n;

                                                if (layno == 0) {
                                                        cblk->numpassesinlayers = 0;
                                                }

                                                n = cblk->numpassesinlayers;

                                                for (passno = cblk->numpassesinlayers; passno < cblk->totalpasses; passno++) {
                                                        mi_UINT32 dr;
                                                        mi_FLOAT64 dd;
                                                        mi_tcd_pass_t *pass = &cblk->passes[passno];

                                                        if (n == 0) {
                                                                dr = pass->rate;
                                                                dd = pass->distortiondec;
                                                        } else {
                                                                dr = pass->rate - cblk->passes[n - 1].rate;
                                                                dd = pass->distortiondec - cblk->passes[n - 1].distortiondec;
                                                        }

                                                        if (!dr) {
                                                                if (dd != 0)
                                                                        n = passno + 1;
                                                                continue;
                                                        }
                                                        if (thresh - (dd / dr) < DBL_EPSILON) /* do not rely on float equality, check with DBL_EPSILON margin */
                                                                n = passno + 1;
                                                }

                                                layer->numpasses = n - cblk->numpassesinlayers;

                                                if (!layer->numpasses) {
                                                        layer->disto = 0;
                                                        continue;
                                                }

                                                if (cblk->numpassesinlayers == 0) {
                                                        layer->len = cblk->passes[n - 1].rate;
                                                        layer->data = cblk->data;
                                                        layer->disto = cblk->passes[n - 1].distortiondec;
                                                } else {
                                                        layer->len = cblk->passes[n - 1].rate - cblk->passes[cblk->numpassesinlayers - 1].rate;
                                                        layer->data = cblk->data + cblk->passes[cblk->numpassesinlayers - 1].rate;
                                                        layer->disto = cblk->passes[n - 1].distortiondec - cblk->passes[cblk->numpassesinlayers - 1].distortiondec;
                                                }

                                                tcd_tile->distolayer[layno] += layer->disto;    /* fixed_quality */

                                                if (final)
                                                        cblk->numpassesinlayers = n;
                                        }
                                }
                        }
                }
        }
}

void mi_tcd_makelayer_fixed(mi_tcd_t *tcd, mi_UINT32 layno, mi_UINT32 final) {
        mi_UINT32 compno, resno, bandno, precno, cblkno;
        mi_INT32 value;                        /*, matrice[tcd_tcp->numlayers][tcd_tile->comps[0].numresolutions][3]; */
        mi_INT32 matrice[10][10][3];
        mi_UINT32 i, j, k;

        mi_cp_t *cp = tcd->cp;
        mi_tcd_tile_t *tcd_tile = tcd->tcd_image->tiles;
        mi_tcp_t *tcd_tcp = tcd->tcp;

        for (compno = 0; compno < tcd_tile->numcomps; compno++) {
                mi_tcd_tilecomp_t *tilec = &tcd_tile->comps[compno];

                for (i = 0; i < tcd_tcp->numlayers; i++) {
                        for (j = 0; j < tilec->numresolutions; j++) {
                                for (k = 0; k < 3; k++) {
                                        matrice[i][j][k] =
                                                (mi_INT32) ((mi_FLOAT32)cp->m_specific_param.m_enc.m_matrice[i * tilec->numresolutions * 3 + j * 3 + k]
                                                * (mi_FLOAT32) (tcd->image->comps[compno].prec / 16.0));
                                }
                        }
                }

                for (resno = 0; resno < tilec->numresolutions; resno++) {
                        mi_tcd_resolution_t *res = &tilec->resolutions[resno];

                        for (bandno = 0; bandno < res->numbands; bandno++) {
                                mi_tcd_band_t *band = &res->bands[bandno];

                                for (precno = 0; precno < res->pw * res->ph; precno++) {
                                        mi_tcd_precinct_t *prc = &band->precincts[precno];

                                        for (cblkno = 0; cblkno < prc->cw * prc->ch; cblkno++) {
                                                mi_tcd_cblk_enc_t *cblk = &prc->cblks.enc[cblkno];
                                                mi_tcd_layer_t *layer = &cblk->layers[layno];
                                                mi_UINT32 n;
                                                mi_INT32 imsb = (mi_INT32)(tcd->image->comps[compno].prec - cblk->numbps); /* number of bit-plan equal to zero */

                                                /* Correction of the matrix of coefficient to include the IMSB information */
                                                if (layno == 0) {
                                                        value = matrice[layno][resno][bandno];
                                                        if (imsb >= value) {
                                                                value = 0;
                                                        } else {
                                                                value -= imsb;
                                                        }
                                                } else {
                                                        value = matrice[layno][resno][bandno] - matrice[layno - 1][resno][bandno];
                                                        if (imsb >= matrice[layno - 1][resno][bandno]) {
                                                                value -= (imsb - matrice[layno - 1][resno][bandno]);
                                                                if (value < 0) {
                                                                        value = 0;
                                                                }
                                                        }
                                                }

                                                if (layno == 0) {
                                                        cblk->numpassesinlayers = 0;
                                                }

                                                n = cblk->numpassesinlayers;
                                                if (cblk->numpassesinlayers == 0) {
                                                        if (value != 0) {
                                                                n = 3 * (mi_UINT32)value - 2 + cblk->numpassesinlayers;
                                                        } else {
                                                                n = cblk->numpassesinlayers;
                                                        }
                                                } else {
                                                        n = 3 * (mi_UINT32)value + cblk->numpassesinlayers;
                                                }

                                                layer->numpasses = n - cblk->numpassesinlayers;

                                                if (!layer->numpasses)
                                                        continue;

                                                if (cblk->numpassesinlayers == 0) {
                                                        layer->len = cblk->passes[n - 1].rate;
                                                        layer->data = cblk->data;
                                                } else {
                                                        layer->len = cblk->passes[n - 1].rate - cblk->passes[cblk->numpassesinlayers - 1].rate;
                                                        layer->data = cblk->data + cblk->passes[cblk->numpassesinlayers - 1].rate;
                                                }

                                                if (final)
                                                        cblk->numpassesinlayers = n;
                                        }
                                }
                        }
                }
        }
}

mi_BOOL mi_tcd_rateallocate(  mi_tcd_t *tcd,
                                                                mi_BYTE *dest,
                                                                mi_UINT32 * p_data_written,
                                                                mi_UINT32 len,
                                                                mi_codestream_info_t *cstr_info)
{
        mi_UINT32 compno, resno, bandno, precno, cblkno, layno;
        mi_UINT32 passno;
        mi_FLOAT64 min, max;
        mi_FLOAT64 cumdisto[100];      /* fixed_quality */
        const mi_FLOAT64 K = 1;                /* 1.1; fixed_quality */
        mi_FLOAT64 maxSE = 0;

        mi_cp_t *cp = tcd->cp;
        mi_tcd_tile_t *tcd_tile = tcd->tcd_image->tiles;
        mi_tcp_t *tcd_tcp = tcd->tcp;

        min = DBL_MAX;
        max = 0;

        tcd_tile->numpix = 0;           /* fixed_quality */

        for (compno = 0; compno < tcd_tile->numcomps; compno++) {
                mi_tcd_tilecomp_t *tilec = &tcd_tile->comps[compno];
                tilec->numpix = 0;

                for (resno = 0; resno < tilec->numresolutions; resno++) {
                        mi_tcd_resolution_t *res = &tilec->resolutions[resno];

                        for (bandno = 0; bandno < res->numbands; bandno++) {
                                mi_tcd_band_t *band = &res->bands[bandno];

                                for (precno = 0; precno < res->pw * res->ph; precno++) {
                                        mi_tcd_precinct_t *prc = &band->precincts[precno];

                                        for (cblkno = 0; cblkno < prc->cw * prc->ch; cblkno++) {
                                                mi_tcd_cblk_enc_t *cblk = &prc->cblks.enc[cblkno];

                                                for (passno = 0; passno < cblk->totalpasses; passno++) {
                                                        mi_tcd_pass_t *pass = &cblk->passes[passno];
                                                        mi_INT32 dr;
                                                        mi_FLOAT64 dd, rdslope;

                                                        if (passno == 0) {
                                                                dr = (mi_INT32)pass->rate;
                                                                dd = pass->distortiondec;
                                                        } else {
                                                                dr = (mi_INT32)(pass->rate - cblk->passes[passno - 1].rate);
                                                                dd = pass->distortiondec - cblk->passes[passno - 1].distortiondec;
                                                        }

                                                        if (dr == 0) {
                                                                continue;
                                                        }

                                                        rdslope = dd / dr;
                                                        if (rdslope < min) {
                                                                min = rdslope;
                                                        }

                                                        if (rdslope > max) {
                                                                max = rdslope;
                                                        }
                                                } /* passno */

                                                /* fixed_quality */
                                                tcd_tile->numpix += ((cblk->x1 - cblk->x0) * (cblk->y1 - cblk->y0));
                                                tilec->numpix += ((cblk->x1 - cblk->x0) * (cblk->y1 - cblk->y0));
                                        } /* cbklno */
                                } /* precno */
                        } /* bandno */
                } /* resno */

                maxSE += (((mi_FLOAT64)(1 << tcd->image->comps[compno].prec) - 1.0)
                        * ((mi_FLOAT64)(1 << tcd->image->comps[compno].prec) -1.0))
                        * ((mi_FLOAT64)(tilec->numpix));
        } /* compno */

        /* index file */
        if(cstr_info) {
                mi_tile_info_t *tile_info = &cstr_info->tile[tcd->tcd_tileno];
                tile_info->numpix = tcd_tile->numpix;
                tile_info->distotile = tcd_tile->distotile;
                tile_info->thresh = (mi_FLOAT64 *) mi_malloc(tcd_tcp->numlayers * sizeof(mi_FLOAT64));
                if (!tile_info->thresh) {
                        /* FIXME event manager error callback */
                        return mi_FALSE;
                }
        }

        for (layno = 0; layno < tcd_tcp->numlayers; layno++) {
                mi_FLOAT64 lo = min;
                mi_FLOAT64 hi = max;
                mi_UINT32 maxlen = tcd_tcp->rates[layno] > 0.0f ? mi_uint_min(((mi_UINT32) ceil(tcd_tcp->rates[layno])), len) : len;
                mi_FLOAT64 goodthresh = 0;
                mi_FLOAT64 stable_thresh = 0;
                mi_UINT32 i;
                mi_FLOAT64 distotarget;                /* fixed_quality */

                /* fixed_quality */
                distotarget = tcd_tile->distotile - ((K * maxSE) / pow((mi_FLOAT32)10, tcd_tcp->distoratio[layno] / 10));

                /* Don't try to find an optimal threshold but rather take everything not included yet, if
                  -r xx,yy,zz,0   (disto_alloc == 1 and rates == 0)
                  -q xx,yy,zz,0   (fixed_quality == 1 and distoratio == 0)
                  ==> possible to have some lossy layers and the last layer for sure lossless */
                if ( ((cp->m_specific_param.m_enc.m_disto_alloc==1) && (tcd_tcp->rates[layno]>0.0f)) || ((cp->m_specific_param.m_enc.m_fixed_quality==1) && (tcd_tcp->distoratio[layno]>0.0))) {
                        mi_t2_t*t2 = mi_t2_create(tcd->image, cp);
                        mi_FLOAT64 thresh = 0;

                        if (t2 == 00) {
                                return mi_FALSE;
                        }

                        for     (i = 0; i < 128; ++i) {
                                mi_FLOAT64 distoachieved = 0;  /* fixed_quality */

                                thresh = (lo + hi) / 2;

                                mi_tcd_makelayer(tcd, layno, thresh, 0);

                                if (cp->m_specific_param.m_enc.m_fixed_quality) {       /* fixed_quality */
                                        if(mi_IS_CINEMA(cp->rsiz)){
                                                if (! mi_t2_encode_packets(t2,tcd->tcd_tileno, tcd_tile, layno + 1, dest, p_data_written, maxlen, cstr_info,tcd->cur_tp_num,tcd->tp_pos,tcd->cur_pino,THRESH_CALC)) {

                                                        lo = thresh;
                                                        continue;
                                                }
                                                else {
                                                        distoachieved = layno == 0 ?
                                                                        tcd_tile->distolayer[0] : cumdisto[layno - 1] + tcd_tile->distolayer[layno];

                                                        if (distoachieved < distotarget) {
                                                                hi=thresh;
                                                                stable_thresh = thresh;
                                                                continue;
                                                        }else{
                                                                lo=thresh;
                                                        }
                                                }
                                        }else{
                                                distoachieved = (layno == 0) ?
                                                                tcd_tile->distolayer[0] : (cumdisto[layno - 1] + tcd_tile->distolayer[layno]);

                                                if (distoachieved < distotarget) {
                                                        hi = thresh;
                                                        stable_thresh = thresh;
                                                        continue;
                                                }
                                                lo = thresh;
                                        }
                                } else {
                                        if (! mi_t2_encode_packets(t2, tcd->tcd_tileno, tcd_tile, layno + 1, dest,p_data_written, maxlen, cstr_info,tcd->cur_tp_num,tcd->tp_pos,tcd->cur_pino,THRESH_CALC))
                                        {
                                                /* TODO: what to do with l ??? seek / tell ??? */
                                                /* mi_event_msg(tcd->cinfo, EVT_INFO, "rate alloc: len=%d, max=%d\n", l, maxlen); */
                                                lo = thresh;
                                                continue;
                                        }

                                        hi = thresh;
                                        stable_thresh = thresh;
                                }
                        }

                        goodthresh = stable_thresh == 0? thresh : stable_thresh;

                        mi_t2_destroy(t2);
                } else {
                        goodthresh = min;
                }

                if(cstr_info) { /* Threshold for Marcela Index */
                        cstr_info->tile[tcd->tcd_tileno].thresh[layno] = goodthresh;
                }

                mi_tcd_makelayer(tcd, layno, goodthresh, 1);

                /* fixed_quality */
                cumdisto[layno] = (layno == 0) ? tcd_tile->distolayer[0] : (cumdisto[layno - 1] + tcd_tile->distolayer[layno]);
        }

        return mi_TRUE;
}

mi_BOOL mi_tcd_init( mi_tcd_t *p_tcd,
                                           mi_image_t * p_image,
                                           mi_cp_t * p_cp )
{
        p_tcd->image = p_image;
        p_tcd->cp = p_cp;

        p_tcd->tcd_image->tiles = (mi_tcd_tile_t *) mi_calloc(1,sizeof(mi_tcd_tile_t));
        if (! p_tcd->tcd_image->tiles) {
                return mi_FALSE;
        }

        p_tcd->tcd_image->tiles->comps = (mi_tcd_tilecomp_t *) mi_calloc(p_image->numcomps,sizeof(mi_tcd_tilecomp_t));
        if (! p_tcd->tcd_image->tiles->comps ) {
                return mi_FALSE;
        }

        p_tcd->tcd_image->tiles->numcomps = p_image->numcomps;
        p_tcd->tp_pos = p_cp->m_specific_param.m_enc.m_tp_pos;

        return mi_TRUE;
}

/**
Destroy a previously created TCD handle
*/
void mi_tcd_destroy(mi_tcd_t *tcd) {
        if (tcd) {
                mi_tcd_free_tile(tcd);

                if (tcd->tcd_image) {
                        mi_free(tcd->tcd_image);
                        tcd->tcd_image = 00;
                }
                mi_free(tcd);
        }
}

mi_BOOL mi_alloc_tile_component_data(mi_tcd_tilecomp_t *l_tilec)
{
	if ((l_tilec->data == 00) || ((l_tilec->data_size_needed > l_tilec->data_size) && (l_tilec->ownsData == mi_FALSE))) {
		l_tilec->data = (mi_INT32 *) mi_aligned_malloc(l_tilec->data_size_needed);
		if (! l_tilec->data ) {
			return mi_FALSE;
		}
		/*fprintf(stderr, "tAllocate data of tilec (int): %d x mi_UINT32n",l_data_size);*/
		l_tilec->data_size = l_tilec->data_size_needed;
		l_tilec->ownsData = mi_TRUE;
	}
	else if (l_tilec->data_size_needed > l_tilec->data_size) {
		/* We don't need to keep old data */
		mi_aligned_free(l_tilec->data);
		l_tilec->data = (mi_INT32 *) mi_aligned_malloc(l_tilec->data_size_needed);
		if (! l_tilec->data ) {
			l_tilec->data_size = 0;
			l_tilec->data_size_needed = 0;
			l_tilec->ownsData = mi_FALSE;
			return mi_FALSE;
		}
		/*fprintf(stderr, "tReallocate data of tilec (int): from %d to %d x mi_UINT32n", l_tilec->data_size, l_data_size);*/
		l_tilec->data_size = l_tilec->data_size_needed;
		l_tilec->ownsData = mi_TRUE;
	}
	return mi_TRUE;
}

/* ----------------------------------------------------------------------- */
//切片编码第一函数（区、代码块、切片的具体坐标处理和具体量化处理）
static INLINE mi_BOOL mi_tcd_init_tile(mi_tcd_t *p_tcd, mi_UINT32 p_tile_no, mi_BOOL isEncoder, mi_FLOAT32 fraction, mi_SIZE_T sizeof_block, mi_event_mgr_t* manager)
{
	mi_UINT32 (*l_gain_ptr)(mi_UINT32) = 00;//应该是子带增益计算函数的指针
	mi_UINT32 compno, resno, bandno,cblkno;
	mi_UINT32 precno;//一个分辨率下的区数量
	mi_tcp_t * l_tcp = 00;//tile coding parameters
	mi_cp_t * l_cp = 00;//coding parameters
	mi_tcd_tile_t * l_tile = 00;//tiles infomation
	mi_tccp_t *l_tccp = 00;//tile-component coding infomation
	mi_tcd_tilecomp_t *l_tilec = 00;//component infomation
	mi_image_comp_t * l_image_comp = 00;//image component
	mi_tcd_resolution_t *l_res = 00;//component resolution infomations
	mi_tcd_band_t *l_band = 00;//所有子带的信息集合
	mi_stepsize_t * l_step_size = 00;//quantilizations step
	mi_tcd_precinct_t *l_current_precinct = 00;
	mi_image_t *l_image = 00;//image
	mi_UINT32 p;/* tile coordinates-integer（我这是切片的横向位置（单位：切片数）） */
	mi_UINT32 q;/* ？ */
	mi_UINT32 l_level_no;//分辨率级数
	mi_UINT32 l_pdx, l_pdy;//区的采样周期（这个采样周期单位貌似是区宽高的位数）
	mi_UINT32 l_gain;
	mi_INT32 l_x0b, l_y0b;
	mi_UINT32 l_tx0, l_ty0;//当前切片在参考网格的坐标
	/* extent of precincts , top left, bottom right（一个分辨率下区集合）**/
	mi_INT32 l_tl_prc_x_start, l_tl_prc_y_start, l_br_prc_x_end, l_br_prc_y_end;
	/* number of precinct for a resolution */
	mi_UINT32 l_nb_precincts;
	/* room needed to store l_nb_precinct precinct for a resolution */
	mi_UINT32 l_nb_precinct_size;
	/* number of code blocks for a precinct*/
	mi_UINT32 l_nb_code_blocks;
	/* room needed to store l_nb_code_blocks code blocks for a precinct*/
	mi_UINT32 l_nb_code_blocks_size;
	/* size of data for a tile （横向*纵向）*/
	mi_UINT32 l_data_size;
	
	l_cp = p_tcd->cp;
	l_tcp = &(l_cp->tcps[p_tile_no]);
	l_tile = p_tcd->tcd_image->tiles;
	l_tccp = l_tcp->tccps;
	l_tilec = l_tile->comps;
	l_image = p_tcd->image;
	l_image_comp = p_tcd->image->comps;
	
	p = p_tile_no % l_cp->tw;       /* tile coordinates */
	q = p_tile_no / l_cp->tw;
	/*fprintf(stderr, "Tile coordinate = %d,%d\n", p, q);*/
	
	/* 4 borders of the tile rescale on the image if necessary */
	l_tx0 = l_cp->tx0 + p * l_cp->tdx; /* can't be greater than l_image->x1 so won't overflow */
	l_tile->x0 = (mi_INT32)mi_uint_max(l_tx0, l_image->x0);//若是图片边缘的切片则原点向图片原点对准
	l_tile->x1 = (mi_INT32)mi_uint_min(mi_uint_adds(l_tx0, l_cp->tdx), l_image->x1);//切片尾点向图片尾点对准
	l_ty0 = l_cp->ty0 + q * l_cp->tdy; /* can't be greater than l_image->y1 so won't overflow */
	l_tile->y0 = (mi_INT32)mi_uint_max(l_ty0, l_image->y0);//切片原点向图片原点对准
	l_tile->y1 = (mi_INT32)mi_uint_min(mi_uint_adds(l_ty0, l_cp->tdy), l_image->y1);//切片尾点向图片尾点对准

	/* testcase 1888.pdf.asan.35.988 */
	if (l_tccp->numresolutions == 0) {
		mi_event_msg(manager, EVT_ERROR, "tiles require at least one resolution\n");
		return mi_FALSE;
	}
	/*fprintf(stderr, "Tile border = %d,%d,%d,%d\n", l_tile->x0, l_tile->y0,l_tile->x1,l_tile->y1);*/
	
	/*tile->numcomps = image->numcomps; */
	for (compno = 0; compno < l_tile->numcomps; ++compno) {
		/*fprintf(stderr, "compno = %d/%d\n", compno, l_tile->numcomps);*/
		l_image_comp->resno_decoded = 0;
		/* border of each l_tile component (global) */
		l_tilec->x0 = mi_int_ceildiv(l_tile->x0, (mi_INT32)l_image_comp->dx);
		l_tilec->y0 = mi_int_ceildiv(l_tile->y0, (mi_INT32)l_image_comp->dy);
		l_tilec->x1 = mi_int_ceildiv(l_tile->x1, (mi_INT32)l_image_comp->dx);
		l_tilec->y1 = mi_int_ceildiv(l_tile->y1, (mi_INT32)l_image_comp->dy);
		/*fprintf(stderr, "\tTile compo border = %d,%d,%d,%d\n", l_tilec->x0, l_tilec->y0,l_tilec->x1,l_tilec->y1);*/
#pragma	region set component resolution infomation
		/* compute l_data_size with overflow check */
		l_data_size = (mi_UINT32)(l_tilec->x1 - l_tilec->x0);
		/* issue 733, l_data_size == 0U, probably something wrong should be checked before getting here */
		if ((l_data_size > 0U) && ((((mi_UINT32)-1) / l_data_size) < (mi_UINT32)(l_tilec->y1 - l_tilec->y0))) {
			mi_event_msg(manager, EVT_ERROR, "Not enough memory for tile data\n");
			return mi_FALSE;
		}
		l_data_size = l_data_size * (mi_UINT32)(l_tilec->y1 - l_tilec->y0);
		
		if ((((mi_UINT32)-1) / (mi_UINT32)sizeof(mi_UINT32)) < l_data_size) {
			mi_event_msg(manager, EVT_ERROR, "Not enough memory for tile data\n");
			return mi_FALSE;
		}
		l_data_size = l_data_size * (mi_UINT32)sizeof(mi_UINT32);
		l_tilec->numresolutions = l_tccp->numresolutions;
		if (l_tccp->numresolutions < l_cp->m_specific_param.m_dec.m_reduce) {
			l_tilec->minimum_num_resolutions = 1;
		}
		else {
			l_tilec->minimum_num_resolutions = l_tccp->numresolutions - l_cp->m_specific_param.m_dec.m_reduce;
		}
		
		l_tilec->data_size_needed = l_data_size;
		if (p_tcd->m_is_decoder && !mi_alloc_tile_component_data(l_tilec)) {
			mi_event_msg(manager, EVT_ERROR, "Not enough memory for tile data\n");
			return mi_FALSE;
		}
		
		l_data_size = l_tilec->numresolutions * (mi_UINT32)sizeof(mi_tcd_resolution_t);
		
		if (l_tilec->resolutions == 00) {
			l_tilec->resolutions = (mi_tcd_resolution_t *) mi_malloc(l_data_size);
			if (! l_tilec->resolutions ) {
				return mi_FALSE;
			}
			/*fprintf(stderr, "\tAllocate resolutions of tilec (mi_tcd_resolution_t): %d\n",l_data_size);*/
			l_tilec->resolutions_size = l_data_size;
			memset(l_tilec->resolutions,0,l_data_size);
		}
		else if (l_data_size > l_tilec->resolutions_size) {
			mi_tcd_resolution_t* new_resolutions = (mi_tcd_resolution_t *) mi_realloc(l_tilec->resolutions, l_data_size);
			if (! new_resolutions) {
				mi_event_msg(manager, EVT_ERROR, "Not enough memory for tile resolutions\n");
				mi_free(l_tilec->resolutions);
				l_tilec->resolutions = NULL;
				l_tilec->resolutions_size = 0;
				return mi_FALSE;
			}
			l_tilec->resolutions = new_resolutions;
			/*fprintf(stderr, "\tReallocate data of tilec (int): from %d to %d x mi_UINT32\n", l_tilec->resolutions_size, l_data_size);*/
			memset(((mi_BYTE*) l_tilec->resolutions)+l_tilec->resolutions_size,0,l_data_size - l_tilec->resolutions_size);
			l_tilec->resolutions_size = l_data_size;
		}
#pragma endregion

		l_level_no = l_tilec->numresolutions;
		l_res = l_tilec->resolutions;
		l_step_size = l_tccp->stepsizes;
		if (l_tccp->qmfbid == 0) {
			l_gain_ptr = &mi_dwt_getgain_real;
		}
		else {
			l_gain_ptr  = &mi_dwt_getgain;
		}
		/*fprintf(stderr, "\tlevel_no=%d\n",l_level_no);*/
		
		for (resno = 0; resno < l_tilec->numresolutions; ++resno) {
			/*fprintf(stderr, "\t\tresno = %d/%d\n", resno, l_tilec->numresolutions);*/

			mi_INT32 tlcbgxstart, tlcbgystart/*, brcbgxend, brcbgyend*/ ;//我猜是切片首个代码块原点
		   /*这个是区尺寸位数（与分辨率区尺寸的位数相等）
		   （一般若用户未指定，则分辨率增一级它就宽高缩一半）
		   （这个会跟l_tccp->cblkw/h相比较，那个小选哪个作为代码块的尺寸）*/
			mi_UINT32 cbgwidthexpn, cbgheightexpn;
			mi_UINT32 cblkwidthexpn, cblkheightexpn;//这个是最终决定下来的代码块尺寸的位数
			
			--l_level_no;
			
			/* border for each resolution level (global) （设置所有分辨率图像的坐标值） */
			l_res->x0 = mi_int_ceildivpow2(l_tilec->x0, (mi_INT32)l_level_no);
			l_res->y0 = mi_int_ceildivpow2(l_tilec->y0, (mi_INT32)l_level_no);
			l_res->x1 = mi_int_ceildivpow2(l_tilec->x1, (mi_INT32)l_level_no);
			l_res->y1 = mi_int_ceildivpow2(l_tilec->y1, (mi_INT32)l_level_no);
			/*fprintf(stderr, "\t\t\tres_x0= %d, res_y0 =%d, res_x1=%d, res_y1=%d\n", l_res->x0, l_res->y0, l_res->x1, l_res->y1);*/
			/* p. 35, table A-23, ISO/IEC FDIS154444-1 : 2000 (18 august 2000)（以分辨率设置每个相应的区的采样周期） */
			l_pdx = l_tccp->prcw[resno];
			l_pdy = l_tccp->prch[resno];
			/*fprintf(stderr, "\t\t\tpdx=%d, pdy=%d\n", l_pdx, l_pdy);*/
			/* p. 64, B.6, ISO/IEC FDIS15444-1 : 2000 (18 august 2000) （表明区的广度） */
			l_tl_prc_x_start = mi_int_floordivpow2(l_res->x0, (mi_INT32)l_pdx) << l_pdx;
			l_tl_prc_y_start = mi_int_floordivpow2(l_res->y0, (mi_INT32)l_pdy) << l_pdy;
			l_br_prc_x_end = mi_int_ceildivpow2(l_res->x1, (mi_INT32)l_pdx) << l_pdx;
			l_br_prc_y_end = mi_int_ceildivpow2(l_res->y1, (mi_INT32)l_pdy) << l_pdy;
			/*fprintf(stderr, "\t\t\tprc_x_start=%d, prc_y_start=%d, br_prc_x_end=%d, br_prc_y_end=%d \n", l_tl_prc_x_start, l_tl_prc_y_start, l_br_prc_x_end ,l_br_prc_y_end );*/
			
			l_res->pw = (l_res->x0 == l_res->x1) ? 0 : (mi_UINT32)((l_br_prc_x_end - l_tl_prc_x_start) >> l_pdx);
			l_res->ph = (l_res->y0 == l_res->y1) ? 0 : (mi_UINT32)((l_br_prc_y_end - l_tl_prc_y_start) >> l_pdy);
			/*fprintf(stderr, "\t\t\tres_pw=%d, res_ph=%d\n", l_res->pw, l_res->ph );*/
			
			l_nb_precincts = l_res->pw * l_res->ph;
			l_nb_precinct_size = l_nb_precincts * (mi_UINT32)sizeof(mi_tcd_precinct_t);
			if (resno == 0) {//已到达图像分解底层，就一个LL子带了
				tlcbgxstart = l_tl_prc_x_start;
				tlcbgystart = l_tl_prc_y_start;
				/*brcbgxend = l_br_prc_x_end;*/
				/* brcbgyend = l_br_prc_y_end;*/
				cbgwidthexpn = l_pdx;
				cbgheightexpn = l_pdy;
				l_res->numbands = 1;//一个子带
			}
			else {
				tlcbgxstart = mi_int_ceildivpow2(l_tl_prc_x_start, 1);
				tlcbgystart = mi_int_ceildivpow2(l_tl_prc_y_start, 1);
				/*brcbgxend = mi_int_ceildivpow2(l_br_prc_x_end, 1);*/
				/*brcbgyend = mi_int_ceildivpow2(l_br_prc_y_end, 1);*/
				cbgwidthexpn = l_pdx - 1;
				cbgheightexpn = l_pdy - 1;
				l_res->numbands = 3;
			}
			
			cblkwidthexpn = mi_uint_min(l_tccp->cblkw, cbgwidthexpn);
			cblkheightexpn = mi_uint_min(l_tccp->cblkh, cbgheightexpn);
			l_band = l_res->bands;
#pragma region set subband infomation			
			for (bandno = 0; bandno < l_res->numbands; ++bandno) {
				mi_INT32 numbps;
				/*fprintf(stderr, "\t\t\tband_no=%d/%d\n", bandno, l_res->numbands );*/
				//子带广度设置
				if (resno == 0) {//此时分辨率级数最高，已到图像分解最底层
					l_band->bandno = 0 ;
					l_band->x0 = mi_int_ceildivpow2(l_tilec->x0, (mi_INT32)l_level_no);
					l_band->y0 = mi_int_ceildivpow2(l_tilec->y0, (mi_INT32)l_level_no);
					l_band->x1 = mi_int_ceildivpow2(l_tilec->x1, (mi_INT32)l_level_no);
					l_band->y1 = mi_int_ceildivpow2(l_tilec->y1, (mi_INT32)l_level_no);
				}
				else {
					l_band->bandno = bandno + 1;
					/* x0b = 1 if bandno = 1 or 3 */
					l_x0b = l_band->bandno&1;
					/* y0b = 1 if bandno = 2 or 3 */
					l_y0b = (mi_INT32)((l_band->bandno)>>1);
					/* l_band border (global) （感觉这个没有用）*/
					l_band->x0 = mi_int64_ceildivpow2(l_tilec->x0 - ((mi_INT64)l_x0b << l_level_no), (mi_INT32)(l_level_no + 1));
					l_band->y0 = mi_int64_ceildivpow2(l_tilec->y0 - ((mi_INT64)l_y0b << l_level_no), (mi_INT32)(l_level_no + 1));
					l_band->x1 = mi_int64_ceildivpow2(l_tilec->x1 - ((mi_INT64)l_x0b << l_level_no), (mi_INT32)(l_level_no + 1));
					l_band->y1 = mi_int64_ceildivpow2(l_tilec->y1 - ((mi_INT64)l_y0b << l_level_no), (mi_INT32)(l_level_no + 1));
				}
				
				/** avoid an if with storing function pointer 子带增益设置 */
				l_gain = (*l_gain_ptr) (l_band->bandno);
				numbps = (mi_INT32)(l_image_comp->prec + l_gain);//课本5.6-3
				l_band->stepsize = (mi_FLOAT32)(((1.0 + l_step_size->mant / 2048.0) * pow(2.0, (mi_INT32) (numbps - l_step_size->expn)))) * fraction;//课本5.6-2
				l_band->numbps = l_step_size->expn + (mi_INT32)l_tccp->numgbits - 1;      /* WHY -1 ? *///课本5.6-6
				
				if (!l_band->precincts && (l_nb_precincts > 0U)) {
					l_band->precincts = (mi_tcd_precinct_t *) mi_malloc( /*3 * */ l_nb_precinct_size);
					if (! l_band->precincts) {
						return mi_FALSE;
					}
					/*fprintf(stderr, "\t\t\t\tAllocate precincts of a band (mi_tcd_precinct_t): %d\n",l_nb_precinct_size);     */
					memset(l_band->precincts,0,l_nb_precinct_size);
					l_band->precincts_data_size = l_nb_precinct_size;
				}
				else if (l_band->precincts_data_size < l_nb_precinct_size) {
					
					mi_tcd_precinct_t * new_precincts = (mi_tcd_precinct_t *) mi_realloc(l_band->precincts,/*3 * */ l_nb_precinct_size);
					if (! new_precincts) {
						mi_event_msg(manager, EVT_ERROR, "Not enough memory to handle band precints\n");
						mi_free(l_band->precincts);
						l_band->precincts = NULL;
						l_band->precincts_data_size = 0;
						return mi_FALSE;
					}
					l_band->precincts = new_precincts;
					/*fprintf(stderr, "\t\t\t\tReallocate precincts of a band (mi_tcd_precinct_t): from %d to %d\n",l_band->precincts_data_size, l_nb_precinct_size);*/
					memset(((mi_BYTE *) l_band->precincts) + l_band->precincts_data_size,0,l_nb_precinct_size - l_band->precincts_data_size);
					l_band->precincts_data_size = l_nb_precinct_size;
				}
#pragma endregion				

#pragma region set precincts in every resolution
				l_current_precinct = l_band->precincts;
				for (precno = 0; precno < l_nb_precincts; ++precno) {
					mi_INT32 tlcblkxstart, tlcblkystart, brcblkxend, brcblkyend;
					mi_INT32 cbgxstart = tlcbgxstart + (mi_INT32)(precno % l_res->pw) * (1 << cbgwidthexpn);//某个区的起始
					mi_INT32 cbgystart = tlcbgystart + (mi_INT32)(precno / l_res->pw) * (1 << cbgheightexpn);
					mi_INT32 cbgxend = cbgxstart + (1 << cbgwidthexpn);
					mi_INT32 cbgyend = cbgystart + (1 << cbgheightexpn);
					/*fprintf(stderr, "\t precno=%d; bandno=%d, resno=%d; compno=%d\n", precno, bandno , resno, compno);*/
					/*fprintf(stderr, "\t tlcbgxstart(=%d) + (precno(=%d) percent res->pw(=%d)) * (1 << cbgwidthexpn(=%d)) \n",tlcbgxstart,precno,l_res->pw,cbgwidthexpn);*/
					
					/* precinct size (global) */
					/*fprintf(stderr, "\t cbgxstart=%d, l_band->x0 = %d \n",cbgxstart, l_band->x0);*/
					
					l_current_precinct->x0 = mi_int_max(cbgxstart, l_band->x0);
					l_current_precinct->y0 = mi_int_max(cbgystart, l_band->y0);
					l_current_precinct->x1 = mi_int_min(cbgxend, l_band->x1);
					l_current_precinct->y1 = mi_int_min(cbgyend, l_band->y1);
					/*fprintf(stderr, "\t prc_x0=%d; prc_y0=%d, prc_x1=%d; prc_y1=%d\n",l_current_precinct->x0, l_current_precinct->y0 ,l_current_precinct->x1, l_current_precinct->y1);*/
					
					tlcblkxstart = mi_int_floordivpow2(l_current_precinct->x0, (mi_INT32)cblkwidthexpn) << cblkwidthexpn;
					/*fprintf(stderr, "\t tlcblkxstart =%d\n",tlcblkxstart );*/
					tlcblkystart = mi_int_floordivpow2(l_current_precinct->y0, (mi_INT32)cblkheightexpn) << cblkheightexpn;
					/*fprintf(stderr, "\t tlcblkystart =%d\n",tlcblkystart );*/
					brcblkxend = mi_int_ceildivpow2(l_current_precinct->x1, (mi_INT32)cblkwidthexpn) << cblkwidthexpn;
					/*fprintf(stderr, "\t brcblkxend =%d\n",brcblkxend );*/
					brcblkyend = mi_int_ceildivpow2(l_current_precinct->y1, (mi_INT32)cblkheightexpn) << cblkheightexpn;
					/*fprintf(stderr, "\t brcblkyend =%d\n",brcblkyend );*/
					l_current_precinct->cw = (mi_UINT32)((brcblkxend - tlcblkxstart) >> cblkwidthexpn);
					l_current_precinct->ch = (mi_UINT32)((brcblkyend - tlcblkystart) >> cblkheightexpn);
					
					l_nb_code_blocks = l_current_precinct->cw * l_current_precinct->ch;
					/*fprintf(stderr, "\t\t\t\t precinct_cw = %d x recinct_ch = %d\n",l_current_precinct->cw, l_current_precinct->ch);      */
					l_nb_code_blocks_size = l_nb_code_blocks * (mi_UINT32)sizeof_block;
					
					if (!l_current_precinct->cblks.blocks && (l_nb_code_blocks > 0U)) {
						l_current_precinct->cblks.blocks = mi_malloc(l_nb_code_blocks_size);
						if (! l_current_precinct->cblks.blocks ) {
							return mi_FALSE;
						}
						/*fprintf(stderr, "\t\t\t\tAllocate cblks of a precinct (mi_tcd_cblk_dec_t): %d\n",l_nb_code_blocks_size);*/
						
						memset(l_current_precinct->cblks.blocks,0,l_nb_code_blocks_size);
						
						l_current_precinct->block_size = l_nb_code_blocks_size;
					}
					else if (l_nb_code_blocks_size > l_current_precinct->block_size) {
						void *new_blocks = mi_realloc(l_current_precinct->cblks.blocks, l_nb_code_blocks_size);
						if (! new_blocks) {
							mi_free(l_current_precinct->cblks.blocks);
							l_current_precinct->cblks.blocks = NULL;
							l_current_precinct->block_size = 0;
							mi_event_msg(manager, EVT_ERROR, "Not enough memory for current precinct codeblock element\n");
							return mi_FALSE;
						}
						l_current_precinct->cblks.blocks = new_blocks;
						/*fprintf(stderr, "\t\t\t\tReallocate cblks of a precinct (mi_tcd_cblk_dec_t): from %d to %d\n",l_current_precinct->block_size, l_nb_code_blocks_size);     */
						
						memset(((mi_BYTE *) l_current_precinct->cblks.blocks) + l_current_precinct->block_size
									 ,0
									 ,l_nb_code_blocks_size - l_current_precinct->block_size);
						
						l_current_precinct->block_size = l_nb_code_blocks_size;
					}
#pragma endregion

#pragma region set tag-tree
					if (! l_current_precinct->incltree) {
						l_current_precinct->incltree = mi_tgt_create(l_current_precinct->cw, l_current_precinct->ch, manager);
					}
					else{
						l_current_precinct->incltree = mi_tgt_init(l_current_precinct->incltree, l_current_precinct->cw, l_current_precinct->ch, manager);
					}

					if (! l_current_precinct->incltree)     {
						mi_event_msg(manager, EVT_WARNING, "No incltree created.\n");
						/*return mi_FALSE;*/
					}

					if (! l_current_precinct->imsbtree) {
						l_current_precinct->imsbtree = mi_tgt_create(l_current_precinct->cw, l_current_precinct->ch, manager);
					}
					else {
						l_current_precinct->imsbtree = mi_tgt_init(l_current_precinct->imsbtree, l_current_precinct->cw, l_current_precinct->ch, manager);
					}

					if (! l_current_precinct->imsbtree) {
						mi_event_msg(manager, EVT_WARNING, "No imsbtree created.\n");
						/*return mi_FALSE;*/
					}

					for (cblkno = 0; cblkno < l_nb_code_blocks; ++cblkno) {
						mi_INT32 cblkxstart = tlcblkxstart + (mi_INT32)(cblkno % l_current_precinct->cw) * (1 << cblkwidthexpn);
						mi_INT32 cblkystart = tlcblkystart + (mi_INT32)(cblkno / l_current_precinct->cw) * (1 << cblkheightexpn);
						mi_INT32 cblkxend = cblkxstart + (1 << cblkwidthexpn);
						mi_INT32 cblkyend = cblkystart + (1 << cblkheightexpn);
						
						if (isEncoder) {
							mi_tcd_cblk_enc_t* l_code_block = l_current_precinct->cblks.enc + cblkno;
							
							if (! mi_tcd_code_block_enc_allocate(l_code_block)) {
								return mi_FALSE;
							}
							/* code-block size (global) */
							l_code_block->x0 = mi_int_max(cblkxstart, l_current_precinct->x0);
							l_code_block->y0 = mi_int_max(cblkystart, l_current_precinct->y0);
							l_code_block->x1 = mi_int_min(cblkxend, l_current_precinct->x1);
							l_code_block->y1 = mi_int_min(cblkyend, l_current_precinct->y1);
							
							if (! mi_tcd_code_block_enc_allocate_data(l_code_block)) {
								return mi_FALSE;
							}
						} else {
							mi_tcd_cblk_dec_t* l_code_block = l_current_precinct->cblks.dec + cblkno;
							
							if (! mi_tcd_code_block_dec_allocate(l_code_block)) {
								return mi_FALSE;
							}
							/* code-block size (global) */
							l_code_block->x0 = mi_int_max(cblkxstart, l_current_precinct->x0);
							l_code_block->y0 = mi_int_max(cblkystart, l_current_precinct->y0);
							l_code_block->x1 = mi_int_min(cblkxend, l_current_precinct->x1);
							l_code_block->y1 = mi_int_min(cblkyend, l_current_precinct->y1);
						}
					}
					++l_current_precinct;
				} /* precno */
				++l_band;
				++l_step_size;
			} /* bandno */
#pragma endregion
			++l_res;
		} /* resno */
		++l_tccp;
		++l_tilec;
		++l_image_comp;
	} /* compno */
	return mi_TRUE;
}

mi_BOOL mi_tcd_init_encode_tile (mi_tcd_t *p_tcd, mi_UINT32 p_tile_no, mi_event_mgr_t* p_manager)
{
	return mi_tcd_init_tile(p_tcd, p_tile_no, mi_TRUE, 1.0F, sizeof(mi_tcd_cblk_enc_t), p_manager);
}

mi_BOOL mi_tcd_init_decode_tile (mi_tcd_t *p_tcd, mi_UINT32 p_tile_no, mi_event_mgr_t* p_manager)
{
	return mi_tcd_init_tile(p_tcd, p_tile_no, mi_FALSE, 0.5F, sizeof(mi_tcd_cblk_dec_t), p_manager);
}

/**
 * Allocates memory for an encoding code block (but not data memory).
 */
static mi_BOOL mi_tcd_code_block_enc_allocate (mi_tcd_cblk_enc_t * p_code_block)
{
	if (! p_code_block->layers) {
		/* no memset since data */
		p_code_block->layers = (mi_tcd_layer_t*) mi_calloc(100, sizeof(mi_tcd_layer_t));
		if (! p_code_block->layers) {
			return mi_FALSE;
		}
	}
	if (! p_code_block->passes) {
		p_code_block->passes = (mi_tcd_pass_t*) mi_calloc(100, sizeof(mi_tcd_pass_t));
		if (! p_code_block->passes) {
			return mi_FALSE;
		}
	}
	return mi_TRUE;
}

/**
 * Allocates data memory for an encoding code block.
 */
static mi_BOOL mi_tcd_code_block_enc_allocate_data (mi_tcd_cblk_enc_t * p_code_block)
{
	mi_UINT32 l_data_size;
	
	l_data_size = (mi_UINT32)((p_code_block->x1 - p_code_block->x0) * (p_code_block->y1 - p_code_block->y0) * (mi_INT32)sizeof(mi_UINT32));
	
	if (l_data_size > p_code_block->data_size) {
		if (p_code_block->data) {
			mi_free(p_code_block->data - 1); /* again, why -1 */
		}
		p_code_block->data = (mi_BYTE*) mi_malloc(l_data_size+1);
		if(! p_code_block->data) {
			p_code_block->data_size = 0U;
			return mi_FALSE;
		}
		p_code_block->data_size = l_data_size;
		
		p_code_block->data[0] = 0;
		p_code_block->data+=1;   /*why +1 ?*/
	}
	return mi_TRUE;
}

/**
 * Allocates memory for a decoding code block.
 */
static mi_BOOL mi_tcd_code_block_dec_allocate (mi_tcd_cblk_dec_t * p_code_block)
{
        if (! p_code_block->data) {

                p_code_block->data = (mi_BYTE*) mi_malloc(mi_J2K_DEFAULT_CBLK_DATA_SIZE);
                if (! p_code_block->data) {
                        return mi_FALSE;
                }
                p_code_block->data_max_size = mi_J2K_DEFAULT_CBLK_DATA_SIZE;
                /*fprintf(stderr, "Allocate 8192 elements of code_block->data\n");*/

                p_code_block->segs = (mi_tcd_seg_t *) mi_calloc(mi_J2K_DEFAULT_NB_SEGS,sizeof(mi_tcd_seg_t));
                if (! p_code_block->segs) {
                        return mi_FALSE;
                }
                /*fprintf(stderr, "Allocate %d elements of code_block->data\n", mi_J2K_DEFAULT_NB_SEGS * sizeof(mi_tcd_seg_t));*/

                p_code_block->m_current_max_segs = mi_J2K_DEFAULT_NB_SEGS;
                /*fprintf(stderr, "m_current_max_segs of code_block->data = %d\n", p_code_block->m_current_max_segs);*/
        } else {
					/* sanitize */
					mi_BYTE* l_data = p_code_block->data;
					mi_UINT32 l_data_max_size = p_code_block->data_max_size;
					mi_tcd_seg_t * l_segs = p_code_block->segs;
					mi_UINT32 l_current_max_segs = p_code_block->m_current_max_segs;

					memset(p_code_block, 0, sizeof(mi_tcd_cblk_dec_t));
					p_code_block->data = l_data;
					p_code_block->data_max_size = l_data_max_size;
					p_code_block->segs = l_segs;
					p_code_block->m_current_max_segs = l_current_max_segs;
				}

        return mi_TRUE;
}

mi_UINT32 mi_tcd_get_decoded_tile_size ( mi_tcd_t *p_tcd )
{
        mi_UINT32 i;
        mi_UINT32 l_data_size = 0;
        mi_image_comp_t * l_img_comp = 00;
        mi_tcd_tilecomp_t * l_tile_comp = 00;
        mi_tcd_resolution_t * l_res = 00;
        mi_UINT32 l_size_comp, l_remaining;

        l_tile_comp = p_tcd->tcd_image->tiles->comps;
        l_img_comp = p_tcd->image->comps;

        for (i=0;i<p_tcd->image->numcomps;++i) {
                l_size_comp = l_img_comp->prec >> 3; /*(/ 8)*/
                l_remaining = l_img_comp->prec & 7;  /* (%8) */

                if(l_remaining) {
                        ++l_size_comp;
                }

                if (l_size_comp == 3) {
                        l_size_comp = 4;
                }

                l_res = l_tile_comp->resolutions + l_tile_comp->minimum_num_resolutions - 1;
                l_data_size += l_size_comp * (mi_UINT32)((l_res->x1 - l_res->x0) * (l_res->y1 - l_res->y0));
                ++l_img_comp;
                ++l_tile_comp;
        }

        return l_data_size;
}
//JPEG2000编码总函数
mi_BOOL mi_tcd_encode_tile(   mi_tcd_t *p_tcd,
                                                        mi_UINT32 p_tile_no,
                                                        mi_BYTE *p_dest,
                                                        mi_UINT32 * p_data_written,
                                                        mi_UINT32 p_max_length,
                                                        mi_codestream_info_t *p_cstr_info)
{

        if (p_tcd->cur_tp_num == 0) {

                p_tcd->tcd_tileno = p_tile_no;
                p_tcd->tcp = &p_tcd->cp->tcps[p_tile_no];

                /* INDEX >> "Precinct_nb_X et Precinct_nb_Y" */
                if(p_cstr_info)  {
                        mi_UINT32 l_num_packs = 0;
                        mi_UINT32 i;
                        mi_tcd_tilecomp_t *l_tilec_idx = &p_tcd->tcd_image->tiles->comps[0];        /* based on component 0 */
                        mi_tccp_t *l_tccp = p_tcd->tcp->tccps; /* based on component 0 */

                        for (i = 0; i < l_tilec_idx->numresolutions; i++) {
                                mi_tcd_resolution_t *l_res_idx = &l_tilec_idx->resolutions[i];

                                p_cstr_info->tile[p_tile_no].pw[i] = (int)l_res_idx->pw;
                                p_cstr_info->tile[p_tile_no].ph[i] = (int)l_res_idx->ph;

                                l_num_packs += l_res_idx->pw * l_res_idx->ph;
                                p_cstr_info->tile[p_tile_no].pdx[i] = (int)l_tccp->prcw[i];
                                p_cstr_info->tile[p_tile_no].pdy[i] = (int)l_tccp->prch[i];
                        }
                        p_cstr_info->tile[p_tile_no].packet = (mi_packet_info_t*) mi_calloc((size_t)p_cstr_info->numcomps * (size_t)p_cstr_info->numlayers * l_num_packs, sizeof(mi_packet_info_t));
                        if (!p_cstr_info->tile[p_tile_no].packet) {
                                /* FIXME event manager error callback */
                                return mi_FALSE;
                        }
                }
                /* << INDEX */

                /* FIXME _ProfStart(PGROUP_DC_SHIFT); */
                /*---------------TILE-------------------*/
                if (! mi_tcd_dc_level_shift_encode(p_tcd)) {
                        return mi_FALSE;
                }
                /* FIXME _ProfStop(PGROUP_DC_SHIFT); */

                /* FIXME _ProfStart(PGROUP_MCT); */
                if (! mi_tcd_mct_encode(p_tcd)) {
                        return mi_FALSE;
                }
                /* FIXME _ProfStop(PGROUP_MCT); */

                /* FIXME _ProfStart(PGROUP_DWT); */
                if (! mi_tcd_dwt_encode(p_tcd)) {
                        return mi_FALSE;
                }
                /* FIXME  _ProfStop(PGROUP_DWT); */

                /* FIXME  _ProfStart(PGROUP_T1); */
                if (! mi_tcd_t1_encode(p_tcd)) {
                        return mi_FALSE;
                }
                /* FIXME _ProfStop(PGROUP_T1); */

                /* FIXME _ProfStart(PGROUP_RATE); */
                if (! mi_tcd_rate_allocate_encode(p_tcd,p_dest,p_max_length,p_cstr_info)) {
                        return mi_FALSE;
                }
                /* FIXME _ProfStop(PGROUP_RATE); */

        }
        /*--------------TIER2------------------*/

        /* INDEX */
        if (p_cstr_info) {
                p_cstr_info->index_write = 1;
        }
        /* FIXME _ProfStart(PGROUP_T2); */

        if (! mi_tcd_t2_encode(p_tcd,p_dest,p_data_written,p_max_length,p_cstr_info)) {
                return mi_FALSE;
        }
        /* FIXME _ProfStop(PGROUP_T2); */

        /*---------------CLEAN-------------------*/

        return mi_TRUE;
}

mi_BOOL mi_tcd_decode_tile(   mi_tcd_t *p_tcd,
                                mi_BYTE *p_src,
                                mi_UINT32 p_max_length,
                                mi_UINT32 p_tile_no,
                                mi_codestream_index_t *p_cstr_index,
                                mi_event_mgr_t *p_manager
                                )
{
        mi_UINT32 l_data_read;
        p_tcd->tcd_tileno = p_tile_no;
        p_tcd->tcp = &(p_tcd->cp->tcps[p_tile_no]);

        /*--------------TIER2------------------*/
        /* FIXME _ProfStart(PGROUP_T2); */
        l_data_read = 0;
        if (! mi_tcd_t2_decode(p_tcd, p_src, &l_data_read, p_max_length, p_cstr_index, p_manager))
        {
                return mi_FALSE;
        }
        /* FIXME _ProfStop(PGROUP_T2); */

        /*------------------TIER1-----------------*/

        /* FIXME _ProfStart(PGROUP_T1); */
        if
                (! mi_tcd_t1_decode(p_tcd))
        {
                return mi_FALSE;
        }
        /* FIXME _ProfStop(PGROUP_T1); */

        /*----------------DWT---------------------*/

        /* FIXME _ProfStart(PGROUP_DWT); */
        if
                (! mi_tcd_dwt_decode(p_tcd))
        {
                return mi_FALSE;
        }
        /* FIXME _ProfStop(PGROUP_DWT); */

        /*----------------MCT-------------------*/
        /* FIXME _ProfStart(PGROUP_MCT); */
        if
                (! mi_tcd_mct_decode(p_tcd, p_manager))
        {
                return mi_FALSE;
        }
        /* FIXME _ProfStop(PGROUP_MCT); */

        /* FIXME _ProfStart(PGROUP_DC_SHIFT); */
        if
                (! mi_tcd_dc_level_shift_decode(p_tcd))
        {
                return mi_FALSE;
        }
        /* FIXME _ProfStop(PGROUP_DC_SHIFT); */


        /*---------------TILE-------------------*/
        return mi_TRUE;
}

mi_BOOL mi_tcd_update_tile_data ( mi_tcd_t *p_tcd,
                                    mi_BYTE * p_dest,
                                    mi_UINT32 p_dest_length
                                    )
{
        mi_UINT32 i,j,k,l_data_size = 0;
        mi_image_comp_t * l_img_comp = 00;
        mi_tcd_tilecomp_t * l_tilec = 00;
        mi_tcd_resolution_t * l_res;
        mi_UINT32 l_size_comp, l_remaining;
        mi_UINT32 l_stride, l_width,l_height;

        l_data_size = mi_tcd_get_decoded_tile_size(p_tcd);
        if (l_data_size > p_dest_length) {
                return mi_FALSE;
        }

        l_tilec = p_tcd->tcd_image->tiles->comps;
        l_img_comp = p_tcd->image->comps;

        for (i=0;i<p_tcd->image->numcomps;++i) {
                l_size_comp = l_img_comp->prec >> 3; /*(/ 8)*/
                l_remaining = l_img_comp->prec & 7;  /* (%8) */
                l_res = l_tilec->resolutions + l_img_comp->resno_decoded;
                l_width = (mi_UINT32)(l_res->x1 - l_res->x0);
                l_height = (mi_UINT32)(l_res->y1 - l_res->y0);
                l_stride = (mi_UINT32)(l_tilec->x1 - l_tilec->x0) - l_width;

                if (l_remaining) {
                        ++l_size_comp;
                }

                if (l_size_comp == 3) {
                        l_size_comp = 4;
                }

                switch (l_size_comp)
                        {
                        case 1:
                                {
                                        mi_CHAR * l_dest_ptr = (mi_CHAR *) p_dest;
                                        const mi_INT32 * l_src_ptr = l_tilec->data;

                                        if (l_img_comp->sgnd) {
                                                for (j=0;j<l_height;++j) {
                                                        for (k=0;k<l_width;++k) {
                                                                *(l_dest_ptr++) = (mi_CHAR) (*(l_src_ptr++));
                                                        }
                                                        l_src_ptr += l_stride;
                                                }
                                        }
                                        else {
                                                for (j=0;j<l_height;++j) {
                                                        for     (k=0;k<l_width;++k) {
                                                                *(l_dest_ptr++) = (mi_CHAR) ((*(l_src_ptr++))&0xff);
                                                        }
                                                        l_src_ptr += l_stride;
                                                }
                                        }

                                        p_dest = (mi_BYTE *)l_dest_ptr;
                                }
                                break;
                        case 2:
                                {
                                        const mi_INT32 * l_src_ptr = l_tilec->data;
                                        mi_INT16 * l_dest_ptr = (mi_INT16 *) p_dest;

                                        if (l_img_comp->sgnd) {
                                                for (j=0;j<l_height;++j) {
                                                        for (k=0;k<l_width;++k) {
                                                                *(l_dest_ptr++) = (mi_INT16) (*(l_src_ptr++));
                                                        }
                                                        l_src_ptr += l_stride;
                                                }
                                        }
                                        else {
                                                for (j=0;j<l_height;++j) {
                                                        for (k=0;k<l_width;++k) {
                                                                *(l_dest_ptr++) = (mi_INT16) ((*(l_src_ptr++))&0xffff);
                                                        }
                                                        l_src_ptr += l_stride;
                                                }
                                        }

                                        p_dest = (mi_BYTE*) l_dest_ptr;
                                }
                                break;
                        case 4:
                                {
                                        mi_INT32 * l_dest_ptr = (mi_INT32 *) p_dest;
                                        mi_INT32 * l_src_ptr = l_tilec->data;

                                        for (j=0;j<l_height;++j) {
                                                for (k=0;k<l_width;++k) {
                                                        *(l_dest_ptr++) = (*(l_src_ptr++));
                                                }
                                                l_src_ptr += l_stride;
                                        }

                                        p_dest = (mi_BYTE*) l_dest_ptr;
                                }
                                break;
                }

                ++l_img_comp;
                ++l_tilec;
        }

        return mi_TRUE;
}




static void mi_tcd_free_tile(mi_tcd_t *p_tcd)
{
        mi_UINT32 compno, resno, bandno, precno;
        mi_tcd_tile_t *l_tile = 00;
        mi_tcd_tilecomp_t *l_tile_comp = 00;
        mi_tcd_resolution_t *l_res = 00;
        mi_tcd_band_t *l_band = 00;
        mi_tcd_precinct_t *l_precinct = 00;
        mi_UINT32 l_nb_resolutions, l_nb_precincts;
        void (* l_tcd_code_block_deallocate) (mi_tcd_precinct_t *) = 00;

        if (! p_tcd) {
                return;
        }

        if (! p_tcd->tcd_image) {
                return;
        }

        if (p_tcd->m_is_decoder) {
                l_tcd_code_block_deallocate = mi_tcd_code_block_dec_deallocate;
        }
        else {
                l_tcd_code_block_deallocate = mi_tcd_code_block_enc_deallocate;
        }

        l_tile = p_tcd->tcd_image->tiles;
        if (! l_tile) {
                return;
        }

        l_tile_comp = l_tile->comps;

        for (compno = 0; compno < l_tile->numcomps; ++compno) {
                l_res = l_tile_comp->resolutions;
                if (l_res) {

                        l_nb_resolutions = l_tile_comp->resolutions_size / sizeof(mi_tcd_resolution_t);
                        for (resno = 0; resno < l_nb_resolutions; ++resno) {
                                l_band = l_res->bands;
                                for     (bandno = 0; bandno < 3; ++bandno) {
                                        l_precinct = l_band->precincts;
                                        if (l_precinct) {

                                                l_nb_precincts = l_band->precincts_data_size / sizeof(mi_tcd_precinct_t);
                                                for (precno = 0; precno < l_nb_precincts; ++precno) {
                                                        mi_tgt_destroy(l_precinct->incltree);
                                                        l_precinct->incltree = 00;
                                                        mi_tgt_destroy(l_precinct->imsbtree);
                                                        l_precinct->imsbtree = 00;
                                                        (*l_tcd_code_block_deallocate) (l_precinct);
                                                        ++l_precinct;
                                                }

                                                mi_free(l_band->precincts);
                                                l_band->precincts = 00;
                                        }
                                        ++l_band;
                                } /* for (resno */
                                ++l_res;
                        }

                        mi_free(l_tile_comp->resolutions);
                        l_tile_comp->resolutions = 00;
                }

                if (l_tile_comp->ownsData && l_tile_comp->data) {
                        mi_aligned_free(l_tile_comp->data);
                        l_tile_comp->data = 00;
                        l_tile_comp->ownsData = 0;
                        l_tile_comp->data_size = 0;
                        l_tile_comp->data_size_needed = 0;
                }
                ++l_tile_comp;
        }

        mi_free(l_tile->comps);
        l_tile->comps = 00;
        mi_free(p_tcd->tcd_image->tiles);
        p_tcd->tcd_image->tiles = 00;
}


static mi_BOOL mi_tcd_t2_decode (mi_tcd_t *p_tcd,
                            mi_BYTE * p_src_data,
                            mi_UINT32 * p_data_read,
                            mi_UINT32 p_max_src_size,
                            mi_codestream_index_t *p_cstr_index,
                            mi_event_mgr_t *p_manager
                            )
{
        mi_t2_t * l_t2;

        l_t2 = mi_t2_create(p_tcd->image, p_tcd->cp);
        if (l_t2 == 00) {
                return mi_FALSE;
        }

        if (! mi_t2_decode_packets(
                                        l_t2,
                                        p_tcd->tcd_tileno,
                                        p_tcd->tcd_image->tiles,
                                        p_src_data,
                                        p_data_read,
                                        p_max_src_size,
                                        p_cstr_index,
                                        p_manager)) {
                mi_t2_destroy(l_t2);
                return mi_FALSE;
        }

        mi_t2_destroy(l_t2);

        /*---------------CLEAN-------------------*/
        return mi_TRUE;
}

static mi_BOOL mi_tcd_t1_decode ( mi_tcd_t *p_tcd )
{
        mi_UINT32 compno;
        mi_t1_t * l_t1;
        mi_tcd_tile_t * l_tile = p_tcd->tcd_image->tiles;
        mi_tcd_tilecomp_t* l_tile_comp = l_tile->comps;
        mi_tccp_t * l_tccp = p_tcd->tcp->tccps;


        l_t1 = mi_t1_create(mi_FALSE);
        if (l_t1 == 00) {
                return mi_FALSE;
        }

        for (compno = 0; compno < l_tile->numcomps; ++compno) {
                /* The +3 is headroom required by the vectorized DWT */
                if (mi_FALSE == mi_t1_decode_cblks(l_t1, l_tile_comp, l_tccp)) {
                        mi_t1_destroy(l_t1);
                        return mi_FALSE;
                }
                ++l_tile_comp;
                ++l_tccp;
        }

        mi_t1_destroy(l_t1);

        return mi_TRUE;
}


static mi_BOOL mi_tcd_dwt_decode ( mi_tcd_t *p_tcd )
{
        mi_UINT32 compno;
        mi_tcd_tile_t * l_tile = p_tcd->tcd_image->tiles;
        mi_tcd_tilecomp_t * l_tile_comp = l_tile->comps;
        mi_tccp_t * l_tccp = p_tcd->tcp->tccps;
        mi_image_comp_t * l_img_comp = p_tcd->image->comps;

        for (compno = 0; compno < l_tile->numcomps; compno++) {
                /*
                if (tcd->cp->reduce != 0) {
                        tcd->image->comps[compno].resno_decoded =
                                tile->comps[compno].numresolutions - tcd->cp->reduce - 1;
                        if (tcd->image->comps[compno].resno_decoded < 0)
                        {
                                return false;
                        }
                }
                numres2decode = tcd->image->comps[compno].resno_decoded + 1;
                if(numres2decode > 0){
                */

                if (l_tccp->qmfbid == 1) {
                        if (! mi_dwt_decode(l_tile_comp, l_img_comp->resno_decoded+1)) {
                                return mi_FALSE;
                        }
                }
                else {
                        if (! mi_dwt_decode_real(l_tile_comp, l_img_comp->resno_decoded+1)) {
                                return mi_FALSE;
                        }
                }

                ++l_tile_comp;
                ++l_img_comp;
                ++l_tccp;
        }

        return mi_TRUE;
}
static mi_BOOL mi_tcd_mct_decode ( mi_tcd_t *p_tcd, mi_event_mgr_t *p_manager)
{
        mi_tcd_tile_t * l_tile = p_tcd->tcd_image->tiles;
        mi_tcp_t * l_tcp = p_tcd->tcp;
        mi_tcd_tilecomp_t * l_tile_comp = l_tile->comps;
        mi_UINT32 l_samples,i;

        if (! l_tcp->mct) {
                return mi_TRUE;
        }

        l_samples = (mi_UINT32)((l_tile_comp->x1 - l_tile_comp->x0) * (l_tile_comp->y1 - l_tile_comp->y0));

        if (l_tile->numcomps >= 3 ){
                /* testcase 1336.pdf.asan.47.376 */
                if ((l_tile->comps[0].x1 - l_tile->comps[0].x0) * (l_tile->comps[0].y1 - l_tile->comps[0].y0) < (mi_INT32)l_samples ||
                    (l_tile->comps[1].x1 - l_tile->comps[1].x0) * (l_tile->comps[1].y1 - l_tile->comps[1].y0) < (mi_INT32)l_samples ||
                    (l_tile->comps[2].x1 - l_tile->comps[2].x0) * (l_tile->comps[2].y1 - l_tile->comps[2].y0) < (mi_INT32)l_samples) {
                        mi_event_msg(p_manager, EVT_ERROR, "Tiles don't all have the same dimension. Skip the MCT step.\n");
                        return mi_FALSE;
                }
                else if (l_tcp->mct == 2) {
                        mi_BYTE ** l_data;

                        if (! l_tcp->m_mct_decoding_matrix) {
                                return mi_TRUE;
                        }

                        l_data = (mi_BYTE **) mi_malloc(l_tile->numcomps*sizeof(mi_BYTE*));
                        if (! l_data) {
                                return mi_FALSE;
                        }

                        for (i=0;i<l_tile->numcomps;++i) {
                                l_data[i] = (mi_BYTE*) l_tile_comp->data;
                                ++l_tile_comp;
                        }

                        if (! mi_mct_decode_custom(/* MCT data */
                                                                        (mi_BYTE*) l_tcp->m_mct_decoding_matrix,
                                                                        /* size of components */
                                                                        l_samples,
                                                                        /* components */
                                                                        l_data,
                                                                        /* nb of components (i.e. size of pData) */
                                                                        l_tile->numcomps,
                                                                        /* tells if the data is signed */
                                                                        p_tcd->image->comps->sgnd)) {
                                mi_free(l_data);
                                return mi_FALSE;
                        }

                        mi_free(l_data);
                }
                else {
                        if (l_tcp->tccps->qmfbid == 1) {
                                mi_mct_decode(     l_tile->comps[0].data,
                                                        l_tile->comps[1].data,
                                                        l_tile->comps[2].data,
                                                        l_samples);
                        }
                        else {
                            mi_mct_decode_real((mi_FLOAT32*)l_tile->comps[0].data,
                                                (mi_FLOAT32*)l_tile->comps[1].data,
                                                (mi_FLOAT32*)l_tile->comps[2].data,
                                                l_samples);
                        }
                }
        }
        else {
                mi_event_msg(p_manager, EVT_ERROR, "Number of components (%d) is inconsistent with a MCT. Skip the MCT step.\n",l_tile->numcomps);
        }

        return mi_TRUE;
}


static mi_BOOL mi_tcd_dc_level_shift_decode ( mi_tcd_t *p_tcd )
{
        mi_UINT32 compno;
        mi_tcd_tilecomp_t * l_tile_comp = 00;
        mi_tccp_t * l_tccp = 00;
        mi_image_comp_t * l_img_comp = 00;
        mi_tcd_resolution_t* l_res = 00;
        mi_tcd_tile_t * l_tile;
        mi_UINT32 l_width,l_height,i,j;
        mi_INT32 * l_current_ptr;
        mi_INT32 l_min, l_max;
        mi_UINT32 l_stride;

        l_tile = p_tcd->tcd_image->tiles;
        l_tile_comp = l_tile->comps;
        l_tccp = p_tcd->tcp->tccps;
        l_img_comp = p_tcd->image->comps;

        for (compno = 0; compno < l_tile->numcomps; compno++) {
                l_res = l_tile_comp->resolutions + l_img_comp->resno_decoded;
                l_width = (mi_UINT32)(l_res->x1 - l_res->x0);
                l_height = (mi_UINT32)(l_res->y1 - l_res->y0);
                l_stride = (mi_UINT32)(l_tile_comp->x1 - l_tile_comp->x0) - l_width;

                assert(l_height == 0 || l_width + l_stride <= l_tile_comp->data_size / l_height); /*MUPDF*/

                if (l_img_comp->sgnd) {
                        l_min = -(1 << (l_img_comp->prec - 1));
                        l_max = (1 << (l_img_comp->prec - 1)) - 1;
                }
                else {
            l_min = 0;
                        l_max = (1 << l_img_comp->prec) - 1;
                }

                l_current_ptr = l_tile_comp->data;

                if (l_tccp->qmfbid == 1) {
                        for (j=0;j<l_height;++j) {
                                for (i = 0; i < l_width; ++i) {
                                        *l_current_ptr = mi_int_clamp(*l_current_ptr + l_tccp->m_dc_level_shift, l_min, l_max);
                                        ++l_current_ptr;
                                }
                                l_current_ptr += l_stride;
                        }
                }
                else {
                        for (j=0;j<l_height;++j) {
                                for (i = 0; i < l_width; ++i) {
                                        mi_FLOAT32 l_value = *((mi_FLOAT32 *) l_current_ptr);
                                        *l_current_ptr = mi_int_clamp((mi_INT32)mi_lrintf(l_value) + l_tccp->m_dc_level_shift, l_min, l_max); ;
                                        ++l_current_ptr;
                                }
                                l_current_ptr += l_stride;
                        }
                }

                ++l_img_comp;
                ++l_tccp;
                ++l_tile_comp;
        }

        return mi_TRUE;
}



/**
 * Deallocates the encoding data of the given precinct.
 */
static void mi_tcd_code_block_dec_deallocate (mi_tcd_precinct_t * p_precinct)
{
        mi_UINT32 cblkno , l_nb_code_blocks;

        mi_tcd_cblk_dec_t * l_code_block = p_precinct->cblks.dec;
        if (l_code_block) {
                /*fprintf(stderr,"deallocate codeblock:{\n");*/
                /*fprintf(stderr,"\t x0=%d, y0=%d, x1=%d, y1=%d\n",l_code_block->x0, l_code_block->y0, l_code_block->x1, l_code_block->y1);*/
                /*fprintf(stderr,"\t numbps=%d, numlenbits=%d, len=%d, numnewpasses=%d, real_num_segs=%d, m_current_max_segs=%d\n ",
                                l_code_block->numbps, l_code_block->numlenbits, l_code_block->len, l_code_block->numnewpasses, l_code_block->real_num_segs, l_code_block->m_current_max_segs );*/


                l_nb_code_blocks = p_precinct->block_size / sizeof(mi_tcd_cblk_dec_t);
                /*fprintf(stderr,"nb_code_blocks =%d\t}\n", l_nb_code_blocks);*/

                for (cblkno = 0; cblkno < l_nb_code_blocks; ++cblkno) {

                        if (l_code_block->data) {
                                mi_free(l_code_block->data);
                                l_code_block->data = 00;
                        }

                        if (l_code_block->segs) {
                                mi_free(l_code_block->segs );
                                l_code_block->segs = 00;
                        }

                        ++l_code_block;
                }

                mi_free(p_precinct->cblks.dec);
                p_precinct->cblks.dec = 00;
        }
}

/**
 * Deallocates the encoding data of the given precinct.
 */
static void mi_tcd_code_block_enc_deallocate (mi_tcd_precinct_t * p_precinct)
{       
        mi_UINT32 cblkno , l_nb_code_blocks;

        mi_tcd_cblk_enc_t * l_code_block = p_precinct->cblks.enc;
        if (l_code_block) {
                l_nb_code_blocks = p_precinct->block_size / sizeof(mi_tcd_cblk_enc_t);
                
                for     (cblkno = 0; cblkno < l_nb_code_blocks; ++cblkno)  {
                        if (l_code_block->data) {
                                mi_free(l_code_block->data - 1);
                                l_code_block->data = 00;
                        }

                        if (l_code_block->layers) {
                                mi_free(l_code_block->layers );
                                l_code_block->layers = 00;
                        }

                        if (l_code_block->passes) {
                                mi_free(l_code_block->passes );
                                l_code_block->passes = 00;
                        }
                        ++l_code_block;
                }

                mi_free(p_precinct->cblks.enc);
                
                p_precinct->cblks.enc = 00;
        }
}

mi_UINT32 mi_tcd_get_encoded_tile_size ( mi_tcd_t *p_tcd )
{
        mi_UINT32 i,l_data_size = 0;
        mi_image_comp_t * l_img_comp = 00;
        mi_tcd_tilecomp_t * l_tilec = 00;
        mi_UINT32 l_size_comp, l_remaining;

        l_tilec = p_tcd->tcd_image->tiles->comps;
        l_img_comp = p_tcd->image->comps;
        for (i=0;i<p_tcd->image->numcomps;++i) {
                l_size_comp = l_img_comp->prec >> 3; /*(/ 8)*/
                l_remaining = l_img_comp->prec & 7;  /* (%8) */

                if (l_remaining) {
                        ++l_size_comp;
                }

                if (l_size_comp == 3) {
                        l_size_comp = 4;
                }

                l_data_size += l_size_comp * (mi_UINT32)((l_tilec->x1 - l_tilec->x0) * (l_tilec->y1 - l_tilec->y0));
                ++l_img_comp;
                ++l_tilec;
        }

        return l_data_size;
}
//应该是直流电平移函数                
static mi_BOOL mi_tcd_dc_level_shift_encode ( mi_tcd_t *p_tcd )
{
        mi_UINT32 compno;
        mi_tcd_tilecomp_t * l_tile_comp = 00;
        mi_tccp_t * l_tccp = 00;
        mi_image_comp_t * l_img_comp = 00;
        mi_tcd_tile_t * l_tile;
        mi_UINT32 l_nb_elem,i;
        mi_INT32 * l_current_ptr;

        l_tile = p_tcd->tcd_image->tiles;
        l_tile_comp = l_tile->comps;
        l_tccp = p_tcd->tcp->tccps;
        l_img_comp = p_tcd->image->comps;

        for (compno = 0; compno < l_tile->numcomps; compno++) {
                l_current_ptr = l_tile_comp->data;
                l_nb_elem = (mi_UINT32)((l_tile_comp->x1 - l_tile_comp->x0) * (l_tile_comp->y1 - l_tile_comp->y0));

                if (l_tccp->qmfbid == 1) {
                        for     (i = 0; i < l_nb_elem; ++i) {
                                *l_current_ptr -= l_tccp->m_dc_level_shift ;
                                ++l_current_ptr;
                        }
                }
                else {
                        for (i = 0; i < l_nb_elem; ++i) {
                                *l_current_ptr = (*l_current_ptr - l_tccp->m_dc_level_shift) * (1 << 11);
                                ++l_current_ptr;
                        }
                }

                ++l_img_comp;
                ++l_tccp;
                ++l_tile_comp;
        }

        return mi_TRUE;
}
//多分量变换函数
static mi_BOOL mi_tcd_mct_encode ( mi_tcd_t *p_tcd )
{
        mi_tcd_tile_t * l_tile = p_tcd->tcd_image->tiles;
        mi_tcd_tilecomp_t * l_tile_comp = p_tcd->tcd_image->tiles->comps;
        mi_UINT32 samples = (mi_UINT32)((l_tile_comp->x1 - l_tile_comp->x0) * (l_tile_comp->y1 - l_tile_comp->y0));
        mi_UINT32 i;
        mi_BYTE ** l_data = 00;
        mi_tcp_t * l_tcp = p_tcd->tcp;

        if(!p_tcd->tcp->mct) {
                return mi_TRUE;
        }

        if (p_tcd->tcp->mct == 2) {
                if (! p_tcd->tcp->m_mct_coding_matrix) {
                        return mi_TRUE;
                }

        l_data = (mi_BYTE **) mi_malloc(l_tile->numcomps*sizeof(mi_BYTE*));
                if (! l_data) {
                        return mi_FALSE;
                }

                for (i=0;i<l_tile->numcomps;++i) {
                        l_data[i] = (mi_BYTE*) l_tile_comp->data;
                        ++l_tile_comp;
                }

                if (! mi_mct_encode_custom(/* MCT data */
                                        (mi_BYTE*) p_tcd->tcp->m_mct_coding_matrix,
                                        /* size of components */
                                        samples,
                                        /* components */
                                        l_data,
                                        /* nb of components (i.e. size of pData) */
                                        l_tile->numcomps,
                                        /* tells if the data is signed */
                                        p_tcd->image->comps->sgnd) )
                {
            mi_free(l_data);
                        return mi_FALSE;
                }

                mi_free(l_data);
        }
        else if (l_tcp->tccps->qmfbid == 0) {
                mi_mct_encode_real(l_tile->comps[0].data, l_tile->comps[1].data, l_tile->comps[2].data, samples);
        }
        else {
                mi_mct_encode(l_tile->comps[0].data, l_tile->comps[1].data, l_tile->comps[2].data, samples);
        }

        return mi_TRUE;
}
//二维小波变换总函数
static mi_BOOL mi_tcd_dwt_encode ( mi_tcd_t *p_tcd )
{
        mi_tcd_tile_t * l_tile = p_tcd->tcd_image->tiles;
        mi_tcd_tilecomp_t * l_tile_comp = p_tcd->tcd_image->tiles->comps;
        mi_tccp_t * l_tccp = p_tcd->tcp->tccps;
        mi_UINT32 compno;

        for (compno = 0; compno < l_tile->numcomps; ++compno) {
                if (l_tccp->qmfbid == 1) {
                        if (! mi_dwt_encode(l_tile_comp)) {
                                return mi_FALSE;
                        }
                }
                else if (l_tccp->qmfbid == 0) {
                        if (! mi_dwt_encode_real(l_tile_comp)) {
                                return mi_FALSE;
                        }
                }

                ++l_tile_comp;
                ++l_tccp;
        }

        return mi_TRUE;
}

static mi_BOOL mi_tcd_t1_encode ( mi_tcd_t *p_tcd )
{
        mi_t1_t * l_t1;
        const mi_FLOAT64 * l_mct_norms;
        mi_UINT32 l_mct_numcomps = 0U;
        mi_tcp_t * l_tcp = p_tcd->tcp;

        l_t1 = mi_t1_create(mi_TRUE);
        if (l_t1 == 00) {
                return mi_FALSE;
        }

        if (l_tcp->mct == 1) {
                l_mct_numcomps = 3U;
                /* irreversible encoding */
                if (l_tcp->tccps->qmfbid == 0) {
                        l_mct_norms = mi_mct_get_mct_norms_real();
                }
                else {
                        l_mct_norms = mi_mct_get_mct_norms();
                }
        }
        else {
                l_mct_numcomps = p_tcd->image->numcomps;
                l_mct_norms = (const mi_FLOAT64 *) (l_tcp->mct_norms);
        }

        if (! mi_t1_encode_cblks(l_t1, p_tcd->tcd_image->tiles , l_tcp, l_mct_norms, l_mct_numcomps)) {
        mi_t1_destroy(l_t1);
                return mi_FALSE;
        }

        mi_t1_destroy(l_t1);

        return mi_TRUE;
}

static mi_BOOL mi_tcd_t2_encode (mi_tcd_t *p_tcd,
                                                mi_BYTE * p_dest_data,
                                                mi_UINT32 * p_data_written,
                                                mi_UINT32 p_max_dest_size,
                                                mi_codestream_info_t *p_cstr_info )
{
        mi_t2_t * l_t2;

        l_t2 = mi_t2_create(p_tcd->image, p_tcd->cp);
        if (l_t2 == 00) {
                return mi_FALSE;
        }

        if (! mi_t2_encode_packets(
                                        l_t2,
                                        p_tcd->tcd_tileno,
                                        p_tcd->tcd_image->tiles,
                                        p_tcd->tcp->numlayers,
                                        p_dest_data,
                                        p_data_written,
                                        p_max_dest_size,
                                        p_cstr_info,
                                        p_tcd->tp_num,
                                        p_tcd->tp_pos,
                                        p_tcd->cur_pino,
                                        FINAL_PASS))
        {
                mi_t2_destroy(l_t2);
                return mi_FALSE;
        }

        mi_t2_destroy(l_t2);

        /*---------------CLEAN-------------------*/
        return mi_TRUE;
}


static mi_BOOL mi_tcd_rate_allocate_encode(  mi_tcd_t *p_tcd,
                                                                            mi_BYTE * p_dest_data,
                                                                            mi_UINT32 p_max_dest_size,
                                                                            mi_codestream_info_t *p_cstr_info )
{
        mi_cp_t * l_cp = p_tcd->cp;
        mi_UINT32 l_nb_written = 0;

        if (p_cstr_info)  {
                p_cstr_info->index_write = 0;
        }

        if (l_cp->m_specific_param.m_enc.m_disto_alloc|| l_cp->m_specific_param.m_enc.m_fixed_quality)  {
                /* fixed_quality */
                /* Normal Rate/distortion allocation */
                if (! mi_tcd_rateallocate(p_tcd, p_dest_data,&l_nb_written, p_max_dest_size, p_cstr_info)) {
                        return mi_FALSE;
                }
        }
        else {
                /* Fixed layer allocation */
                mi_tcd_rateallocate_fixed(p_tcd);
        }

        return mi_TRUE;
}

//跟obj_j2k_get_tile_data逻辑差别不大
mi_BOOL mi_tcd_copy_tile_data (       mi_tcd_t *p_tcd,
                                                                    mi_BYTE * p_src,
                                                                    mi_UINT32 p_src_length )
{
        mi_UINT32 i,j,l_data_size = 0;
        mi_image_comp_t * l_img_comp = 00;
        mi_tcd_tilecomp_t * l_tilec = 00;
        mi_UINT32 l_size_comp, l_remaining;
        mi_UINT32 l_nb_elem;

        l_data_size = mi_tcd_get_encoded_tile_size(p_tcd);
        if (l_data_size != p_src_length) {
                return mi_FALSE;
        }

        l_tilec = p_tcd->tcd_image->tiles->comps;
        l_img_comp = p_tcd->image->comps;
        for (i=0;i<p_tcd->image->numcomps;++i) {
                l_size_comp = l_img_comp->prec >> 3; /*(/ 8)*/
                l_remaining = l_img_comp->prec & 7;  /* (%8) */
                l_nb_elem = (mi_UINT32)((l_tilec->x1 - l_tilec->x0) * (l_tilec->y1 - l_tilec->y0));

                if (l_remaining) {
                        ++l_size_comp;
                }

                if (l_size_comp == 3) {
                        l_size_comp = 4;
                }

                switch (l_size_comp) {
                        case 1:
                                {
                                        mi_CHAR * l_src_ptr = (mi_CHAR *) p_src;
                                        mi_INT32 * l_dest_ptr = l_tilec->data;

                                        if (l_img_comp->sgnd) {
                                                for (j=0;j<l_nb_elem;++j) {
                                                        *(l_dest_ptr++) = (mi_INT32) (*(l_src_ptr++));
                                                }
                                        }
                                        else {
                                                for (j=0;j<l_nb_elem;++j) {
                                                        *(l_dest_ptr++) = (*(l_src_ptr++))&0xff;
                                                }
                                        }

                                        p_src = (mi_BYTE*) l_src_ptr;
                                }
                                break;
                        case 2:
                                {
                                        mi_INT32 * l_dest_ptr = l_tilec->data;
                                        mi_INT16 * l_src_ptr = (mi_INT16 *) p_src;

                                        if (l_img_comp->sgnd) {
                                                for (j=0;j<l_nb_elem;++j) {
                                                        *(l_dest_ptr++) = (mi_INT32) (*(l_src_ptr++));
                                                }
                                        }
                                        else {
                                                for (j=0;j<l_nb_elem;++j) {
                                                        *(l_dest_ptr++) = (*(l_src_ptr++))&0xffff;
                                                }
                                        }

                                        p_src = (mi_BYTE*) l_src_ptr;
                                }
                                break;
                        case 4:
                                {
                                        mi_INT32 * l_src_ptr = (mi_INT32 *) p_src;
                                        mi_INT32 * l_dest_ptr = l_tilec->data;

                                        for (j=0;j<l_nb_elem;++j) {
                                                *(l_dest_ptr++) = (mi_INT32) (*(l_src_ptr++));
                                        }

                                        p_src = (mi_BYTE*) l_src_ptr;
                                }
                                break;
                }

                ++l_img_comp;
                ++l_tilec;
        }

        return mi_TRUE;
}
