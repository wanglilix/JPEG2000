
#ifdef _WIN32
#include <windows.h>
#endif /* _WIN32 */

#include "mi_includes.h"
#include"mi_config_private.h"

/* ---------------------------------------------------------------------- */
/* Functions to set the message handlers */

mi_BOOL mi_CALLCONV mi_set_info_handler(	mi_codec_t * p_codec, 
											mi_msg_callback p_callback,
											void * p_user_data)
{
	mi_codec_private_t * l_codec = (mi_codec_private_t *) p_codec;
	if(! l_codec){
		return mi_FALSE;
	}
	
	l_codec->m_event_mgr.info_handler = p_callback;
	l_codec->m_event_mgr.m_info_data = p_user_data;
	
	return mi_TRUE;
}

mi_BOOL mi_CALLCONV mi_set_warning_handler(	mi_codec_t * p_codec, 
												mi_msg_callback p_callback,
												void * p_user_data)
{
	mi_codec_private_t * l_codec = (mi_codec_private_t *) p_codec;
	if (! l_codec) {
		return mi_FALSE;
	}
	
	l_codec->m_event_mgr.warning_handler = p_callback;
	l_codec->m_event_mgr.m_warning_data = p_user_data;
	
	return mi_TRUE;
}

mi_BOOL mi_CALLCONV mi_set_error_handler(mi_codec_t * p_codec, 
											mi_msg_callback p_callback,
											void * p_user_data)
{
	mi_codec_private_t * l_codec = (mi_codec_private_t *) p_codec;
	if (! l_codec) {
		return mi_FALSE;
	}
	
	l_codec->m_event_mgr.error_handler = p_callback;
	l_codec->m_event_mgr.m_error_data = p_user_data;
	
	return mi_TRUE;
}

/* ---------------------------------------------------------------------- */

static mi_SIZE_T mi_read_from_file (void * p_buffer, mi_SIZE_T p_nb_bytes, FILE * p_file)
{
	mi_SIZE_T l_nb_read = fread(p_buffer,1,p_nb_bytes,p_file);
	return l_nb_read ? l_nb_read : (mi_SIZE_T)-1;
}

static mi_UINT64 mi_get_data_length_from_file (FILE * p_file)
{
	mi_OFF_T file_length = 0;

	mi_FSEEK(p_file, 0, SEEK_END);
	file_length = (mi_OFF_T)mi_FTELL(p_file);
	mi_FSEEK(p_file, 0, SEEK_SET);

	return (mi_UINT64)file_length;
}

static mi_SIZE_T mi_write_from_file (void * p_buffer, mi_SIZE_T p_nb_bytes, FILE * p_file)
{
	return fwrite(p_buffer,1,p_nb_bytes,p_file);
}

static mi_OFF_T mi_skip_from_file (mi_OFF_T p_nb_bytes, FILE * p_user_data)
{
	if (mi_FSEEK(p_user_data,p_nb_bytes,SEEK_CUR)) {
		return -1;
	}

	return p_nb_bytes;
}

static mi_BOOL mi_seek_from_file (mi_OFF_T p_nb_bytes, FILE * p_user_data)
{
	if (mi_FSEEK(p_user_data,p_nb_bytes,SEEK_SET)) {
		return mi_FALSE;
	}

	return mi_TRUE;
}

/* ---------------------------------------------------------------------- */
#ifdef _WIN32
#endif /* _WIN32 */

/* ---------------------------------------------------------------------- */

const char* mi_CALLCONV mi_version(void) {
    return mi_PACKAGE_VERSION;
}

/* ---------------------------------------------------------------------- */
/* DECOMPRESSION FUNCTIONS*/

