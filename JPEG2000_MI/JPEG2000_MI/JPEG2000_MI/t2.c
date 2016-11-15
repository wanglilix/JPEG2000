
#include "mi_includes.h"

/** @defgroup T2 T2 - Implementation of a tier-2 coding */
/*@{*/

/** @name Local static functions */
/*@{*/

static void mi_t2_putcommacode(mi_bio_t *bio, mi_INT32 n);

static mi_UINT32 mi_t2_getcommacode(mi_bio_t *bio); 
/**
Variable length code for signalling delta Zil (truncation point)
@param bio  Bit Input/Output component
@param n    delta Zil
*/
static void mi_t2_putnumpasses(mi_bio_t *bio, mi_UINT32 n);
static mi_UINT32 mi_t2_getnumpasses(mi_bio_t *bio);

/**
Encode a packet of a tile to a destination buffer
@param tileno Number of the tile encoded
@param tile Tile for which to write the packets
@param tcp Tile coding parameters
@param pi Packet identity
@param dest Destination buffer
@param p_data_written   FIXME DOC
@param len Length of the destination buffer
@param cstr_info Codestream information structure
@return
*/
static mi_BOOL mi_t2_encode_packet(   mi_UINT32 tileno,
                                        mi_tcd_tile_t *tile,
                                        mi_tcp_t *tcp,
                                        mi_pi_iterator_t *pi,
                                        mi_BYTE *dest,
                                        mi_UINT32 * p_data_written,
                                        mi_UINT32 len,
                                        mi_codestream_info_t *cstr_info);

/**
Decode a packet of a tile from a source buffer
@param t2 T2 handle
@param tile Tile for which to write the packets
@param tcp Tile coding parameters
@param pi Packet identity
@param src Source buffer
@param data_read   FIXME DOC
@param max_length  FIXME DOC
@param pack_info Packet information

@return  FIXME DOC
*/
static mi_BOOL mi_t2_decode_packet(   mi_t2_t* t2,
                                        mi_tcd_tile_t *tile,
                                        mi_tcp_t *tcp,
                                        mi_pi_iterator_t *pi,
                                        mi_BYTE *src,
                                        mi_UINT32 * data_read,
                                        mi_UINT32 max_length,
                                        mi_packet_info_t *pack_info,
                                        mi_event_mgr_t *p_manager);

static mi_BOOL mi_t2_skip_packet( mi_t2_t* p_t2,
                                    mi_tcd_tile_t *p_tile,
                                    mi_tcp_t *p_tcp,
                                    mi_pi_iterator_t *p_pi,
                                    mi_BYTE *p_src,
                                    mi_UINT32 * p_data_read,
                                    mi_UINT32 p_max_length,
                                    mi_packet_info_t *p_pack_info,
                                    mi_event_mgr_t *p_manager);

static mi_BOOL mi_t2_read_packet_header(  mi_t2_t* p_t2,
                                            mi_tcd_tile_t *p_tile,
                                            mi_tcp_t *p_tcp,
                                            mi_pi_iterator_t *p_pi,
                                            mi_BOOL * p_is_data_present,
                                            mi_BYTE *p_src_data,
                                            mi_UINT32 * p_data_read,
                                            mi_UINT32 p_max_length,
                                            mi_packet_info_t *p_pack_info,
                                            mi_event_mgr_t *p_manager);

static mi_BOOL mi_t2_read_packet_data(mi_t2_t* p_t2,
                                        mi_tcd_tile_t *p_tile,
                                        mi_pi_iterator_t *p_pi,
                                        mi_BYTE *p_src_data,
                                        mi_UINT32 * p_data_read,
                                        mi_UINT32 p_max_length,
                                        mi_packet_info_t *pack_info,
                                        mi_event_mgr_t *p_manager);

static mi_BOOL mi_t2_skip_packet_data(mi_t2_t* p_t2,
                                        mi_tcd_tile_t *p_tile,
                                        mi_pi_iterator_t *p_pi,
                                        mi_UINT32 * p_data_read,
                                        mi_UINT32 p_max_length,
                                        mi_packet_info_t *pack_info,
                                        mi_event_mgr_t *p_manager);

/**
@param cblk
@param index
@param cblksty
@param first
*/
static mi_BOOL mi_t2_init_seg(    mi_tcd_cblk_dec_t* cblk,
                                    mi_UINT32 index,
                                    mi_UINT32 cblksty,
                                    mi_UINT32 first);

/*@}*/

/*@}*/

/* ----------------------------------------------------------------------- */

/* #define RESTART 0x04 */
static void mi_t2_putcommacode(mi_bio_t *bio, mi_INT32 n) {
        while (--n >= 0) {
                mi_bio_write(bio, 1, 1);
        }
        mi_bio_write(bio, 0, 1);
}

static mi_UINT32 mi_t2_getcommacode(mi_bio_t *bio)
{
    mi_UINT32 n = 0;
    while (mi_bio_read(bio, 1)) {
	    ++n;
    }
    return n;
}

static void mi_t2_putnumpasses(mi_bio_t *bio, mi_UINT32 n) {
        if (n == 1) {
                mi_bio_write(bio, 0, 1);
        } else if (n == 2) {
                mi_bio_write(bio, 2, 2);
        } else if (n <= 5) {
                mi_bio_write(bio, 0xc | (n - 3), 4);
        } else if (n <= 36) {
                mi_bio_write(bio, 0x1e0 | (n - 6), 9);
        } else if (n <= 164) {
                mi_bio_write(bio, 0xff80 | (n - 37), 16);
        }
}

static mi_UINT32 mi_t2_getnumpasses(mi_bio_t *bio) {
        mi_UINT32 n;
        if (!mi_bio_read(bio, 1))
                return 1;
        if (!mi_bio_read(bio, 1))
                return 2;
        if ((n = mi_bio_read(bio, 2)) != 3)
                return (3 + n);
        if ((n = mi_bio_read(bio, 5)) != 31)
                return (6 + n);
        return (37 + mi_bio_read(bio, 7));
}

/* ----------------------------------------------------------------------- */

