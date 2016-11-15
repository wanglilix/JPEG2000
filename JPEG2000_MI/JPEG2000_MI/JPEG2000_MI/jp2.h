
#ifndef __JP2_H
#define __JP2_H
#include"openjpeg.h"
#include"j2k.h"
#include"event.h"
#include"cio.h"
/**
@file jp2.h
@brief The JPEG-2000 file format Reader/Writer (JP2)

*/

/** @defgroup JP2 JP2 - JPEG-2000 file format reader/writer */
/*@{*/

/*#define JPIP_JPIP 0x6a706970*/

#define     JP2_JP   0x6a502020    /**< JPEG 2000 signature box */
#define     JP2_FTYP 0x66747970    /**< File type box */
#define     JP2_JP2H 0x6a703268    /**< JP2 header box (super-box) */
#define     JP2_IHDR 0x69686472    /**< Image header box */
#define     JP2_COLR 0x636f6c72    /**< Colour specification box */
#define     JP2_JP2C 0x6a703263    /**< Contiguous codestream box */
#define     JP2_URL  0x75726c20    /**< Data entry URL box */
#define     JP2_PCLR 0x70636c72    /**< Palette box */
#define     JP2_CMAP 0x636d6170    /**< Component Mapping box */
#define     JP2_CDEF 0x63646566    /**< Channel Definition box */
#define     JP2_DTBL 0x6474626c    /**< Data Reference box */
#define     JP2_BPCC 0x62706363    /**< Bits per component box */
#define     JP2_JP2  0x6a703220    /**< File type fields */

/* For the future */
/* #define JP2_RES 0x72657320 */  /**< Resolution box (super-box) */
/* #define JP2_JP2I 0x6a703269 */  /**< Intellectual property box */
/* #define JP2_XML  0x786d6c20 */  /**< XML box */
/* #define JP2_UUID 0x75756994 */  /**< UUID box */
/* #define JP2_UINF 0x75696e66 */  /**< UUID info box (super-box) */
/* #define JP2_ULST 0x756c7374 */  /**< UUID list box */

/* ----------------------------------------------------------------------- */

typedef enum
{
  JP2_STATE_NONE            = 0x0,
  JP2_STATE_SIGNATURE       = 0x1,
  JP2_STATE_FILE_TYPE       = 0x2,
  JP2_STATE_HEADER          = 0x4,
  JP2_STATE_CODESTREAM      = 0x8,
  JP2_STATE_END_CODESTREAM  = 0x10,
  JP2_STATE_UNKNOWN         = 0x7fffffff /* ISO C restricts enumerator values to range of 'int' */
}
JP2_STATE;

typedef enum
{
  JP2_IMG_STATE_NONE        = 0x0,
  JP2_IMG_STATE_UNKNOWN     = 0x7fffffff
}
JP2_IMG_STATE;

/** 
Channel description: channel index, type, association
*/
typedef struct mi_jp2_cdef_info
{
    mi_UINT16 cn, typ, asoc;
} mi_jp2_cdef_info_t;

/** 
Channel descriptions and number of descriptions
*/
typedef struct mi_jp2_cdef
{
    mi_jp2_cdef_info_t *info;
    mi_UINT16 n;
} mi_jp2_cdef_t;

/** 
Component mappings: channel index, mapping type, palette index
*/
typedef struct mi_jp2_cmap_comp
{
    mi_UINT16 cmp;
    mi_BYTE mtyp, pcol;
} mi_jp2_cmap_comp_t;

/** 
Palette data: table entries, palette columns
*/
typedef struct mi_jp2_pclr
{
    mi_UINT32 *entries;
    mi_BYTE *channel_sign;
    mi_BYTE *channel_size;
    mi_jp2_cmap_comp_t *cmap;
    mi_UINT16 nr_entries;
    mi_BYTE nr_channels;
} mi_jp2_pclr_t;

/** 
Collector for ICC profile, palette, component mapping, channel description 
*/
typedef struct mi_jp2_color
{
    mi_BYTE *icc_profile_buf;
    mi_UINT32 icc_profile_len;

    mi_jp2_cdef_t *jp2_cdef;
    mi_jp2_pclr_t *jp2_pclr;
    mi_BYTE jp2_has_colr;
} mi_jp2_color_t;

/** 
JP2 component
*/
typedef struct mi_jp2_comps {
  mi_UINT32 depth;      
  mi_UINT32 sgnd;       
  mi_UINT32 bpcc;
} mi_jp2_comps_t;