mi_codec_t* mi_CALLCONV mi_create_decompress(mi_CODEC_FORMAT p_format)
{
	mi_codec_private_t *l_codec = 00;

	l_codec = (mi_codec_private_t*) mi_calloc(1, sizeof(mi_codec_private_t));
	if (!l_codec){
		return 00;
	}

	l_codec->is_decompressor = 1;

	switch (p_format) {
		case mi_CODEC_J2K:
			l_codec->mi_dump_codec = (void (*) (void*, mi_INT32, FILE*)) j2k_dump;

			l_codec->mi_get_codec_info = (mi_codestream_info_v2_t* (*) (void*) ) j2k_get_cstr_info;

			l_codec->mi_get_codec_index = (mi_codestream_index_t* (*) (void*) ) j2k_get_cstr_index;

			l_codec->m_codec_data.m_decompression.mi_decode =
					(mi_BOOL (*) (	void *,
									struct mi_stream_private *,
									mi_image_t*, struct mi_event_mgr * )) mi_j2k_decode;

			l_codec->m_codec_data.m_decompression.mi_end_decompress =
					(mi_BOOL (*) (	void *,
									struct mi_stream_private *,
									struct mi_event_mgr *)) mi_j2k_end_decompress;

			l_codec->m_codec_data.m_decompression.mi_read_header =
					(mi_BOOL (*) (	struct mi_stream_private *,
									void *,
									mi_image_t **,
									struct mi_event_mgr * )) mi_j2k_read_header;

			l_codec->m_codec_data.m_decompression.mi_destroy =
					(void (*) (void *))mi_j2k_destroy;

			l_codec->m_codec_data.m_decompression.mi_setup_decoder =
					(void (*) (void * , mi_dparameters_t * )) mi_j2k_setup_decoder;

			l_codec->m_codec_data.m_decompression.mi_read_tile_header =
					(mi_BOOL (*) (	void *,
									mi_UINT32*,
									mi_UINT32*,
									mi_INT32*, mi_INT32*,
									mi_INT32*, mi_INT32*,
									mi_UINT32*,
									mi_BOOL*,
									struct mi_stream_private *,
									struct mi_event_mgr * )) mi_j2k_read_tile_header;

			l_codec->m_codec_data.m_decompression.mi_decode_tile_data =
					(mi_BOOL (*) ( void *, 
                                    mi_UINT32, 
                                    mi_BYTE*, 
                                    mi_UINT32, 
                                    struct mi_stream_private *,
                                    struct mi_event_mgr *)) mi_j2k_decode_tile;

			l_codec->m_codec_data.m_decompression.mi_set_decode_area =
					(mi_BOOL (*) ( void *, 
                                    mi_image_t*, 
                                    mi_INT32, mi_INT32, mi_INT32, mi_INT32, 
                                    struct mi_event_mgr *)) mi_j2k_set_decode_area;

			l_codec->m_codec_data.m_decompression.mi_get_decoded_tile = 
                    (mi_BOOL (*) ( void *p_codec,
								    mi_stream_private_t *p_cio,
								    mi_image_t *p_image,
								    struct mi_event_mgr * p_manager,
								    mi_UINT32 tile_index)) mi_j2k_get_tile;

			l_codec->m_codec_data.m_decompression.mi_set_decoded_resolution_factor = 
                    (mi_BOOL (*) ( void * p_codec,
									mi_UINT32 res_factor,
									struct mi_event_mgr * p_manager)) mi_j2k_set_decoded_resolution_factor;

			l_codec->m_codec = mi_j2k_create_decompress();

			if (! l_codec->m_codec) {
				mi_free(l_codec);
				return NULL;
			}

			break;

		case mi_CODEC_JP2:
			/* get a JP2 decoder handle */
			l_codec->mi_dump_codec = (void (*) (void*, mi_INT32, FILE*)) jp2_dump;

			l_codec->mi_get_codec_info = (mi_codestream_info_v2_t* (*) (void*) ) jp2_get_cstr_info;

			l_codec->mi_get_codec_index = (mi_codestream_index_t* (*) (void*) ) jp2_get_cstr_index;

			l_codec->m_codec_data.m_decompression.mi_decode =
					(mi_BOOL (*) (	void *,
									struct mi_stream_private *,
									mi_image_t*,
									struct mi_event_mgr * )) mi_jp2_decode;

			l_codec->m_codec_data.m_decompression.mi_end_decompress =  
                    (mi_BOOL (*) ( void *,
                                    struct mi_stream_private *,
                                    struct mi_event_mgr *)) mi_jp2_end_decompress;

			l_codec->m_codec_data.m_decompression.mi_read_header =  
                    (mi_BOOL (*) ( struct mi_stream_private *,
					                void *,
					                mi_image_t **,
					                struct mi_event_mgr * )) mi_jp2_read_header;

			l_codec->m_codec_data.m_decompression.mi_read_tile_header = 
                    (mi_BOOL (*) ( void *,
					                mi_UINT32*,
					                mi_UINT32*,
					                mi_INT32*,
					                mi_INT32*,
					                mi_INT32 * ,
					                mi_INT32 * ,
					                mi_UINT32 * ,
					                mi_BOOL *,
					                struct mi_stream_private *,
					                struct mi_event_mgr * )) mi_jp2_read_tile_header;

			l_codec->m_codec_data.m_decompression.mi_decode_tile_data = 
                    (mi_BOOL (*) ( void *,
                                    mi_UINT32,mi_BYTE*,mi_UINT32,
                                    struct mi_stream_private *,
                                    struct mi_event_mgr * )) mi_jp2_decode_tile;

			l_codec->m_codec_data.m_decompression.mi_destroy = (void (*) (void *))mi_jp2_destroy;

			l_codec->m_codec_data.m_decompression.mi_setup_decoder = 
                    (void (*) (void * ,mi_dparameters_t * )) mi_jp2_setup_decoder;

			l_codec->m_codec_data.m_decompression.mi_set_decode_area = 
                    (mi_BOOL (*) ( void *,
                                    mi_image_t*, 
                                    mi_INT32,mi_INT32,mi_INT32,mi_INT32,
                                    struct mi_event_mgr * )) mi_jp2_set_decode_area;

			l_codec->m_codec_data.m_decompression.mi_get_decoded_tile = 
                    (mi_BOOL (*) ( void *p_codec,
									mi_stream_private_t *p_cio,
									mi_image_t *p_image,
									struct mi_event_mgr * p_manager,
									mi_UINT32 tile_index)) mi_jp2_get_tile;

			l_codec->m_codec_data.m_decompression.mi_set_decoded_resolution_factor = 
                    (mi_BOOL (*) ( void * p_codec,
						    		mi_UINT32 res_factor,
							    	mi_event_mgr_t * p_manager)) mi_jp2_set_decoded_resolution_factor;

			l_codec->m_codec = mi_jp2_create(mi_TRUE);

			if (! l_codec->m_codec) {
				mi_free(l_codec);
				return 00;
			}

			break;
		case mi_CODEC_UNKNOWN:
		case mi_CODEC_JPT:
		default:
			mi_free(l_codec);
			return 00;
	}

	mi_set_default_event_handler(&(l_codec->m_event_mgr));
	return (mi_codec_t*) l_codec;
}