mi_BOOL mi_t2_encode_packets( mi_t2_t* p_t2,
                                mi_UINT32 p_tile_no,
                                mi_tcd_tile_t *p_tile,
                                mi_UINT32 p_maxlayers,
                                mi_BYTE *p_dest,
                                mi_UINT32 * p_data_written,
                                mi_UINT32 p_max_len,
                                mi_codestream_info_t *cstr_info,
                                mi_UINT32 p_tp_num,
                                mi_INT32 p_tp_pos,
                                mi_UINT32 p_pino,
                                J2K_T2_MODE p_t2_mode)
{
        mi_BYTE *l_current_data = p_dest;
        mi_UINT32 l_nb_bytes = 0;
        mi_UINT32 compno;
        mi_UINT32 poc;
        mi_pi_iterator_t *l_pi = 00;
        mi_pi_iterator_t *l_current_pi = 00;
        mi_image_t *l_image = p_t2->image;
        mi_cp_t *l_cp = p_t2->cp;
        mi_tcp_t *l_tcp = &l_cp->tcps[p_tile_no];
        mi_UINT32 pocno = (l_cp->rsiz == mi_PROFILE_CINEMA_4K)? 2: 1;
        mi_UINT32 l_max_comp = l_cp->m_specific_param.m_enc.m_max_comp_size > 0 ? l_image->numcomps : 1;
        mi_UINT32 l_nb_pocs = l_tcp->numpocs + 1;

        l_pi = mi_pi_initialise_encode(l_image, l_cp, p_tile_no, p_t2_mode);
        if (!l_pi) {
                return mi_FALSE;
        }

        * p_data_written = 0;

        if (p_t2_mode == THRESH_CALC ){ /* Calculating threshold */
                l_current_pi = l_pi;

                for     (compno = 0; compno < l_max_comp; ++compno) {
                        mi_UINT32 l_comp_len = 0;
                        l_current_pi = l_pi;

                        for (poc = 0; poc < pocno ; ++poc) {
                                mi_UINT32 l_tp_num = compno;

                                /* TODO MSD : check why this function cannot fail (cf. v1) */
                                mi_pi_create_encode(l_pi, l_cp,p_tile_no,poc,l_tp_num,p_tp_pos,p_t2_mode);

                                if (l_current_pi->poc.prg == mi_PROG_UNKNOWN) {
                                    /* TODO ADE : add an error */
                                    mi_pi_destroy(l_pi, l_nb_pocs);
                                    return mi_FALSE;
                                }
                                while (mi_pi_next(l_current_pi)) {
                                        if (l_current_pi->layno < p_maxlayers) {
                                                l_nb_bytes = 0;

                                                if (! mi_t2_encode_packet(p_tile_no,p_tile, l_tcp, l_current_pi, l_current_data, &l_nb_bytes, p_max_len, cstr_info)) {
                                                        mi_pi_destroy(l_pi, l_nb_pocs);
                                                        return mi_FALSE;
                                                }

                                                l_comp_len += l_nb_bytes;
                                                l_current_data += l_nb_bytes;
                                                p_max_len -= l_nb_bytes;

                                                * p_data_written += l_nb_bytes;
                                        }
                                }

                                if (l_cp->m_specific_param.m_enc.m_max_comp_size) {
                                        if (l_comp_len > l_cp->m_specific_param.m_enc.m_max_comp_size) {
                                                mi_pi_destroy(l_pi, l_nb_pocs);
                                                return mi_FALSE;
                                        }
                                }

                                ++l_current_pi;
                        }
                }
        }
        else {  /* t2_mode == FINAL_PASS  */
                mi_pi_create_encode(l_pi, l_cp,p_tile_no,p_pino,p_tp_num,p_tp_pos,p_t2_mode);

                l_current_pi = &l_pi[p_pino];
                if (l_current_pi->poc.prg == mi_PROG_UNKNOWN) {
                    /* TODO ADE : add an error */
                    mi_pi_destroy(l_pi, l_nb_pocs);
                    return mi_FALSE;
                }
                while (mi_pi_next(l_current_pi)) {
                        if (l_current_pi->layno < p_maxlayers) {
                                l_nb_bytes=0;

                                if (! mi_t2_encode_packet(p_tile_no,p_tile, l_tcp, l_current_pi, l_current_data, &l_nb_bytes, p_max_len, cstr_info)) {
                                        mi_pi_destroy(l_pi, l_nb_pocs);
                                        return mi_FALSE;
                                }

                                l_current_data += l_nb_bytes;
                                p_max_len -= l_nb_bytes;

                                * p_data_written += l_nb_bytes;

                                /* INDEX >> */
                                if(cstr_info) {
                                        if(cstr_info->index_write) {
                                                mi_tile_info_t *info_TL = &cstr_info->tile[p_tile_no];
                                                mi_packet_info_t *info_PK = &info_TL->packet[cstr_info->packno];
                                                if (!cstr_info->packno) {
                                                        info_PK->start_pos = info_TL->end_header + 1;
                                                } else {
                                                        info_PK->start_pos = ((l_cp->m_specific_param.m_enc.m_tp_on | l_tcp->POC)&& info_PK->start_pos) ? info_PK->start_pos : info_TL->packet[cstr_info->packno - 1].end_pos + 1;
                                                }
                                                info_PK->end_pos = info_PK->start_pos + l_nb_bytes - 1;
                                                info_PK->end_ph_pos += info_PK->start_pos - 1;  /* End of packet header which now only represents the distance
                                                                                                                                                                                                                                                   to start of packet is incremented by value of start of packet*/
                                        }

                                        cstr_info->packno++;
                                }
                                /* << INDEX */
                                ++p_tile->packno;
                        }
                }
        }

        mi_pi_destroy(l_pi, l_nb_pocs);

        return mi_TRUE;
}

/* see issue 80 */
#if 0
#define JAS_FPRINTF fprintf
#else
/* issue 290 */
static void mi_null_jas_fprintf(FILE* file, const char * format, ...)
{
  (void)file;
  (void)format;
}
#define JAS_FPRINTF mi_null_jas_fprintf
#endif