/**
JPEG-2000 file format reader/writer
*/
typedef struct mi_jp2
{
  /** handle to the J2K codec  */
  mi_j2k_t *j2k;
  /** list of validation procedures */
  struct mi_procedure_list * m_validation_list;
  /** list of execution procedures */
  struct mi_procedure_list * m_procedure_list;

  /* width of image */
  mi_UINT32 w;
  /* height of image */
  mi_UINT32 h;
  /* number of components in the image */
  mi_UINT32 numcomps;
  mi_UINT32 bpc;
  mi_UINT32 C;
  mi_UINT32 UnkC;
  mi_UINT32 IPR;
  mi_UINT32 meth;
  mi_UINT32 approx;
  mi_UINT32 enumcs;
  mi_UINT32 precedence;
  mi_UINT32 brand;
  mi_UINT32 minversion;
  mi_UINT32 numcl;
  mi_UINT32 *cl;
  mi_jp2_comps_t *comps;
  /* FIXME: The following two variables are used to save offset
    as we write out a JP2 file to disk. This mecanism is not flexible
    as codec writers will need to extand those fields as new part
    of the standard are implemented.
  */
    mi_OFF_T j2k_codestream_offset;
    mi_OFF_T jpip_iptr_offset;
    mi_BOOL jpip_on;
  mi_UINT32 jp2_state;
  mi_UINT32 jp2_img_state;

  mi_jp2_color_t color;
    
    mi_BOOL ignore_pclr_cmap_cdef;
}
mi_jp2_t;

/**
JP2 Box
*/
typedef struct mi_jp2_box {
    mi_UINT32 length;
    mi_UINT32 type;
    mi_INT32 init_pos;
} mi_jp2_box_t;

typedef struct mi_jp2_header_handler
{
  /* marker value */
  mi_UINT32 id;
  /* action linked to the marker */
  mi_BOOL (*handler) (     mi_jp2_t *jp2, 
                            mi_BYTE *p_header_data, 
                            mi_UINT32 p_header_size, 
                            mi_event_mgr_t * p_manager);
}
mi_jp2_header_handler_t;


typedef struct mi_jp2_img_header_writer_handler 
{
  /* action to perform */
  mi_BYTE*   (*handler) (mi_jp2_t *jp2, mi_UINT32 * p_data_size);
  /* result of the action : data */
  mi_BYTE*   m_data;
  /* size of data */
  mi_UINT32  m_size;
} 
mi_jp2_img_header_writer_handler_t;

/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */

/**
Setup the decoder decoding parameters using user parameters.
Decoding parameters are returned in jp2->j2k->cp.
@param jp2 JP2 decompressor handle
@param parameters decompression parameters
*/
void mi_jp2_setup_decoder(mi_jp2_t *jp2, mi_dparameters_t *parameters);

/**
 * Decode an image from a JPEG-2000 file stream
 * @param jp2 JP2 decompressor handle
 * @param p_stream  FIXME DOC
 * @param p_image   FIXME DOC
 * @param p_manager FIXME DOC
 *
 * @return Returns a decoded image if successful, returns NULL otherwise
*/
mi_BOOL mi_jp2_decode(mi_jp2_t *jp2,
                        mi_stream_private_t *p_stream,
            mi_image_t* p_image,
            mi_event_mgr_t * p_manager);

/**
 * Setup the encoder parameters using the current image and using user parameters. 
 * Coding parameters are returned in jp2->j2k->cp. 
 *
 * @param jp2 JP2 compressor handle
 * @param parameters compression parameters
 * @param image input filled image
 * @param p_manager  FIXME DOC
 * @return mi_TRUE if successful, mi_FALSE otherwise
*/
mi_BOOL mi_jp2_setup_encoder(  mi_jp2_t *jp2, 
                            mi_cparameters_t *parameters, 
                            mi_image_t *image, 
                            mi_event_mgr_t * p_manager);

/**
Encode an image into a JPEG-2000 file stream
@param jp2      JP2 compressor handle
@param stream    Output buffer stream
@param p_manager  event manager
@return Returns true if successful, returns false otherwise
*/
mi_BOOL mi_jp2_encode(  mi_jp2_t *jp2, 
              mi_stream_private_t *stream, 
              mi_event_mgr_t * p_manager);


/**
 * Starts a compression scheme, i.e. validates the codec parameters, writes the header.
 *
 * @param  jp2    the jpeg2000 file codec.
 * @param  stream    the stream object.
 * @param  p_image   FIXME DOC
 * @param p_manager FIXME DOC
 *
 * @return true if the codec is valid.
 */
mi_BOOL mi_jp2_start_compress(mi_jp2_t *jp2,
                                mi_stream_private_t *stream,
                                mi_image_t * p_image,
                                mi_event_mgr_t * p_manager);


/**
 * Ends the compression procedures and possibiliy add data to be read after the
 * codestream.
 */
mi_BOOL mi_jp2_end_compress(  mi_jp2_t *jp2,
                  mi_stream_private_t *cio,
                  mi_event_mgr_t * p_manager);

/* ----------------------------------------------------------------------- */

/**
 * Ends the decompression procedures and possibiliy add data to be read after the
 * codestream.
 */
mi_BOOL mi_jp2_end_decompress(mi_jp2_t *jp2, 
                                mi_stream_private_t *cio,
                                mi_event_mgr_t * p_manager);

/**
 * Reads a jpeg2000 file header structure.
 *
 * @param p_stream the stream to read data from.
 * @param jp2 the jpeg2000 file header structure.
 * @param p_image   FIXME DOC
 * @param p_manager the user event manager.
 *
 * @return true if the box is valid.
 */
