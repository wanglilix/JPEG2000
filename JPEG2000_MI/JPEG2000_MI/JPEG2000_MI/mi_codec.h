
#ifndef __mi_CODEC_H
#define __mi_CODEC_H
#include"openjpeg.h"
#include"cio.h"
#include"event.h"
/**
@file mi_codec.h
*/


/**
 * Main codec handler used for compression or decompression.
 */
//编解码器结构（用时变成编码器结构或解码器结构）
typedef struct mi_codec_private
{
    /** 编解码器结构 FIXME DOC */
    union
    {
        /**
         * Decompression handler.（解码器）
         */
        struct mi_decompression
        {
            /** Main header reading function handler */
            mi_BOOL (*mi_read_header) ( struct mi_stream_private * cio,
                                          void * p_codec,
                                          mi_image_t **p_image,
                                          struct mi_event_mgr * p_manager);

            /** Decoding function */
            mi_BOOL (*mi_decode) ( void * p_codec,
                                     struct mi_stream_private * p_cio,
                                     mi_image_t * p_image,
                                     struct mi_event_mgr * p_manager);

            /** FIXME DOC */
            mi_BOOL (*mi_read_tile_header)( void * p_codec,
                                              mi_UINT32 * p_tile_index,
                                              mi_UINT32 * p_data_size,
                                              mi_INT32 * p_tile_x0,
                                              mi_INT32 * p_tile_y0,
                                              mi_INT32 * p_tile_x1,
                                              mi_INT32 * p_tile_y1,
                                              mi_UINT32 * p_nb_comps,
                                              mi_BOOL * p_should_go_on,
                                              struct mi_stream_private * p_cio,
                                              struct mi_event_mgr * p_manager);

            /** FIXME DOC */
            mi_BOOL (*mi_decode_tile_data)( void * p_codec,
                                              mi_UINT32 p_tile_index,
                                              mi_BYTE * p_data,
                                              mi_UINT32 p_data_size,
                                              struct mi_stream_private * p_cio,
                                              struct mi_event_mgr * p_manager);

            /** Reading function used after codestream if necessary */
            mi_BOOL (* mi_end_decompress) ( void *p_codec,
                                              struct mi_stream_private * cio,
                                              struct mi_event_mgr * p_manager);

            /** Codec destroy function handler */
            void (*mi_destroy) (void * p_codec);

            /** Setup decoder function handler */
            void (*mi_setup_decoder) ( void * p_codec, mi_dparameters_t * p_param);

            /** Set decode area function handler */
            mi_BOOL (*mi_set_decode_area) ( void * p_codec,
                                              mi_image_t * p_image,
                                              mi_INT32 p_start_x,
                                              mi_INT32 p_end_x,
                                              mi_INT32 p_start_y,
                                              mi_INT32 p_end_y,
                                              struct mi_event_mgr * p_manager);

            /** Get tile function */
            mi_BOOL (*mi_get_decoded_tile) ( void *p_codec,
                                               mi_stream_private_t * p_cio,
                                               mi_image_t *p_image,
                                               struct mi_event_mgr * p_manager,
                                               mi_UINT32 tile_index);

            /** Set the decoded resolution factor */
            mi_BOOL (*mi_set_decoded_resolution_factor) ( void * p_codec,
                                                            mi_UINT32 res_factor,
                                                            mi_event_mgr_t * p_manager);
        } m_decompression;

        /**
         * Compression handler. （编码器）FIXME DOC
         */
        struct mi_compression
        {
            mi_BOOL (* mi_start_compress) ( void *p_codec,
                                              struct mi_stream_private * cio,
                                              struct mi_image * p_image,
                                              struct mi_event_mgr * p_manager);

            mi_BOOL (* mi_encode) ( void * p_codec,
                                      struct mi_stream_private *p_cio,
                                      struct mi_event_mgr * p_manager);

            mi_BOOL (* mi_write_tile) ( void * p_codec,
                                          mi_UINT32 p_tile_index,
                                          mi_BYTE * p_data,
                                          mi_UINT32 p_data_size,
                                          struct mi_stream_private * p_cio,
                                          struct mi_event_mgr * p_manager);

            mi_BOOL (* mi_end_compress) (	void * p_codec,
                                            struct mi_stream_private * p_cio,
                                            struct mi_event_mgr * p_manager);

            void (* mi_destroy) (void * p_codec);

            mi_BOOL (* mi_setup_encoder) ( void * p_codec,
                                             mi_cparameters_t * p_param,
                                             struct mi_image * p_image,
                                             struct mi_event_mgr * p_manager);
        } m_compression;
    } m_codec_data;
    /** FIXME DOC*/
    void * m_codec;
    /** Event handler */
    mi_event_mgr_t m_event_mgr;
    /** Flag to indicate if the codec is used to decode or encode*/
    mi_BOOL is_decompressor;
    void (*mi_dump_codec) (void * p_codec, mi_INT32 info_flag, FILE* output_stream);
    mi_codestream_info_v2_t* (*mi_get_codec_info)(void* p_codec);
    mi_codestream_index_t* (*mi_get_codec_index)(void* p_codec);
}mi_codec_private_t;


#endif /* __mi_CODEC_H */