mi_BOOL mi_t2_decode_packets( mi_t2_t *p_t2,
                                mi_UINT32 p_tile_no,
                                mi_tcd_tile_t *p_tile,
                                mi_BYTE *p_src,
                                mi_UINT32 * p_data_read,
                                mi_UINT32 p_max_len,
                                mi_codestream_index_t *p_cstr_index,
                                mi_event_mgr_t *p_manager)
{
        mi_BYTE *l_current_data = p_src;
        mi_pi_iterator_t *l_pi = 00;
        mi_UINT32 pino;
        mi_image_t *l_image = p_t2->image;
        mi_cp_t *l_cp = p_t2->cp;
        mi_tcp_t *l_tcp = &(p_t2->cp->tcps[p_tile_no]);
        mi_UINT32 l_nb_bytes_read;
        mi_UINT32 l_nb_pocs = l_tcp->numpocs + 1;
        mi_pi_iterator_t *l_current_pi = 00;
        mi_packet_info_t *l_pack_info = 00;
        mi_image_comp_t* l_img_comp = 00;

        mi_ARG_NOT_USED(p_cstr_index);



        /* create a packet iterator */
        l_pi = mi_pi_create_decode(l_image, l_cp, p_tile_no);
        if (!l_pi) {
                return mi_FALSE;
        }


        l_current_pi = l_pi;

        for     (pino = 0; pino <= l_tcp->numpocs; ++pino) {

                /* if the resolution needed is too low, one dim of the tilec could be equal to zero
                 * and no packets are used to decode this resolution and
                 * l_current_pi->resno is always >= p_tile->comps[l_current_pi->compno].minimum_num_resolutions
                 * and no l_img_comp->resno_decoded are computed
                 */
                mi_BOOL* first_pass_failed = NULL;
					
                if (l_current_pi->poc.prg == mi_PROG_UNKNOWN) {
                    /* TODO ADE : add an error */
                    mi_pi_destroy(l_pi, l_nb_pocs);
                    return mi_FALSE;
                }
					
                first_pass_failed = (mi_BOOL*)mi_malloc(l_image->numcomps * sizeof(mi_BOOL));
                if (!first_pass_failed)
                {
                    mi_pi_destroy(l_pi,l_nb_pocs);
                    return mi_FALSE;
                }
                memset(first_pass_failed, mi_TRUE, l_image->numcomps * sizeof(mi_BOOL));

                while (mi_pi_next(l_current_pi)) {
                  JAS_FPRINTF( stderr, "packet offset=00000166 prg=%d cmptno=%02d rlvlno=%02d prcno=%03d lyrno=%02d\n\n",
                    l_current_pi->poc.prg1, l_current_pi->compno, l_current_pi->resno, l_current_pi->precno, l_current_pi->layno );

                        if (l_tcp->num_layers_to_decode > l_current_pi->layno
                                        && l_current_pi->resno < p_tile->comps[l_current_pi->compno].minimum_num_resolutions) {
                                l_nb_bytes_read = 0;

                                first_pass_failed[l_current_pi->compno] = mi_FALSE;

                                if (! mi_t2_decode_packet(p_t2,p_tile,l_tcp,l_current_pi,l_current_data,&l_nb_bytes_read,p_max_len,l_pack_info, p_manager)) {
                                        mi_pi_destroy(l_pi,l_nb_pocs);
                                        mi_free(first_pass_failed);
                                        return mi_FALSE;
                                }

                                l_img_comp = &(l_image->comps[l_current_pi->compno]);
                                l_img_comp->resno_decoded = mi_uint_max(l_current_pi->resno, l_img_comp->resno_decoded);
                        }
                        else {
                                l_nb_bytes_read = 0;
                                if (! mi_t2_skip_packet(p_t2,p_tile,l_tcp,l_current_pi,l_current_data,&l_nb_bytes_read,p_max_len,l_pack_info, p_manager)) {
                                        mi_pi_destroy(l_pi,l_nb_pocs);
                                        mi_free(first_pass_failed);
                                        return mi_FALSE;
                                }
                        }

                        if (first_pass_failed[l_current_pi->compno]) {
                                l_img_comp = &(l_image->comps[l_current_pi->compno]);
                                if (l_img_comp->resno_decoded == 0)
                                        l_img_comp->resno_decoded = p_tile->comps[l_current_pi->compno].minimum_num_resolutions - 1;
                        }

                        l_current_data += l_nb_bytes_read;
                        p_max_len -= l_nb_bytes_read;
                }
                ++l_current_pi;

                mi_free(first_pass_failed);
        }

        /* don't forget to release pi */
        mi_pi_destroy(l_pi,l_nb_pocs);
        *p_data_read = (mi_UINT32)(l_current_data - p_src);
        return mi_TRUE;
}

/* ----------------------------------------------------------------------- */

/**
 * Creates a Tier 2 handle
 *
 * @param       p_image         Source or destination image
 * @param       p_cp            Image coding parameters.
 * @return              a new T2 handle if successful, NULL otherwise.
*/
mi_t2_t* mi_t2_create(mi_image_t *p_image, mi_cp_t *p_cp)
{
        /* create the t2 structure */
        mi_t2_t *l_t2 = (mi_t2_t*)mi_calloc(1,sizeof(mi_t2_t));
        if (!l_t2) {
                return NULL;
        }

        l_t2->image = p_image;
        l_t2->cp = p_cp;

        return l_t2;
}

void mi_t2_destroy(mi_t2_t *t2) {
        if(t2) {
                mi_free(t2);
        }
}

static mi_BOOL mi_t2_decode_packet(  mi_t2_t* p_t2,
                                mi_tcd_tile_t *p_tile,
                                mi_tcp_t *p_tcp,
                                mi_pi_iterator_t *p_pi,
                                mi_BYTE *p_src,
                                mi_UINT32 * p_data_read,
                                mi_UINT32 p_max_length,
                                mi_packet_info_t *p_pack_info,
                                mi_event_mgr_t *p_manager)
{
        mi_BOOL l_read_data;
        mi_UINT32 l_nb_bytes_read = 0;
        mi_UINT32 l_nb_total_bytes_read = 0;

        *p_data_read = 0;

        if (! mi_t2_read_packet_header(p_t2,p_tile,p_tcp,p_pi,&l_read_data,p_src,&l_nb_bytes_read,p_max_length,p_pack_info, p_manager)) {
                return mi_FALSE;
        }

        p_src += l_nb_bytes_read;
        l_nb_total_bytes_read += l_nb_bytes_read;
        p_max_length -= l_nb_bytes_read;

        /* we should read data for the packet */
        if (l_read_data) {
                l_nb_bytes_read = 0;

                if (! mi_t2_read_packet_data(p_t2,p_tile,p_pi,p_src,&l_nb_bytes_read,p_max_length,p_pack_info, p_manager)) {
                        return mi_FALSE;
                }

                l_nb_total_bytes_read += l_nb_bytes_read;
        }

        *p_data_read = l_nb_total_bytes_read;

        return mi_TRUE;
}

