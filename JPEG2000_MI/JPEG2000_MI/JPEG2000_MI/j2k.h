
#ifndef __J2K_H
#define __J2K_H
/**
@file j2k.h
@brief The JPEG-2000 Codestream Reader/Writer (J2K)

The functions in J2K.C have for goal to read/write the several parts of the codestream: markers and data.
*/

/** @defgroup J2K J2K - JPEG-2000 codestream reader/writer */
/*@{*/

#define J2K_CP_CSTY_PRT 0x01
#define J2K_CP_CSTY_SOP 0x02
#define J2K_CP_CSTY_EPH 0x04
#define J2K_CCP_CSTY_PRT 0x01
#define J2K_CCP_CBLKSTY_LAZY 0x01     /**< Selective arithmetic coding bypass */
#define J2K_CCP_CBLKSTY_RESET 0x02    /**< Reset context probabilities on coding pass boundaries */
#define J2K_CCP_CBLKSTY_TERMALL 0x04  /**< Termination on each coding pass */
#define J2K_CCP_CBLKSTY_VSC 0x08      /**< Vertically stripe causal context */
#define J2K_CCP_CBLKSTY_PTERM 0x10    /**< Predictable termination */
#define J2K_CCP_CBLKSTY_SEGSYM 0x20   /**< Segmentation symbols are used */
#define J2K_CCP_QNTSTY_NOQNT 0
#define J2K_CCP_QNTSTY_SIQNT 1
#define J2K_CCP_QNTSTY_SEQNT 2

#define mi_J2K_DEFAULT_CBLK_DATA_SIZE 8192

/* ----------------------------------------------------------------------- */

#define J2K_MS_SOC 0xff4f	/**< SOC marker value */
#define J2K_MS_SOT 0xff90	/**< SOT marker value */
#define J2K_MS_SOD 0xff93	/**< SOD marker value */
#define J2K_MS_EOC 0xffd9	/**< EOC marker value */
#define J2K_MS_SIZ 0xff51	/**< SIZ marker value */
#define J2K_MS_COD 0xff52	/**< COD marker value */
#define J2K_MS_COC 0xff53	/**< COC marker value */
#define J2K_MS_RGN 0xff5e	/**< RGN marker value */
#define J2K_MS_QCD 0xff5c	/**< QCD marker value */
#define J2K_MS_QCC 0xff5d	/**< QCC marker value */
#define J2K_MS_POC 0xff5f	/**< POC marker value */
#define J2K_MS_TLM 0xff55	/**< TLM marker value */
#define J2K_MS_PLM 0xff57	/**< PLM marker value */
#define J2K_MS_PLT 0xff58	/**< PLT marker value */
#define J2K_MS_PPM 0xff60	/**< PPM marker value */
#define J2K_MS_PPT 0xff61	/**< PPT marker value */
#define J2K_MS_SOP 0xff91	/**< SOP marker value */
#define J2K_MS_EPH 0xff92	/**< EPH marker value */
#define J2K_MS_CRG 0xff63	/**< CRG marker value */
#define J2K_MS_COM 0xff64	/**< COM marker value */
#define J2K_MS_CBD 0xff78	/**< CBD marker value */
#define J2K_MS_MCC 0xff75	/**< MCC marker value */
#define J2K_MS_MCT 0xff74	/**< MCT marker value */
#define J2K_MS_MCO 0xff77	/**< MCO marker value */

#define J2K_MS_UNK 0		/**< UNKNOWN marker value */

/* ----------------------------------------------------------------------- */

/**
 * Values that specify the status of the decoding process when decoding the main header.
 * These values may be combined with a | operator.
 * */