void mi_CALLCONV mi_set_default_decoder_parameters(mi_dparameters_t *parameters) {
	if(parameters) {
		memset(parameters, 0, sizeof(mi_dparameters_t));
		/* default decoding parameters */
		parameters->cp_layer = 0;
		parameters->cp_reduce = 0;

		parameters->decod_format = -1;
		parameters->cod_format = -1;
		parameters->flags = 0;		

	}
}

mi_BOOL mi_CALLCONV mi_setup_decoder(mi_codec_t *p_codec,
                                        mi_dparameters_t *parameters 
										)
{
	if (p_codec && parameters) { 
		mi_codec_private_t * l_codec = (mi_codec_private_t *) p_codec;

		if (! l_codec->is_decompressor) {
			mi_event_msg(&(l_codec->m_event_mgr), EVT_ERROR, 
                "Codec provided to the mi_setup_decoder function is not a decompressor handler.\n");
			return mi_FALSE;
		}

		l_codec->m_codec_data.m_decompression.mi_setup_decoder(l_codec->m_codec,
																parameters);
		return mi_TRUE;
	}
	return mi_FALSE;
}

mi_BOOL mi_CALLCONV mi_read_header (	mi_stream_t *p_stream,
										mi_codec_t *p_codec,
										mi_image_t **p_image )
{
	if (p_codec && p_stream) {
		mi_codec_private_t* l_codec = (mi_codec_private_t*) p_codec;
		mi_stream_private_t* l_stream = (mi_stream_private_t*) p_stream;

		if(! l_codec->is_decompressor) {
			mi_event_msg(&(l_codec->m_event_mgr), EVT_ERROR, 
                "Codec provided to the mi_read_header function is not a decompressor handler.\n");
			return mi_FALSE;
		}

		return l_codec->m_codec_data.m_decompression.mi_read_header(	l_stream,
																		l_codec->m_codec,
																		p_image,
																		&(l_codec->m_event_mgr) );
	}

	return mi_FALSE;
}