static mi_BOOL mi_t2_encode_packet(  mi_UINT32 tileno,
                                mi_tcd_tile_t * tile,
                                mi_tcp_t * tcp,
                                mi_pi_iterator_t *pi,
                                mi_BYTE *dest,
                                mi_UINT32 * p_data_written,
                                mi_UINT32 length,
                                mi_codestream_info_t *cstr_info)
{
        mi_UINT32 bandno, cblkno;
        mi_BYTE* c = dest;
        mi_UINT32 l_nb_bytes;
        mi_UINT32 compno = pi->compno;     /* component value */
        mi_UINT32 resno  = pi->resno;      /* resolution level value */
        mi_UINT32 precno = pi->precno;     /* precinct value */
        mi_UINT32 layno  = pi->layno;      /* quality layer value */
        mi_UINT32 l_nb_blocks;
        mi_tcd_band_t *band = 00;
        mi_tcd_cblk_enc_t* cblk = 00;
        mi_tcd_pass_t *pass = 00;

        mi_tcd_tilecomp_t *tilec = &tile->comps[compno];
        mi_tcd_resolution_t *res = &tilec->resolutions[resno];

        mi_bio_t *bio = 00;    /* BIO component */

        /* <SOP 0xff91> */
        if (tcp->csty & J2K_CP_CSTY_SOP) {//默认没有SOP
                c[0] = 255;
                c[1] = 145;
                c[2] = 0;
                c[3] = 4;
                c[4] = (tile->packno >> 8) & 0xff; /* packno is uint32_t */
                c[5] = tile->packno & 0xff;
                c += 6;
                length -= 6;
        }
        /* </SOP> */

        if (!layno) {//若没有质量层，则创建质量层
                band = res->bands;

                for(bandno = 0; bandno < res->numbands; ++bandno) {
                        mi_tcd_precinct_t *prc = &band->precincts[precno];

                        mi_tgt_reset(prc->incltree);
                        mi_tgt_reset(prc->imsbtree);

                        l_nb_blocks = prc->cw * prc->ch;
                        for     (cblkno = 0; cblkno < l_nb_blocks; ++cblkno) {
                                cblk = &prc->cblks.enc[cblkno];

                                cblk->numpasses = 0;
                                mi_tgt_setvalue(prc->imsbtree, cblkno, band->numbps - (mi_INT32)cblk->numbps);
                        }
                        ++band;
                }
        }

        bio = mi_bio_create();
        if (!bio) {
                /* FIXME event manager error callback */
                return mi_FALSE;
        }
        mi_bio_init_enc(bio, c, length);
        mi_bio_write(bio, 1, 1);           /* Empty header bit */

        /* Writing Packet header */
        band = res->bands;
        for (bandno = 0; bandno < res->numbands; ++bandno)      {
                mi_tcd_precinct_t *prc = &band->precincts[precno];

                l_nb_blocks = prc->cw * prc->ch;
                cblk = prc->cblks.enc;

                for (cblkno = 0; cblkno < l_nb_blocks; ++cblkno) {
                        mi_tcd_layer_t *layer = &cblk->layers[layno];

                        if (!cblk->numpasses && layer->numpasses) {
                                mi_tgt_setvalue(prc->incltree, cblkno, (mi_INT32)layno);
                        }

                        ++cblk;
                }

                cblk = prc->cblks.enc;
                for (cblkno = 0; cblkno < l_nb_blocks; cblkno++) {
                        mi_tcd_layer_t *layer = &cblk->layers[layno];
                        mi_UINT32 increment = 0;
                        mi_UINT32 nump = 0;
                        mi_UINT32 len = 0, passno;
                        mi_UINT32 l_nb_passes;

                        /* cblk inclusion bits */
                        if (!cblk->numpasses) {
                                mi_tgt_encode(bio, prc->incltree, cblkno, (mi_INT32)(layno + 1));
                        } else {
                                mi_bio_write(bio, layer->numpasses != 0, 1);
                        }

                        /* if cblk not included, go to the next cblk  */
                        if (!layer->numpasses) {
                                ++cblk;
                                continue;
                        }

                        /* if first instance of cblk --> zero bit-planes information */
                        if (!cblk->numpasses) {
                                cblk->numlenbits = 3;
                                mi_tgt_encode(bio, prc->imsbtree, cblkno, 999);
                        }

                        /* number of coding passes included */
                        mi_t2_putnumpasses(bio, layer->numpasses);
                        l_nb_passes = cblk->numpasses + layer->numpasses;
                        pass = cblk->passes +  cblk->numpasses;

                        /* computation of the increase of the length indicator and insertion in the header     */
                        for (passno = cblk->numpasses; passno < l_nb_passes; ++passno) {
                                ++nump;
                                len += pass->len;

                                if (pass->term || passno == (cblk->numpasses + layer->numpasses) - 1) {
                                  increment = (mi_UINT32)mi_int_max((mi_INT32)increment, mi_int_floorlog2((mi_INT32)len) + 1
                                    - ((mi_INT32)cblk->numlenbits + mi_int_floorlog2((mi_INT32)nump)));
                                        len = 0;
                                        nump = 0;
                                }

                                ++pass;
                        }
                        mi_t2_putcommacode(bio, (mi_INT32)increment);

                        /* computation of the new Length indicator */
                        cblk->numlenbits += increment;

                        pass = cblk->passes +  cblk->numpasses;
                        /* insertion of the codeword segment length */
                        for (passno = cblk->numpasses; passno < l_nb_passes; ++passno) {
                                nump++;
                                len += pass->len;

                                if (pass->term || passno == (cblk->numpasses + layer->numpasses) - 1) {
                                        mi_bio_write(bio, (mi_UINT32)len, cblk->numlenbits + (mi_UINT32)mi_int_floorlog2((mi_INT32)nump));
                                        len = 0;
                                        nump = 0;
                                }
                                ++pass;
                        }

                        ++cblk;
                }

                ++band;
        }

        if (!mi_bio_flush(bio)) {
                mi_bio_destroy(bio);
                return mi_FALSE;               /* modified to eliminate longjmp !! */
        }

        l_nb_bytes = (mi_UINT32)mi_bio_numbytes(bio);
        c += l_nb_bytes;
        length -= l_nb_bytes;

        mi_bio_destroy(bio);

        /* <EPH 0xff92> */
        if (tcp->csty & J2K_CP_CSTY_EPH) {
                c[0] = 255;
                c[1] = 146;
                c += 2;
                length -= 2;
        }
        /* </EPH> */

        /* << INDEX */
        /* End of packet header position. Currently only represents the distance to start of packet
           Will be updated later by incrementing with packet start value*/
        if(cstr_info && cstr_info->index_write) {
                mi_packet_info_t *info_PK = &cstr_info->tile[tileno].packet[cstr_info->packno];
                info_PK->end_ph_pos = (mi_INT32)(c - dest);
        }
        /* INDEX >> */

        /* Writing the packet body */
        band = res->bands;
        for (bandno = 0; bandno < res->numbands; bandno++) {
                mi_tcd_precinct_t *prc = &band->precincts[precno];

                l_nb_blocks = prc->cw * prc->ch;
                cblk = prc->cblks.enc;

                for (cblkno = 0; cblkno < l_nb_blocks; ++cblkno) {
                        mi_tcd_layer_t *layer = &cblk->layers[layno];

                        if (!layer->numpasses) {
                                ++cblk;
                                continue;
                        }

                        if (layer->len > length) {
                                return mi_FALSE;
                        }

                        memcpy(c, layer->data, layer->len);
                        cblk->numpasses += layer->numpasses;
                        c += layer->len;
                        length -= layer->len;

                        /* << INDEX */
                        if(cstr_info && cstr_info->index_write) {
                                mi_packet_info_t *info_PK = &cstr_info->tile[tileno].packet[cstr_info->packno];
                                info_PK->disto += layer->disto;
                                if (cstr_info->D_max < info_PK->disto) {
                                        cstr_info->D_max = info_PK->disto;
                                }
                        }

                        ++cblk;
                        /* INDEX >> */
                }
                ++band;
        }

        assert( c >= dest );
        * p_data_written += (mi_UINT32)(c - dest);

        return mi_TRUE;
}

