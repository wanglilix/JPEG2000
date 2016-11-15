
#include "mi_includes.h"

/** @defgroup JP2 JP2 - JPEG-2000 file format reader/writer */
/*@{*/

#define mi_BOX_SIZE	1024

/** @name Local static functions */
/*@{*/

/*static void jp2_write_url(mi_cio_t *cio, char *Idx_file);*/

/**
 * Reads a IHDR box - Image Header box
 *
 * @param	p_image_header_data			pointer to actual data (already read from file)
 * @param	jp2							the jpeg2000 file codec.
 * @param	p_image_header_size			the size of the image header
 * @param	p_manager					the user event manager.
 *
 * @return	true if the image header is valid, false else.
 */
static mi_BOOL mi_jp2_read_ihdr(  mi_jp2_t *jp2,
                                    mi_BYTE *p_image_header_data,
                                    mi_UINT32 p_image_header_size,
                                    mi_event_mgr_t * p_manager );

/**
 * Writes the Image Header box - Image Header box.
 *
 * @param jp2					jpeg2000 file codec.
 * @param p_nb_bytes_written	pointer to store the nb of bytes written by the function.
 *
 * @return	the data being copied.
*/
static mi_BYTE * mi_jp2_write_ihdr(mi_jp2_t *jp2,
                                     mi_UINT32 * p_nb_bytes_written );

/**
 * Writes the Bit per Component box.
 *
 * @param	jp2						jpeg2000 file codec.
 * @param	p_nb_bytes_written		pointer to store the nb of bytes written by the function.
 *
 * @return	the data being copied.
*/
static mi_BYTE * mi_jp2_write_bpcc(	mi_jp2_t *jp2,
                                        mi_UINT32 * p_nb_bytes_written );

/**
 * Reads a Bit per Component box.
 *
 * @param	p_bpc_header_data			pointer to actual data (already read from file)
 * @param	jp2							the jpeg2000 file codec.
 * @param	p_bpc_header_size			the size of the bpc header
 * @param	p_manager					the user event manager.
 *
 * @return	true if the bpc header is valid, false else.
 */
static mi_BOOL mi_jp2_read_bpcc(  mi_jp2_t *jp2,
                                    mi_BYTE * p_bpc_header_data,
                                    mi_UINT32 p_bpc_header_size,
                                    mi_event_mgr_t * p_manager );

static mi_BOOL mi_jp2_read_cdef(	mi_jp2_t * jp2,
                                    mi_BYTE * p_cdef_header_data,
                                    mi_UINT32 p_cdef_header_size,
                                    mi_event_mgr_t * p_manager );

static void mi_jp2_apply_cdef(mi_image_t *image, mi_jp2_color_t *color, mi_event_mgr_t *);

/**
 * Writes the Channel Definition box.
 *
 * @param jp2					jpeg2000 file codec.
 * @param p_nb_bytes_written	pointer to store the nb of bytes written by the function.
 *
 * @return	the data being copied.
 */
static mi_BYTE * mi_jp2_write_cdef(   mi_jp2_t *jp2,
                                                                         mi_UINT32 * p_nb_bytes_written );

/**
 * Writes the Colour Specification box.
 *
 * @param jp2					jpeg2000 file codec.
 * @param p_nb_bytes_written	pointer to store the nb of bytes written by the function.
 *
 * @return	the data being copied.
*/
static mi_BYTE * mi_jp2_write_colr(   mi_jp2_t *jp2,
                                        mi_UINT32 * p_nb_bytes_written );

/**
 * Writes a FTYP box - File type box
 *
 * @param	cio			the stream to write data to.
 * @param	jp2			the jpeg2000 file codec.
 * @param	p_manager	the user event manager.
 *
 * @return	true if writing was successful.
 */
static mi_BOOL mi_jp2_write_ftyp(	mi_jp2_t *jp2,
                                    mi_stream_private_t *cio,
                                    mi_event_mgr_t * p_manager );

/**
 * Reads a a FTYP box - File type box
 *
 * @param	p_header_data	the data contained in the FTYP box.
 * @param	jp2				the jpeg2000 file codec.
 * @param	p_header_size	the size of the data contained in the FTYP box.
 * @param	p_manager		the user event manager.
 *
 * @return true if the FTYP box is valid.
 */
static mi_BOOL mi_jp2_read_ftyp(	mi_jp2_t *jp2,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager );

static mi_BOOL mi_jp2_skip_jp2c(	mi_jp2_t *jp2,
                            mi_stream_private_t *cio,
                            mi_event_mgr_t * p_manager );

/**
 * Reads the Jpeg2000 file Header box - JP2 Header box (warning, this is a super box).
 *
 * @param	p_header_data	the data contained in the file header box.
 * @param	jp2				the jpeg2000 file codec.
 * @param	p_header_size	the size of the data contained in the file header box.
 * @param	p_manager		the user event manager.
 *
 * @return true if the JP2 Header box was successfully recognized.
*/
static mi_BOOL mi_jp2_read_jp2h(  mi_jp2_t *jp2,
                                    mi_BYTE *p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager );

/**
 * Writes the Jpeg2000 file Header box - JP2 Header box (warning, this is a super box).
 *
 * @param  jp2      the jpeg2000 file codec.
 * @param  stream      the stream to write data to.
 * @param  p_manager  user event manager.
 *
 * @return true if writing was successful.
 */
static mi_BOOL mi_jp2_write_jp2h(mi_jp2_t *jp2,
                                                        mi_stream_private_t *stream,
                                                        mi_event_mgr_t * p_manager );

/**
 * Writes the Jpeg2000 codestream Header box - JP2C Header box. This function must be called AFTER the coding has been done.
 *
 * @param	cio			the stream to write data to.
 * @param	jp2			the jpeg2000 file codec.
 * @param	p_manager	user event manager.
 *
 * @return true if writing was successful.
*/
static mi_BOOL mi_jp2_write_jp2c(	mi_jp2_t *jp2,
                                    mi_stream_private_t *cio,
                                    mi_event_mgr_t * p_manager );

/**
 * Reads a jpeg2000 file signature box.
 *
 * @param	p_header_data	the data contained in the signature box.
 * @param	jp2				the jpeg2000 file codec.
 * @param	p_header_size	the size of the data contained in the signature box.
 * @param	p_manager		the user event manager.
 *
 * @return true if the file signature box is valid.
 */
static mi_BOOL mi_jp2_read_jp(mi_jp2_t *jp2,
                                mi_BYTE * p_header_data,
                                mi_UINT32 p_header_size,
                                mi_event_mgr_t * p_manager);

/**
 * Writes a jpeg2000 file signature box.
 *
 * @param cio the stream to write data to.
 * @param	jp2			the jpeg2000 file codec.
 * @param p_manager the user event manager.
 *
 * @return true if writing was successful.
 */
static mi_BOOL mi_jp2_write_jp(	mi_jp2_t *jp2,
                                    mi_stream_private_t *cio,
                                    mi_event_mgr_t * p_manager );

/**
Apply collected palette data
@param color Collector for profile, cdef and pclr data
@param image
*/
static void mi_jp2_apply_pclr(mi_image_t *image, mi_jp2_color_t *color);

static void mi_jp2_free_pclr(mi_jp2_color_t *color);

/**
 * Collect palette data
 *
 * @param jp2 JP2 handle
 * @param p_pclr_header_data    FIXME DOC
 * @param p_pclr_header_size    FIXME DOC
 * @param p_manager
 *
 * @return Returns true if successful, returns false otherwise
*/
static mi_BOOL mi_jp2_read_pclr(	mi_jp2_t *jp2,
                                    mi_BYTE * p_pclr_header_data,
                                    mi_UINT32 p_pclr_header_size,
                                    mi_event_mgr_t * p_manager );

/**
 * Collect component mapping data
 *
 * @param jp2                 JP2 handle
 * @param p_cmap_header_data  FIXME DOC
 * @param p_cmap_header_size  FIXME DOC
 * @param p_manager           FIXME DOC
 *
 * @return Returns true if successful, returns false otherwise
*/

static mi_BOOL mi_jp2_read_cmap(	mi_jp2_t * jp2,
                                    mi_BYTE * p_cmap_header_data,
                                    mi_UINT32 p_cmap_header_size,
                                    mi_event_mgr_t * p_manager );

/**
 * Reads the Color Specification box.
 *
 * @param	p_colr_header_data			pointer to actual data (already read from file)
 * @param	jp2							the jpeg2000 file codec.
 * @param	p_colr_header_size			the size of the color header
 * @param	p_manager					the user event manager.
 *
 * @return	true if the bpc header is valid, false else.
*/
static mi_BOOL mi_jp2_read_colr(  mi_jp2_t *jp2,
                                    mi_BYTE * p_colr_header_data,
                                    mi_UINT32 p_colr_header_size,
                                    mi_event_mgr_t * p_manager );

/*@}*/

/*@}*/

/**
 * Sets up the procedures to do on writing header after the codestream.
 * Developpers wanting to extend the library can add their own writing procedures.
 */
static mi_BOOL mi_jp2_setup_end_header_writing (mi_jp2_t *jp2, mi_event_mgr_t * p_manager);

/**
 * Sets up the procedures to do on reading header after the codestream.
 * Developpers wanting to extend the library can add their own writing procedures.
 */
static mi_BOOL mi_jp2_setup_end_header_reading (mi_jp2_t *jp2, mi_event_mgr_t * p_manager);

/**
 * Reads a jpeg2000 file header structure.
 *
 * @param jp2 the jpeg2000 file header structure.
 * @param stream the stream to read data from.
 * @param p_manager the user event manager.
 *
 * @return true if the box is valid.
 */
static mi_BOOL mi_jp2_read_header_procedure(  mi_jp2_t *jp2,
                                                mi_stream_private_t *stream,
                                                mi_event_mgr_t * p_manager );

/**
 * Excutes the given procedures on the given codec.
 *
 * @param	p_procedure_list	the list of procedures to execute
 * @param	jp2					the jpeg2000 file codec to execute the procedures on.
 * @param	stream					the stream to execute the procedures on.
 * @param	p_manager			the user manager.
 *
 * @return	true				if all the procedures were successfully executed.
 */
static mi_BOOL mi_jp2_exec (  mi_jp2_t * jp2,
                                mi_procedure_list_t * p_procedure_list,
                                mi_stream_private_t *stream,
                                mi_event_mgr_t * p_manager );

/**
 * Reads a box header. The box is the way data is packed inside a jpeg2000 file structure.
 *
 * @param	cio						the input stream to read data from.
 * @param	box						the box structure to fill.
 * @param	p_number_bytes_read		pointer to an int that will store the number of bytes read from the stream (shoul usually be 2).
 * @param	p_manager				user event manager.
 *
 * @return	true if the box is recognized, false otherwise
*/
static mi_BOOL mi_jp2_read_boxhdr(mi_jp2_box_t *box,
                                    mi_UINT32 * p_number_bytes_read,
                                    mi_stream_private_t *cio,
                                    mi_event_mgr_t * p_manager);

/**
 * Sets up the validation ,i.e. adds the procedures to lauch to make sure the codec parameters
 * are valid. Developpers wanting to extend the library can add their own validation procedures.
 */
static mi_BOOL mi_jp2_setup_encoding_validation (mi_jp2_t *jp2, mi_event_mgr_t * p_manager);

/**
 * Sets up the procedures to do on writing header. Developpers wanting to extend the library can add their own writing procedures.
 */
static mi_BOOL mi_jp2_setup_header_writing (mi_jp2_t *jp2, mi_event_mgr_t * p_manager);

static mi_BOOL mi_jp2_default_validation (	mi_jp2_t * jp2,
                                        mi_stream_private_t *cio,
                                        mi_event_mgr_t * p_manager );

/**
 * Finds the image execution function related to the given box id.
 *
 * @param	p_id	the id of the handler to fetch.
 *
 * @return	the given handler or NULL if it could not be found.
 */
static const mi_jp2_header_handler_t * mi_jp2_img_find_handler (mi_UINT32 p_id);

/**
 * Finds the execution function related to the given box id.
 *
 * @param	p_id	the id of the handler to fetch.
 *
 * @return	the given handler or NULL if it could not be found.
 */
static const mi_jp2_header_handler_t * mi_jp2_find_handler (mi_UINT32 p_id );

static const mi_jp2_header_handler_t jp2_header [] =
{
    {JP2_JP,mi_jp2_read_jp},
    {JP2_FTYP,mi_jp2_read_ftyp},
    {JP2_JP2H,mi_jp2_read_jp2h}
};

static const mi_jp2_header_handler_t jp2_img_header [] =
{
    {JP2_IHDR,mi_jp2_read_ihdr},
    {JP2_COLR,mi_jp2_read_colr},
    {JP2_BPCC,mi_jp2_read_bpcc},
    {JP2_PCLR,mi_jp2_read_pclr},
    {JP2_CMAP,mi_jp2_read_cmap},
    {JP2_CDEF,mi_jp2_read_cdef}

};

