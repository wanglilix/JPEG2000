
#ifndef __TCD_H
#define __TCD_H
/**
@file tcd.h
@brief Implementation of a tile coder/decoder (TCD)

The functions in TCD.C encode or decode each tile independently from
each other. The functions in TCD.C are used by other functions in J2K.C.
*/

/** @defgroup TCD TCD - Implementation of a tile coder/decoder */
/*@{*/

/**
FIXME DOC
*/
typedef struct mi_tcd_seg {
	mi_BYTE ** data;
	mi_UINT32 dataindex;
	mi_UINT32 numpasses;
	mi_UINT32 real_num_passes;
	mi_UINT32 len;
	mi_UINT32 maxpasses;
	mi_UINT32 numnewpasses;
	mi_UINT32 newlen;
} mi_tcd_seg_t;

/**
FIXME DOC
*/
typedef struct mi_tcd_pass {
	mi_UINT32 rate;
	mi_FLOAT64 distortiondec;
	mi_UINT32 len;
	mi_UINT32 term : 1;
} mi_tcd_pass_t;

/**
FIXME DOC
*/
typedef struct mi_tcd_layer {
	mi_UINT32 numpasses;		/* Number of passes in the layer */
	mi_UINT32 len;				/* len of information */
	mi_FLOAT64 disto;			/* add for index (Cfr. Marcela) */
	mi_BYTE *data;				/* data */
} mi_tcd_layer_t;

/**
FIXME DOC（应该是代码块信息（编码时））
*/
typedef struct mi_tcd_cblk_enc {
	mi_BYTE* data;               /* Data */
	mi_tcd_layer_t* layers;      /* layer information */
	mi_tcd_pass_t* passes;       /* information about the passes */
	mi_INT32 x0, y0, x1, y1;     /* dimension of the code-blocks : left upper corner (x0, y0) right low corner (x1,y1) */
	mi_UINT32 numbps;
	mi_UINT32 numlenbits;
	mi_UINT32 data_size;         /* Size of allocated data buffer */
	mi_UINT32 numpasses;         /* number of pass already done for the code-blocks */
	mi_UINT32 numpassesinlayers; /* number of passes in the layer */
	mi_UINT32 totalpasses;	      /* total number of passes */
} mi_tcd_cblk_enc_t;


typedef struct mi_tcd_cblk_dec {
	mi_BYTE * data;				/* Data */
	mi_tcd_seg_t* segs;			/* segments information */
	mi_INT32 x0, y0, x1, y1;		/* position of the code-blocks : left upper corner (x0, y0) right low corner (x1,y1) */
	mi_UINT32 numbps;
	mi_UINT32 numlenbits;
	mi_UINT32 data_max_size;		/* Size of allocated data buffer */
	mi_UINT32 data_current_size;	/* Size of used data buffer */
	mi_UINT32 numnewpasses;		/* number of pass added to the code-blocks */
	mi_UINT32 numsegs;				/* number of segments */
	mi_UINT32 real_num_segs;
	mi_UINT32 m_current_max_segs;
} mi_tcd_cblk_dec_t;

/**
FIXME DOC（我猜这是区信息）
*/
typedef struct mi_tcd_precinct {
	mi_INT32 x0, y0, x1, y1;		/* dimension of the precinct : left upper corner (x0, y0) right low corner (x1,y1) */
	mi_UINT32 cw, ch;				/* number of precinct in width and height */
	union{							/* code-blocks information */
		mi_tcd_cblk_enc_t* enc;
		mi_tcd_cblk_dec_t* dec;
		void*               blocks;
	} cblks;
	mi_UINT32 block_size;			/* size taken by cblks (in bytes) */
	mi_tgt_tree_t *incltree;	    /* inclusion tree */
	mi_tgt_tree_t *imsbtree;	    /* IMSB tree （应该指的是MSB-LSB的四叉树）*/
} mi_tcd_precinct_t;

/**
FIXME DOC（我猜这是子带信息）
*/
typedef struct mi_tcd_band {
	mi_INT32 x0, y0, x1, y1;		/* dimension of the subband : left upper corner (x0, y0) right low corner (x1,y1) */
	mi_UINT32 bandno;//子带编号（左上0，右上1，左下2，右下3）
	mi_tcd_precinct_t *precincts;	/* precinct information */
	mi_UINT32 precincts_data_size;	/* size of data taken by precincts */
	mi_INT32 numbps;//子带量化值的整数位数
	mi_FLOAT32 stepsize;//量化步长
} mi_tcd_band_t;