static mi_BOOL mi_t2_skip_packet( mi_t2_t* p_t2,
                                    mi_tcd_tile_t *p_tile,
                                    mi_tcp_t *p_tcp,
                                    mi_pi_iterator_t *p_pi,
                                    mi_BYTE *p_src,
                                    mi_UINT32 * p_data_read,
                                    mi_UINT32 p_max_length,
                                    mi_packet_info_t *p_pack_info,
                                    mi_event_mgr_t *p_manager)
{
        mi_BOOL l_read_data;
        mi_UINT32 l_nb_bytes_read = 0;
        mi_UINT32 l_nb_total_bytes_read = 0;

        *p_data_read = 0;

        if (! mi_t2_read_packet_header(p_t2,p_tile,p_tcp,p_pi,&l_read_data,p_src,&l_nb_bytes_read,p_max_length,p_pack_info, p_manager)) {
                return mi_FALSE;
        }

        p_src += l_nb_bytes_read;
        l_nb_total_bytes_read += l_nb_bytes_read;
        p_max_length -= l_nb_bytes_read;

        /* we should read data for the packet */
        if (l_read_data) {
                l_nb_bytes_read = 0;

                if (! mi_t2_skip_packet_data(p_t2,p_tile,p_pi,&l_nb_bytes_read,p_max_length,p_pack_info, p_manager)) {
                        return mi_FALSE;
                }

                l_nb_total_bytes_read += l_nb_bytes_read;
        }
        *p_data_read = l_nb_total_bytes_read;

        return mi_TRUE;
}


static mi_BOOL mi_t2_read_packet_header( mi_t2_t* p_t2,
                                    mi_tcd_tile_t *p_tile,
                                    mi_tcp_t *p_tcp,
                                    mi_pi_iterator_t *p_pi,
                                    mi_BOOL * p_is_data_present,
                                    mi_BYTE *p_src_data,
                                    mi_UINT32 * p_data_read,
                                    mi_UINT32 p_max_length,
                                    mi_packet_info_t *p_pack_info,
                                    mi_event_mgr_t *p_manager)