typedef enum J2K_STATUS {
	J2K_STATE_NONE  =  0x0000, /**< a SOC marker is expected */
	J2K_STATE_MHSOC  = 0x0001, /**< a SOC marker is expected */
	J2K_STATE_MHSIZ  = 0x0002, /**< a SIZ marker is expected */
	J2K_STATE_MH     = 0x0004, /**< the decoding process is in the main header */
	J2K_STATE_TPHSOT = 0x0008, /**< the decoding process is in a tile part header and expects a SOT marker */
	J2K_STATE_TPH    = 0x0010, /**< the decoding process is in a tile part header */
	J2K_STATE_MT     = 0x0020, /**< the EOC marker has just been read */
	J2K_STATE_NEOC   = 0x0040, /**< the decoding process must not expect a EOC marker because the codestream is truncated */

	J2K_STATE_EOC	 = 0x0100, /**< the decoding process has encountered the EOC marker */
	J2K_STATE_ERR    = 0x8000  /**< the decoding process has encountered an error (FIXME warning V1 = 0x0080)*/
} J2K_STATUS;

/**
 * Type of elements storing in the MCT data
 */
typedef enum MCT_ELEMENT_TYPE
{
	MCT_TYPE_INT16 = 0,		/** MCT data is stored as signed shorts*/
	MCT_TYPE_INT32 = 1,		/** MCT data is stored as signed integers*/
	MCT_TYPE_FLOAT = 2,		/** MCT data is stored as floats*/
	MCT_TYPE_DOUBLE = 3		/** MCT data is stored as doubles*/
} J2K_MCT_ELEMENT_TYPE;

/**
 * Type of MCT array
 */
typedef enum MCT_ARRAY_TYPE
{
	MCT_TYPE_DEPENDENCY = 0,
	MCT_TYPE_DECORRELATION = 1,
	MCT_TYPE_OFFSET = 2
} J2K_MCT_ARRAY_TYPE;

/* ----------------------------------------------------------------------- */

/** 
T2 encoding mode 
*/
typedef enum T2_MODE {
	THRESH_CALC = 0,	/** Function called in Rate allocation process*/
	FINAL_PASS = 1		/** Function called in Tier 2 process*/
}J2K_T2_MODE;

/**
 * Quantization stepsize
 */
typedef struct mi_stepsize {
	/** exponent */
	mi_INT32 expn;
	/** mantissa */
	mi_INT32 mant;
} mi_stepsize_t;

/**
Tile-component coding parameters
*/
typedef struct mi_tccp
{
	/** coding style */
	mi_UINT32 csty;
	/** number of resolutions（我猜这指的是小波分解次数） */
	mi_UINT32 numresolutions;
	/** code-blocks width（代码块宽的位数） */
	mi_UINT32 cblkw;
	/** code-blocks height （代码块高的位数）*/
	mi_UINT32 cblkh;
	/** code-block coding style */
	mi_UINT32 cblksty;
	/** discrete wavelet transform identifier （这个意思应该是：1——5/3小波，0——9/7小波）*/
	mi_UINT32 qmfbid;
	/** quantisation style */
	mi_UINT32 qntsty;
	/** stepsizes used for quantization */
	mi_stepsize_t stepsizes[mi_J2K_MAXBANDS];
	/** number of guard bits */
	mi_UINT32 numgbits;
	/** Region Of Interest shift */
	mi_INT32 roishift;
	/** precinct width （这个宽度貌似指的是宽度的位数）*/
	mi_UINT32 prcw[mi_J2K_MAXRLVLS];
	/** precinct height （这个高度貌似指的是高度的位数）*/
	mi_UINT32 prch[mi_J2K_MAXRLVLS];
	/** the dc_level_shift （我猜就是直流电平移）**/
	mi_INT32 m_dc_level_shift;
}mi_tccp_t;



/**
 * FIXME DOC
 */
typedef struct mi_mct_data
{
	J2K_MCT_ELEMENT_TYPE m_element_type;
	J2K_MCT_ARRAY_TYPE	 m_array_type;
	mi_UINT32			 m_index;
	mi_BYTE *			 m_data;
	mi_UINT32			 m_data_size;
}
mi_mct_data_t;

/**
 * FIXME DOC
 */
typedef struct mi_simple_mcc_decorrelation_data
{
	mi_UINT32			 m_index;
	mi_UINT32			 m_nb_comps;
	mi_mct_data_t *	 m_decorrelation_array;
	mi_mct_data_t *	 m_offset_array;
	mi_UINT32			 m_is_irreversible : 1;
}
mi_simple_mcc_decorrelation_data_t;