/**
FIXME DOC（分辨率信息）
*/
typedef struct mi_tcd_resolution {
	mi_INT32 x0, y0, x1, y1;		/* dimension of the resolution level : left upper corner (x0, y0) right low corner (x1,y1) */
	mi_UINT32 pw, ph;//一个分辨率内横/纵向的区数
	mi_UINT32 numbands;			/* number sub-band for the resolution level */
	mi_tcd_band_t bands[3];		/* subband information */
} mi_tcd_resolution_t;

/**
FIXME DOC（我猜这是一个图像分量下的一个切片）
*/
typedef struct mi_tcd_tilecomp
{
	mi_INT32 x0, y0, x1, y1;           /* dimension of component : left upper corner (x0, y0) right low corner (x1,y1)（单位为dx） */
	mi_UINT32 numresolutions;          /* number of resolutions level */
	mi_UINT32 minimum_num_resolutions; /* number of resolutions level to decode (at max)*/
	mi_tcd_resolution_t *resolutions;  /* resolutions information */
	mi_UINT32 resolutions_size;        /* size of data for resolutions (in bytes) */
	mi_INT32 *data;                    /* data of the component */
	mi_BOOL  ownsData;                 /* if true, then need to free after usage, otherwise do not free */
	mi_UINT32 data_size_needed;        /* we may either need to allocate this amount of data, or re-use image data and ignore this value */
	mi_UINT32 data_size;               /* size of the data of the component */
	mi_INT32 numpix;                   /* add fixed_quality */
} mi_tcd_tilecomp_t;


/**
FIXME DOC
*/
typedef struct mi_tcd_tile {
	mi_INT32 x0, y0, x1, y1;		/* dimension of the tile : left upper corner (x0, y0) right low corner (x1,y1) */
	mi_UINT32 numcomps;			/* number of components in tile */
	mi_tcd_tilecomp_t *comps;	/* Components information */
	mi_INT32 numpix;				/* add fixed_quality */
	mi_FLOAT64 distotile;			/* add fixed_quality */
	mi_FLOAT64 distolayer[100];	/* add fixed_quality */
	mi_UINT32 packno;              /* packet number */
} mi_tcd_tile_t;

/**
FIXME DOC
*/
typedef struct mi_tcd_image
{
	mi_tcd_tile_t *tiles;		/* Tiles information */
}
mi_tcd_image_t;


/**
Tile coder/decoder
*/
typedef struct mi_tcd
{
	/** Position of the tilepart flag in Progression order*/
	mi_INT32 tp_pos;
	/** Tile part number*/
	mi_UINT32 tp_num;
	/** Current tile part number*/
	mi_UINT32 cur_tp_num;
	/** Total number of tileparts of the current tile*/
	mi_UINT32 cur_totnum_tp;
	/** Current Packet iterator number */
	mi_UINT32 cur_pino;
	/** info on each image tile */
	mi_tcd_image_t *tcd_image;
	/** image header */
	mi_image_t *image;
	/** coding parameters */
	mi_cp_t *cp;
	/** coding/decoding parameters common to all tiles */
	mi_tcp_t *tcp;
	/** current encoded/decoded tile */
	mi_UINT32 tcd_tileno;
	/** tell if the tcd is a decoder. */
	mi_UINT32 m_is_decoder : 1;
} mi_tcd_t;

/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */

/**
Dump the content of a tcd structure
*/
/*void tcd_dump(FILE *fd, mi_tcd_t *tcd, mi_tcd_image_t *img);*/ /* TODO MSD shoul use the new v2 structures */ 

/**
Create a new TCD handle
@param p_is_decoder FIXME DOC
@return Returns a new TCD handle if successful returns NULL otherwise
*/
mi_tcd_t* mi_tcd_create(mi_BOOL p_is_decoder);

/**
Destroy a previously created TCD handle
@param tcd TCD handle to destroy
*/
void mi_tcd_destroy(mi_tcd_t *tcd);