/**
 * Reads a box header. The box is the way data is packed inside a jpeg2000 file structure. Data is read from a character string
 *
 * @param	box						the box structure to fill.
 * @param	p_data					the character string to read data from.
 * @param	p_number_bytes_read		pointer to an int that will store the number of bytes read from the stream (shoul usually be 2).
 * @param	p_box_max_size			the maximum number of bytes in the box.
 * @param	p_manager         FIXME DOC
 *
 * @return	true if the box is recognized, false otherwise
*/
static mi_BOOL mi_jp2_read_boxhdr_char(   mi_jp2_box_t *box,
                                            mi_BYTE * p_data,
                                            mi_UINT32 * p_number_bytes_read,
                                            mi_UINT32 p_box_max_size,
                                            mi_event_mgr_t * p_manager );

/**
 * Sets up the validation ,i.e. adds the procedures to lauch to make sure the codec parameters
 * are valid. Developpers wanting to extend the library can add their own validation procedures.
 */
static mi_BOOL mi_jp2_setup_decoding_validation (mi_jp2_t *jp2, mi_event_mgr_t * p_manager);

/**
 * Sets up the procedures to do on reading header.
 * Developpers wanting to extend the library can add their own writing procedures.
 */
static mi_BOOL mi_jp2_setup_header_reading (mi_jp2_t *jp2, mi_event_mgr_t * p_manager);

/* ----------------------------------------------------------------------- */
static mi_BOOL mi_jp2_read_boxhdr(mi_jp2_box_t *box,
                              mi_UINT32 * p_number_bytes_read,
                              mi_stream_private_t *cio,
                              mi_event_mgr_t * p_manager )
{
    /* read header from file */
    mi_BYTE l_data_header [8];

    /* preconditions */
    assert(cio != 00);
    assert(box != 00);
    assert(p_number_bytes_read != 00);
    assert(p_manager != 00);

    *p_number_bytes_read = (mi_UINT32)mi_stream_read_data(cio,l_data_header,8,p_manager);
    if (*p_number_bytes_read != 8) {
        return mi_FALSE;
    }

    /* process read data */
    mi_read_bytes(l_data_header,&(box->length), 4);
    mi_read_bytes(l_data_header+4,&(box->type), 4);
    
  if(box->length == 0)/* last box */
  {
    const mi_OFF_T bleft = mi_stream_get_number_byte_left(cio);
    if (bleft > (mi_OFF_T)(0xFFFFFFFFU - 8U)) {
      mi_event_msg(p_manager, EVT_ERROR, "Cannot handle box sizes higher than 2^32\n");
      return mi_FALSE;
    }
    box->length = (mi_UINT32)bleft + 8U;
    assert( (mi_OFF_T)box->length == bleft + 8 );
    return mi_TRUE;
  }

    /* do we have a "special very large box ?" */
    /* read then the XLBox */
    if (box->length == 1) {
        mi_UINT32 l_xl_part_size;

        mi_UINT32 l_nb_bytes_read = (mi_UINT32)mi_stream_read_data(cio,l_data_header,8,p_manager);
        if (l_nb_bytes_read != 8) {
            if (l_nb_bytes_read > 0) {
                *p_number_bytes_read += l_nb_bytes_read;
            }

            return mi_FALSE;
        }

        *p_number_bytes_read = 16;
        mi_read_bytes(l_data_header,&l_xl_part_size, 4);
        if (l_xl_part_size != 0) {
            mi_event_msg(p_manager, EVT_ERROR, "Cannot handle box sizes higher than 2^32\n");
            return mi_FALSE;
        }
        mi_read_bytes(l_data_header+4,&(box->length), 4);
    }
    return mi_TRUE;
}

static mi_BOOL mi_jp2_read_ihdr( mi_jp2_t *jp2,
                            mi_BYTE *p_image_header_data,
                            mi_UINT32 p_image_header_size,
                            mi_event_mgr_t * p_manager )
{
    /* preconditions */
    assert(p_image_header_data != 00);
    assert(jp2 != 00);
    assert(p_manager != 00);

    if (p_image_header_size != 14) {
        mi_event_msg(p_manager, EVT_ERROR, "Bad image header box (bad size)\n");
        return mi_FALSE;
    }

    mi_read_bytes(p_image_header_data,&(jp2->h),4);			/* HEIGHT */
    p_image_header_data += 4;
    mi_read_bytes(p_image_header_data,&(jp2->w),4);			/* WIDTH */
    p_image_header_data += 4;
    mi_read_bytes(p_image_header_data,&(jp2->numcomps),2);		/* NC */
    p_image_header_data += 2;

    /* allocate memory for components */
    jp2->comps = (mi_jp2_comps_t*) mi_calloc(jp2->numcomps, sizeof(mi_jp2_comps_t));
    if (jp2->comps == 0) {
        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to handle image header (ihdr)\n");
        return mi_FALSE;
    }

    mi_read_bytes(p_image_header_data,&(jp2->bpc),1);			/* BPC */
    ++ p_image_header_data;

    mi_read_bytes(p_image_header_data,&(jp2->C),1);			/* C */
    ++ p_image_header_data;

    /* Should be equal to 7 cf. chapter about image header box of the norm */
    if (jp2->C != 7){
        mi_event_msg(p_manager, EVT_INFO, "JP2 IHDR box: compression type indicate that the file is not a conforming JP2 file (%d) \n", jp2->C);
    }

    mi_read_bytes(p_image_header_data,&(jp2->UnkC),1);			/* UnkC */
    ++ p_image_header_data;
    mi_read_bytes(p_image_header_data,&(jp2->IPR),1);			/* IPR */
    ++ p_image_header_data;

    return mi_TRUE;
}

static mi_BYTE * mi_jp2_write_ihdr(mi_jp2_t *jp2,
                              mi_UINT32 * p_nb_bytes_written
                              )
{
    mi_BYTE * l_ihdr_data,* l_current_ihdr_ptr;
    
    /* preconditions */
    assert(jp2 != 00);
    assert(p_nb_bytes_written != 00);

    /* default image header is 22 bytes wide */
    l_ihdr_data = (mi_BYTE *) mi_calloc(1,22);
    if (l_ihdr_data == 00) {
        return 00;
    }

    l_current_ihdr_ptr = l_ihdr_data;
    
    mi_write_bytes(l_current_ihdr_ptr,22,4);				/* write box size */
    l_current_ihdr_ptr+=4;

    mi_write_bytes(l_current_ihdr_ptr,JP2_IHDR, 4);		/* IHDR */
    l_current_ihdr_ptr+=4;
    
    mi_write_bytes(l_current_ihdr_ptr,jp2->h, 4);		/* HEIGHT */
    l_current_ihdr_ptr+=4;
    
    mi_write_bytes(l_current_ihdr_ptr, jp2->w, 4);		/* WIDTH */
    l_current_ihdr_ptr+=4;
    
    mi_write_bytes(l_current_ihdr_ptr, jp2->numcomps, 2);		/* NC */
    l_current_ihdr_ptr+=2;
    
    mi_write_bytes(l_current_ihdr_ptr, jp2->bpc, 1);		/* BPC */
    ++l_current_ihdr_ptr;
    
    mi_write_bytes(l_current_ihdr_ptr, jp2->C, 1);		/* C : Always 7 */
    ++l_current_ihdr_ptr;
    
    mi_write_bytes(l_current_ihdr_ptr, jp2->UnkC, 1);		/* UnkC, colorspace unknown */
    ++l_current_ihdr_ptr;
    
    mi_write_bytes(l_current_ihdr_ptr, jp2->IPR, 1);		/* IPR, no intellectual property */
    ++l_current_ihdr_ptr;
    
    *p_nb_bytes_written = 22;
    
    return l_ihdr_data;
}

static mi_BYTE * mi_jp2_write_bpcc(	mi_jp2_t *jp2,
                                mi_UINT32 * p_nb_bytes_written
                                )
{
    mi_UINT32 i;
    /* room for 8 bytes for box and 1 byte for each component */
    mi_UINT32 l_bpcc_size;
    mi_BYTE * l_bpcc_data,* l_current_bpcc_ptr;
    
    /* preconditions */
    assert(jp2 != 00);
    assert(p_nb_bytes_written != 00);
    l_bpcc_size = 8 + jp2->numcomps;

    l_bpcc_data = (mi_BYTE *) mi_calloc(1,l_bpcc_size);
    if (l_bpcc_data == 00) {
        return 00;
    }

    l_current_bpcc_ptr = l_bpcc_data;

    mi_write_bytes(l_current_bpcc_ptr,l_bpcc_size,4);				/* write box size */
    l_current_bpcc_ptr += 4;
    
    mi_write_bytes(l_current_bpcc_ptr,JP2_BPCC,4);					/* BPCC */
    l_current_bpcc_ptr += 4;

    for (i = 0; i < jp2->numcomps; ++i)  {
        mi_write_bytes(l_current_bpcc_ptr, jp2->comps[i].bpcc, 1); /* write each component information */
        ++l_current_bpcc_ptr;
    }

    *p_nb_bytes_written = l_bpcc_size;
    
    return l_bpcc_data;
}

static mi_BOOL mi_jp2_read_bpcc( mi_jp2_t *jp2,
                            mi_BYTE * p_bpc_header_data,
                            mi_UINT32 p_bpc_header_size,
                            mi_event_mgr_t * p_manager
                            )
{
    mi_UINT32 i;

    /* preconditions */
    assert(p_bpc_header_data != 00);
    assert(jp2 != 00);
    assert(p_manager != 00);

    
    if (jp2->bpc != 255 ){
        mi_event_msg(p_manager, EVT_WARNING, "A BPCC header box is available although BPC given by the IHDR box (%d) indicate components bit depth is constant\n",jp2->bpc);
    }

    /* and length is relevant */
    if (p_bpc_header_size != jp2->numcomps) {
        mi_event_msg(p_manager, EVT_ERROR, "Bad BPCC header box (bad size)\n");
        return mi_FALSE;
    }

    /* read info for each component */
    for (i = 0; i < jp2->numcomps; ++i) {
        mi_read_bytes(p_bpc_header_data,&jp2->comps[i].bpcc ,1);	/* read each BPCC component */
        ++p_bpc_header_data;
    }

    return mi_TRUE;
}
static mi_BYTE * mi_jp2_write_cdef(mi_jp2_t *jp2, mi_UINT32 * p_nb_bytes_written)
{
    /* room for 8 bytes for box, 2 for n */
    mi_UINT32 l_cdef_size = 10;
    mi_BYTE * l_cdef_data,* l_current_cdef_ptr;
    mi_UINT32 l_value;
    mi_UINT16 i;

    /* preconditions */
    assert(jp2 != 00);
    assert(p_nb_bytes_written != 00);
    assert(jp2->color.jp2_cdef != 00);
    assert(jp2->color.jp2_cdef->info != 00);
    assert(jp2->color.jp2_cdef->n > 0U);

    l_cdef_size += 6U * jp2->color.jp2_cdef->n;

    l_cdef_data = (mi_BYTE *) mi_malloc(l_cdef_size);
    if (l_cdef_data == 00) {
        return 00;
    }

    l_current_cdef_ptr = l_cdef_data;
    
    mi_write_bytes(l_current_cdef_ptr,l_cdef_size,4);			/* write box size */
    l_current_cdef_ptr += 4;

    mi_write_bytes(l_current_cdef_ptr,JP2_CDEF,4);					/* BPCC */
    l_current_cdef_ptr += 4;

    l_value = jp2->color.jp2_cdef->n;
    mi_write_bytes(l_current_cdef_ptr,l_value,2);					/* N */
    l_current_cdef_ptr += 2;

    for (i = 0U; i < jp2->color.jp2_cdef->n; ++i) {
        l_value = jp2->color.jp2_cdef->info[i].cn;
        mi_write_bytes(l_current_cdef_ptr,l_value,2);					/* Cni */
        l_current_cdef_ptr += 2;
        l_value = jp2->color.jp2_cdef->info[i].typ;
        mi_write_bytes(l_current_cdef_ptr,l_value,2);					/* Typi */
        l_current_cdef_ptr += 2;
        l_value = jp2->color.jp2_cdef->info[i].asoc;
        mi_write_bytes(l_current_cdef_ptr,l_value,2);					/* Asoci */
        l_current_cdef_ptr += 2;
    }
    *p_nb_bytes_written = l_cdef_size;

    return l_cdef_data;
}

static mi_BYTE * mi_jp2_write_colr(  mi_jp2_t *jp2,
                                mi_UINT32 * p_nb_bytes_written
                                )
{
    /* room for 8 bytes for box 3 for common data and variable upon profile*/
    mi_UINT32 l_colr_size = 11;
    mi_BYTE * l_colr_data,* l_current_colr_ptr;

    /* preconditions */
    assert(jp2 != 00);
    assert(p_nb_bytes_written != 00);
    assert(jp2->meth == 1 || jp2->meth == 2);

    switch (jp2->meth) { 
        case 1 :
            l_colr_size += 4; /* EnumCS */
            break;
        case 2 :
            assert(jp2->color.icc_profile_len);	/* ICC profile */
            l_colr_size += jp2->color.icc_profile_len;
            break;
        default :
            return 00;
    }

    l_colr_data = (mi_BYTE *) mi_calloc(1,l_colr_size);
    if (l_colr_data == 00) {
        return 00;
    }
    
    l_current_colr_ptr = l_colr_data;

    mi_write_bytes(l_current_colr_ptr,l_colr_size,4);				/* write box size */
    l_current_colr_ptr += 4;
    
    mi_write_bytes(l_current_colr_ptr,JP2_COLR,4);					/* BPCC */
    l_current_colr_ptr += 4;
    
    mi_write_bytes(l_current_colr_ptr, jp2->meth,1);				/* METH */
    ++l_current_colr_ptr;
    
    mi_write_bytes(l_current_colr_ptr, jp2->precedence,1);			/* PRECEDENCE */
    ++l_current_colr_ptr;
    
    mi_write_bytes(l_current_colr_ptr, jp2->approx,1);				/* APPROX */
    ++l_current_colr_ptr;
    
    if (jp2->meth == 1) { /* Meth value is restricted to 1 or 2 (Table I.9 of part 1) */
        mi_write_bytes(l_current_colr_ptr, jp2->enumcs,4); }       /* EnumCS */
    else {
        if (jp2->meth == 2) {                                      /* ICC profile */
            mi_UINT32 i;
            for(i = 0; i < jp2->color.icc_profile_len; ++i) {
                mi_write_bytes(l_current_colr_ptr, jp2->color.icc_profile_buf[i], 1);
                ++l_current_colr_ptr;
            }
        }
    }

    *p_nb_bytes_written = l_colr_size;
    
    return l_colr_data;
}

