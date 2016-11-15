
#ifndef __T2_H
#define __T2_H
/**
@file t2.h
@brief Implementation of a tier-2 coding (packetization of code-block data) (T2)

*/

/** @defgroup T2 T2 - Implementation of a tier-2 coding */
/*@{*/

/**
Tier-2 coding
*/
typedef struct mi_t2 {

	/** Encoding: pointer to the src image. Decoding: pointer to the dst image. */
	mi_image_t *image;
	/** pointer to the image coding parameters */
	mi_cp_t *cp;
} mi_t2_t;

/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */

/**
Encode the packets of a tile to a destination buffer
@param t2               T2 handle
@param tileno           number of the tile encoded
@param tile             the tile for which to write the packets
@param maxlayers        maximum number of layers
@param dest             the destination buffer
@param p_data_written   FIXME DOC
@param len              the length of the destination buffer
@param cstr_info        Codestream information structure
@param tpnum            Tile part number of the current tile
@param tppos            The position of the tile part flag in the progression order
@param pino             FIXME DOC
@param t2_mode          If == 0 In Threshold calculation ,If == 1 Final pass
*/
mi_BOOL mi_t2_encode_packets(	mi_t2_t* t2,
								mi_UINT32 tileno,
								mi_tcd_tile_t *tile,
								mi_UINT32 maxlayers,
								mi_BYTE *dest,
								mi_UINT32 * p_data_written,
								mi_UINT32 len,
								mi_codestream_info_t *cstr_info,
								mi_UINT32 tpnum,
								mi_INT32 tppos,
								mi_UINT32 pino,
								J2K_T2_MODE t2_mode);

/**
Decode the packets of a tile from a source buffer
@param t2 T2 handle
@param tileno number that identifies the tile for which to decode the packets
@param tile tile for which to decode the packets
@param src         FIXME DOC
@param p_data_read the source buffer
@param len length of the source buffer
@param cstr_info   FIXME DOC

@return FIXME DOC
 */
mi_BOOL mi_t2_decode_packets(	mi_t2_t *t2,
                                mi_UINT32 tileno,
                                mi_tcd_tile_t *tile,
                                mi_BYTE *src,
                                mi_UINT32 * p_data_read,
                                mi_UINT32 len,
                                mi_codestream_index_t *cstr_info,
                                mi_event_mgr_t *p_manager);

/**
 * Creates a Tier 2 handle
 *
 * @param	p_image		Source or destination image
 * @param	p_cp		Image coding parameters.
 * @return		a new T2 handle if successful, NULL otherwise.
*/
mi_t2_t* mi_t2_create(mi_image_t *p_image, mi_cp_t *p_cp);

/**
Destroy a T2 handle
@param t2 T2 handle to destroy
*/
void mi_t2_destroy(mi_t2_t *t2);

/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/

#endif /* __T2_H */