/**
 * Initialize the tile coder and may reuse some memory.
 * @param	p_tcd		TCD handle.
 * @param	p_image		raw image.
 * @param	p_cp		coding parameters.
 *
 * @return true if the encoding values could be set (false otherwise).
*/
mi_BOOL mi_tcd_init(	mi_tcd_t *p_tcd,
						mi_image_t * p_image,
						mi_cp_t * p_cp );

/**
 * Allocates memory for decoding a specific tile.
 *
 * @param	p_tcd		the tile decoder.
 * @param	p_tile_no	the index of the tile received in sequence. This not necessarily lead to the
 * tile at index p_tile_no.
 * @param p_manager the event manager.
 *
 * @return	true if the remaining data is sufficient.
 */
mi_BOOL mi_tcd_init_decode_tile(mi_tcd_t *p_tcd, mi_UINT32 p_tile_no, mi_event_mgr_t* p_manager);

void mi_tcd_makelayer_fixed(mi_tcd_t *tcd, mi_UINT32 layno, mi_UINT32 final);

void mi_tcd_rateallocate_fixed(mi_tcd_t *tcd);

void mi_tcd_makelayer(	mi_tcd_t *tcd,
						mi_UINT32 layno,
						mi_FLOAT64 thresh,
						mi_UINT32 final);

mi_BOOL mi_tcd_rateallocate(	mi_tcd_t *tcd,
								mi_BYTE *dest,
								mi_UINT32 * p_data_written,
								mi_UINT32 len,
								mi_codestream_info_t *cstr_info);

/**
 * Gets the maximum tile size that will be taken by the tile once decoded.
 */
mi_UINT32 mi_tcd_get_decoded_tile_size (mi_tcd_t *p_tcd );

/**
 * Encodes a tile from the raw image into the given buffer.
 * @param	p_tcd			Tile Coder handle
 * @param	p_tile_no		Index of the tile to encode.
 * @param	p_dest			Destination buffer
 * @param	p_data_written	pointer to an int that is incremented by the number of bytes really written on p_dest
 * @param	p_len			Maximum length of the destination buffer
 * @param	p_cstr_info		Codestream information structure
 * @return  true if the coding is successful.
*/
mi_BOOL mi_tcd_encode_tile(   mi_tcd_t *p_tcd,
							    mi_UINT32 p_tile_no,
							    mi_BYTE *p_dest,
							    mi_UINT32 * p_data_written,
							    mi_UINT32 p_len,
							    struct mi_codestream_info *p_cstr_info);


/**
Decode a tile from a buffer into a raw image
@param tcd TCD handle
@param src Source buffer
@param len Length of source buffer
@param tileno Number that identifies one of the tiles to be decoded
@param cstr_info  FIXME DOC
@param manager the event manager.
*/
mi_BOOL mi_tcd_decode_tile(   mi_tcd_t *tcd,
							    mi_BYTE *src,
							    mi_UINT32 len,
							    mi_UINT32 tileno,
							    mi_codestream_index_t *cstr_info,
							    mi_event_mgr_t *manager);


/**
 * Copies tile data from the system onto the given memory block.
 */
mi_BOOL mi_tcd_update_tile_data (	mi_tcd_t *p_tcd,
								    mi_BYTE * p_dest,
								    mi_UINT32 p_dest_length );

/**
 *
 */
mi_UINT32 mi_tcd_get_encoded_tile_size ( mi_tcd_t *p_tcd );

/**
 * Initialize the tile coder and may reuse some meory.
 *
 * @param	p_tcd		TCD handle.
 * @param	p_tile_no	current tile index to encode.
 * @param p_manager the event manager.
 *
 * @return true if the encoding values could be set (false otherwise).
*/
mi_BOOL mi_tcd_init_encode_tile (	mi_tcd_t *p_tcd,
								    mi_UINT32 p_tile_no, mi_event_mgr_t* p_manager );

/**
 * Copies tile data from the given memory block onto the system.
 */
mi_BOOL mi_tcd_copy_tile_data (mi_tcd_t *p_tcd,
                                 mi_BYTE * p_src,
                                 mi_UINT32 p_src_length );

/**
 * Allocates tile component data
 *
 *
 */
mi_BOOL mi_alloc_tile_component_data(mi_tcd_tilecomp_t *l_tilec);

/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/

#endif /* __TCD_H */