mi_BOOL mi_CALLCONV mi_decode(   mi_codec_t *p_codec,
                                    mi_stream_t *p_stream,
                                    mi_image_t* p_image)
{
	if (p_codec && p_stream) {
		mi_codec_private_t * l_codec = (mi_codec_private_t *) p_codec;
		mi_stream_private_t * l_stream = (mi_stream_private_t *) p_stream;

		if (! l_codec->is_decompressor) {
			return mi_FALSE;
		}

		return l_codec->m_codec_data.m_decompression.mi_decode(l_codec->m_codec,
																l_stream,
																p_image,
																&(l_codec->m_event_mgr) );
	}

	return mi_FALSE;
}

mi_BOOL mi_CALLCONV mi_set_decode_area(	mi_codec_t *p_codec,
											mi_image_t* p_image,
											mi_INT32 p_start_x, mi_INT32 p_start_y,
											mi_INT32 p_end_x, mi_INT32 p_end_y
											)
{
	if (p_codec) {
		mi_codec_private_t * l_codec = (mi_codec_private_t *) p_codec;
		
		if (! l_codec->is_decompressor) {
			return mi_FALSE;
		}

		return  l_codec->m_codec_data.m_decompression.mi_set_decode_area(	l_codec->m_codec,
																			p_image,
																			p_start_x, p_start_y,
																			p_end_x, p_end_y,
																			&(l_codec->m_event_mgr) );
	}
	return mi_FALSE;
}

mi_BOOL mi_CALLCONV mi_read_tile_header(	mi_codec_t *p_codec,
											mi_stream_t * p_stream,
											mi_UINT32 * p_tile_index,
											mi_UINT32 * p_data_size,
											mi_INT32 * p_tile_x0, mi_INT32 * p_tile_y0,
											mi_INT32 * p_tile_x1, mi_INT32 * p_tile_y1,
											mi_UINT32 * p_nb_comps,
											mi_BOOL * p_should_go_on)
{
	if (p_codec && p_stream && p_data_size && p_tile_index) {
		mi_codec_private_t * l_codec = (mi_codec_private_t *) p_codec;
		mi_stream_private_t * l_stream = (mi_stream_private_t *) p_stream;

		if (! l_codec->is_decompressor) {
			return mi_FALSE;
		}

		return l_codec->m_codec_data.m_decompression.mi_read_tile_header(	l_codec->m_codec,
																			p_tile_index,
																			p_data_size,
																			p_tile_x0, p_tile_y0,
																			p_tile_x1, p_tile_y1,
																			p_nb_comps,
																			p_should_go_on,
																			l_stream,
																			&(l_codec->m_event_mgr));
	}
	return mi_FALSE;
}

mi_BOOL mi_CALLCONV mi_decode_tile_data(	mi_codec_t *p_codec,
											mi_UINT32 p_tile_index,
											mi_BYTE * p_data,
											mi_UINT32 p_data_size,
											mi_stream_t *p_stream
											)
{
	if (p_codec && p_data && p_stream) {
		mi_codec_private_t * l_codec = (mi_codec_private_t *) p_codec;
		mi_stream_private_t * l_stream = (mi_stream_private_t *) p_stream;

		if (! l_codec->is_decompressor) {
			return mi_FALSE;
		}

		return l_codec->m_codec_data.m_decompression.mi_decode_tile_data(	l_codec->m_codec,
																			p_tile_index,
																			p_data,
																			p_data_size,
																			l_stream,
																			&(l_codec->m_event_mgr) );
	}
	return mi_FALSE;
}