{
        /* loop */
        mi_UINT32 bandno, cblkno;
        mi_UINT32 l_nb_code_blocks;
        mi_UINT32 l_remaining_length;
        mi_UINT32 l_header_length;
        mi_UINT32 * l_modified_length_ptr = 00;
        mi_BYTE *l_current_data = p_src_data;
        mi_cp_t *l_cp = p_t2->cp;
        mi_bio_t *l_bio = 00;  /* BIO component */
        mi_tcd_band_t *l_band = 00;
        mi_tcd_cblk_dec_t* l_cblk = 00;
        mi_tcd_resolution_t* l_res = &p_tile->comps[p_pi->compno].resolutions[p_pi->resno];

        mi_BYTE *l_header_data = 00;
        mi_BYTE **l_header_data_start = 00;

        mi_UINT32 l_present;

        if (p_pi->layno == 0) {
                l_band = l_res->bands;

                /* reset tagtrees */
                for (bandno = 0; bandno < l_res->numbands; ++bandno) {
                        if ( ! ((l_band->x1-l_band->x0 == 0)||(l_band->y1-l_band->y0 == 0)) ) {
                                mi_tcd_precinct_t *l_prc = &l_band->precincts[p_pi->precno];
                                if (!(p_pi->precno < (l_band->precincts_data_size / sizeof(mi_tcd_precinct_t)))) {
                                        mi_event_msg(p_manager, EVT_ERROR, "Invalid precinct\n");
                                        return mi_FALSE;
                                }
													
													
                                mi_tgt_reset(l_prc->incltree);
                                mi_tgt_reset(l_prc->imsbtree);
                                l_cblk = l_prc->cblks.dec;

                                l_nb_code_blocks = l_prc->cw * l_prc->ch;
                                for (cblkno = 0; cblkno < l_nb_code_blocks; ++cblkno) {
                                        l_cblk->numsegs = 0;
                                        l_cblk->real_num_segs = 0;
                                        ++l_cblk;
                                }
                        }

                        ++l_band;
                }
        }

        /* SOP markers */

        if (p_tcp->csty & J2K_CP_CSTY_SOP) {
                if (p_max_length < 6) {
												mi_event_msg(p_manager, EVT_WARNING, "Not enough space for expected SOP marker\n");
                } else if ((*l_current_data) != 0xff || (*(l_current_data + 1) != 0x91)) {
                        mi_event_msg(p_manager, EVT_WARNING, "Expected SOP marker\n");
                } else {
                        l_current_data += 6;
                }

                /** TODO : check the Nsop value */
        }

        /*
        When the marker PPT/PPM is used the packet header are store in PPT/PPM marker
        This part deal with this caracteristic
        step 1: Read packet header in the saved structure
        step 2: Return to codestream for decoding
        */

        l_bio = mi_bio_create();
        if (! l_bio) {
                return mi_FALSE;
        }

        if (l_cp->ppm == 1) { /* PPM */
                l_header_data_start = &l_cp->ppm_data;
                l_header_data = *l_header_data_start;
                l_modified_length_ptr = &(l_cp->ppm_len);

        }
        else if (p_tcp->ppt == 1) { /* PPT */
                l_header_data_start = &(p_tcp->ppt_data);
                l_header_data = *l_header_data_start;
                l_modified_length_ptr = &(p_tcp->ppt_len);
        }
        else {  /* Normal Case */
                l_header_data_start = &(l_current_data);
                l_header_data = *l_header_data_start;
                l_remaining_length = (mi_UINT32)(p_src_data+p_max_length-l_header_data);
                l_modified_length_ptr = &(l_remaining_length);
        }

        mi_bio_init_dec(l_bio, l_header_data,*l_modified_length_ptr);

        l_present = mi_bio_read(l_bio, 1);
        JAS_FPRINTF(stderr, "present=%d \n", l_present );
        if (!l_present) {
            /* TODO MSD: no test to control the output of this function*/
                mi_bio_inalign(l_bio);
                l_header_data += mi_bio_numbytes(l_bio);
                mi_bio_destroy(l_bio);

                /* EPH markers */
                if (p_tcp->csty & J2K_CP_CSTY_EPH) {
                        if ((*l_modified_length_ptr - (mi_UINT32)(l_header_data - *l_header_data_start)) < 2U) {
                                mi_event_msg(p_manager, EVT_WARNING, "Not enough space for expected EPH marker\n");
                        } else if ((*l_header_data) != 0xff || (*(l_header_data + 1) != 0x92)) {
                                mi_event_msg(p_manager, EVT_WARNING, "Expected EPH marker\n");
                        } else {
                                l_header_data += 2;
                        }
                }

                l_header_length = (mi_UINT32)(l_header_data - *l_header_data_start);
                *l_modified_length_ptr -= l_header_length;
                *l_header_data_start += l_header_length;

                /* << INDEX */
                /* End of packet header position. Currently only represents the distance to start of packet
                   Will be updated later by incrementing with packet start value */
                if (p_pack_info) {
                        p_pack_info->end_ph_pos = (mi_INT32)(l_current_data - p_src_data);
                }
                /* INDEX >> */

                * p_is_data_present = mi_FALSE;
                *p_data_read = (mi_UINT32)(l_current_data - p_src_data);
                return mi_TRUE;
        }

        l_band = l_res->bands;
        for (bandno = 0; bandno < l_res->numbands; ++bandno) {
                mi_tcd_precinct_t *l_prc = &(l_band->precincts[p_pi->precno]);

                if ((l_band->x1-l_band->x0 == 0)||(l_band->y1-l_band->y0 == 0)) {
                        ++l_band;
                        continue;
                }

                l_nb_code_blocks = l_prc->cw * l_prc->ch;
                l_cblk = l_prc->cblks.dec;
                for (cblkno = 0; cblkno < l_nb_code_blocks; cblkno++) {
                        mi_UINT32 l_included,l_increment, l_segno;
                        mi_INT32 n;

                        /* if cblk not yet included before --> inclusion tagtree */
                        if (!l_cblk->numsegs) {
                                l_included = mi_tgt_decode(l_bio, l_prc->incltree, cblkno, (mi_INT32)(p_pi->layno + 1));
                                /* else one bit */
                        }
                        else {
                                l_included = mi_bio_read(l_bio, 1);
                        }

                        /* if cblk not included */
                        if (!l_included) {
                                l_cblk->numnewpasses = 0;
                                ++l_cblk;
                                JAS_FPRINTF(stderr, "included=%d \n", l_included);
                                continue;
                        }

                        /* if cblk not yet included --> zero-bitplane tagtree */
                        if (!l_cblk->numsegs) {
                                mi_UINT32 i = 0;

                                while (!mi_tgt_decode(l_bio, l_prc->imsbtree, cblkno, (mi_INT32)i)) {
                                        ++i;
                                }

                                l_cblk->numbps = (mi_UINT32)l_band->numbps + 1 - i;
                                l_cblk->numlenbits = 3;
                        }

                        /* number of coding passes */
                        l_cblk->numnewpasses = mi_t2_getnumpasses(l_bio);
                        l_increment = mi_t2_getcommacode(l_bio);

                        /* length indicator increment */
                        l_cblk->numlenbits += l_increment;
                        l_segno = 0;

                        if (!l_cblk->numsegs) {
                                if (! mi_t2_init_seg(l_cblk, l_segno, p_tcp->tccps[p_pi->compno].cblksty, 1)) {
                                        mi_bio_destroy(l_bio);
                                        return mi_FALSE;
                                }
                        }
                        else {
                                l_segno = l_cblk->numsegs - 1;
                                if (l_cblk->segs[l_segno].numpasses == l_cblk->segs[l_segno].maxpasses) {
                                        ++l_segno;
                                        if (! mi_t2_init_seg(l_cblk, l_segno, p_tcp->tccps[p_pi->compno].cblksty, 0)) {
                                                mi_bio_destroy(l_bio);
                                                return mi_FALSE;
                                        }
                                }
                        }
                        n = (mi_INT32)l_cblk->numnewpasses;

                        do {
                                l_cblk->segs[l_segno].numnewpasses = (mi_UINT32)mi_int_min((mi_INT32)(l_cblk->segs[l_segno].maxpasses - l_cblk->segs[l_segno].numpasses), n);
                                l_cblk->segs[l_segno].newlen = mi_bio_read(l_bio, l_cblk->numlenbits + mi_uint_floorlog2(l_cblk->segs[l_segno].numnewpasses));
                                        JAS_FPRINTF(stderr, "included=%d numnewpasses=%d increment=%d len=%d \n", l_included, l_cblk->segs[l_segno].numnewpasses, l_increment, l_cblk->segs[l_segno].newlen );

                                n -= (mi_INT32)l_cblk->segs[l_segno].numnewpasses;
                                if (n > 0) {
                                        ++l_segno;

                                        if (! mi_t2_init_seg(l_cblk, l_segno, p_tcp->tccps[p_pi->compno].cblksty, 0)) {
                                                mi_bio_destroy(l_bio);
                                                return mi_FALSE;
                                        }
                                }
                        } while (n > 0);

                        ++l_cblk;
                }

                ++l_band;
        }

        if (!mi_bio_inalign(l_bio)) {
                mi_bio_destroy(l_bio);
                return mi_FALSE;
        }

        l_header_data += mi_bio_numbytes(l_bio);
        mi_bio_destroy(l_bio);

        /* EPH markers */
        if (p_tcp->csty & J2K_CP_CSTY_EPH) {
                if ((*l_modified_length_ptr - (mi_UINT32)(l_header_data - *l_header_data_start)) < 2U) {
                        mi_event_msg(p_manager, EVT_WARNING, "Not enough space for expected EPH marker\n");
                } else if ((*l_header_data) != 0xff || (*(l_header_data + 1) != 0x92)) {
                        mi_event_msg(p_manager, EVT_WARNING, "Expected EPH marker\n");
                } else {
                        l_header_data += 2;
                }
        }

        l_header_length = (mi_UINT32)(l_header_data - *l_header_data_start);
        JAS_FPRINTF( stderr, "hdrlen=%d \n", l_header_length );
        JAS_FPRINTF( stderr, "packet body\n");
        *l_modified_length_ptr -= l_header_length;
        *l_header_data_start += l_header_length;

        /* << INDEX */
        /* End of packet header position. Currently only represents the distance to start of packet
         Will be updated later by incrementing with packet start value */
        if (p_pack_info) {
                p_pack_info->end_ph_pos = (mi_INT32)(l_current_data - p_src_data);
        }
        /* INDEX >> */

        *p_is_data_present = mi_TRUE;
        *p_data_read = (mi_UINT32)(l_current_data - p_src_data);

        return mi_TRUE;
}