typedef struct mi_ppx_struct
{
	mi_BYTE*   m_data; /* m_data == NULL => Zppx not read yet */
	mi_UINT32	m_data_size;
} mi_ppx;

/**
Tile coding parameters :
this structure is used to store coding/decoding parameters common to all
tiles (information like COD, COC in main header)
*/
typedef struct mi_tcp
{
	/** coding style */
	mi_UINT32 csty;
	/** progression order */
	mi_PROG_ORDER prg;
	/** number of layers */
	mi_UINT32 numlayers;
	mi_UINT32 num_layers_to_decode;
	/** multi-component transform identifier */
	mi_UINT32 mct;
	/** rates of layers */
	mi_FLOAT32 rates[100];
	/** number of progression order changes */
	mi_UINT32 numpocs;
	/** progression order changes */
	mi_poc_t pocs[32];
	
	/** number of ppt markers (reserved size) */
	mi_UINT32 ppt_markers_count;
	/** ppt markers data (table indexed by Zppt) */
	mi_ppx* ppt_markers;
	
	/** packet header store there for future use in t2_decode_packet */
	mi_BYTE *ppt_data;
	/** used to keep a track of the allocated memory */
	mi_BYTE *ppt_buffer;
	/** Number of bytes stored inside ppt_data*/
	mi_UINT32 ppt_data_size;
	/** size of ppt_data*/
	mi_UINT32 ppt_len;
	/** add fixed_quality */
	mi_FLOAT32 distoratio[100];
	/** tile-component coding parameters */
	mi_tccp_t *tccps;
	/** number of tile parts for the tile. */
	mi_UINT32 m_nb_tile_parts;
	/** data for the tile */
	mi_BYTE *		m_data;
	/** size of data */
	mi_UINT32		m_data_size;
	/** encoding norms */
	mi_FLOAT64 *	mct_norms;
	/** the mct decoding matrix */
	mi_FLOAT32 *	m_mct_decoding_matrix;
	/** the mct coding matrix */
	mi_FLOAT32 *	m_mct_coding_matrix;
	/** mct records */
	mi_mct_data_t * m_mct_records;
	/** the number of mct records. */
	mi_UINT32 m_nb_mct_records;
	/** the max number of mct records. */
	mi_UINT32 m_nb_max_mct_records;
	/** mcc records */
	mi_simple_mcc_decorrelation_data_t * m_mcc_records;
	/** the number of mct records. */
	mi_UINT32 m_nb_mcc_records;
	/** the max number of mct records. */
	mi_UINT32 m_nb_max_mcc_records;


	/***** FLAGS *******/
	/** If cod == 1 --> there was a COD marker for the present tile */
	mi_UINT32 cod : 1;
	/** If ppt == 1 --> there was a PPT marker for the present tile */
	mi_UINT32 ppt : 1;
	/** indicates if a POC marker has been used O:NO, 1:YES */
	mi_UINT32 POC : 1;
} mi_tcp_t;




typedef struct mi_encoding_param
{
	/** Maximum rate for each component. If == 0, component size limitation is not considered */
	mi_UINT32 m_max_comp_size;
	/** Position of tile part flag in progression order*/
	mi_INT32 m_tp_pos;
	/** fixed layer */
	mi_INT32 *m_matrice;
	/** Flag determining tile part generation*/
	mi_BYTE m_tp_flag;
	/** allocation by rate/distortion */
	mi_UINT32 m_disto_alloc : 1;
	/** allocation by fixed layer */
	mi_UINT32 m_fixed_alloc : 1;
	/** add fixed_quality */
	mi_UINT32 m_fixed_quality : 1;
	/** Enabling Tile part generation*/
	mi_UINT32 m_tp_on : 1;
}mi_encoding_param_t;

typedef struct mi_decoding_param
{
	/** if != 0, then original dimension divided by 2^(reduce); if == 0 or not used, image is decoded to the full resolution */
	mi_UINT32 m_reduce;
	/** if != 0, then only the first "layer" layers are decoded; if == 0 or not used, all the quality layers are decoded */
	mi_UINT32 m_layer;
}mi_decoding_param_t;