static void mi_jp2_free_pclr(mi_jp2_color_t *color)
{
    mi_free(color->jp2_pclr->channel_sign);
    mi_free(color->jp2_pclr->channel_size);
    mi_free(color->jp2_pclr->entries);

    if(color->jp2_pclr->cmap) mi_free(color->jp2_pclr->cmap);

    mi_free(color->jp2_pclr); color->jp2_pclr = NULL;
}

static mi_BOOL mi_jp2_check_color(mi_image_t *image, mi_jp2_color_t *color, mi_event_mgr_t *p_manager)
{
    mi_UINT16 i;

    /* testcase 4149.pdf.SIGSEGV.cf7.3501 */
    if (color->jp2_cdef) {
        mi_jp2_cdef_info_t *info = color->jp2_cdef->info;
        mi_UINT16 n = color->jp2_cdef->n;
        mi_UINT32 nr_channels = image->numcomps; /* FIXME image->numcomps == jp2->numcomps before color is applied ??? */

        /* cdef applies to cmap channels if any */
        if (color->jp2_pclr && color->jp2_pclr->cmap) {
            nr_channels = (mi_UINT32)color->jp2_pclr->nr_channels;
        }

        for (i = 0; i < n; i++) {
            if (info[i].cn >= nr_channels) {
                mi_event_msg(p_manager, EVT_ERROR, "Invalid component index %d (>= %d).\n", info[i].cn, nr_channels);
                return mi_FALSE;
            }
            if (info[i].asoc == 65535U) continue;

            if (info[i].asoc > 0 && (mi_UINT32)(info[i].asoc - 1) >= nr_channels) {
                mi_event_msg(p_manager, EVT_ERROR, "Invalid component index %d (>= %d).\n", info[i].asoc - 1, nr_channels);
                return mi_FALSE;
            }
        }

        /* issue 397 */
        /* ISO 15444-1 states that if cdef is present, it shall contain a complete list of channel definitions. */
        while (nr_channels > 0)
        {
            for(i = 0; i < n; ++i) {
                if ((mi_UINT32)info[i].cn == (nr_channels - 1U)) {
                    break;
                }
            }
            if (i == n) {
                mi_event_msg(p_manager, EVT_ERROR, "Incomplete channel definitions.\n");
                return mi_FALSE;
            }
            --nr_channels;
        }
    }

    /* testcases 451.pdf.SIGSEGV.f4c.3723, 451.pdf.SIGSEGV.5b5.3723 and
       66ea31acbb0f23a2bbc91f64d69a03f5_signal_sigsegv_13937c0_7030_5725.pdf */
    if (color->jp2_pclr && color->jp2_pclr->cmap) {
        mi_UINT16 nr_channels = color->jp2_pclr->nr_channels;
        mi_jp2_cmap_comp_t *cmap = color->jp2_pclr->cmap;
        mi_BOOL *pcol_usage, is_sane = mi_TRUE;

        /* verify that all original components match an existing one */
        for (i = 0; i < nr_channels; i++) {
            if (cmap[i].cmp >= image->numcomps) {
                mi_event_msg(p_manager, EVT_ERROR, "Invalid component index %d (>= %d).\n", cmap[i].cmp, image->numcomps);
                is_sane = mi_FALSE;
            }
        }

        pcol_usage = (mi_BOOL *) mi_calloc(nr_channels, sizeof(mi_BOOL));
        if (!pcol_usage) {
            mi_event_msg(p_manager, EVT_ERROR, "Unexpected OOM.\n");
            return mi_FALSE;
        }
        /* verify that no component is targeted more than once */
        for (i = 0; i < nr_channels; i++) {
      mi_UINT16 pcol = cmap[i].pcol;
      assert(cmap[i].mtyp == 0 || cmap[i].mtyp == 1);
            if (pcol >= nr_channels) {
                mi_event_msg(p_manager, EVT_ERROR, "Invalid component/palette index for direct mapping %d.\n", pcol);
                is_sane = mi_FALSE;
            }
            else if (pcol_usage[pcol] && cmap[i].mtyp == 1) {
                mi_event_msg(p_manager, EVT_ERROR, "Component %d is mapped twice.\n", pcol);
                is_sane = mi_FALSE;
            }
      else if (cmap[i].mtyp == 0 && cmap[i].pcol != 0) {
        /* I.5.3.5 PCOL: If the value of the MTYP field for this channel is 0, then
         * the value of this field shall be 0. */
                mi_event_msg(p_manager, EVT_ERROR, "Direct use at #%d however pcol=%d.\n", i, pcol);
                is_sane = mi_FALSE;
      }
            else
                pcol_usage[pcol] = mi_TRUE;
        }
        /* verify that all components are targeted at least once */
        for (i = 0; i < nr_channels; i++) {
            if (!pcol_usage[i] && cmap[i].mtyp != 0) {
                mi_event_msg(p_manager, EVT_ERROR, "Component %d doesn't have a mapping.\n", i);
                is_sane = mi_FALSE;
            }
        }
        /* Issue 235/447 weird cmap */
        if (1 && is_sane && (image->numcomps==1U)) {
            for (i = 0; i < nr_channels; i++) {
                if (!pcol_usage[i]) {
                    is_sane = 0U;
                    mi_event_msg(p_manager, EVT_WARNING, "Component mapping seems wrong. Trying to correct.\n", i);
                    break;
                }
            }
            if (!is_sane) {
                is_sane = mi_TRUE;
                for (i = 0; i < nr_channels; i++) {
                    cmap[i].mtyp = 1U;
                    cmap[i].pcol = (mi_BYTE) i;
                }
            }
        }
        mi_free(pcol_usage);
        if (!is_sane) {
            return mi_FALSE;
        }
    }

    return mi_TRUE;
}

/* file9.jp2 */
static void mi_jp2_apply_pclr(mi_image_t *image, mi_jp2_color_t *color)
{
    mi_image_comp_t *old_comps, *new_comps;
    mi_BYTE *channel_size, *channel_sign;
    mi_UINT32 *entries;
    mi_jp2_cmap_comp_t *cmap;
    mi_INT32 *src, *dst;
    mi_UINT32 j, max;
    mi_UINT16 i, nr_channels, cmp, pcol;
    mi_INT32 k, top_k;

    channel_size = color->jp2_pclr->channel_size;
    channel_sign = color->jp2_pclr->channel_sign;
    entries = color->jp2_pclr->entries;
    cmap = color->jp2_pclr->cmap;
    nr_channels = color->jp2_pclr->nr_channels;

    old_comps = image->comps;
    new_comps = (mi_image_comp_t*)
            mi_malloc(nr_channels * sizeof(mi_image_comp_t));
    if (!new_comps) {
        /* FIXME no error code for mi_jp2_apply_pclr */
        /* FIXME event manager error callback */
        return;
    }
    for(i = 0; i < nr_channels; ++i) {
        pcol = cmap[i].pcol; cmp = cmap[i].cmp;

        /* Direct use */
    if(cmap[i].mtyp == 0){
      assert( pcol == 0 );
      new_comps[i] = old_comps[cmp];
    } else {
      assert( i == pcol );
      new_comps[pcol] = old_comps[cmp];
    }

        /* Palette mapping: */
        new_comps[i].data = (mi_INT32*)
                mi_malloc(old_comps[cmp].w * old_comps[cmp].h * sizeof(mi_INT32));
        if (!new_comps[i].data) {
            mi_free(new_comps);
            new_comps = NULL;
            /* FIXME no error code for mi_jp2_apply_pclr */
            /* FIXME event manager error callback */
            return;
        }
        new_comps[i].prec = channel_size[i];
        new_comps[i].sgnd = channel_sign[i];
    }

    top_k = color->jp2_pclr->nr_entries - 1;

    for(i = 0; i < nr_channels; ++i) {
        /* Palette mapping: */
        cmp = cmap[i].cmp; pcol = cmap[i].pcol;
        src = old_comps[cmp].data;
    assert( src );
        max = new_comps[pcol].w * new_comps[pcol].h;

        /* Direct use: */
    if(cmap[i].mtyp == 0) {
      assert( cmp == 0 );
      dst = new_comps[i].data;
      assert( dst );
      for(j = 0; j < max; ++j) {
        dst[j] = src[j];
      }
    }
    else {
      assert( i == pcol );
      dst = new_comps[pcol].data;
      assert( dst );
      for(j = 0; j < max; ++j) {
        /* The index */
        if((k = src[j]) < 0) k = 0; else if(k > top_k) k = top_k;

        /* The colour */
        dst[j] = (mi_INT32)entries[k * nr_channels + pcol];
        }
    }
    }

    max = image->numcomps;
    for(i = 0; i < max; ++i) {
        if(old_comps[i].data) mi_free(old_comps[i].data);
    }

    mi_free(old_comps);
    image->comps = new_comps;
    image->numcomps = nr_channels;

    mi_jp2_free_pclr(color);

}/* apply_pclr() */

static mi_BOOL mi_jp2_read_pclr(	mi_jp2_t *jp2,
                            mi_BYTE * p_pclr_header_data,
                            mi_UINT32 p_pclr_header_size,
                            mi_event_mgr_t * p_manager
                            )
{
    mi_jp2_pclr_t *jp2_pclr;
    mi_BYTE *channel_size, *channel_sign;
    mi_UINT32 *entries;
    mi_UINT16 nr_entries,nr_channels;
    mi_UINT16 i, j;
    mi_UINT32 l_value;
    mi_BYTE *orig_header_data = p_pclr_header_data;

    /* preconditions */
    assert(p_pclr_header_data != 00);
    assert(jp2 != 00);
    assert(p_manager != 00);
    (void)p_pclr_header_size;

    if(jp2->color.jp2_pclr)
        return mi_FALSE;

    if (p_pclr_header_size < 3)
        return mi_FALSE;

    mi_read_bytes(p_pclr_header_data, &l_value , 2);	/* NE */
    p_pclr_header_data += 2;
    nr_entries = (mi_UINT16) l_value;
    if ((nr_entries == 0U) || (nr_entries > 1024U)) {
        mi_event_msg(p_manager, EVT_ERROR, "Invalid PCLR box. Reports %d entries\n", (int)nr_entries);
        return mi_FALSE;
    }

    mi_read_bytes(p_pclr_header_data, &l_value , 1);	/* NPC */
    ++p_pclr_header_data;
    nr_channels = (mi_UINT16) l_value;
    if (nr_channels == 0U) {
        mi_event_msg(p_manager, EVT_ERROR, "Invalid PCLR box. Reports 0 palette columns\n");
        return mi_FALSE;
    }

    if (p_pclr_header_size < 3 + (mi_UINT32)nr_channels)
        return mi_FALSE;

    entries = (mi_UINT32*) mi_malloc((size_t)nr_channels * nr_entries * sizeof(mi_UINT32));
    if (!entries)
        return mi_FALSE;
    channel_size = (mi_BYTE*) mi_malloc(nr_channels);
    if (!channel_size)
    {
        mi_free(entries);
        return mi_FALSE;
    }
    channel_sign = (mi_BYTE*) mi_malloc(nr_channels);
    if (!channel_sign)
    {
        mi_free(entries);
        mi_free(channel_size);
        return mi_FALSE;
    }

    jp2_pclr = (mi_jp2_pclr_t*)mi_malloc(sizeof(mi_jp2_pclr_t));
    if (!jp2_pclr)
    {
        mi_free(entries);
        mi_free(channel_size);
        mi_free(channel_sign);
        return mi_FALSE;
    }

    jp2_pclr->channel_sign = channel_sign;
    jp2_pclr->channel_size = channel_size;
    jp2_pclr->entries = entries;
    jp2_pclr->nr_entries = nr_entries;
    jp2_pclr->nr_channels = (mi_BYTE) l_value;
    jp2_pclr->cmap = NULL;

    jp2->color.jp2_pclr = jp2_pclr;

    for(i = 0; i < nr_channels; ++i) {
        mi_read_bytes(p_pclr_header_data, &l_value , 1);	/* Bi */
        ++p_pclr_header_data;

        channel_size[i] = (mi_BYTE)((l_value & 0x7f) + 1);
        channel_sign[i] = (l_value & 0x80) ? 1 : 0;
    }

    for(j = 0; j < nr_entries; ++j) {
        for(i = 0; i < nr_channels; ++i) {
            mi_UINT32 bytes_to_read = (mi_UINT32)((channel_size[i]+7)>>3);

            if (bytes_to_read > sizeof(mi_UINT32))
                bytes_to_read = sizeof(mi_UINT32);
            if ((ptrdiff_t)p_pclr_header_size < (ptrdiff_t)(p_pclr_header_data - orig_header_data) + (ptrdiff_t)bytes_to_read)
                return mi_FALSE;

            mi_read_bytes(p_pclr_header_data, &l_value , bytes_to_read);	/* Cji */
            p_pclr_header_data += bytes_to_read;
            *entries = (mi_UINT32) l_value;
            entries++;
        }
    }

    return mi_TRUE;
}