mi_BOOL mi_CALLCONV mi_get_decoded_tile(	mi_codec_t *p_codec,
											mi_stream_t *p_stream,
											mi_image_t *p_image,
											mi_UINT32 tile_index)
{
	if (p_codec && p_stream) {
		mi_codec_private_t * l_codec = (mi_codec_private_t *) p_codec;
		mi_stream_private_t * l_stream = (mi_stream_private_t *) p_stream;

		if (! l_codec->is_decompressor) {
			return mi_FALSE;
		}
		
		return l_codec->m_codec_data.m_decompression.mi_get_decoded_tile(	l_codec->m_codec,
																			l_stream,
																			p_image,
																			&(l_codec->m_event_mgr),
																			tile_index);
	}

	return mi_FALSE;
}

mi_BOOL mi_CALLCONV mi_set_decoded_resolution_factor(mi_codec_t *p_codec, 
														mi_UINT32 res_factor )
{
	mi_codec_private_t * l_codec = (mi_codec_private_t *) p_codec;

	if ( !l_codec ){
		return mi_FALSE;
	}

	return l_codec->m_codec_data.m_decompression.mi_set_decoded_resolution_factor(l_codec->m_codec,
																			res_factor,
																			&(l_codec->m_event_mgr) );
}

/* ---------------------------------------------------------------------- */
/* COMPRESSION FUNCTIONS*/

mi_codec_t* mi_CALLCONV mi_create_compress(mi_CODEC_FORMAT p_format)
{
	mi_codec_private_t *l_codec = 00;

	l_codec = (mi_codec_private_t*)mi_calloc(1, sizeof(mi_codec_private_t));
	if (!l_codec) {
		return 00;
	}
	
	l_codec->is_decompressor = 0;

	switch(p_format) {
		case mi_CODEC_J2K:
			l_codec->m_codec_data.m_compression.mi_encode = (mi_BOOL (*) (void *,
																			struct mi_stream_private *,
																			struct mi_event_mgr * )) mi_j2k_encode;

			l_codec->m_codec_data.m_compression.mi_end_compress = (mi_BOOL (*) (	void *,
																					struct mi_stream_private *,
																					struct mi_event_mgr *)) mi_j2k_end_compress;

			l_codec->m_codec_data.m_compression.mi_start_compress = (mi_BOOL (*) (void *,
																					struct mi_stream_private *,
																					struct mi_image * ,
																					struct mi_event_mgr *)) mi_j2k_start_compress;

			l_codec->m_codec_data.m_compression.mi_write_tile = (mi_BOOL (*) (void *,
																				mi_UINT32,
																				mi_BYTE*,
																				mi_UINT32,
																				struct mi_stream_private *,
																				struct mi_event_mgr *) ) mi_j2k_write_tile;

			l_codec->m_codec_data.m_compression.mi_destroy = (void (*) (void *)) mi_j2k_destroy;

			l_codec->m_codec_data.m_compression.mi_setup_encoder = (mi_BOOL (*) (	void *,
																				mi_cparameters_t *,
																				struct mi_image *,
																				struct mi_event_mgr * )) mi_j2k_setup_encoder;

			l_codec->m_codec = mi_j2k_create_compress();
			if (! l_codec->m_codec) {
				mi_free(l_codec);
				return 00;
			}

			break;

		case mi_CODEC_JP2:
			/* get a JP2 decoder handle */
			l_codec->m_codec_data.m_compression.mi_encode = (mi_BOOL (*) (void *,
																			struct mi_stream_private *,
																			struct mi_event_mgr * )) mi_jp2_encode;

			l_codec->m_codec_data.m_compression.mi_end_compress = (mi_BOOL (*) (	void *,
																					struct mi_stream_private *,
																					struct mi_event_mgr *)) mi_jp2_end_compress;

			l_codec->m_codec_data.m_compression.mi_start_compress = (mi_BOOL (*) (void *,
																					struct mi_stream_private *,
																					struct mi_image * ,
																					struct mi_event_mgr *))  mi_jp2_start_compress;

			l_codec->m_codec_data.m_compression.mi_write_tile = (mi_BOOL (*) (void *,
																				mi_UINT32,
																				mi_BYTE*,
																				mi_UINT32,
																				struct mi_stream_private *,
																				struct mi_event_mgr *)) mi_jp2_write_tile;

			l_codec->m_codec_data.m_compression.mi_destroy = (void (*) (void *)) mi_jp2_destroy;

			l_codec->m_codec_data.m_compression.mi_setup_encoder = (mi_BOOL (*) (	void *,
																				mi_cparameters_t *,
																				struct mi_image *,
																				struct mi_event_mgr * )) mi_jp2_setup_encoder;

			l_codec->m_codec = mi_jp2_create(mi_FALSE);
			if (! l_codec->m_codec) {
				mi_free(l_codec);
				return 00;
			}

			break;

		case mi_CODEC_UNKNOWN:
		case mi_CODEC_JPT:
		default:
			mi_free(l_codec);
			return 00;
	}

	mi_set_default_event_handler(&(l_codec->m_event_mgr));
	return (mi_codec_t*) l_codec;
}