/**
 * Coding parameters
 */
typedef struct mi_cp
{
	/** Size of the image in bits*/
	/*int img_size;*/
	/** Rsiz*/
    mi_UINT16 rsiz;
	/** XTOsiz */
	mi_UINT32 tx0; /* MSD see norm */
	/** YTOsiz */
	mi_UINT32 ty0; /* MSD see norm */
	/** XTsiz */
	mi_UINT32 tdx;
	/** YTsiz */
	mi_UINT32 tdy;
	/** comment */
	mi_CHAR *comment;
	/** number of tiles in width */
	mi_UINT32 tw;
	/** number of tiles in heigt */
	mi_UINT32 th;

	/** number of ppm markers (reserved size) */
	mi_UINT32 ppm_markers_count;
	/** ppm markers data (table indexed by Zppm) */
	mi_ppx* ppm_markers;
	
	/** packet header store there for future use in t2_decode_packet */
	mi_BYTE *ppm_data;
	/** size of the ppm_data*/
	mi_UINT32 ppm_len;
	/** size of the ppm_data*/
	mi_UINT32 ppm_data_read;

	mi_BYTE *ppm_data_current;

	/** packet header storage original buffer */
	mi_BYTE *ppm_buffer;
	/** pointer remaining on the first byte of the first header if ppm is used */
	mi_BYTE *ppm_data_first;
	/** Number of bytes actually stored inside the ppm_data */
	mi_UINT32 ppm_data_size;
	/** use in case of multiple marker PPM (number of info already store) */
	mi_INT32 ppm_store;
	/** use in case of multiple marker PPM (case on non-finished previous info) */
	mi_INT32 ppm_previous;

	/** tile coding parameters */
	mi_tcp_t *tcps;

	union
	{
		mi_decoding_param_t m_dec;
		mi_encoding_param_t m_enc;
	}m_specific_param;

	/******** FLAGS *********/
	/** if ppm == 1 --> there was a PPM marker*/
	mi_UINT32 ppm : 1;
	/** tells if the parameter is a coding or decoding one */
	mi_UINT32 m_is_decoder : 1;
/* <<UniPG */
} mi_cp_t;


typedef struct mi_j2k_dec
{
	/** locate in which part of the codestream the decoder is (main header, tile header, end) */
	mi_UINT32 m_state;
	/**
	 * store decoding parameters common to all tiles (information like COD, COC in main header)
	 */
	mi_tcp_t *m_default_tcp;
	mi_BYTE  *m_header_data;
	mi_UINT32 m_header_data_size;
	/** to tell the tile part length */
	mi_UINT32 m_sot_length;
	/** Only tiles index in the correct range will be decoded.*/
	mi_UINT32 m_start_tile_x;
	mi_UINT32 m_start_tile_y;
	mi_UINT32 m_end_tile_x;
	mi_UINT32 m_end_tile_y;
	/**
	 * Decoded area set by the user
	 */
	mi_UINT32 m_DA_x0;
	mi_UINT32 m_DA_y0;
	mi_UINT32 m_DA_x1;
	mi_UINT32 m_DA_y1;

	/** Index of the tile to decode (used in get_tile) */
	mi_INT32 m_tile_ind_to_dec;
	/** Position of the last SOT marker read */
	mi_OFF_T m_last_sot_read_pos;

	/**
	 * Indicate that the current tile-part is assume as the last tile part of the codestream.
	 * It is useful in the case of PSot is equal to zero. The sot length will be compute in the
	 * SOD reader function. FIXME NOT USED for the moment
	 */
	mi_BOOL   m_last_tile_part;
	/** to tell that a tile can be decoded. */
	mi_UINT32 m_can_decode			: 1;
	mi_UINT32 m_discard_tiles		: 1;
	mi_UINT32 m_skip_data			: 1;
	/** TNsot correction : see issue 254 **/
	mi_UINT32 m_nb_tile_parts_correction_checked : 1;
	mi_UINT32 m_nb_tile_parts_correction : 1;

} mi_j2k_dec_t;