static mi_BOOL mi_jp2_read_cmap(	mi_jp2_t * jp2,
                            mi_BYTE * p_cmap_header_data,
                            mi_UINT32 p_cmap_header_size,
                            mi_event_mgr_t * p_manager
                            )
{
    mi_jp2_cmap_comp_t *cmap;
    mi_BYTE i, nr_channels;
    mi_UINT32 l_value;

    /* preconditions */
    assert(jp2 != 00);
    assert(p_cmap_header_data != 00);
    assert(p_manager != 00);
    (void)p_cmap_header_size;

    /* Need nr_channels: */
    if(jp2->color.jp2_pclr == NULL) {
        mi_event_msg(p_manager, EVT_ERROR, "Need to read a PCLR box before the CMAP box.\n");
        return mi_FALSE;
    }

    /* Part 1, I.5.3.5: 'There shall be at most one Component Mapping box
     * inside a JP2 Header box' :
    */
    if(jp2->color.jp2_pclr->cmap) {
        mi_event_msg(p_manager, EVT_ERROR, "Only one CMAP box is allowed.\n");
        return mi_FALSE;
    }

    nr_channels = jp2->color.jp2_pclr->nr_channels;
    if (p_cmap_header_size < (mi_UINT32)nr_channels * 4) {
        mi_event_msg(p_manager, EVT_ERROR, "Insufficient data for CMAP box.\n");
        return mi_FALSE;
    }

    cmap = (mi_jp2_cmap_comp_t*) mi_malloc(nr_channels * sizeof(mi_jp2_cmap_comp_t));
    if (!cmap)
        return mi_FALSE;


    for(i = 0; i < nr_channels; ++i) {
        mi_read_bytes(p_cmap_header_data, &l_value, 2);			/* CMP^i */
        p_cmap_header_data +=2;
        cmap[i].cmp = (mi_UINT16) l_value;

        mi_read_bytes(p_cmap_header_data, &l_value, 1);			/* MTYP^i */
        ++p_cmap_header_data;
        cmap[i].mtyp = (mi_BYTE) l_value;

        mi_read_bytes(p_cmap_header_data, &l_value, 1);			/* PCOL^i */
        ++p_cmap_header_data;
        cmap[i].pcol = (mi_BYTE) l_value;
    }

    jp2->color.jp2_pclr->cmap = cmap;

    return mi_TRUE;
}

static void mi_jp2_apply_cdef(mi_image_t *image, mi_jp2_color_t *color, mi_event_mgr_t *manager)
{
    mi_jp2_cdef_info_t *info;
    mi_UINT16 i, n, cn, asoc, acn;
    
    info = color->jp2_cdef->info;
    n = color->jp2_cdef->n;
    
    for(i = 0; i < n; ++i)
    {
        /* WATCH: acn = asoc - 1 ! */
        asoc = info[i].asoc;
        cn = info[i].cn;
        
        if( cn >= image->numcomps)
        {
            mi_event_msg(manager, EVT_WARNING, "mi_jp2_apply_cdef: cn=%d, numcomps=%d\n", cn, image->numcomps);
            continue;
        }
        if(asoc == 0 || asoc == 65535)
        {
            image->comps[cn].alpha = info[i].typ;
            continue;
        }
        
        acn = (mi_UINT16)(asoc - 1);
        if( acn >= image->numcomps )
        {
            mi_event_msg(manager, EVT_WARNING, "mi_jp2_apply_cdef: acn=%d, numcomps=%d\n", acn, image->numcomps);
            continue;
        }
        
        /* Swap only if color channel */
        if((cn != acn) && (info[i].typ == 0))
        {
            mi_image_comp_t saved;
            mi_UINT16 j;
            
            memcpy(&saved, &image->comps[cn], sizeof(mi_image_comp_t));
            memcpy(&image->comps[cn], &image->comps[acn], sizeof(mi_image_comp_t));
            memcpy(&image->comps[acn], &saved, sizeof(mi_image_comp_t));
            
            /* Swap channels in following channel definitions, don't bother with j <= i that are already processed */
            for (j = (mi_UINT16)(i + 1U); j < n ; ++j)
            {
                if (info[j].cn == cn) {
                    info[j].cn = acn;
                }
                else if (info[j].cn == acn) {
                    info[j].cn = cn;
                }
                /* asoc is related to color index. Do not update. */
            }
        }
        
        image->comps[cn].alpha = info[i].typ;
    }
    
    if(color->jp2_cdef->info) mi_free(color->jp2_cdef->info);
    
    mi_free(color->jp2_cdef); color->jp2_cdef = NULL;
    
}/* jp2_apply_cdef() */

static mi_BOOL mi_jp2_read_cdef(	mi_jp2_t * jp2,
                            mi_BYTE * p_cdef_header_data,
                            mi_UINT32 p_cdef_header_size,
                            mi_event_mgr_t * p_manager
                            )
{
    mi_jp2_cdef_info_t *cdef_info;
    mi_UINT16 i;
    mi_UINT32 l_value;

    /* preconditions */
    assert(jp2 != 00);
    assert(p_cdef_header_data != 00);
    assert(p_manager != 00);
    (void)p_cdef_header_size;

    /* Part 1, I.5.3.6: 'The shall be at most one Channel Definition box
     * inside a JP2 Header box.'*/
    if(jp2->color.jp2_cdef) return mi_FALSE;

    if (p_cdef_header_size < 2) {
        mi_event_msg(p_manager, EVT_ERROR, "Insufficient data for CDEF box.\n");
        return mi_FALSE;
    }

    mi_read_bytes(p_cdef_header_data,&l_value ,2);			/* N */
    p_cdef_header_data+= 2;

    if ( (mi_UINT16)l_value == 0){ /* szukw000: FIXME */
        mi_event_msg(p_manager, EVT_ERROR, "Number of channel description is equal to zero in CDEF box.\n");
        return mi_FALSE;
    }

    if (p_cdef_header_size < 2 + (mi_UINT32)(mi_UINT16)l_value * 6) {
        mi_event_msg(p_manager, EVT_ERROR, "Insufficient data for CDEF box.\n");
        return mi_FALSE;
    }

    cdef_info = (mi_jp2_cdef_info_t*) mi_malloc(l_value * sizeof(mi_jp2_cdef_info_t));
    if (!cdef_info)
        return mi_FALSE;

    jp2->color.jp2_cdef = (mi_jp2_cdef_t*)mi_malloc(sizeof(mi_jp2_cdef_t));
    if(!jp2->color.jp2_cdef)
    {
        mi_free(cdef_info);
        return mi_FALSE;
    }
    jp2->color.jp2_cdef->info = cdef_info;
    jp2->color.jp2_cdef->n = (mi_UINT16) l_value;

    for(i = 0; i < jp2->color.jp2_cdef->n; ++i) {
        mi_read_bytes(p_cdef_header_data, &l_value, 2);			/* Cn^i */
        p_cdef_header_data +=2;
        cdef_info[i].cn = (mi_UINT16) l_value;

        mi_read_bytes(p_cdef_header_data, &l_value, 2);			/* Typ^i */
        p_cdef_header_data +=2;
        cdef_info[i].typ = (mi_UINT16) l_value;

        mi_read_bytes(p_cdef_header_data, &l_value, 2);			/* Asoc^i */
        p_cdef_header_data +=2;
        cdef_info[i].asoc = (mi_UINT16) l_value;
   }

    return mi_TRUE;
}

static mi_BOOL mi_jp2_read_colr( mi_jp2_t *jp2,
                            mi_BYTE * p_colr_header_data,
                            mi_UINT32 p_colr_header_size,
                            mi_event_mgr_t * p_manager
                            )
{
    mi_UINT32 l_value;

    /* preconditions */
    assert(jp2 != 00);
    assert(p_colr_header_data != 00);
    assert(p_manager != 00);

    if (p_colr_header_size < 3) {
        mi_event_msg(p_manager, EVT_ERROR, "Bad COLR header box (bad size)\n");
        return mi_FALSE;
    }

    /* Part 1, I.5.3.3 : 'A conforming JP2 reader shall ignore all Colour
     * Specification boxes after the first.'
    */
    if(jp2->color.jp2_has_colr) {
        mi_event_msg(p_manager, EVT_INFO, "A conforming JP2 reader shall ignore all Colour Specification boxes after the first, so we ignore this one.\n");
        p_colr_header_data += p_colr_header_size;
        return mi_TRUE;
    }

    mi_read_bytes(p_colr_header_data,&jp2->meth ,1);			/* METH */
    ++p_colr_header_data;

    mi_read_bytes(p_colr_header_data,&jp2->precedence ,1);		/* PRECEDENCE */
    ++p_colr_header_data;

    mi_read_bytes(p_colr_header_data,&jp2->approx ,1);			/* APPROX */
    ++p_colr_header_data;

    if (jp2->meth == 1) {
        if (p_colr_header_size < 7) {
            mi_event_msg(p_manager, EVT_ERROR, "Bad COLR header box (bad size: %d)\n", p_colr_header_size);
            return mi_FALSE;
        }
        if ((p_colr_header_size > 7) && (jp2->enumcs != 14)) { /* handled below for CIELab) */
            /* testcase Altona_Technical_v20_x4.pdf */
            mi_event_msg(p_manager, EVT_WARNING, "Bad COLR header box (bad size: %d)\n", p_colr_header_size);
        }

        mi_read_bytes(p_colr_header_data,&jp2->enumcs ,4);			/* EnumCS */

        p_colr_header_data += 4;

        if(jp2->enumcs == 14)/* CIELab */
        {
            mi_UINT32 *cielab;
            mi_UINT32 rl, ol, ra, oa, rb, ob, il;

            cielab = (mi_UINT32*)mi_malloc(9 * sizeof(mi_UINT32));
            if(cielab == NULL){
                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory for cielab\n");
                return mi_FALSE;
            }
            cielab[0] = 14; /* enumcs */
            
            /* default values */
            rl = ra = rb = ol = oa = ob = 0;
            il = 0x00443530; /* D50 */
            cielab[1] = 0x44454600;/* DEF */

            if(p_colr_header_size == 35)
            {
                mi_read_bytes(p_colr_header_data, &rl, 4);
                p_colr_header_data += 4;
                mi_read_bytes(p_colr_header_data, &ol, 4);
                p_colr_header_data += 4;
                mi_read_bytes(p_colr_header_data, &ra, 4);
                p_colr_header_data += 4;
                mi_read_bytes(p_colr_header_data, &oa, 4);
                p_colr_header_data += 4;
                mi_read_bytes(p_colr_header_data, &rb, 4);
                p_colr_header_data += 4;
                mi_read_bytes(p_colr_header_data, &ob, 4);
                p_colr_header_data += 4;
                mi_read_bytes(p_colr_header_data, &il, 4);
                p_colr_header_data += 4;
                
                cielab[1] = 0;
            }
            else if(p_colr_header_size != 7)
            {
                mi_event_msg(p_manager, EVT_WARNING, "Bad COLR header box (CIELab, bad size: %d)\n", p_colr_header_size);
            }
            cielab[2] = rl; cielab[4] = ra; cielab[6] = rb;
            cielab[3] = ol; cielab[5] = oa; cielab[7] = ob;
            cielab[8] = il;

            jp2->color.icc_profile_buf = (mi_BYTE*)cielab;
            jp2->color.icc_profile_len = 0;
        }
        jp2->color.jp2_has_colr = 1;
    }
    else if (jp2->meth == 2) {
        /* ICC profile */
        mi_INT32 it_icc_value = 0;
        mi_INT32 icc_len = (mi_INT32)p_colr_header_size - 3;

        jp2->color.icc_profile_len = (mi_UINT32)icc_len;
        jp2->color.icc_profile_buf = (mi_BYTE*) mi_calloc(1,(size_t)icc_len);
        if (!jp2->color.icc_profile_buf)
        {
            jp2->color.icc_profile_len = 0;
            return mi_FALSE;
        }

        for (it_icc_value = 0; it_icc_value < icc_len; ++it_icc_value)
        {
            mi_read_bytes(p_colr_header_data,&l_value,1);		/* icc values */
            ++p_colr_header_data;
            jp2->color.icc_profile_buf[it_icc_value] = (mi_BYTE) l_value;
        }
        
        jp2->color.jp2_has_colr = 1;
    }
    else if (jp2->meth > 2)
    {
        /*	ISO/IEC 15444-1:2004 (E), Table I.9 Legal METH values:
        conforming JP2 reader shall ignore the entire Colour Specification box.*/
        mi_event_msg(p_manager, EVT_INFO, "COLR BOX meth value is not a regular value (%d), "
            "so we will ignore the entire Colour Specification box. \n", jp2->meth);
    }
    return mi_TRUE;
}