static mi_BOOL mi_t2_read_packet_data(   mi_t2_t* p_t2,
                                    mi_tcd_tile_t *p_tile,
                                    mi_pi_iterator_t *p_pi,
                                    mi_BYTE *p_src_data,
                                    mi_UINT32 * p_data_read,
                                    mi_UINT32 p_max_length,
                                    mi_packet_info_t *pack_info,
                                    mi_event_mgr_t* p_manager)
{
        mi_UINT32 bandno, cblkno;
        mi_UINT32 l_nb_code_blocks;
        mi_BYTE *l_current_data = p_src_data;
        mi_tcd_band_t *l_band = 00;
        mi_tcd_cblk_dec_t* l_cblk = 00;
        mi_tcd_resolution_t* l_res = &p_tile->comps[p_pi->compno].resolutions[p_pi->resno];

        mi_ARG_NOT_USED(p_t2);
        mi_ARG_NOT_USED(pack_info);

        l_band = l_res->bands;
        for (bandno = 0; bandno < l_res->numbands; ++bandno) {
                mi_tcd_precinct_t *l_prc = &l_band->precincts[p_pi->precno];

                if ((l_band->x1-l_band->x0 == 0)||(l_band->y1-l_band->y0 == 0)) {
                        ++l_band;
                        continue;
                }

                l_nb_code_blocks = l_prc->cw * l_prc->ch;
                l_cblk = l_prc->cblks.dec;

                for (cblkno = 0; cblkno < l_nb_code_blocks; ++cblkno) {
                        mi_tcd_seg_t *l_seg = 00;

                        if (!l_cblk->numnewpasses) {
                                /* nothing to do */
                                ++l_cblk;
                                continue;
                        }

                        if (!l_cblk->numsegs) {
                                l_seg = l_cblk->segs;
                                ++l_cblk->numsegs;
                                l_cblk->data_current_size = 0;
                        }
                        else {
                                l_seg = &l_cblk->segs[l_cblk->numsegs - 1];

                                if (l_seg->numpasses == l_seg->maxpasses) {
                                        ++l_seg;
                                        ++l_cblk->numsegs;
                                }
                        }

                        do {
                                /* Check possible overflow (on l_current_data only, assumes input args already checked) then size */
                                if ((((mi_SIZE_T)l_current_data + (mi_SIZE_T)l_seg->newlen) < (mi_SIZE_T)l_current_data) || (l_current_data + l_seg->newlen > p_src_data + p_max_length)) {
                                        mi_event_msg(p_manager, EVT_ERROR, "read: segment too long (%d) with max (%d) for codeblock %d (p=%d, b=%d, r=%d, c=%d)\n",
																								l_seg->newlen, p_max_length, cblkno, p_pi->precno, bandno, p_pi->resno, p_pi->compno);
                                        return mi_FALSE;
                                }
                                /* Check possible overflow on size */
                                if ((l_cblk->data_current_size + l_seg->newlen) < l_cblk->data_current_size) {
                                        mi_event_msg(p_manager, EVT_ERROR, "read: segment too long (%d) with current size (%d > %d) for codeblock %d (p=%d, b=%d, r=%d, c=%d)\n",
                                                l_seg->newlen, l_cblk->data_current_size, 0xFFFFFFFF - l_seg->newlen, cblkno, p_pi->precno, bandno, p_pi->resno, p_pi->compno);
                                        return mi_FALSE;
                                }
                                /* Check if the cblk->data have allocated enough memory */
                                if ((l_cblk->data_current_size + l_seg->newlen) > l_cblk->data_max_size) {
                                    mi_BYTE* new_cblk_data = (mi_BYTE*) mi_realloc(l_cblk->data, l_cblk->data_current_size + l_seg->newlen);
                                    if(! new_cblk_data) {
                                        mi_free(l_cblk->data);
                                        l_cblk->data = NULL;
                                        l_cblk->data_max_size = 0;
                                        /* mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to realloc code block cata!\n"); */
                                        return mi_FALSE;
                                    }
                                    l_cblk->data_max_size = l_cblk->data_current_size + l_seg->newlen;
                                    l_cblk->data = new_cblk_data;
                                }
                               
                                memcpy(l_cblk->data + l_cblk->data_current_size, l_current_data, l_seg->newlen);

                                if (l_seg->numpasses == 0) {
                                        l_seg->data = &l_cblk->data;
                                        l_seg->dataindex = l_cblk->data_current_size;
                                }

                                l_current_data += l_seg->newlen;
                                l_seg->numpasses += l_seg->numnewpasses;
                                l_cblk->numnewpasses -= l_seg->numnewpasses;

                                l_seg->real_num_passes = l_seg->numpasses;
                                l_cblk->data_current_size += l_seg->newlen;
                                l_seg->len += l_seg->newlen;

                                if (l_cblk->numnewpasses > 0) {
                                        ++l_seg;
                                        ++l_cblk->numsegs;
                                }
                        } while (l_cblk->numnewpasses > 0);

                        l_cblk->real_num_segs = l_cblk->numsegs;
                        ++l_cblk;
                } /* next code_block */

                ++l_band;
        }

        *(p_data_read) = (mi_UINT32)(l_current_data - p_src_data);


        return mi_TRUE;
}