typedef struct mi_j2k_enc
{
	/** Tile part number, regardless of poc, for each new poc, tp is reset to 1*/
	mi_UINT32 m_current_poc_tile_part_number; /* tp_num */

	/** Tile part number currently coding, taking into account POC. m_current_tile_part_number holds the total number of tile parts while encoding the last tile part.*/
	mi_UINT32 m_current_tile_part_number; /*cur_tp_num */

	/**
	locate the start position of the TLM marker
	after encoding the tilepart, a jump (in j2k_write_sod) is done to the TLM marker to store the value of its length.
	*/
    mi_OFF_T m_tlm_start;
	/**
	 * Stores the sizes of the tlm.
	 */
	mi_BYTE * m_tlm_sot_offsets_buffer;
	/**
	 * The current offset of the tlm buffer.
	 */
	mi_BYTE * m_tlm_sot_offsets_current;

	/** Total num of tile parts in whole image = num tiles* num tileparts in each tile*/
	/** used in TLMmarker*/
	mi_UINT32 m_total_tile_parts;	 /* totnum_tp */

	/* encoded data for a tile */
	mi_BYTE * m_encoded_tile_data;

	/* size of the encoded_data */
	mi_UINT32 m_encoded_tile_size;

	/* encoded data for a tile */
	mi_BYTE * m_header_tile_data;

	/* size of the encoded_data */
	mi_UINT32 m_header_tile_data_size;


} mi_j2k_enc_t;



struct mi_tcd;
/**
JPEG-2000 codestream reader/writer
*/
typedef struct mi_j2k
{
	/* J2K codestream is decoded*/
	mi_BOOL m_is_decoder;

	/* FIXME DOC*/
	union
	{
		mi_j2k_dec_t m_decoder;
		mi_j2k_enc_t m_encoder;
	}
	m_specific_param;

	/** pointer to the internal/private encoded / decoded image */
	mi_image_t* m_private_image;

	/* pointer to the output image (decoded)*/
	mi_image_t* m_output_image;

	/** Coding parameters */
	mi_cp_t m_cp;

	/** the list of procedures to exec **/
	mi_procedure_list_t *	m_procedure_list;

	/** the list of validation procedures to follow to make sure the code is valid **/
	mi_procedure_list_t *	m_validation_list;

	/** helper used to write the index file */
	mi_codestream_index_t *cstr_index;

	/** number of the tile curently concern by coding/decoding */
	mi_UINT32 m_current_tile_number;

	/** the current tile coder/decoder **/
	struct mi_tcd *	m_tcd;
}mi_j2k_t;




/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */

/**
Setup the decoder decoding parameters using user parameters.
Decoding parameters are returned in j2k->cp. 
@param j2k J2K decompressor handle
@param parameters decompression parameters
*/
void mi_j2k_setup_decoder(mi_j2k_t *j2k, mi_dparameters_t *parameters);

/**
 * Creates a J2K compression structure
 *
 * @return Returns a handle to a J2K compressor if successful, returns NULL otherwise
*/
mi_j2k_t* mi_j2k_create_compress(void);


mi_BOOL mi_j2k_setup_encoder(	mi_j2k_t *p_j2k,
						    mi_cparameters_t *parameters,
						    mi_image_t *image,
						    mi_event_mgr_t * p_manager);

/**
Converts an enum type progression order to string type
*/
char *mi_j2k_convert_progression_order(mi_PROG_ORDER prg_order);

/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/

/**
 * Ends the decompression procedures and possibiliy add data to be read after the
 * codestream.
 */
mi_BOOL mi_j2k_end_decompress(mi_j2k_t *j2k,
                                mi_stream_private_t *p_stream,
                                mi_event_mgr_t * p_manager);

/**
 * Reads a jpeg2000 codestream header structure.
 *
 * @param p_stream the stream to read data from.
 * @param p_j2k the jpeg2000 codec.
 * @param p_image FIXME DOC
 * @param p_manager the user event manager.
 *
 * @return true if the box is valid.
 */