void mi_CALLCONV mi_set_default_encoder_parameters(mi_cparameters_t *parameters) {
	if(parameters) {
		memset(parameters, 0, sizeof(mi_cparameters_t));
		/* default coding parameters */
        parameters->cp_cinema = mi_OFF; /* DEPRECATED */
        parameters->rsiz = mi_PROFILE_NONE;
		parameters->max_comp_size = 0;
		parameters->numresolution = 6;
        parameters->cp_rsiz = mi_STD_RSIZ; /* DEPRECATED */
		parameters->cblockw_init = 64;
		parameters->cblockh_init = 64;
		parameters->prog_order = mi_LRCP;
		parameters->roi_compno = -1;		/* no ROI */
		parameters->subsampling_dx = 1;
		parameters->subsampling_dy = 1;
		parameters->tp_on = 0;
		parameters->decod_format = -1;
		parameters->cod_format = -1;
		parameters->tcp_rates[0] = 0;   
		parameters->tcp_numlayers = 0;
		parameters->cp_disto_alloc = 0;
		parameters->cp_fixed_alloc = 0;
		parameters->cp_fixed_quality = 0;
		parameters->jpip_on = mi_FALSE;

	}
}

mi_BOOL mi_CALLCONV mi_setup_encoder(mi_codec_t *p_codec, 
										mi_cparameters_t *parameters, 
										mi_image_t *p_image)
{
	if (p_codec && parameters && p_image) {
		mi_codec_private_t * l_codec = (mi_codec_private_t *) p_codec;

		if (! l_codec->is_decompressor) {
			return l_codec->m_codec_data.m_compression.mi_setup_encoder(	l_codec->m_codec,
																	parameters,
																	p_image,
																	&(l_codec->m_event_mgr) );
		}
	}

	return mi_FALSE;
}

mi_BOOL mi_CALLCONV mi_start_compress (	mi_codec_t *p_codec,
											mi_image_t * p_image,
											mi_stream_t *p_stream)
{
	if (p_codec && p_stream) {
		mi_codec_private_t * l_codec = (mi_codec_private_t *) p_codec;
		mi_stream_private_t * l_stream = (mi_stream_private_t *) p_stream;

		if (! l_codec->is_decompressor) {
			return l_codec->m_codec_data.m_compression.mi_start_compress(	l_codec->m_codec,
																			l_stream,
																			p_image,
																			&(l_codec->m_event_mgr));
		}
	}

	return mi_FALSE;
}