mi_BOOL mi_jp2_decode(mi_jp2_t *jp2,
                        mi_stream_private_t *p_stream,
                        mi_image_t* p_image,
                        mi_event_mgr_t * p_manager)
{
    if (!p_image)
        return mi_FALSE;

    /* J2K decoding */
    if( ! mi_j2k_decode(jp2->j2k, p_stream, p_image, p_manager) ) {
        mi_event_msg(p_manager, EVT_ERROR, "Failed to decode the codestream in the JP2 file\n");
        return mi_FALSE;
    }

    if (!jp2->ignore_pclr_cmap_cdef){
        if (!mi_jp2_check_color(p_image, &(jp2->color), p_manager)) {
            return mi_FALSE;
        }

        /* Set Image Color Space */
        if (jp2->enumcs == 16)
            p_image->color_space = mi_CLRSPC_SRGB;
        else if (jp2->enumcs == 17)
            p_image->color_space = mi_CLRSPC_GRAY;
        else if (jp2->enumcs == 18)
            p_image->color_space = mi_CLRSPC_SYCC;
        else if (jp2->enumcs == 24)
            p_image->color_space = mi_CLRSPC_EYCC;
        else if (jp2->enumcs == 12)
            p_image->color_space = mi_CLRSPC_CMYK;
        else
            p_image->color_space = mi_CLRSPC_UNKNOWN;

        if(jp2->color.jp2_pclr) {
            /* Part 1, I.5.3.4: Either both or none : */
            if( !jp2->color.jp2_pclr->cmap)
                mi_jp2_free_pclr(&(jp2->color));
            else
                mi_jp2_apply_pclr(p_image, &(jp2->color));
        }

        /* Apply the color space if needed */
        if(jp2->color.jp2_cdef) {
            mi_jp2_apply_cdef(p_image, &(jp2->color), p_manager);
        }

        if(jp2->color.icc_profile_buf) {
            p_image->icc_profile_buf = jp2->color.icc_profile_buf;
            p_image->icc_profile_len = jp2->color.icc_profile_len;
            jp2->color.icc_profile_buf = NULL;
        }
    }

    return mi_TRUE;
}

static mi_BOOL mi_jp2_write_jp2h(mi_jp2_t *jp2,
                            mi_stream_private_t *stream,
                            mi_event_mgr_t * p_manager
                            )
{
    mi_jp2_img_header_writer_handler_t l_writers [4];
    mi_jp2_img_header_writer_handler_t * l_current_writer;

    mi_INT32 i, l_nb_pass;
    /* size of data for super box*/
    mi_UINT32 l_jp2h_size = 8;
    mi_BOOL l_result = mi_TRUE;

    /* to store the data of the super box */
    mi_BYTE l_jp2h_data [8];
    
    /* preconditions */
    assert(stream != 00);
    assert(jp2 != 00);
    assert(p_manager != 00);

    memset(l_writers,0,sizeof(l_writers));

    if (jp2->bpc == 255) {
        l_nb_pass = 3;
        l_writers[0].handler = mi_jp2_write_ihdr;
        l_writers[1].handler = mi_jp2_write_bpcc;
        l_writers[2].handler = mi_jp2_write_colr;
    }
    else {
        l_nb_pass = 2;
        l_writers[0].handler = mi_jp2_write_ihdr;
        l_writers[1].handler = mi_jp2_write_colr;
    }
    
    if (jp2->color.jp2_cdef != NULL) {
        l_writers[l_nb_pass].handler = mi_jp2_write_cdef;
        l_nb_pass++;
    }
    
    /* write box header */
    /* write JP2H type */
    mi_write_bytes(l_jp2h_data+4,JP2_JP2H,4);

    l_current_writer = l_writers;
    for (i=0;i<l_nb_pass;++i) {
        l_current_writer->m_data = l_current_writer->handler(jp2,&(l_current_writer->m_size));
        if (l_current_writer->m_data == 00) {
            mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to hold JP2 Header data\n");
            l_result = mi_FALSE;
            break;
        }

        l_jp2h_size += l_current_writer->m_size;
        ++l_current_writer;
    }

    if (! l_result) {
        l_current_writer = l_writers;
        for (i=0;i<l_nb_pass;++i) {
            if (l_current_writer->m_data != 00) {
                mi_free(l_current_writer->m_data );
            }
            ++l_current_writer;
        }

        return mi_FALSE;
    }

    /* write super box size */
    mi_write_bytes(l_jp2h_data,l_jp2h_size,4);
    
    /* write super box data on stream */
    if (mi_stream_write_data(stream,l_jp2h_data,8,p_manager) != 8) {
        mi_event_msg(p_manager, EVT_ERROR, "Stream error while writing JP2 Header box\n");
        l_result = mi_FALSE;
    }
    
    if (l_result) {
        l_current_writer = l_writers;
        for (i=0;i<l_nb_pass;++i) {
            if (mi_stream_write_data(stream,l_current_writer->m_data,l_current_writer->m_size,p_manager) != l_current_writer->m_size) {
                mi_event_msg(p_manager, EVT_ERROR, "Stream error while writing JP2 Header box\n");
                l_result = mi_FALSE;
                break;
            }
            ++l_current_writer;
        }
    }

    l_current_writer = l_writers;
    
    /* cleanup */
    for (i=0;i<l_nb_pass;++i) {
        if (l_current_writer->m_data != 00) {
            mi_free(l_current_writer->m_data );
        }
        ++l_current_writer;
    }

    return l_result;
}

static mi_BOOL mi_jp2_write_ftyp(mi_jp2_t *jp2,
                            mi_stream_private_t *cio,
                            mi_event_mgr_t * p_manager )
{
    mi_UINT32 i;
    mi_UINT32 l_ftyp_size;
    mi_BYTE * l_ftyp_data, * l_current_data_ptr;
    mi_BOOL l_result;

    /* preconditions */
    assert(cio != 00);
    assert(jp2 != 00);
    assert(p_manager != 00);
    l_ftyp_size = 16 + 4 * jp2->numcl;

    l_ftyp_data = (mi_BYTE *) mi_calloc(1,l_ftyp_size);
    
    if (l_ftyp_data == 00) {
        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to handle ftyp data\n");
        return mi_FALSE;
    }

    l_current_data_ptr = l_ftyp_data;

    mi_write_bytes(l_current_data_ptr, l_ftyp_size,4); /* box size */
    l_current_data_ptr += 4;

    mi_write_bytes(l_current_data_ptr, JP2_FTYP,4); /* FTYP */
    l_current_data_ptr += 4;

    mi_write_bytes(l_current_data_ptr, jp2->brand,4); /* BR */
    l_current_data_ptr += 4;

    mi_write_bytes(l_current_data_ptr, jp2->minversion,4); /* MinV */
    l_current_data_ptr += 4;

    for (i = 0; i < jp2->numcl; i++)  {
        mi_write_bytes(l_current_data_ptr, jp2->cl[i],4);	/* CL */
    }
    
    l_result = (mi_stream_write_data(cio,l_ftyp_data,l_ftyp_size,p_manager) == l_ftyp_size);
    if (! l_result)
    {
        mi_event_msg(p_manager, EVT_ERROR, "Error while writing ftyp data to stream\n");
    }

    mi_free(l_ftyp_data);
    
    return l_result;
}

static mi_BOOL mi_jp2_write_jp2c(mi_jp2_t *jp2,
                            mi_stream_private_t *cio,
                            mi_event_mgr_t * p_manager )
{
    mi_OFF_T j2k_codestream_exit;
    mi_BYTE l_data_header [8];
    
    /* preconditions */
    assert(jp2 != 00);
    assert(cio != 00);
    assert(p_manager != 00);
    assert(mi_stream_has_seek(cio));
    
    j2k_codestream_exit = mi_stream_tell(cio);
    mi_write_bytes(l_data_header,
                    (mi_UINT32) (j2k_codestream_exit - jp2->j2k_codestream_offset),
                    4); /* size of codestream */
    mi_write_bytes(l_data_header + 4,JP2_JP2C,4);									   /* JP2C */

    if (! mi_stream_seek(cio,jp2->j2k_codestream_offset,p_manager)) {
        mi_event_msg(p_manager, EVT_ERROR, "Failed to seek in the stream.\n");
        return mi_FALSE;
    }
    
    if (mi_stream_write_data(cio,l_data_header,8,p_manager) != 8) {
        mi_event_msg(p_manager, EVT_ERROR, "Failed to seek in the stream.\n");
        return mi_FALSE;
    }

    if (! mi_stream_seek(cio,j2k_codestream_exit,p_manager)) {
        mi_event_msg(p_manager, EVT_ERROR, "Failed to seek in the stream.\n");
        return mi_FALSE;
    }

    return mi_TRUE;
}

static mi_BOOL mi_jp2_write_jp(	mi_jp2_t *jp2,
                            mi_stream_private_t *cio,
                            mi_event_mgr_t * p_manager )
{
    /* 12 bytes will be read */
    mi_BYTE l_signature_data [12];

    /* preconditions */
    assert(cio != 00);
    assert(jp2 != 00);
    assert(p_manager != 00);

    /* write box length */
    mi_write_bytes(l_signature_data,12,4);
    /* writes box type */
    mi_write_bytes(l_signature_data+4,JP2_JP,4);
    /* writes magic number*/
    mi_write_bytes(l_signature_data+8,0x0d0a870a,4);
    
    if (mi_stream_write_data(cio,l_signature_data,12,p_manager) != 12) {
        return mi_FALSE;
    }

    return mi_TRUE;
}

/* ----------------------------------------------------------------------- */
/* JP2 decoder interface                                             */
/* ----------------------------------------------------------------------- */

void mi_jp2_setup_decoder(mi_jp2_t *jp2, mi_dparameters_t *parameters)
{
    /* setup the J2K codec */
    mi_j2k_setup_decoder(jp2->j2k, parameters);

    /* further JP2 initializations go here */
    jp2->color.jp2_has_colr = 0;
    jp2->ignore_pclr_cmap_cdef = parameters->flags & mi_DPARAMETERS_IGNORE_PCLR_CMAP_CDEF_FLAG;
}

/* ----------------------------------------------------------------------- */
/* JP2 encoder interface                                             */
/* ----------------------------------------------------------------------- */