mi_BOOL mi_j2k_read_header(	mi_stream_private_t *p_stream,
                                mi_j2k_t* p_j2k,
                                mi_image_t** p_image,
                                mi_event_mgr_t* p_manager );


/**
 * Destroys a jpeg2000 codec.
 *
 * @param	p_j2k	the jpeg20000 structure to destroy.
 */
void mi_j2k_destroy (mi_j2k_t *p_j2k);

/**
 * Destroys a codestream index structure.
 *
 * @param	p_cstr_ind	the codestream index parameter to destroy.
 */
void j2k_destroy_cstr_index (mi_codestream_index_t *p_cstr_ind);

/**
 * Decode tile data.
 * @param	p_j2k		the jpeg2000 codec.
 * @param	p_tile_index
 * @param p_data       FIXME DOC
 * @param p_data_size  FIXME DOC
 * @param	p_stream			the stream to write data to.
 * @param	p_manager	the user event manager.
 */
mi_BOOL mi_j2k_decode_tile (  mi_j2k_t * p_j2k,
                                mi_UINT32 p_tile_index,
                                mi_BYTE * p_data,
                                mi_UINT32 p_data_size,
                                mi_stream_private_t *p_stream,
                                mi_event_mgr_t * p_manager );

/**
 * Reads a tile header.
 * @param	p_j2k		the jpeg2000 codec.
 * @param	p_tile_index FIXME DOC
 * @param	p_data_size FIXME DOC
 * @param	p_tile_x0 FIXME DOC
 * @param	p_tile_y0 FIXME DOC
 * @param	p_tile_x1 FIXME DOC
 * @param	p_tile_y1 FIXME DOC
 * @param	p_nb_comps FIXME DOC
 * @param	p_go_on FIXME DOC
 * @param	p_stream			the stream to write data to.
 * @param	p_manager	the user event manager.
 */
mi_BOOL mi_j2k_read_tile_header ( mi_j2k_t * p_j2k,
                                    mi_UINT32 * p_tile_index,
                                    mi_UINT32 * p_data_size,
                                    mi_INT32 * p_tile_x0,
                                    mi_INT32 * p_tile_y0,
                                    mi_INT32 * p_tile_x1,
                                    mi_INT32 * p_tile_y1,
                                    mi_UINT32 * p_nb_comps,
                                    mi_BOOL * p_go_on,
                                    mi_stream_private_t *p_stream,
                                    mi_event_mgr_t * p_manager );


/**
 * Sets the given area to be decoded. This function should be called right after mi_read_header and before any tile header reading.
 *
 * @param	p_j2k			the jpeg2000 codec.
 * @param	p_image     FIXME DOC
 * @param	p_start_x		the left position of the rectangle to decode (in image coordinates).
 * @param	p_start_y		the up position of the rectangle to decode (in image coordinates).
 * @param	p_end_x			the right position of the rectangle to decode (in image coordinates).
 * @param	p_end_y			the bottom position of the rectangle to decode (in image coordinates).
 * @param	p_manager		the user event manager
 *
 * @return	true			if the area could be set.
 */
mi_BOOL mi_j2k_set_decode_area(	mi_j2k_t *p_j2k,
								    mi_image_t* p_image,
								    mi_INT32 p_start_x, mi_INT32 p_start_y,
								    mi_INT32 p_end_x, mi_INT32 p_end_y,
								    mi_event_mgr_t * p_manager );

/**
 * Creates a J2K decompression structure.
 *
 * @return a handle to a J2K decompressor if successful, NULL otherwise.
 */
mi_j2k_t* mi_j2k_create_decompress(void);


/**
 * Dump some elements from the J2K decompression structure .
 *
 *@param p_j2k				the jpeg2000 codec.
 *@param flag				flag to describe what elments are dump.
 *@param out_stream			output stream where dump the elements.
 *
*/
void j2k_dump (mi_j2k_t* p_j2k, mi_INT32 flag, FILE* out_stream);