mi_BOOL mi_CALLCONV mi_encode(mi_codec_t *p_info, mi_stream_t *p_stream)
{
	if (p_info && p_stream) {
		mi_codec_private_t * l_codec = (mi_codec_private_t *) p_info;
		mi_stream_private_t * l_stream = (mi_stream_private_t *) p_stream;

		if (! l_codec->is_decompressor) {
			return l_codec->m_codec_data.m_compression.mi_encode(	l_codec->m_codec,
															l_stream,
															&(l_codec->m_event_mgr));
		}
	}

	return mi_FALSE;

}

mi_BOOL mi_CALLCONV mi_end_compress (mi_codec_t *p_codec,
										mi_stream_t *p_stream)
{
	if (p_codec && p_stream) {
		mi_codec_private_t * l_codec = (mi_codec_private_t *) p_codec;
		mi_stream_private_t * l_stream = (mi_stream_private_t *) p_stream;

		if (! l_codec->is_decompressor) {
			return l_codec->m_codec_data.m_compression.mi_end_compress(l_codec->m_codec,
																		l_stream,
																		&(l_codec->m_event_mgr));
		}
	}
	return mi_FALSE;

}

mi_BOOL mi_CALLCONV mi_end_decompress (	mi_codec_t *p_codec,
											mi_stream_t *p_stream)
{
	if (p_codec && p_stream) {
		mi_codec_private_t * l_codec = (mi_codec_private_t *) p_codec;
		mi_stream_private_t * l_stream = (mi_stream_private_t *) p_stream;

		if (! l_codec->is_decompressor) {
			return mi_FALSE;
		}
		
		return l_codec->m_codec_data.m_decompression.mi_end_decompress(l_codec->m_codec,
																		l_stream,
																		&(l_codec->m_event_mgr) );
	}

	return mi_FALSE;
}

mi_BOOL mi_CALLCONV mi_set_MCT(mi_cparameters_t *parameters,
                                  mi_FLOAT32 * pEncodingMatrix,
                                  mi_INT32 * p_dc_shift,mi_UINT32 pNbComp)
{
	mi_UINT32 l_matrix_size = pNbComp * pNbComp * (mi_UINT32)sizeof(mi_FLOAT32);
	mi_UINT32 l_dc_shift_size = pNbComp * (mi_UINT32)sizeof(mi_INT32);
	mi_UINT32 l_mct_total_size = l_matrix_size + l_dc_shift_size;

	/* add MCT capability */
    if (mi_IS_PART2(parameters->rsiz)) {
        parameters->rsiz |= mi_EXTENSION_MCT;
    } else {
        parameters->rsiz = ((mi_PROFILE_PART2) | (mi_EXTENSION_MCT));
    }
	parameters->irreversible = 1;

	/* use array based MCT */
	parameters->tcp_mct = 2;
	parameters->mct_data = mi_malloc(l_mct_total_size);
	if (! parameters->mct_data) {
		return mi_FALSE;
	}

	memcpy(parameters->mct_data,pEncodingMatrix,l_matrix_size);
	memcpy(((mi_BYTE *) parameters->mct_data) +  l_matrix_size,p_dc_shift,l_dc_shift_size);

	return mi_TRUE;
}

mi_BOOL mi_CALLCONV mi_write_tile (	mi_codec_t *p_codec,
										mi_UINT32 p_tile_index,
										mi_BYTE * p_data,
										mi_UINT32 p_data_size,
										mi_stream_t *p_stream )
{
	if (p_codec && p_stream && p_data) {
		mi_codec_private_t * l_codec = (mi_codec_private_t *) p_codec;
		mi_stream_private_t * l_stream = (mi_stream_private_t *) p_stream;

		if (l_codec->is_decompressor) {
			return mi_FALSE;
		}

		return l_codec->m_codec_data.m_compression.mi_write_tile(	l_codec->m_codec,
																	p_tile_index,
																	p_data,
																	p_data_size,
																	l_stream,
																	&(l_codec->m_event_mgr) );
	}

	return mi_FALSE;
}

/* ---------------------------------------------------------------------- */