mi_BOOL mi_jp2_setup_encoder(	mi_jp2_t *jp2,
                            mi_cparameters_t *parameters,
                            mi_image_t *image,
                            mi_event_mgr_t * p_manager)
{
    mi_UINT32 i;
    mi_UINT32 depth_0;
  mi_UINT32 sign;
    mi_UINT32 alpha_count;
    mi_UINT32 color_channels = 0U;
    mi_UINT32 alpha_channel = 0U;
    

    if(!jp2 || !parameters || !image)
        return mi_FALSE;

    /* setup the J2K codec */
    /* ------------------- */

    /* Check if number of components respects standard */
    if (image->numcomps < 1 || image->numcomps > 16384) {
        mi_event_msg(p_manager, EVT_ERROR, "Invalid number of components specified while setting up JP2 encoder\n");
        return mi_FALSE;
    }

    if (mi_j2k_setup_encoder(jp2->j2k, parameters, image, p_manager ) == mi_FALSE) {
        return mi_FALSE;
    }

    /* setup the JP2 codec */
    /* ------------------- */
    
    /* Profile box */

    jp2->brand = JP2_JP2;	/* BR */
    jp2->minversion = 0;	/* MinV */
    jp2->numcl = 1;
    jp2->cl = (mi_UINT32*) mi_malloc(jp2->numcl * sizeof(mi_UINT32));
    if (!jp2->cl){
        jp2->cl = NULL;
        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory when setup the JP2 encoder\n");
        return mi_FALSE;
    }
    jp2->cl[0] = JP2_JP2;	/* CL0 : JP2 */

    /* Image Header box */

    jp2->numcomps = image->numcomps;	/* NC */
    jp2->comps = (mi_jp2_comps_t*) mi_malloc(jp2->numcomps * sizeof(mi_jp2_comps_t));
    if (!jp2->comps) {
        jp2->comps = NULL;
        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory when setup the JP2 encoder\n");
        /* Memory of jp2->cl will be freed by mi_jp2_destroy */
        return mi_FALSE;
    }

    jp2->h = image->y1 - image->y0;		/* HEIGHT */
    jp2->w = image->x1 - image->x0;		/* WIDTH */
    /* BPC */
    depth_0 = image->comps[0].prec - 1;
    sign = image->comps[0].sgnd;
    jp2->bpc = depth_0 + (sign << 7);
    for (i = 1; i < image->numcomps; i++) {
        mi_UINT32 depth = image->comps[i].prec - 1;
        sign = image->comps[i].sgnd;
        if (depth_0 != depth)
            jp2->bpc = 255;
    }
    jp2->C = 7;			/* C : Always 7 */
    jp2->UnkC = 0;		/* UnkC, colorspace specified in colr box */
    jp2->IPR = 0;		/* IPR, no intellectual property */
    
    /* BitsPerComponent box */
    for (i = 0; i < image->numcomps; i++) {
        jp2->comps[i].bpcc = image->comps[i].prec - 1 + (image->comps[i].sgnd << 7);
    }

    /* Colour Specification box */
    if(image->icc_profile_len) {
        jp2->meth = 2;
        jp2->enumcs = 0;
    } 
    else {
        jp2->meth = 1;
        if (image->color_space == 1)
            jp2->enumcs = 16;	/* sRGB as defined by IEC 61966-2-1 */
        else if (image->color_space == 2)
            jp2->enumcs = 17;	/* greyscale */
        else if (image->color_space == 3)
            jp2->enumcs = 18;	/* YUV */
    }

    /* Channel Definition box */
    /* FIXME not provided by parameters */
    /* We try to do what we can... */
    alpha_count = 0U;
    for (i = 0; i < image->numcomps; i++) {
        if (image->comps[i].alpha != 0) {
            alpha_count++;
            alpha_channel = i;
        }
    }
    if (alpha_count == 1U) { /* no way to deal with more than 1 alpha channel */
        switch (jp2->enumcs) {
            case 16:
            case 18:
                color_channels = 3;
                break;
            case 17:
                color_channels = 1;
                break;
            default:
                alpha_count = 0U;
                break;
        }
        if (alpha_count == 0U) {
            mi_event_msg(p_manager, EVT_WARNING, "Alpha channel specified but unknown enumcs. No cdef box will be created.\n");
        } else if (image->numcomps < (color_channels+1)) {
            mi_event_msg(p_manager, EVT_WARNING, "Alpha channel specified but not enough image components for an automatic cdef box creation.\n");
            alpha_count = 0U;
        } else if ((mi_UINT32)alpha_channel < color_channels) {
            mi_event_msg(p_manager, EVT_WARNING, "Alpha channel position conflicts with color channel. No cdef box will be created.\n");
            alpha_count = 0U;
        }
    } else if (alpha_count > 1) {
        mi_event_msg(p_manager, EVT_WARNING, "Multiple alpha channels specified. No cdef box will be created.\n");
    }
    if (alpha_count == 1U) { /* if here, we know what we can do */
        jp2->color.jp2_cdef = (mi_jp2_cdef_t*)mi_malloc(sizeof(mi_jp2_cdef_t));
        if(!jp2->color.jp2_cdef) {
            mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to setup the JP2 encoder\n");
            return mi_FALSE;
        }
        /* no memset needed, all values will be overwritten except if jp2->color.jp2_cdef->info allocation fails, */
        /* in which case jp2->color.jp2_cdef->info will be NULL => valid for destruction */
        jp2->color.jp2_cdef->info = (mi_jp2_cdef_info_t*) mi_malloc(image->numcomps * sizeof(mi_jp2_cdef_info_t));
        if (!jp2->color.jp2_cdef->info) {
            /* memory will be freed by mi_jp2_destroy */
            mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to setup the JP2 encoder\n");
            return mi_FALSE;
        }
        jp2->color.jp2_cdef->n = (mi_UINT16) image->numcomps; /* cast is valid : image->numcomps [1,16384] */
        for (i = 0U; i < color_channels; i++) {
            jp2->color.jp2_cdef->info[i].cn = (mi_UINT16)i; /* cast is valid : image->numcomps [1,16384] */
            jp2->color.jp2_cdef->info[i].typ = 0U;
            jp2->color.jp2_cdef->info[i].asoc = (mi_UINT16)(i+1U); /* No overflow + cast is valid : image->numcomps [1,16384] */
        }
        for (; i < image->numcomps; i++) {
            if (image->comps[i].alpha != 0) { /* we'll be here exactly once */
                jp2->color.jp2_cdef->info[i].cn = (mi_UINT16)i; /* cast is valid : image->numcomps [1,16384] */
                jp2->color.jp2_cdef->info[i].typ = 1U; /* Opacity channel */
                jp2->color.jp2_cdef->info[i].asoc = 0U; /* Apply alpha channel to the whole image */
            } else {
                /* Unknown channel */
                jp2->color.jp2_cdef->info[i].cn = (mi_UINT16)i; /* cast is valid : image->numcomps [1,16384] */
                jp2->color.jp2_cdef->info[i].typ = 65535U;
                jp2->color.jp2_cdef->info[i].asoc = 65535U;
            }
        }
    }

    jp2->precedence = 0;	/* PRECEDENCE */
    jp2->approx = 0;		/* APPROX */

    jp2->jpip_on = parameters->jpip_on;

    return mi_TRUE;
}

mi_BOOL mi_jp2_encode(mi_jp2_t *jp2,
                        mi_stream_private_t *stream,
                        mi_event_mgr_t * p_manager)
{
    return mi_j2k_encode(jp2->j2k, stream, p_manager);
}

mi_BOOL mi_jp2_end_decompress(mi_jp2_t *jp2,
                                mi_stream_private_t *cio,
                                mi_event_mgr_t * p_manager
                                )
{
    /* preconditions */
    assert(jp2 != 00);
    assert(cio != 00);
    assert(p_manager != 00);

    /* customization of the end encoding */
    if (! mi_jp2_setup_end_header_reading(jp2, p_manager)) {
        return mi_FALSE;
    }

    /* write header */
    if (! mi_jp2_exec (jp2,jp2->m_procedure_list,cio,p_manager)) {
        return mi_FALSE;
    }

    return mi_j2k_end_decompress(jp2->j2k, cio, p_manager);
}

mi_BOOL mi_jp2_end_compress(	mi_jp2_t *jp2,
                                mi_stream_private_t *cio,
                                mi_event_mgr_t * p_manager
                                )
{
    /* preconditions */
    assert(jp2 != 00);
    assert(cio != 00);
    assert(p_manager != 00);

    /* customization of the end encoding */
    if (! mi_jp2_setup_end_header_writing(jp2, p_manager)) {
        return mi_FALSE;
    }

    if (! mi_j2k_end_compress(jp2->j2k,cio,p_manager)) {
        return mi_FALSE;
    }

    /* write header */
    return mi_jp2_exec(jp2,jp2->m_procedure_list,cio,p_manager);
}

static mi_BOOL mi_jp2_setup_end_header_writing (mi_jp2_t *jp2, mi_event_mgr_t * p_manager)
{
    /* preconditions */
    assert(jp2 != 00);
    assert(p_manager != 00);

    if (! mi_procedure_list_add_procedure(jp2->m_procedure_list,(mi_procedure)mi_jp2_write_jp2c, p_manager)) {
        return mi_FALSE;
    }
    /* DEVELOPER CORNER, add your custom procedures */
    return mi_TRUE;
}

static mi_BOOL mi_jp2_setup_end_header_reading (mi_jp2_t *jp2, mi_event_mgr_t * p_manager)
{
    /* preconditions */
    assert(jp2 != 00);
    assert(p_manager != 00);
    
    if (! mi_procedure_list_add_procedure(jp2->m_procedure_list,(mi_procedure)mi_jp2_read_header_procedure, p_manager)) {
        return mi_FALSE;
    }
    /* DEVELOPER CORNER, add your custom procedures */
    
    return mi_TRUE;
}

static mi_BOOL mi_jp2_default_validation (	mi_jp2_t * jp2,
                                        mi_stream_private_t *cio,
                                        mi_event_mgr_t * p_manager
                                        )
{
    mi_BOOL l_is_valid = mi_TRUE;
    mi_UINT32 i;

    /* preconditions */
    assert(jp2 != 00);
    assert(cio != 00);
    assert(p_manager != 00);

    /* JPEG2000 codec validation */

    /* STATE checking */
    /* make sure the state is at 0 */
    l_is_valid &= (jp2->jp2_state == JP2_STATE_NONE);

    /* make sure not reading a jp2h ???? WEIRD */
    l_is_valid &= (jp2->jp2_img_state == JP2_IMG_STATE_NONE);

    /* POINTER validation */
    /* make sure a j2k codec is present */
    l_is_valid &= (jp2->j2k != 00);

    /* make sure a procedure list is present */
    l_is_valid &= (jp2->m_procedure_list != 00);

    /* make sure a validation list is present */
    l_is_valid &= (jp2->m_validation_list != 00);

    /* PARAMETER VALIDATION */
    /* number of components */
    l_is_valid &= (jp2->numcl > 0);
    /* width */
    l_is_valid &= (jp2->h > 0);
    /* height */
    l_is_valid &= (jp2->w > 0);
    /* precision */
    for (i = 0; i < jp2->numcomps; ++i)	{
        l_is_valid &= ((jp2->comps[i].bpcc & 0x7FU) < 38U); /* 0 is valid, ignore sign for check */
    }

    /* METH */
    l_is_valid &= ((jp2->meth > 0) && (jp2->meth < 3));

    /* stream validation */
    /* back and forth is needed */
    l_is_valid &= mi_stream_has_seek(cio);

    return l_is_valid;
}

static mi_BOOL mi_jp2_read_header_procedure(  mi_jp2_t *jp2,
                                                mi_stream_private_t *stream,
                                                mi_event_mgr_t * p_manager
                                                )
{
    mi_jp2_box_t box;
    mi_UINT32 l_nb_bytes_read;
    const mi_jp2_header_handler_t * l_current_handler;
    const mi_jp2_header_handler_t * l_current_handler_misplaced;
    mi_UINT32 l_last_data_size = mi_BOX_SIZE;
    mi_UINT32 l_current_data_size;
    mi_BYTE * l_current_data = 00;

    /* preconditions */
    assert(stream != 00);
    assert(jp2 != 00);
    assert(p_manager != 00);

    l_current_data = (mi_BYTE*)mi_calloc(1,l_last_data_size);

    if (l_current_data == 00) {
        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to handle jpeg2000 file header\n");
        return mi_FALSE;
    }

    while (mi_jp2_read_boxhdr(&box,&l_nb_bytes_read,stream,p_manager)) {
        /* is it the codestream box ? */
        if (box.type == JP2_JP2C) {
            if (jp2->jp2_state & JP2_STATE_HEADER) {
                jp2->jp2_state |= JP2_STATE_CODESTREAM;
                mi_free(l_current_data);
                return mi_TRUE;
            }
            else {
                mi_event_msg(p_manager, EVT_ERROR, "bad placed jpeg codestream\n");
                mi_free(l_current_data);
                return mi_FALSE;
            }
        }
        else if	(box.length == 0) {
            mi_event_msg(p_manager, EVT_ERROR, "Cannot handle box of undefined sizes\n");
            mi_free(l_current_data);
            return mi_FALSE;
        }
        /* testcase 1851.pdf.SIGSEGV.ce9.948 */
        else if (box.length < l_nb_bytes_read) {
            mi_event_msg(p_manager, EVT_ERROR, "invalid box size %d (%x)\n", box.length, box.type);
            mi_free(l_current_data);
            return mi_FALSE;
        }

        l_current_handler = mi_jp2_find_handler(box.type);
        l_current_handler_misplaced = mi_jp2_img_find_handler(box.type);
        l_current_data_size = box.length - l_nb_bytes_read;

        if ((l_current_handler != 00) || (l_current_handler_misplaced != 00)) {
            if (l_current_handler == 00) {
                mi_event_msg(p_manager, EVT_WARNING, "Found a misplaced '%c%c%c%c' box outside jp2h box\n", (mi_BYTE)(box.type>>24), (mi_BYTE)(box.type>>16), (mi_BYTE)(box.type>>8), (mi_BYTE)(box.type>>0));
                if (jp2->jp2_state & JP2_STATE_HEADER) {
                    /* read anyway, we already have jp2h */
                    l_current_handler = l_current_handler_misplaced;
                } else {
                    mi_event_msg(p_manager, EVT_WARNING, "JPEG2000 Header box not read yet, '%c%c%c%c' box will be ignored\n", (mi_BYTE)(box.type>>24), (mi_BYTE)(box.type>>16), (mi_BYTE)(box.type>>8), (mi_BYTE)(box.type>>0));
                    jp2->jp2_state |= JP2_STATE_UNKNOWN;
                    if (mi_stream_skip(stream,l_current_data_size,p_manager) != l_current_data_size) {
                            mi_event_msg(p_manager, EVT_ERROR, "Problem with skipping JPEG2000 box, stream error\n");
                            mi_free(l_current_data);
                            return mi_FALSE;
                    }
                    continue;
                }
            }
            if ((mi_OFF_T)l_current_data_size > mi_stream_get_number_byte_left(stream)) {
                /* do not even try to malloc if we can't read */
                mi_event_msg(p_manager, EVT_ERROR, "Invalid box size %d for box '%c%c%c%c'. Need %d bytes, %d bytes remaining \n", box.length, (mi_BYTE)(box.type>>24), (mi_BYTE)(box.type>>16), (mi_BYTE)(box.type>>8), (mi_BYTE)(box.type>>0), l_current_data_size, (mi_UINT32)mi_stream_get_number_byte_left(stream));
                mi_free(l_current_data);
                return mi_FALSE;
            }
            if (l_current_data_size > l_last_data_size) {
                mi_BYTE* new_current_data = (mi_BYTE*)mi_realloc(l_current_data,l_current_data_size);
                if (!new_current_data) {
                    mi_free(l_current_data);
                    mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to handle jpeg2000 box\n");
                    return mi_FALSE;
                }
                l_current_data = new_current_data;
                l_last_data_size = l_current_data_size;
            }

            l_nb_bytes_read = (mi_UINT32)mi_stream_read_data(stream,l_current_data,l_current_data_size,p_manager);
            if (l_nb_bytes_read != l_current_data_size) {
                mi_event_msg(p_manager, EVT_ERROR, "Problem with reading JPEG2000 box, stream error\n");
                mi_free(l_current_data);                
                return mi_FALSE;
            }

            if (! l_current_handler->handler(jp2,l_current_data,l_current_data_size,p_manager)) {
                mi_free(l_current_data);
                return mi_FALSE;
            }
        }
        else {
            if (!(jp2->jp2_state & JP2_STATE_SIGNATURE)) {
                mi_event_msg(p_manager, EVT_ERROR, "Malformed JP2 file format: first box must be JPEG 2000 signature box\n");
                mi_free(l_current_data);
                return mi_FALSE;
            }
            if (!(jp2->jp2_state & JP2_STATE_FILE_TYPE)) {
                mi_event_msg(p_manager, EVT_ERROR, "Malformed JP2 file format: second box must be file type box\n");
                mi_free(l_current_data);
                return mi_FALSE;
            }
            jp2->jp2_state |= JP2_STATE_UNKNOWN;
            if (mi_stream_skip(stream,l_current_data_size,p_manager) != l_current_data_size) {
                mi_event_msg(p_manager, EVT_ERROR, "Problem with skipping JPEG2000 box, stream error\n");
                mi_free(l_current_data);
                return mi_FALSE;
            }
        }
    }

    mi_free(l_current_data);

    return mi_TRUE;
}