mi_BOOL mi_jp2_read_header(  mi_stream_private_t *p_stream,
                                mi_jp2_t *jp2,
                                mi_image_t ** p_image,
                                mi_event_mgr_t * p_manager );

/**
 * Reads a tile header.
 * @param  p_jp2         the jpeg2000 codec.
 * @param  p_tile_index  FIXME DOC
 * @param  p_data_size   FIXME DOC
 * @param  p_tile_x0     FIXME DOC
 * @param  p_tile_y0     FIXME DOC
 * @param  p_tile_x1     FIXME DOC
 * @param  p_tile_y1     FIXME DOC
 * @param  p_nb_comps    FIXME DOC
 * @param  p_go_on       FIXME DOC
 * @param  p_stream      the stream to write data to.
 * @param  p_manager     the user event manager.
 */
mi_BOOL mi_jp2_read_tile_header ( mi_jp2_t * p_jp2,
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
 * Writes a tile.
 *
 * @param  p_jp2    the jpeg2000 codec.
 * @param p_tile_index  FIXME DOC
 * @param p_data        FIXME DOC
 * @param p_data_size   FIXME DOC
 * @param  p_stream      the stream to write data to.
 * @param  p_manager  the user event manager.
 */
mi_BOOL mi_jp2_write_tile (  mi_jp2_t *p_jp2,
                    mi_UINT32 p_tile_index,
                    mi_BYTE * p_data,
                    mi_UINT32 p_data_size,
                    mi_stream_private_t *p_stream,
                    mi_event_mgr_t * p_manager );

/**
 * Decode tile data.
 * @param  p_jp2    the jpeg2000 codec.
 * @param  p_tile_index FIXME DOC
 * @param  p_data       FIXME DOC
 * @param  p_data_size  FIXME DOC
 * @param  p_stream      the stream to write data to.
 * @param  p_manager  the user event manager.
 *
 * @return FIXME DOC
 */
mi_BOOL mi_jp2_decode_tile (  mi_jp2_t * p_jp2,
                                mi_UINT32 p_tile_index,
                                mi_BYTE * p_data,
                                mi_UINT32 p_data_size,
                                mi_stream_private_t *p_stream,
                                mi_event_mgr_t * p_manager );

/**
 * Creates a jpeg2000 file decompressor.
 *
 * @return  an empty jpeg2000 file codec.
 */
mi_jp2_t* mi_jp2_create (mi_BOOL p_is_decoder);

/**
Destroy a JP2 decompressor handle
@param jp2 JP2 decompressor handle to destroy
*/
void mi_jp2_destroy(mi_jp2_t *jp2);


/**
 * Sets the given area to be decoded. This function should be called right after mi_read_header and before any tile header reading.
 *
 * @param  p_jp2      the jpeg2000 codec.
 * @param  p_image     FIXME DOC
 * @param  p_start_x   the left position of the rectangle to decode (in image coordinates).
 * @param  p_start_y    the up position of the rectangle to decode (in image coordinates).
 * @param  p_end_x      the right position of the rectangle to decode (in image coordinates).
 * @param  p_end_y      the bottom position of the rectangle to decode (in image coordinates).
 * @param  p_manager    the user event manager
 *
 * @return  true      if the area could be set.
 */
mi_BOOL mi_jp2_set_decode_area(  mi_jp2_t *p_jp2,
                    mi_image_t* p_image,
                    mi_INT32 p_start_x, mi_INT32 p_start_y,
                    mi_INT32 p_end_x, mi_INT32 p_end_y,
                    mi_event_mgr_t * p_manager );

 /**
 * 
 */
mi_BOOL mi_jp2_get_tile(  mi_jp2_t *p_jp2,
                            mi_stream_private_t *p_stream,
                            mi_image_t* p_image,
                            mi_event_mgr_t * p_manager,
                            mi_UINT32 tile_index );


/**
 * 
 */
mi_BOOL mi_jp2_set_decoded_resolution_factor(mi_jp2_t *p_jp2, 
                                               mi_UINT32 res_factor, 
                                               mi_event_mgr_t * p_manager);


/* TODO MSD: clean these 3 functions */
/**
 * Dump some elements from the JP2 decompression structure .
 *
 *@param p_jp2        the jp2 codec.
 *@param flag        flag to describe what elments are dump.
 *@param out_stream      output stream where dump the elements.
 *
*/
void jp2_dump (mi_jp2_t* p_jp2, mi_INT32 flag, FILE* out_stream);

/**
 * Get the codestream info from a JPEG2000 codec.
 *
 *@param  p_jp2        jp2 codec.
 *
 *@return  the codestream information extract from the jpg2000 codec
 */
mi_codestream_info_v2_t* jp2_get_cstr_info(mi_jp2_t* p_jp2);

/**
 * Get the codestream index from a JPEG2000 codec.
 *
 *@param  p_jp2        jp2 codec.
 *
 *@return  the codestream index extract from the jpg2000 codec
 */
mi_codestream_index_t* jp2_get_cstr_index(mi_jp2_t* p_jp2);


/*@}*/

/*@}*/

#endif /* __JP2_H */