void mi_CALLCONV mi_destroy_codec(mi_codec_t *p_codec)
{
	if (p_codec) {
		mi_codec_private_t * l_codec = (mi_codec_private_t *) p_codec;

		if (l_codec->is_decompressor) {
			l_codec->m_codec_data.m_decompression.mi_destroy(l_codec->m_codec);
		}
		else {
			l_codec->m_codec_data.m_compression.mi_destroy(l_codec->m_codec);
		}

		l_codec->m_codec = 00;
		mi_free(l_codec);
	}
}

/* ---------------------------------------------------------------------- */

void mi_CALLCONV mi_dump_codec(	mi_codec_t *p_codec,
									mi_INT32 info_flag,
									FILE* output_stream)
{
	if (p_codec) {
		mi_codec_private_t* l_codec = (mi_codec_private_t*) p_codec;

		l_codec->mi_dump_codec(l_codec->m_codec, info_flag, output_stream);
		return;
	}

	/* TODO return error */
	/* fprintf(stderr, "[ERROR] Input parameter of the dump_codec function are incorrect.\n"); */
	return;
}

mi_codestream_info_v2_t* mi_CALLCONV mi_get_cstr_info(mi_codec_t *p_codec)
{
	if (p_codec) {
		mi_codec_private_t* l_codec = (mi_codec_private_t*) p_codec;

		return l_codec->mi_get_codec_info(l_codec->m_codec);
	}

	return NULL;
}

void mi_CALLCONV mi_destroy_cstr_info(mi_codestream_info_v2_t **cstr_info) {
	if (cstr_info) {

		if ((*cstr_info)->m_default_tile_info.tccp_info){
			mi_free((*cstr_info)->m_default_tile_info.tccp_info);
		}

		if ((*cstr_info)->tile_info){
			/* FIXME not used for the moment*/
		}

		mi_free((*cstr_info));
		(*cstr_info) = NULL;
	}
}

mi_codestream_index_t * mi_CALLCONV mi_get_cstr_index(mi_codec_t *p_codec)
{
	if (p_codec) {
		mi_codec_private_t* l_codec = (mi_codec_private_t*) p_codec;

		return l_codec->mi_get_codec_index(l_codec->m_codec);
	}

	return NULL;
}

void mi_CALLCONV mi_destroy_cstr_index(mi_codestream_index_t **p_cstr_index)
{
	if (*p_cstr_index){
		j2k_destroy_cstr_index(*p_cstr_index);
		(*p_cstr_index) = NULL;
	}
}

mi_stream_t* mi_CALLCONV mi_stream_create_default_file_stream (const char *fname, mi_BOOL p_is_read_stream)
{
    return mi_stream_create_file_stream(fname, mi_J2K_STREAM_CHUNK_SIZE, p_is_read_stream);
}

mi_stream_t* mi_CALLCONV mi_stream_create_file_stream (
        const char *fname, 
		mi_SIZE_T p_size, 
        mi_BOOL p_is_read_stream)
{
    mi_stream_t* l_stream = 00;
    FILE *p_file;
    const char *mode;

    if (! fname) {
        return NULL;
    }
    
    if(p_is_read_stream) mode = "rb"; else mode = "wb";

    p_file = fopen(fname, mode);

    if (! p_file) {
	    return NULL;
    }

    l_stream = mi_stream_create(p_size,p_is_read_stream);
    if (! l_stream) {
        fclose(p_file);
        return NULL;
    }

    mi_stream_set_user_data(l_stream, p_file, (mi_stream_free_user_data_fn) fclose);
    mi_stream_set_user_data_length(l_stream, mi_get_data_length_from_file(p_file));
    mi_stream_set_read_function(l_stream, (mi_stream_read_fn) mi_read_from_file);
    mi_stream_set_write_function(l_stream, (mi_stream_write_fn) mi_write_from_file);
    mi_stream_set_skip_function(l_stream, (mi_stream_skip_fn) mi_skip_from_file);
    mi_stream_set_seek_function(l_stream, (mi_stream_seek_fn) mi_seek_from_file);

    return l_stream;
}