/**
 * Excutes the given procedures on the given codec.
 *
 * @param	p_procedure_list	the list of procedures to execute
 * @param	jp2					the jpeg2000 file codec to execute the procedures on.
 * @param	stream					the stream to execute the procedures on.
 * @param	p_manager			the user manager.
 *
 * @return	true				if all the procedures were successfully executed.
 */
static mi_BOOL mi_jp2_exec (  mi_jp2_t * jp2,
                                mi_procedure_list_t * p_procedure_list,
                                mi_stream_private_t *stream,
                                mi_event_mgr_t * p_manager
                                )

{
    mi_BOOL (** l_procedure) (mi_jp2_t * jp2, mi_stream_private_t *, mi_event_mgr_t *) = 00;
    mi_BOOL l_result = mi_TRUE;
    mi_UINT32 l_nb_proc, i;

    /* preconditions */
    assert(p_procedure_list != 00);
    assert(jp2 != 00);
    assert(stream != 00);
    assert(p_manager != 00);

    l_nb_proc = mi_procedure_list_get_nb_procedures(p_procedure_list);
    l_procedure = (mi_BOOL (**) (mi_jp2_t * jp2, mi_stream_private_t *, mi_event_mgr_t *)) mi_procedure_list_get_first_procedure(p_procedure_list);

    for	(i=0;i<l_nb_proc;++i) {
        l_result = l_result && (*l_procedure) (jp2,stream,p_manager);
        ++l_procedure;
    }

    /* and clear the procedure list at the end. */
    mi_procedure_list_clear(p_procedure_list);
    return l_result;
}

mi_BOOL mi_jp2_start_compress(mi_jp2_t *jp2,
                                mi_stream_private_t *stream,
                                mi_image_t * p_image,
                                mi_event_mgr_t * p_manager
                                )
{
    /* preconditions */
    assert(jp2 != 00);
    assert(stream != 00);
    assert(p_manager != 00);

    /* customization of the validation */
    if (! mi_jp2_setup_encoding_validation (jp2, p_manager)) {
        return mi_FALSE;
    }

    /* validation of the parameters codec */
    if (! mi_jp2_exec(jp2,jp2->m_validation_list,stream,p_manager)) {
        return mi_FALSE;
    }

    /* customization of the encoding */
    if (! mi_jp2_setup_header_writing(jp2, p_manager)) {
        return mi_FALSE;
    }

    /* write header */
    if (! mi_jp2_exec (jp2,jp2->m_procedure_list,stream,p_manager)) {
        return mi_FALSE;
    }

    return mi_j2k_start_compress(jp2->j2k,stream,p_image,p_manager);
}

static const mi_jp2_header_handler_t * mi_jp2_find_handler (mi_UINT32 p_id)
{
    mi_UINT32 i, l_handler_size = sizeof(jp2_header) / sizeof(mi_jp2_header_handler_t);

    for (i=0;i<l_handler_size;++i) {
        if (jp2_header[i].id == p_id) {
            return &jp2_header[i];
        }
    }
    return NULL;
}

/**
 * Finds the image execution function related to the given box id.
 *
 * @param	p_id	the id of the handler to fetch.
 *
 * @return	the given handler or 00 if it could not be found.
 */
static const mi_jp2_header_handler_t * mi_jp2_img_find_handler (mi_UINT32 p_id)
{
    mi_UINT32 i, l_handler_size = sizeof(jp2_img_header) / sizeof(mi_jp2_header_handler_t);
    for (i=0;i<l_handler_size;++i)
    {
        if (jp2_img_header[i].id == p_id) {
            return &jp2_img_header[i];
        }
    }

    return NULL;
}

/**
 * Reads a jpeg2000 file signature box.
 *
 * @param	p_header_data	the data contained in the signature box.
 * @param	jp2				the jpeg2000 file codec.
 * @param	p_header_size	the size of the data contained in the signature box.
 * @param	p_manager		the user event manager.
 *
 * @return true if the file signature box is valid.
 */
static mi_BOOL mi_jp2_read_jp(mi_jp2_t *jp2,
                                mi_BYTE * p_header_data,
                                mi_UINT32 p_header_size,
                                mi_event_mgr_t * p_manager
                                )

{
    mi_UINT32 l_magic_number;

    /* preconditions */
    assert(p_header_data != 00);
    assert(jp2 != 00);
    assert(p_manager != 00);

    if (jp2->jp2_state != JP2_STATE_NONE) {
        mi_event_msg(p_manager, EVT_ERROR, "The signature box must be the first box in the file.\n");
        return mi_FALSE;
    }

    /* assure length of data is correct (4 -> magic number) */
    if (p_header_size != 4) {
        mi_event_msg(p_manager, EVT_ERROR, "Error with JP signature Box size\n");
        return mi_FALSE;
    }

    /* rearrange data */
    mi_read_bytes(p_header_data,&l_magic_number,4);
    if (l_magic_number != 0x0d0a870a ) {
        mi_event_msg(p_manager, EVT_ERROR, "Error with JP Signature : bad magic number\n");
        return mi_FALSE;
    }

    jp2->jp2_state |= JP2_STATE_SIGNATURE;

    return mi_TRUE;
}

/**
 * Reads a a FTYP box - File type box
 *
 * @param	p_header_data	the data contained in the FTYP box.
 * @param	jp2				the jpeg2000 file codec.
 * @param	p_header_size	the size of the data contained in the FTYP box.
 * @param	p_manager		the user event manager.
 *
 * @return true if the FTYP box is valid.
 */
static mi_BOOL mi_jp2_read_ftyp(	mi_jp2_t *jp2,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager
                                    )
{
    mi_UINT32 i, l_remaining_bytes;

    /* preconditions */
    assert(p_header_data != 00);
    assert(jp2 != 00);
    assert(p_manager != 00);

    if (jp2->jp2_state != JP2_STATE_SIGNATURE) {
        mi_event_msg(p_manager, EVT_ERROR, "The ftyp box must be the second box in the file.\n");
        return mi_FALSE;
    }

    /* assure length of data is correct */
    if (p_header_size < 8) {
        mi_event_msg(p_manager, EVT_ERROR, "Error with FTYP signature Box size\n");
        return mi_FALSE;
    }

    mi_read_bytes(p_header_data,&jp2->brand,4);		/* BR */
    p_header_data += 4;

    mi_read_bytes(p_header_data,&jp2->minversion,4);		/* MinV */
    p_header_data += 4;

    l_remaining_bytes = p_header_size - 8;

    /* the number of remaining bytes should be a multiple of 4 */
    if ((l_remaining_bytes & 0x3) != 0) {
        mi_event_msg(p_manager, EVT_ERROR, "Error with FTYP signature Box size\n");
        return mi_FALSE;
    }

    /* div by 4 */
    jp2->numcl = l_remaining_bytes >> 2;
    if (jp2->numcl) {
        jp2->cl = (mi_UINT32 *) mi_calloc(jp2->numcl, sizeof(mi_UINT32));
        if (jp2->cl == 00) {
            mi_event_msg(p_manager, EVT_ERROR, "Not enough memory with FTYP Box\n");
            return mi_FALSE;
        }
    }

    for (i = 0; i < jp2->numcl; ++i)
    {
        mi_read_bytes(p_header_data,&jp2->cl[i],4);		/* CLi */
        p_header_data += 4;
    }

    jp2->jp2_state |= JP2_STATE_FILE_TYPE;

    return mi_TRUE;
}

static mi_BOOL mi_jp2_skip_jp2c(	mi_jp2_t *jp2,
                            mi_stream_private_t *stream,
                            mi_event_mgr_t * p_manager )
{
    /* preconditions */
    assert(jp2 != 00);
    assert(stream != 00);
    assert(p_manager != 00);

    jp2->j2k_codestream_offset = mi_stream_tell(stream);

    if (mi_stream_skip(stream,8,p_manager) != 8) {
        return mi_FALSE;
    }

    return mi_TRUE;
}

static mi_BOOL mi_jpip_skip_iptr(	mi_jp2_t *jp2,
  mi_stream_private_t *stream,
  mi_event_mgr_t * p_manager )
{
  /* preconditions */
  assert(jp2 != 00);
  assert(stream != 00);
  assert(p_manager != 00);

  jp2->jpip_iptr_offset = mi_stream_tell(stream);

  if (mi_stream_skip(stream,24,p_manager) != 24) {
    return mi_FALSE;
  }

  return mi_TRUE;
}

/**
 * Reads the Jpeg2000 file Header box - JP2 Header box (warning, this is a super box).
 *
 * @param	p_header_data	the data contained in the file header box.
 * @param	jp2				the jpeg2000 file codec.
 * @param	p_header_size	the size of the data contained in the file header box.
 * @param	p_manager		the user event manager.
 *
 * @return true if the JP2 Header box was successfully recognized.
*/
static mi_BOOL mi_jp2_read_jp2h(  mi_jp2_t *jp2,
                                    mi_BYTE *p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager
                                    )
{
    mi_UINT32 l_box_size=0, l_current_data_size = 0;
    mi_jp2_box_t box;
    const mi_jp2_header_handler_t * l_current_handler;
    mi_BOOL l_has_ihdr = 0;

    /* preconditions */
    assert(p_header_data != 00);
    assert(jp2 != 00);
    assert(p_manager != 00);

    /* make sure the box is well placed */
    if ((jp2->jp2_state & JP2_STATE_FILE_TYPE) != JP2_STATE_FILE_TYPE ) {
        mi_event_msg(p_manager, EVT_ERROR, "The  box must be the first box in the file.\n");
        return mi_FALSE;
    }

    jp2->jp2_img_state = JP2_IMG_STATE_NONE;

    /* iterate while remaining data */
    while (p_header_size > 0) {

        if (! mi_jp2_read_boxhdr_char(&box,p_header_data,&l_box_size,p_header_size, p_manager)) {
            mi_event_msg(p_manager, EVT_ERROR, "Stream error while reading JP2 Header box\n");
            return mi_FALSE;
        }

        if (box.length > p_header_size) {
            mi_event_msg(p_manager, EVT_ERROR, "Stream error while reading JP2 Header box: box length is inconsistent.\n");
            return mi_FALSE;
        }

        l_current_handler = mi_jp2_img_find_handler(box.type);
        l_current_data_size = box.length - l_box_size;
        p_header_data += l_box_size;

        if (l_current_handler != 00) {
            if (! l_current_handler->handler(jp2,p_header_data,l_current_data_size,p_manager)) {
                return mi_FALSE;
            }
        }
        else {
            jp2->jp2_img_state |= JP2_IMG_STATE_UNKNOWN;
        }

        if (box.type == JP2_IHDR) {
            l_has_ihdr = 1;
        }

        p_header_data += l_current_data_size;
        p_header_size -= box.length;
    }

    if (l_has_ihdr == 0) {
        mi_event_msg(p_manager, EVT_ERROR, "Stream error while reading JP2 Header box: no 'ihdr' box.\n");
        return mi_FALSE;
    }

    jp2->jp2_state |= JP2_STATE_HEADER;

    return mi_TRUE;
}