static mi_BOOL mi_t2_skip_packet_data(   mi_t2_t* p_t2,
                                    mi_tcd_tile_t *p_tile,
                                    mi_pi_iterator_t *p_pi,
                                    mi_UINT32 * p_data_read,
                                    mi_UINT32 p_max_length,
                                    mi_packet_info_t *pack_info,
                                    mi_event_mgr_t *p_manager)
{
        mi_UINT32 bandno, cblkno;
        mi_UINT32 l_nb_code_blocks;
        mi_tcd_band_t *l_band = 00;
        mi_tcd_cblk_dec_t* l_cblk = 00;
        mi_tcd_resolution_t* l_res = &p_tile->comps[p_pi->compno].resolutions[p_pi->resno];

        mi_ARG_NOT_USED(p_t2);
        mi_ARG_NOT_USED(pack_info);

        *p_data_read = 0;
        l_band = l_res->bands;

        for (bandno = 0; bandno < l_res->numbands; ++bandno) {
                mi_tcd_precinct_t *l_prc = &l_band->precincts[p_pi->precno];

                if ((l_band->x1-l_band->x0 == 0)||(l_band->y1-l_band->y0 == 0)) {
                        ++l_band;
                        continue;
                }

                l_nb_code_blocks = l_prc->cw * l_prc->ch;
                l_cblk = l_prc->cblks.dec;

                for (cblkno = 0; cblkno < l_nb_code_blocks; ++cblkno) {
                        mi_tcd_seg_t *l_seg = 00;

                        if (!l_cblk->numnewpasses) {
                                /* nothing to do */
                                ++l_cblk;
                                continue;
                        }

                        if (!l_cblk->numsegs) {
                                l_seg = l_cblk->segs;
                                ++l_cblk->numsegs;
                                l_cblk->data_current_size = 0;
                        }
                        else {
                                l_seg = &l_cblk->segs[l_cblk->numsegs - 1];

                                if (l_seg->numpasses == l_seg->maxpasses) {
                                        ++l_seg;
                                        ++l_cblk->numsegs;
                                }
                        }

                        do {
                                /* Check possible overflow then size */
                                if (((*p_data_read + l_seg->newlen) < (*p_data_read)) || ((*p_data_read + l_seg->newlen) > p_max_length)) {
                                        mi_event_msg(p_manager, EVT_ERROR, "skip: segment too long (%d) with max (%d) for codeblock %d (p=%d, b=%d, r=%d, c=%d)\n",
                                                l_seg->newlen, p_max_length, cblkno, p_pi->precno, bandno, p_pi->resno, p_pi->compno);
                                        return mi_FALSE;
                                }
                                        JAS_FPRINTF(stderr, "p_data_read (%d) newlen (%d) \n", *p_data_read, l_seg->newlen );
                                *(p_data_read) += l_seg->newlen;

                                l_seg->numpasses += l_seg->numnewpasses;
                                l_cblk->numnewpasses -= l_seg->numnewpasses;
                                if (l_cblk->numnewpasses > 0)
                                {
                                        ++l_seg;
                                        ++l_cblk->numsegs;
                                }
                        } while (l_cblk->numnewpasses > 0);

                        ++l_cblk;
                }

                ++l_band;
        }

        return mi_TRUE;
}


static mi_BOOL mi_t2_init_seg(   mi_tcd_cblk_dec_t* cblk,
                            mi_UINT32 index, 
                            mi_UINT32 cblksty, 
                            mi_UINT32 first)
{
        mi_tcd_seg_t* seg = 00;
        mi_UINT32 l_nb_segs = index + 1;

        if (l_nb_segs > cblk->m_current_max_segs) {
                mi_tcd_seg_t* new_segs;
                cblk->m_current_max_segs += mi_J2K_DEFAULT_NB_SEGS;

                new_segs = (mi_tcd_seg_t*) mi_realloc(cblk->segs, cblk->m_current_max_segs * sizeof(mi_tcd_seg_t));
                if(! new_segs) {
                        mi_free(cblk->segs);
                        cblk->segs = NULL;
                        cblk->m_current_max_segs = 0;
                        /* mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to initialize segment %d\n", l_nb_segs); */
                        return mi_FALSE;
                }
                cblk->segs = new_segs;
        }

        seg = &cblk->segs[index];
        memset(seg,0,sizeof(mi_tcd_seg_t));

        if (cblksty & J2K_CCP_CBLKSTY_TERMALL) {
                seg->maxpasses = 1;
        }
        else if (cblksty & J2K_CCP_CBLKSTY_LAZY) {
                if (first) {
                        seg->maxpasses = 10;
                } else {
                        seg->maxpasses = (((seg - 1)->maxpasses == 1) || ((seg - 1)->maxpasses == 10)) ? 2 : 1;
                }
        } else {
                seg->maxpasses = 109;
        }

        return mi_TRUE;
}