/**
 * Dump an image header structure.
 *
 *@param image			the image header to dump.
 *@param dev_dump_flag		flag to describe if we are in the case of this function is use outside j2k_dump function
 *@param out_stream			output stream where dump the elements.
 */
void j2k_dump_image_header(mi_image_t* image, mi_BOOL dev_dump_flag, FILE* out_stream);

/**
 * Dump a component image header structure.
 *
 *@param comp		the component image header to dump.
 *@param dev_dump_flag		flag to describe if we are in the case of this function is use outside j2k_dump function
 *@param out_stream			output stream where dump the elements.
 */
void j2k_dump_image_comp_header(mi_image_comp_t* comp, mi_BOOL dev_dump_flag, FILE* out_stream);

/**
 * Get the codestream info from a JPEG2000 codec.
 *
 *@param	p_j2k				the component image header to dump.
 *
 *@return	the codestream information extract from the jpg2000 codec
 */
mi_codestream_info_v2_t* j2k_get_cstr_info(mi_j2k_t* p_j2k);

/**
 * Get the codestream index from a JPEG2000 codec.
 *
 *@param	p_j2k				the component image header to dump.
 *
 *@return	the codestream index extract from the jpg2000 codec
 */
mi_codestream_index_t* j2k_get_cstr_index(mi_j2k_t* p_j2k);

/**
 * Decode an image from a JPEG-2000 codestream
 * @param j2k J2K decompressor handle
 * @param p_stream  FIXME DOC
 * @param p_image   FIXME DOC
 * @param p_manager FIXME DOC
 * @return FIXME DOC
*/
mi_BOOL mi_j2k_decode(mi_j2k_t *j2k,
                        mi_stream_private_t *p_stream,
                        mi_image_t *p_image,
                        mi_event_mgr_t *p_manager);


mi_BOOL mi_j2k_get_tile(	mi_j2k_t *p_j2k,
			    			mi_stream_private_t *p_stream,
				    		mi_image_t* p_image,
					    	mi_event_mgr_t * p_manager,
						    mi_UINT32 tile_index );

mi_BOOL mi_j2k_set_decoded_resolution_factor(mi_j2k_t *p_j2k, 
                                               mi_UINT32 res_factor,
                                               mi_event_mgr_t * p_manager);


/**
 * Writes a tile.
 * @param	p_j2k		the jpeg2000 codec.
 * @param p_tile_index FIXME DOC
 * @param p_data FIXME DOC
 * @param p_data_size FIXME DOC
 * @param	p_stream			the stream to write data to.
 * @param	p_manager	the user event manager.
 */
mi_BOOL mi_j2k_write_tile (	mi_j2k_t * p_j2k,
							    mi_UINT32 p_tile_index,
							    mi_BYTE * p_data,
							    mi_UINT32 p_data_size,
							    mi_stream_private_t *p_stream,
							    mi_event_mgr_t * p_manager );

/**
 * Encodes an image into a JPEG-2000 codestream
 */
mi_BOOL mi_j2k_encode(	mi_j2k_t * p_j2k,
			    			mi_stream_private_t *cio,
				    		mi_event_mgr_t * p_manager );

/**
 * Starts a compression scheme, i.e. validates the codec parameters, writes the header.
 *
 * @param	p_j2k		the jpeg2000 codec.
 * @param	p_stream			the stream object.
 * @param	p_image FIXME DOC
 * @param	p_manager	the user event manager.
 *
 * @return true if the codec is valid.
 */
mi_BOOL mi_j2k_start_compress(mi_j2k_t *p_j2k,
							    mi_stream_private_t *p_stream,
							    mi_image_t * p_image,
							    mi_event_mgr_t * p_manager);

/**
 * Ends the compression procedures and possibiliy add data to be read after the
 * codestream.
 */
mi_BOOL mi_j2k_end_compress( 	mi_j2k_t *p_j2k,
							    mi_stream_private_t *cio,
							    mi_event_mgr_t * p_manager);

mi_BOOL mi_j2k_setup_mct_encoding (mi_tcp_t * p_tcp, mi_image_t * p_image);


#endif /* __J2K_H */