static mi_BOOL mi_jp2_read_boxhdr_char(   mi_jp2_box_t *box,
                                     mi_BYTE * p_data,
                                     mi_UINT32 * p_number_bytes_read,
                                     mi_UINT32 p_box_max_size,
                                     mi_event_mgr_t * p_manager
                                     )
{
    mi_UINT32 l_value;

    /* preconditions */
    assert(p_data != 00);
    assert(box != 00);
    assert(p_number_bytes_read != 00);
    assert(p_manager != 00);

    if (p_box_max_size < 8) {
        mi_event_msg(p_manager, EVT_ERROR, "Cannot handle box of less than 8 bytes\n");
        return mi_FALSE;
    }

    /* process read data */
    mi_read_bytes(p_data, &l_value, 4);
    p_data += 4;
    box->length = (mi_UINT32)(l_value);

    mi_read_bytes(p_data, &l_value, 4);
    p_data += 4;
    box->type = (mi_UINT32)(l_value);

    *p_number_bytes_read = 8;

    /* do we have a "special very large box ?" */
    /* read then the XLBox */
    if (box->length == 1) {
        mi_UINT32 l_xl_part_size;

        if (p_box_max_size < 16) {
            mi_event_msg(p_manager, EVT_ERROR, "Cannot handle XL box of less than 16 bytes\n");
            return mi_FALSE;
        }

        mi_read_bytes(p_data,&l_xl_part_size, 4);
        p_data += 4;
        *p_number_bytes_read += 4;

        if (l_xl_part_size != 0) {
            mi_event_msg(p_manager, EVT_ERROR, "Cannot handle box sizes higher than 2^32\n");
            return mi_FALSE;
        }

        mi_read_bytes(p_data, &l_value, 4);
        *p_number_bytes_read += 4;
        box->length = (mi_UINT32)(l_value);

        if (box->length == 0) {
            mi_event_msg(p_manager, EVT_ERROR, "Cannot handle box of undefined sizes\n");
            return mi_FALSE;
        }
    }
    else if (box->length == 0) {
        mi_event_msg(p_manager, EVT_ERROR, "Cannot handle box of undefined sizes\n");
        return mi_FALSE;
    }
    if (box->length < *p_number_bytes_read) {
        mi_event_msg(p_manager, EVT_ERROR, "Box length is inconsistent.\n");
        return mi_FALSE;
    }
    return mi_TRUE;
}

mi_BOOL mi_jp2_read_header(	mi_stream_private_t *p_stream,
                                mi_jp2_t *jp2,
                                mi_image_t ** p_image,
                                mi_event_mgr_t * p_manager
                                )
{
    /* preconditions */
    assert(jp2 != 00);
    assert(p_stream != 00);
    assert(p_manager != 00);

    /* customization of the validation */
    if (! mi_jp2_setup_decoding_validation (jp2, p_manager)) {
        return mi_FALSE;
    }

    /* customization of the encoding */
    if (! mi_jp2_setup_header_reading(jp2, p_manager)) {
        return mi_FALSE;
    }

    /* validation of the parameters codec */
    if (! mi_jp2_exec(jp2,jp2->m_validation_list,p_stream,p_manager)) {
        return mi_FALSE;
    }

    /* read header */
    if (! mi_jp2_exec (jp2,jp2->m_procedure_list,p_stream,p_manager)) {
        return mi_FALSE;
    }

    return mi_j2k_read_header(	p_stream,
                            jp2->j2k,
                            p_image,
                            p_manager);
}

static mi_BOOL mi_jp2_setup_encoding_validation (mi_jp2_t *jp2, mi_event_mgr_t * p_manager)
{
    /* preconditions */
    assert(jp2 != 00);
    assert(p_manager != 00);

    if (! mi_procedure_list_add_procedure(jp2->m_validation_list, (mi_procedure)mi_jp2_default_validation, p_manager)) {
        return mi_FALSE;
    }
    /* DEVELOPER CORNER, add your custom validation procedure */
    
    return mi_TRUE;
}

static mi_BOOL mi_jp2_setup_decoding_validation (mi_jp2_t *jp2, mi_event_mgr_t * p_manager)
{
    /* preconditions */
    assert(jp2 != 00);
    assert(p_manager != 00);
    
    /* DEVELOPER CORNER, add your custom validation procedure */
    
    return mi_TRUE;
}

static mi_BOOL mi_jp2_setup_header_writing (mi_jp2_t *jp2, mi_event_mgr_t * p_manager)
{
    /* preconditions */
    assert(jp2 != 00);
    assert(p_manager != 00);

    if (! mi_procedure_list_add_procedure(jp2->m_procedure_list,(mi_procedure)mi_jp2_write_jp, p_manager)) {
        return mi_FALSE;
    }
    if (! mi_procedure_list_add_procedure(jp2->m_procedure_list,(mi_procedure)mi_jp2_write_ftyp, p_manager)) {
        return mi_FALSE;
    }
    if (! mi_procedure_list_add_procedure(jp2->m_procedure_list,(mi_procedure)mi_jp2_write_jp2h, p_manager)) {
        return mi_FALSE;
    }
    if( jp2->jpip_on ) {
        if (! mi_procedure_list_add_procedure(jp2->m_procedure_list,(mi_procedure)mi_jpip_skip_iptr, p_manager)) {
            return mi_FALSE;
        }
    }
    if (! mi_procedure_list_add_procedure(jp2->m_procedure_list,(mi_procedure)mi_jp2_skip_jp2c,p_manager)) {
        return mi_FALSE;
    }

    /* DEVELOPER CORNER, insert your custom procedures */

    return mi_TRUE;
}

static mi_BOOL mi_jp2_setup_header_reading (mi_jp2_t *jp2, mi_event_mgr_t * p_manager)
{
    /* preconditions */
    assert(jp2 != 00);
    assert(p_manager != 00);

    if (! mi_procedure_list_add_procedure(jp2->m_procedure_list,(mi_procedure)mi_jp2_read_header_procedure, p_manager)) {
        return mi_FALSE;
    }
    
    /* DEVELOPER CORNER, add your custom procedures */
    
    return mi_TRUE;
}

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
                                    mi_event_mgr_t * p_manager
                                    )
{
    return mi_j2k_read_tile_header(p_jp2->j2k,
                                p_tile_index,
                                p_data_size,
                                p_tile_x0, p_tile_y0,
                                p_tile_x1, p_tile_y1,
                                p_nb_comps,
                                p_go_on,
                                p_stream,
                                p_manager);
}

mi_BOOL mi_jp2_write_tile (	mi_jp2_t *p_jp2,
                                mi_UINT32 p_tile_index,
                                mi_BYTE * p_data,
                                mi_UINT32 p_data_size,
                                mi_stream_private_t *p_stream,
                                mi_event_mgr_t * p_manager
                                )

{
    return mi_j2k_write_tile (p_jp2->j2k,p_tile_index,p_data,p_data_size,p_stream,p_manager);
}

mi_BOOL mi_jp2_decode_tile (  mi_jp2_t * p_jp2,
                                mi_UINT32 p_tile_index,
                                mi_BYTE * p_data,
                                mi_UINT32 p_data_size,
                                mi_stream_private_t *p_stream,
                                mi_event_mgr_t * p_manager
                                )
{
    return mi_j2k_decode_tile (p_jp2->j2k,p_tile_index,p_data,p_data_size,p_stream,p_manager);
}

void mi_jp2_destroy(mi_jp2_t *jp2)
{
    if (jp2) {
        /* destroy the J2K codec */
        mi_j2k_destroy(jp2->j2k);
        jp2->j2k = 00;

        if (jp2->comps) {
            mi_free(jp2->comps);
            jp2->comps = 00;
        }

        if (jp2->cl) {
            mi_free(jp2->cl);
            jp2->cl = 00;
        }

        if (jp2->color.icc_profile_buf) {
            mi_free(jp2->color.icc_profile_buf);
            jp2->color.icc_profile_buf = 00;
        }

        if (jp2->color.jp2_cdef) {
            if (jp2->color.jp2_cdef->info) {
                mi_free(jp2->color.jp2_cdef->info);
                jp2->color.jp2_cdef->info = NULL;
            }

            mi_free(jp2->color.jp2_cdef);
            jp2->color.jp2_cdef = 00;
        }

        if (jp2->color.jp2_pclr) {
            if (jp2->color.jp2_pclr->cmap) {
                mi_free(jp2->color.jp2_pclr->cmap);
                jp2->color.jp2_pclr->cmap = NULL;
            }
            if (jp2->color.jp2_pclr->channel_sign) {
                mi_free(jp2->color.jp2_pclr->channel_sign);
                jp2->color.jp2_pclr->channel_sign = NULL;
            }
            if (jp2->color.jp2_pclr->channel_size) {
                mi_free(jp2->color.jp2_pclr->channel_size);
                jp2->color.jp2_pclr->channel_size = NULL;
            }
            if (jp2->color.jp2_pclr->entries) {
                mi_free(jp2->color.jp2_pclr->entries);
                jp2->color.jp2_pclr->entries = NULL;
            }

            mi_free(jp2->color.jp2_pclr);
            jp2->color.jp2_pclr = 00;
        }

        if (jp2->m_validation_list) {
            mi_procedure_list_destroy(jp2->m_validation_list);
            jp2->m_validation_list = 00;
        }

        if (jp2->m_procedure_list) {
            mi_procedure_list_destroy(jp2->m_procedure_list);
            jp2->m_procedure_list = 00;
        }

        mi_free(jp2);
    }
}

mi_BOOL mi_jp2_set_decode_area(	mi_jp2_t *p_jp2,
                                    mi_image_t* p_image,
                                    mi_INT32 p_start_x, mi_INT32 p_start_y,
                                    mi_INT32 p_end_x, mi_INT32 p_end_y,
                                    mi_event_mgr_t * p_manager
                                    )
{
    return mi_j2k_set_decode_area(p_jp2->j2k, p_image, p_start_x, p_start_y, p_end_x, p_end_y, p_manager);
}

mi_BOOL mi_jp2_get_tile(	mi_jp2_t *p_jp2,
                            mi_stream_private_t *p_stream,
                            mi_image_t* p_image,
                            mi_event_mgr_t * p_manager,
                            mi_UINT32 tile_index
                            )
{
    if (!p_image)
        return mi_FALSE;

    mi_event_msg(p_manager, EVT_WARNING, "JP2 box which are after the codestream will not be read by this function.\n");

    if (! mi_j2k_get_tile(p_jp2->j2k, p_stream, p_image, p_manager, tile_index) ){
        mi_event_msg(p_manager, EVT_ERROR, "Failed to decode the codestream in the JP2 file\n");
        return mi_FALSE;
    }

    if (!mi_jp2_check_color(p_image, &(p_jp2->color), p_manager)) {
        return mi_FALSE;
    }

    /* Set Image Color Space */
    if (p_jp2->enumcs == 16)
        p_image->color_space = mi_CLRSPC_SRGB;
    else if (p_jp2->enumcs == 17)
        p_image->color_space = mi_CLRSPC_GRAY;
    else if (p_jp2->enumcs == 18)
        p_image->color_space = mi_CLRSPC_SYCC;
    else if (p_jp2->enumcs == 24)
        p_image->color_space = mi_CLRSPC_EYCC;
    else if (p_jp2->enumcs == 12)
        p_image->color_space = mi_CLRSPC_CMYK;
    else
        p_image->color_space = mi_CLRSPC_UNKNOWN;

    if(p_jp2->color.jp2_pclr) {
        /* Part 1, I.5.3.4: Either both or none : */
        if( !p_jp2->color.jp2_pclr->cmap)
            mi_jp2_free_pclr(&(p_jp2->color));
        else
            mi_jp2_apply_pclr(p_image, &(p_jp2->color));
    }
    
    /* Apply the color space if needed */
    if(p_jp2->color.jp2_cdef) {
        mi_jp2_apply_cdef(p_image, &(p_jp2->color), p_manager);
    }

    if(p_jp2->color.icc_profile_buf) {
        p_image->icc_profile_buf = p_jp2->color.icc_profile_buf;
        p_image->icc_profile_len = p_jp2->color.icc_profile_len;
        p_jp2->color.icc_profile_buf = NULL;
    }

    return mi_TRUE;
}

/* ----------------------------------------------------------------------- */
/* JP2 encoder interface                                             */
/* ----------------------------------------------------------------------- */

mi_jp2_t* mi_jp2_create(mi_BOOL p_is_decoder)
{
    mi_jp2_t *jp2 = (mi_jp2_t*)mi_calloc(1,sizeof(mi_jp2_t));
    if (jp2) {

        /* create the J2K codec */
        if (! p_is_decoder) {
            jp2->j2k = mi_j2k_create_compress();
        }
        else {
            jp2->j2k = mi_j2k_create_decompress();
        }

        if (jp2->j2k == 00) {
            mi_jp2_destroy(jp2);
            return 00;
        }

        /* Color structure */
        jp2->color.icc_profile_buf = NULL;
        jp2->color.icc_profile_len = 0;
        jp2->color.jp2_cdef = NULL;
        jp2->color.jp2_pclr = NULL;
        jp2->color.jp2_has_colr = 0;

        /* validation list creation */
        jp2->m_validation_list = mi_procedure_list_create();
        if (! jp2->m_validation_list) {
            mi_jp2_destroy(jp2);
            return 00;
        }

        /* execution list creation */
        jp2->m_procedure_list = mi_procedure_list_create();
        if (! jp2->m_procedure_list) {
            mi_jp2_destroy(jp2);
            return 00;
        }
    }

    return jp2;
}

void jp2_dump(mi_jp2_t* p_jp2, mi_INT32 flag, FILE* out_stream)
{
    /* preconditions */
    assert(p_jp2 != 00);

    j2k_dump(p_jp2->j2k,
                    flag,
                    out_stream);
}

mi_codestream_index_t* jp2_get_cstr_index(mi_jp2_t* p_jp2)
{
    return j2k_get_cstr_index(p_jp2->j2k);
}

mi_codestream_info_v2_t* jp2_get_cstr_info(mi_jp2_t* p_jp2)
{
    return j2k_get_cstr_info(p_jp2->j2k);
}

mi_BOOL mi_jp2_set_decoded_resolution_factor(mi_jp2_t *p_jp2,
                                               mi_UINT32 res_factor,
                                               mi_event_mgr_t * p_manager)
{
    return mi_j2k_set_decoded_resolution_factor(p_jp2->j2k, res_factor, p_manager);
}

/* JPIP specific */

