
#include "mi_includes.h"

/** @defgroup J2K J2K - JPEG-2000 codestream reader/writer */
/*@{*/

/** @name Local static functions */
/*@{*/

/**
 * Sets up the procedures to do on reading header. Developpers wanting to extend the library can add their own reading procedures.
 */
static mi_BOOL mi_j2k_setup_header_reading (mi_j2k_t *p_j2k, mi_event_mgr_t * p_manager);

/**
 * The read header procedure.
 */
static mi_BOOL mi_j2k_read_header_procedure(  mi_j2k_t *p_j2k,
                                                mi_stream_private_t *p_stream,
                                                mi_event_mgr_t * p_manager);

/**
 * The default encoding validation procedure without any extension.
 *
 * @param       p_j2k                   the jpeg2000 codec to validate.
 * @param       p_stream                the input stream to validate.
 * @param       p_manager               the user event manager.
 *
 * @return true if the parameters are correct.
 */
static mi_BOOL mi_j2k_encoding_validation (   mi_j2k_t * p_j2k,
                                                mi_stream_private_t *p_stream,
                                                mi_event_mgr_t * p_manager );

/**
 * The default decoding validation procedure without any extension.
 *
 * @param       p_j2k                   the jpeg2000 codec to validate.
 * @param       p_stream                                the input stream to validate.
 * @param       p_manager               the user event manager.
 *
 * @return true if the parameters are correct.
 */
static mi_BOOL mi_j2k_decoding_validation (   mi_j2k_t * p_j2k,
                                                mi_stream_private_t *p_stream,
                                                mi_event_mgr_t * p_manager );

/**
 * Sets up the validation ,i.e. adds the procedures to lauch to make sure the codec parameters
 * are valid. Developpers wanting to extend the library can add their own validation procedures.
 */
static mi_BOOL mi_j2k_setup_encoding_validation (mi_j2k_t *p_j2k, mi_event_mgr_t * p_manager);

/**
 * Sets up the validation ,i.e. adds the procedures to lauch to make sure the codec parameters
 * are valid. Developpers wanting to extend the library can add their own validation procedures.
 */
static mi_BOOL mi_j2k_setup_decoding_validation (mi_j2k_t *p_j2k, mi_event_mgr_t * p_manager);

/**
 * Sets up the validation ,i.e. adds the procedures to lauch to make sure the codec parameters
 * are valid. Developpers wanting to extend the library can add their own validation procedures.
 */
static mi_BOOL mi_j2k_setup_end_compress (mi_j2k_t *p_j2k, mi_event_mgr_t * p_manager);

/**
 * The mct encoding validation procedure.
 *
 * @param       p_j2k                   the jpeg2000 codec to validate.
 * @param       p_stream                                the input stream to validate.
 * @param       p_manager               the user event manager.
 *
 * @return true if the parameters are correct.
 */
static mi_BOOL mi_j2k_mct_validation (mi_j2k_t * p_j2k,
                                        mi_stream_private_t *p_stream,
                                        mi_event_mgr_t * p_manager );

/**
 * Builds the tcd decoder to use to decode tile.
 */
static mi_BOOL mi_j2k_build_decoder ( mi_j2k_t * p_j2k,
                                        mi_stream_private_t *p_stream,
                                        mi_event_mgr_t * p_manager );
/**
 * Builds the tcd encoder to use to encode tile.
 */
static mi_BOOL mi_j2k_build_encoder ( mi_j2k_t * p_j2k,
                                        mi_stream_private_t *p_stream,
                                        mi_event_mgr_t * p_manager );

/**
 * Creates a tile-coder decoder.
 *
 * @param       p_stream                        the stream to write data to.
 * @param       p_j2k                           J2K codec.
 * @param       p_manager                   the user event manager.
*/
static mi_BOOL mi_j2k_create_tcd(     mi_j2k_t *p_j2k,
                                                                    mi_stream_private_t *p_stream,
                                                                    mi_event_mgr_t * p_manager );

/**
 * Excutes the given procedures on the given codec.
 *
 * @param       p_procedure_list        the list of procedures to execute
 * @param       p_j2k                           the jpeg2000 codec to execute the procedures on.
 * @param       p_stream                        the stream to execute the procedures on.
 * @param       p_manager                       the user manager.
 *
 * @return      true                            if all the procedures were successfully executed.
 */
static mi_BOOL mi_j2k_exec (  mi_j2k_t * p_j2k,
                            mi_procedure_list_t * p_procedure_list,
                            mi_stream_private_t *p_stream,
                            mi_event_mgr_t * p_manager);

/**
 * Updates the rates of the tcp.
 *
 * @param       p_stream                                the stream to write data to.
 * @param       p_j2k                           J2K codec.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_update_rates(   mi_j2k_t *p_j2k,
                                                                            mi_stream_private_t *p_stream,
                                                                            mi_event_mgr_t * p_manager );

/**
 * Copies the decoding tile parameters onto all the tile parameters.
 * Creates also the tile decoder.
 */
static mi_BOOL mi_j2k_copy_default_tcp_and_create_tcd (       mi_j2k_t * p_j2k,
                                                            mi_stream_private_t *p_stream,
                                                            mi_event_mgr_t * p_manager );

/**
 * Destroys the memory associated with the decoding of headers.
 */
static mi_BOOL mi_j2k_destroy_header_memory ( mi_j2k_t * p_j2k,
                                                mi_stream_private_t *p_stream,
                                                mi_event_mgr_t * p_manager );

/**
 * Reads the lookup table containing all the marker, status and action, and returns the handler associated
 * with the marker value.
 * @param       p_id            Marker value to look up
 *
 * @return      the handler associated with the id.
*/
static const struct mi_dec_memory_marker_handler * mi_j2k_get_marker_handler (mi_UINT32 p_id);

/**
 * Destroys a tile coding parameter structure.
 *
 * @param       p_tcp           the tile coding parameter to destroy.
 */
static void mi_j2k_tcp_destroy (mi_tcp_t *p_tcp);

/**
 * Destroys the data inside a tile coding parameter structure.
 *
 * @param       p_tcp           the tile coding parameter which contain data to destroy.
 */
static void mi_j2k_tcp_data_destroy (mi_tcp_t *p_tcp);

/**
 * Destroys a coding parameter structure.
 *
 * @param       p_cp            the coding parameter to destroy.
 */
static void mi_j2k_cp_destroy (mi_cp_t *p_cp);

/**
 * Compare 2 a SPCod/ SPCoc elements, i.e. the coding style of a given component of a tile.
 *
 * @param       p_j2k            J2K codec.
 * @param       p_tile_no        Tile number
 * @param       p_first_comp_no  The 1st component number to compare.
 * @param       p_second_comp_no The 1st component number to compare.
 *
 * @return mi_TRUE if SPCdod are equals.
 */
static mi_BOOL mi_j2k_compare_SPCod_SPCoc(mi_j2k_t *p_j2k, mi_UINT32 p_tile_no, mi_UINT32 p_first_comp_no, mi_UINT32 p_second_comp_no);

/**
 * Writes a SPCod or SPCoc element, i.e. the coding style of a given component of a tile.
 *
 * @param       p_j2k           J2K codec.
 * @param       p_tile_no       FIXME DOC
 * @param       p_comp_no       the component number to output.
 * @param       p_data          FIXME DOC
 * @param       p_header_size   FIXME DOC
 * @param       p_manager       the user event manager.
 *
 * @return FIXME DOC
*/
static mi_BOOL mi_j2k_write_SPCod_SPCoc(      mi_j2k_t *p_j2k,
                                                                                    mi_UINT32 p_tile_no,
                                                                                    mi_UINT32 p_comp_no,
                                                                                    mi_BYTE * p_data,
                                                                                    mi_UINT32 * p_header_size,
                                                                                    mi_event_mgr_t * p_manager );

/**
 * Gets the size taken by writing a SPCod or SPCoc for the given tile and component.
 *
 * @param       p_j2k                   the J2K codec.
 * @param       p_tile_no               the tile index.
 * @param       p_comp_no               the component being outputted.
 *
 * @return      the number of bytes taken by the SPCod element.
 */
static mi_UINT32 mi_j2k_get_SPCod_SPCoc_size (mi_j2k_t *p_j2k,
                                                                                            mi_UINT32 p_tile_no,
                                                                                            mi_UINT32 p_comp_no );

/**
 * Reads a SPCod or SPCoc element, i.e. the coding style of a given component of a tile.
 * @param       p_j2k           the jpeg2000 codec.
 * @param       compno          FIXME DOC
 * @param       p_header_data   the data contained in the COM box.
 * @param       p_header_size   the size of the data contained in the COM marker.
 * @param       p_manager       the user event manager.
*/
static mi_BOOL mi_j2k_read_SPCod_SPCoc(   mi_j2k_t *p_j2k,
                                            mi_UINT32 compno,
                                            mi_BYTE * p_header_data,
                                            mi_UINT32 * p_header_size,
                                            mi_event_mgr_t * p_manager );

/**
 * Gets the size taken by writing SQcd or SQcc element, i.e. the quantization values of a band in the QCD or QCC.
 *
 * @param       p_tile_no               the tile index.
 * @param       p_comp_no               the component being outputted.
 * @param       p_j2k                   the J2K codec.
 *
 * @return      the number of bytes taken by the SPCod element.
 */
static mi_UINT32 mi_j2k_get_SQcd_SQcc_size (  mi_j2k_t *p_j2k,
                                                                                    mi_UINT32 p_tile_no,
                                                                                    mi_UINT32 p_comp_no );

/**
 * Compares 2 SQcd or SQcc element, i.e. the quantization values of a band in the QCD or QCC.
 *
 * @param       p_j2k                   J2K codec.
 * @param       p_tile_no               the tile to output.
 * @param       p_first_comp_no         the first component number to compare.
 * @param       p_second_comp_no        the second component number to compare.
 *
 * @return mi_TRUE if equals.
 */
static mi_BOOL mi_j2k_compare_SQcd_SQcc(mi_j2k_t *p_j2k, mi_UINT32 p_tile_no, mi_UINT32 p_first_comp_no, mi_UINT32 p_second_comp_no);


/**
 * Writes a SQcd or SQcc element, i.e. the quantization values of a band in the QCD or QCC.
 *
 * @param       p_tile_no               the tile to output.
 * @param       p_comp_no               the component number to output.
 * @param       p_data                  the data buffer.
 * @param       p_header_size   pointer to the size of the data buffer, it is changed by the function.
 * @param       p_j2k                   J2K codec.
 * @param       p_manager               the user event manager.
 *
*/
static mi_BOOL mi_j2k_write_SQcd_SQcc(mi_j2k_t *p_j2k,
                                                                            mi_UINT32 p_tile_no,
                                                                            mi_UINT32 p_comp_no,
                                                                            mi_BYTE * p_data,
                                                                            mi_UINT32 * p_header_size,
                                                                            mi_event_mgr_t * p_manager);

/**
 * Updates the Tile Length Marker.
 */
static void mi_j2k_update_tlm ( mi_j2k_t * p_j2k, mi_UINT32 p_tile_part_size);

/**
 * Reads a SQcd or SQcc element, i.e. the quantization values of a band in the QCD or QCC.
 *
 * @param       p_j2k           J2K codec.
 * @param       compno          the component number to output.
 * @param       p_header_data   the data buffer.
 * @param       p_header_size   pointer to the size of the data buffer, it is changed by the function.
 * @param       p_manager       the user event manager.
 *
*/
static mi_BOOL mi_j2k_read_SQcd_SQcc( mi_j2k_t *p_j2k,
                                        mi_UINT32 compno,
                                        mi_BYTE * p_header_data,
                                        mi_UINT32 * p_header_size,
                                        mi_event_mgr_t * p_manager );

/**
 * Copies the tile component parameters of all the component from the first tile component.
 *
 * @param               p_j2k           the J2k codec.
 */
static void mi_j2k_copy_tile_component_parameters( mi_j2k_t *p_j2k );

/**
 * Copies the tile quantization parameters of all the component from the first tile component.
 *
 * @param               p_j2k           the J2k codec.
 */
static void mi_j2k_copy_tile_quantization_parameters( mi_j2k_t *p_j2k );

/**
 * Reads the tiles.
 */
static mi_BOOL mi_j2k_decode_tiles (  mi_j2k_t *p_j2k,
                                        mi_stream_private_t *p_stream,
                                        mi_event_mgr_t * p_manager);

static mi_BOOL mi_j2k_pre_write_tile ( mi_j2k_t * p_j2k,
                                                                             mi_UINT32 p_tile_index,
                                                                             mi_stream_private_t *p_stream,
                                                                             mi_event_mgr_t * p_manager );

static mi_BOOL mi_j2k_update_image_data (mi_tcd_t * p_tcd, mi_BYTE * p_data, mi_image_t* p_output_image);

static void mi_get_tile_dimensions(mi_image_t * l_image,
																		mi_tcd_tilecomp_t * l_tilec,
																		mi_image_comp_t * l_img_comp,
																		mi_UINT32* l_size_comp,
																		mi_UINT32* l_width,
																		mi_UINT32* l_height,
																		mi_UINT32* l_offset_x,
																		mi_UINT32* l_offset_y,
																		mi_UINT32* l_image_width,
																		mi_UINT32* l_stride,
																		mi_UINT32* l_tile_offset);

static void mi_j2k_get_tile_data (mi_tcd_t * p_tcd, mi_BYTE * p_data);

static mi_BOOL mi_j2k_post_write_tile (mi_j2k_t * p_j2k,
                                                                             mi_stream_private_t *p_stream,
                                                                             mi_event_mgr_t * p_manager );

/**
 * Sets up the procedures to do on writing header.
 * Developers wanting to extend the library can add their own writing procedures.
 */
static mi_BOOL mi_j2k_setup_header_writing (mi_j2k_t *p_j2k, mi_event_mgr_t * p_manager);

static mi_BOOL mi_j2k_write_first_tile_part(  mi_j2k_t *p_j2k,
                                                                                            mi_BYTE * p_data,
                                                                                            mi_UINT32 * p_data_written,
                                                                                            mi_UINT32 p_total_data_size,
                                                                                            mi_stream_private_t *p_stream,
                                                                                            struct mi_event_mgr * p_manager );

static mi_BOOL mi_j2k_write_all_tile_parts(   mi_j2k_t *p_j2k,
                                                                                            mi_BYTE * p_data,
                                                                                            mi_UINT32 * p_data_written,
                                                                                            mi_UINT32 p_total_data_size,
                                                                                            mi_stream_private_t *p_stream,
                                                                                            struct mi_event_mgr * p_manager );

/**
 * Gets the offset of the header.
 *
 * @param       p_stream                the stream to write data to.
 * @param       p_j2k                   J2K codec.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_get_end_header( mi_j2k_t *p_j2k,
                                        mi_stream_private_t *p_stream,
                                        mi_event_mgr_t * p_manager );

static mi_BOOL mi_j2k_allocate_tile_element_cstr_index(mi_j2k_t *p_j2k);

/*
 * -----------------------------------------------------------------------
 * -----------------------------------------------------------------------
 * -----------------------------------------------------------------------
 */

/**
 * Writes the SOC marker (Start Of Codestream)
 *
 * @param       p_stream                        the stream to write data to.
 * @param       p_j2k                   J2K codec.
 * @param       p_manager       the user event manager.
*/
static mi_BOOL mi_j2k_write_soc(      mi_j2k_t *p_j2k,
                                                        mi_stream_private_t *p_stream,
                                                            mi_event_mgr_t * p_manager );

/**
 * Reads a SOC marker (Start of Codestream)
 * @param       p_j2k           the jpeg2000 file codec.
 * @param       p_stream        XXX needs data
 * @param       p_manager       the user event manager.
*/
static mi_BOOL mi_j2k_read_soc(   mi_j2k_t *p_j2k,
                                    mi_stream_private_t *p_stream,
                                    mi_event_mgr_t * p_manager );

/**
 * Writes the SIZ marker (image and tile size)
 *
 * @param       p_j2k           J2K codec.
 * @param       p_stream        the stream to write data to.
 * @param       p_manager       the user event manager.
*/
static mi_BOOL mi_j2k_write_siz(      mi_j2k_t *p_j2k,
                                                                mi_stream_private_t *p_stream,
                                                                mi_event_mgr_t * p_manager );

/**
 * Reads a SIZ marker (image and tile size)
 * @param       p_j2k           the jpeg2000 file codec.
 * @param       p_header_data   the data contained in the SIZ box.
 * @param       p_header_size   the size of the data contained in the SIZ marker.
 * @param       p_manager       the user event manager.
*/
static mi_BOOL mi_j2k_read_siz(mi_j2k_t *p_j2k,
                                 mi_BYTE * p_header_data,
                                 mi_UINT32 p_header_size,
                                 mi_event_mgr_t * p_manager);

/**
 * Writes the COM marker (comment)
 *
 * @param       p_stream                        the stream to write data to.
 * @param       p_j2k                   J2K codec.
 * @param       p_manager       the user event manager.
*/
static mi_BOOL mi_j2k_write_com(      mi_j2k_t *p_j2k,
                                                                        mi_stream_private_t *p_stream,
                                                                        mi_event_mgr_t * p_manager );

/**
 * Reads a COM marker (comments)
 * @param       p_j2k           the jpeg2000 file codec.
 * @param       p_header_data   the data contained in the COM box.
 * @param       p_header_size   the size of the data contained in the COM marker.
 * @param       p_manager       the user event manager.
*/
static mi_BOOL mi_j2k_read_com (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager );
/**
 * Writes the COD marker (Coding style default)
 *
 * @param       p_stream                        the stream to write data to.
 * @param       p_j2k                   J2K codec.
 * @param       p_manager       the user event manager.
*/
static mi_BOOL mi_j2k_write_cod(      mi_j2k_t *p_j2k,
                                                                        mi_stream_private_t *p_stream,
                                                                        mi_event_mgr_t * p_manager );

/**
 * Reads a COD marker (Coding Styke defaults)
 * @param       p_header_data   the data contained in the COD box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the COD marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_cod (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager);

/**
 * Compares 2 COC markers (Coding style component)
 *
 * @param       p_j2k            J2K codec.
 * @param       p_first_comp_no  the index of the first component to compare.
 * @param       p_second_comp_no the index of the second component to compare.
 *
 * @return      mi_TRUE if equals
 */
static mi_BOOL mi_j2k_compare_coc(mi_j2k_t *p_j2k, mi_UINT32 p_first_comp_no, mi_UINT32 p_second_comp_no);

/**
 * Writes the COC marker (Coding style component)
 *
 * @param       p_j2k       J2K codec.
 * @param       p_comp_no   the index of the component to output.
 * @param       p_stream    the stream to write data to.
 * @param       p_manager   the user event manager.
*/
static mi_BOOL mi_j2k_write_coc(  mi_j2k_t *p_j2k,
                                                                mi_UINT32 p_comp_no,
                                                                mi_stream_private_t *p_stream,
                                                                mi_event_mgr_t * p_manager );

/**
 * Writes the COC marker (Coding style component)
 *
 * @param       p_j2k                   J2K codec.
 * @param       p_comp_no               the index of the component to output.
 * @param       p_data          FIXME DOC
 * @param       p_data_written  FIXME DOC
 * @param       p_manager               the user event manager.
*/
static void mi_j2k_write_coc_in_memory(mi_j2k_t *p_j2k,
                                                                            mi_UINT32 p_comp_no,
                                                                            mi_BYTE * p_data,
                                                                            mi_UINT32 * p_data_written,
                                                                            mi_event_mgr_t * p_manager );

/**
 * Gets the maximum size taken by a coc.
 *
 * @param       p_j2k   the jpeg2000 codec to use.
 */
static mi_UINT32 mi_j2k_get_max_coc_size(mi_j2k_t *p_j2k);

/**
 * Reads a COC marker (Coding Style Component)
 * @param       p_header_data   the data contained in the COC box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the COC marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_coc (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager );

/**
 * Writes the QCD marker (quantization default)
 *
 * @param       p_j2k                   J2K codec.
 * @param       p_stream                the stream to write data to.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_write_qcd(      mi_j2k_t *p_j2k,
                                                                        mi_stream_private_t *p_stream,
                                                                        mi_event_mgr_t * p_manager );

/**
 * Reads a QCD marker (Quantization defaults)
 * @param       p_header_data   the data contained in the QCD box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the QCD marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_qcd (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager );

/**
 * Compare QCC markers (quantization component)
 *
 * @param       p_j2k                 J2K codec.
 * @param       p_first_comp_no       the index of the first component to compare.
 * @param       p_second_comp_no      the index of the second component to compare.
 *
 * @return mi_TRUE if equals.
 */
static mi_BOOL mi_j2k_compare_qcc(mi_j2k_t *p_j2k, mi_UINT32 p_first_comp_no, mi_UINT32 p_second_comp_no);

/**
 * Writes the QCC marker (quantization component)
 *
 * @param       p_comp_no       the index of the component to output.
 * @param       p_stream                the stream to write data to.
 * @param       p_j2k                   J2K codec.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_write_qcc(      mi_j2k_t *p_j2k,
                                                                        mi_UINT32 p_comp_no,
                                                                        mi_stream_private_t *p_stream,
                                                                        mi_event_mgr_t * p_manager );

/**
 * Writes the QCC marker (quantization component)
 *
 * @param       p_j2k           J2K codec.
 * @param       p_comp_no       the index of the component to output.
 * @param       p_data          FIXME DOC
 * @param       p_data_written  the stream to write data to.
 * @param       p_manager       the user event manager.
*/
static void mi_j2k_write_qcc_in_memory(mi_j2k_t *p_j2k,
                                                                            mi_UINT32 p_comp_no,
                                                                            mi_BYTE * p_data,
                                                                            mi_UINT32 * p_data_written,
                                                                            mi_event_mgr_t * p_manager );

/**
 * Gets the maximum size taken by a qcc.
 */
static mi_UINT32 mi_j2k_get_max_qcc_size (mi_j2k_t *p_j2k);

/**
 * Reads a QCC marker (Quantization component)
 * @param       p_header_data   the data contained in the QCC box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the QCC marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_qcc(   mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager);
/**
 * Writes the POC marker (Progression Order Change)
 *
 * @param       p_stream                                the stream to write data to.
 * @param       p_j2k                           J2K codec.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_write_poc(      mi_j2k_t *p_j2k,
                                                                        mi_stream_private_t *p_stream,
                                                                        mi_event_mgr_t * p_manager );
/**
 * Writes the POC marker (Progression Order Change)
 *
 * @param       p_j2k          J2K codec.
 * @param       p_data         FIXME DOC
 * @param       p_data_written the stream to write data to.
 * @param       p_manager      the user event manager.
 */
static void mi_j2k_write_poc_in_memory(mi_j2k_t *p_j2k,
                                                                            mi_BYTE * p_data,
                                                                            mi_UINT32 * p_data_written,
                                                                            mi_event_mgr_t * p_manager );
/**
 * Gets the maximum size taken by the writing of a POC.
 */
static mi_UINT32 mi_j2k_get_max_poc_size(mi_j2k_t *p_j2k);

/**
 * Reads a POC marker (Progression Order Change)
 *
 * @param       p_header_data   the data contained in the POC box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the POC marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_poc (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager );

/**
 * Gets the maximum size taken by the toc headers of all the tile parts of any given tile.
 */
static mi_UINT32 mi_j2k_get_max_toc_size (mi_j2k_t *p_j2k);

/**
 * Gets the maximum size taken by the headers of the SOT.
 *
 * @param       p_j2k   the jpeg2000 codec to use.
 */
static mi_UINT32 mi_j2k_get_specific_header_sizes(mi_j2k_t *p_j2k);

/**
 * Reads a CRG marker (Component registration)
 *
 * @param       p_header_data   the data contained in the TLM box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the TLM marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_crg (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager );
/**
 * Reads a TLM marker (Tile Length Marker)
 *
 * @param       p_header_data   the data contained in the TLM box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the TLM marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_tlm (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager);

/**
 * Writes the updated tlm.
 *
 * @param       p_stream                the stream to write data to.
 * @param       p_j2k                   J2K codec.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_write_updated_tlm(      mi_j2k_t *p_j2k,
                                            mi_stream_private_t *p_stream,
                                            mi_event_mgr_t * p_manager );

/**
 * Reads a PLM marker (Packet length, main header marker)
 *
 * @param       p_header_data   the data contained in the TLM box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the TLM marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_plm (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager);
/**
 * Reads a PLT marker (Packet length, tile-part header)
 *
 * @param       p_header_data   the data contained in the PLT box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the PLT marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_plt (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager );

/**
 * Reads a PPM marker (Packed headers, main header)
 *
 * @param       p_header_data   the data contained in the POC box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the POC marker.
 * @param       p_manager               the user event manager.
 */

static mi_BOOL mi_j2k_read_ppm (
																		 mi_j2k_t *p_j2k,
																		 mi_BYTE * p_header_data,
																		 mi_UINT32 p_header_size,
																		 mi_event_mgr_t * p_manager );

/**
 * Merges all PPM markers read (Packed headers, main header)
 *
 * @param       p_cp      main coding parameters.
 * @param       p_manager the user event manager.
 */
static mi_BOOL mi_j2k_merge_ppm ( mi_cp_t *p_cp, mi_event_mgr_t * p_manager );

/**
 * Reads a PPT marker (Packed packet headers, tile-part header)
 *
 * @param       p_header_data   the data contained in the PPT box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the PPT marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_ppt (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager );

/**
 * Merges all PPT markers read (Packed headers, tile-part header)
 *
 * @param       p_tcp   the tile.
 * @param       p_manager               the user event manager.
 */
static mi_BOOL mi_j2k_merge_ppt (  mi_tcp_t *p_tcp,
																	   mi_event_mgr_t * p_manager );


/**
 * Writes the TLM marker (Tile Length Marker)
 *
 * @param       p_stream                                the stream to write data to.
 * @param       p_j2k                           J2K codec.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_write_tlm(      mi_j2k_t *p_j2k,
                                                                        mi_stream_private_t *p_stream,
                                                                        mi_event_mgr_t * p_manager );

/**
 * Writes the SOT marker (Start of tile-part)
 *
 * @param       p_j2k            J2K codec.
 * @param       p_data           FIXME DOC
 * @param       p_data_written   FIXME DOC
 * @param       p_stream         the stream to write data to.
 * @param       p_manager        the user event manager.
*/
static mi_BOOL mi_j2k_write_sot(      mi_j2k_t *p_j2k,
                                                                        mi_BYTE * p_data,
                                                                        mi_UINT32 * p_data_written,
                                                                        const mi_stream_private_t *p_stream,
                                                                        mi_event_mgr_t * p_manager );

/**
 * Reads values from a SOT marker (Start of tile-part)
 *
 * the j2k decoder state is not affected. No side effects, no checks except for p_header_size.
 *
 * @param       p_header_data   the data contained in the SOT marker.
 * @param       p_header_size   the size of the data contained in the SOT marker.
 * @param       p_tile_no       Isot.
 * @param       p_tot_len       Psot.
 * @param       p_current_part  TPsot.
 * @param       p_num_parts     TNsot.
 * @param       p_manager       the user event manager.
 */
static mi_BOOL mi_j2k_get_sot_values(mi_BYTE *  p_header_data,
																			 mi_UINT32  p_header_size,
																			 mi_UINT32* p_tile_no,
																			 mi_UINT32* p_tot_len,
																			 mi_UINT32* p_current_part,
																			 mi_UINT32* p_num_parts,
																			 mi_event_mgr_t * p_manager );
/**
 * Reads a SOT marker (Start of tile-part)
 *
 * @param       p_header_data   the data contained in the SOT marker.
 * @param       p_j2k           the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the PPT marker.
 * @param       p_manager       the user event manager.
*/
static mi_BOOL mi_j2k_read_sot (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager );
/**
 * Writes the SOD marker (Start of data)
 *
 * @param       p_j2k               J2K codec.
 * @param       p_tile_coder        FIXME DOC
 * @param       p_data              FIXME DOC
 * @param       p_data_written      FIXME DOC
 * @param       p_total_data_size   FIXME DOC
 * @param       p_stream            the stream to write data to.
 * @param       p_manager           the user event manager.
*/
static mi_BOOL mi_j2k_write_sod(      mi_j2k_t *p_j2k,
                                                                        mi_tcd_t * p_tile_coder,
                                                                        mi_BYTE * p_data,
                                                                        mi_UINT32 * p_data_written,
                                                                        mi_UINT32 p_total_data_size,
                                                                        const mi_stream_private_t *p_stream,
                                                                        mi_event_mgr_t * p_manager );

/**
 * Reads a SOD marker (Start Of Data)
 *
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_stream                FIXME DOC
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_sod(   mi_j2k_t *p_j2k,
                                    mi_stream_private_t *p_stream,
                                    mi_event_mgr_t * p_manager );

static void mi_j2k_update_tlm (mi_j2k_t * p_j2k, mi_UINT32 p_tile_part_size )
{
        mi_write_bytes(p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_current,p_j2k->m_current_tile_number,1);            /* PSOT */
        ++p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_current;

        mi_write_bytes(p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_current,p_tile_part_size,4);                                        /* PSOT */
        p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_current += 4;
}

/**
 * Writes the RGN marker (Region Of Interest)
 *
 * @param       p_tile_no               the tile to output
 * @param       p_comp_no               the component to output
 * @param       nb_comps                the number of components
 * @param       p_stream                the stream to write data to.
 * @param       p_j2k                   J2K codec.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_write_rgn(  mi_j2k_t *p_j2k,
                                    mi_UINT32 p_tile_no,
                                    mi_UINT32 p_comp_no,
                                    mi_UINT32 nb_comps,
                                    mi_stream_private_t *p_stream,
                                    mi_event_mgr_t * p_manager );

/**
 * Reads a RGN marker (Region Of Interest)
 *
 * @param       p_header_data   the data contained in the POC box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the POC marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_rgn (mi_j2k_t *p_j2k,
                                  mi_BYTE * p_header_data,
                                  mi_UINT32 p_header_size,
                                  mi_event_mgr_t * p_manager );

/**
 * Writes the EOC marker (End of Codestream)
 *
 * @param       p_stream                the stream to write data to.
 * @param       p_j2k                   J2K codec.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_write_eoc(      mi_j2k_t *p_j2k,
                                    mi_stream_private_t *p_stream,
                                    mi_event_mgr_t * p_manager );


/**
 * Writes the CBD-MCT-MCC-MCO markers (Multi components transform)
 *
 * @param       p_stream                        the stream to write data to.
 * @param       p_j2k                   J2K codec.
 * @param       p_manager       the user event manager.
*/
static mi_BOOL mi_j2k_write_mct_data_group(   mi_j2k_t *p_j2k,
                                                mi_stream_private_t *p_stream,
                                                mi_event_mgr_t * p_manager );

/**
 * Inits the Info
 *
 * @param       p_stream                the stream to write data to.
 * @param       p_j2k                   J2K codec.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_init_info(      mi_j2k_t *p_j2k,
                                    mi_stream_private_t *p_stream,
                                    mi_event_mgr_t * p_manager );

/**
Add main header marker information
@param cstr_index    Codestream information structure
@param type         marker type
@param pos          byte offset of marker segment
@param len          length of marker segment
 */
static mi_BOOL mi_j2k_add_mhmarker(mi_codestream_index_t *cstr_index, mi_UINT32 type, mi_OFF_T pos, mi_UINT32 len) ;
/**
Add tile header marker information
@param tileno       tile index number
@param cstr_index   Codestream information structure
@param type         marker type
@param pos          byte offset of marker segment
@param len          length of marker segment
 */
static mi_BOOL mi_j2k_add_tlmarker(mi_UINT32 tileno, mi_codestream_index_t *cstr_index, mi_UINT32 type, mi_OFF_T pos, mi_UINT32 len);

/**
 * Reads an unknown marker
 *
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_stream                the stream object to read from.
 * @param       output_marker           FIXME DOC
 * @param       p_manager               the user event manager.
 *
 * @return      true                    if the marker could be deduced.
*/
static mi_BOOL mi_j2k_read_unk( mi_j2k_t *p_j2k,
                                  mi_stream_private_t *p_stream,
                                  mi_UINT32 *output_marker,
                                  mi_event_mgr_t * p_manager );

/**
 * Writes the MCT marker (Multiple Component Transform)
 *
 * @param       p_j2k           J2K codec.
 * @param       p_mct_record    FIXME DOC
 * @param       p_stream        the stream to write data to.
 * @param       p_manager       the user event manager.
*/
static mi_BOOL mi_j2k_write_mct_record(       mi_j2k_t *p_j2k,
                                                                                    mi_mct_data_t * p_mct_record,
                                            mi_stream_private_t *p_stream,
                                            mi_event_mgr_t * p_manager );

/**
 * Reads a MCT marker (Multiple Component Transform)
 *
 * @param       p_header_data   the data contained in the MCT box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the MCT marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_mct (      mi_j2k_t *p_j2k,
                                                                    mi_BYTE * p_header_data,
                                                                    mi_UINT32 p_header_size,
                                                                    mi_event_mgr_t * p_manager );

/**
 * Writes the MCC marker (Multiple Component Collection)
 *
 * @param       p_j2k                   J2K codec.
 * @param       p_mcc_record            FIXME DOC
 * @param       p_stream                the stream to write data to.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_write_mcc_record(   mi_j2k_t *p_j2k,
                                            mi_simple_mcc_decorrelation_data_t * p_mcc_record,
                                            mi_stream_private_t *p_stream,
                                            mi_event_mgr_t * p_manager );

/**
 * Reads a MCC marker (Multiple Component Collection)
 *
 * @param       p_header_data   the data contained in the MCC box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the MCC marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_mcc (      mi_j2k_t *p_j2k,
                                                                    mi_BYTE * p_header_data,
                                                                    mi_UINT32 p_header_size,
                                                                    mi_event_mgr_t * p_manager );

/**
 * Writes the MCO marker (Multiple component transformation ordering)
 *
 * @param       p_stream                                the stream to write data to.
 * @param       p_j2k                           J2K codec.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_write_mco(      mi_j2k_t *p_j2k,
                                    mi_stream_private_t *p_stream,
                                    mi_event_mgr_t * p_manager );

/**
 * Reads a MCO marker (Multiple Component Transform Ordering)
 *
 * @param       p_header_data   the data contained in the MCO box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the MCO marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_mco (      mi_j2k_t *p_j2k,
                                                                    mi_BYTE * p_header_data,
                                                                    mi_UINT32 p_header_size,
                                                                    mi_event_mgr_t * p_manager );

static mi_BOOL mi_j2k_add_mct(mi_tcp_t * p_tcp, mi_image_t * p_image, mi_UINT32 p_index);

static void  mi_j2k_read_int16_to_float (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem);
static void  mi_j2k_read_int32_to_float (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem);
static void  mi_j2k_read_float32_to_float (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem);
static void  mi_j2k_read_float64_to_float (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem);

static void  mi_j2k_read_int16_to_int32 (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem);
static void  mi_j2k_read_int32_to_int32 (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem);
static void  mi_j2k_read_float32_to_int32 (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem);
static void  mi_j2k_read_float64_to_int32 (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem);

static void  mi_j2k_write_float_to_int16 (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem);
static void  mi_j2k_write_float_to_int32 (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem);
static void  mi_j2k_write_float_to_float (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem);
static void  mi_j2k_write_float_to_float64 (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem);

/**
 * Ends the encoding, i.e. frees memory.
 *
 * @param       p_stream                the stream to write data to.
 * @param       p_j2k                   J2K codec.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_end_encoding(   mi_j2k_t *p_j2k,
                                                                            mi_stream_private_t *p_stream,
                                                                            mi_event_mgr_t * p_manager );

/**
 * Writes the CBD marker (Component bit depth definition)
 *
 * @param       p_stream                                the stream to write data to.
 * @param       p_j2k                           J2K codec.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_write_cbd(      mi_j2k_t *p_j2k,
                                                                    mi_stream_private_t *p_stream,
                                                                        mi_event_mgr_t * p_manager );

/**
 * Reads a CBD marker (Component bit depth definition)
 * @param       p_header_data   the data contained in the CBD box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the CBD marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_cbd (      mi_j2k_t *p_j2k,
                                                                mi_BYTE * p_header_data,
                                                                mi_UINT32 p_header_size,
                                                                mi_event_mgr_t * p_manager);


/**
 * Writes COC marker for each component.
 *
 * @param       p_stream                the stream to write data to.
 * @param       p_j2k                   J2K codec.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_write_all_coc( mi_j2k_t *p_j2k,
                                                                        mi_stream_private_t *p_stream,
                                                                        mi_event_mgr_t * p_manager );

/**
 * Writes QCC marker for each component.
 *
 * @param       p_stream                the stream to write data to.
 * @param       p_j2k                   J2K codec.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_write_all_qcc( mi_j2k_t *p_j2k,
                                                                        mi_stream_private_t *p_stream,
                                                                        mi_event_mgr_t * p_manager );

/**
 * Writes regions of interests.
 *
 * @param       p_stream                the stream to write data to.
 * @param       p_j2k                   J2K codec.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_write_regions(  mi_j2k_t *p_j2k,
                                                                        mi_stream_private_t *p_stream,
                                                                        mi_event_mgr_t * p_manager );

/**
 * Writes EPC ????
 *
 * @param       p_stream                the stream to write data to.
 * @param       p_j2k                   J2K codec.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_write_epc(      mi_j2k_t *p_j2k,
                                                                    mi_stream_private_t *p_stream,
                                                                    mi_event_mgr_t * p_manager );

/**
 * Checks the progression order changes values. Tells of the poc given as input are valid.
 * A nice message is outputted at errors.
 *
 * @param       p_pocs                  the progression order changes.
 * @param       p_nb_pocs               the number of progression order changes.
 * @param       p_nb_resolutions        the number of resolutions.
 * @param       numcomps                the number of components
 * @param       numlayers               the number of layers.
 * @param       p_manager               the user event manager.
 *
 * @return      true if the pocs are valid.
 */
static mi_BOOL mi_j2k_check_poc_val(  const mi_poc_t *p_pocs,
                                                                            mi_UINT32 p_nb_pocs,
                                                                            mi_UINT32 p_nb_resolutions,
                                                                            mi_UINT32 numcomps,
                                                                            mi_UINT32 numlayers,
                                                                            mi_event_mgr_t * p_manager);

/**
 * Gets the number of tile parts used for the given change of progression (if any) and the given tile.
 *
 * @param               cp                      the coding parameters.
 * @param               pino            the offset of the given poc (i.e. its position in the coding parameter).
 * @param               tileno          the given tile.
 *
 * @return              the number of tile parts.
 */
static mi_UINT32 mi_j2k_get_num_tp( mi_cp_t *cp, mi_UINT32 pino, mi_UINT32 tileno);

/**
 * Calculates the total number of tile parts needed by the encoder to
 * encode such an image. If not enough memory is available, then the function return false.
 *
 * @param       p_nb_tiles      pointer that will hold the number of tile parts.
 * @param       cp                      the coding parameters for the image.
 * @param       image           the image to encode.
 * @param       p_j2k                   the p_j2k encoder.
 * @param       p_manager       the user event manager.
 *
 * @return true if the function was successful, false else.
 */
static mi_BOOL mi_j2k_calculate_tp(   mi_j2k_t *p_j2k,
                                                                            mi_cp_t *cp,
                                                                            mi_UINT32 * p_nb_tiles,
                                                                            mi_image_t *image,
                                                                            mi_event_mgr_t * p_manager);

static void mi_j2k_dump_MH_info(mi_j2k_t* p_j2k, FILE* out_stream);

static void mi_j2k_dump_MH_index(mi_j2k_t* p_j2k, FILE* out_stream);

static mi_codestream_index_t* mi_j2k_create_cstr_index(void);

static mi_FLOAT32 mi_j2k_get_tp_stride (mi_tcp_t * p_tcp);

static mi_FLOAT32 mi_j2k_get_default_stride (mi_tcp_t * p_tcp);

static int mi_j2k_initialise_4K_poc(mi_poc_t *POC, int numres);

static void mi_j2k_set_cinema_parameters(mi_cparameters_t *parameters, mi_image_t *image, mi_event_mgr_t *p_manager);

static mi_BOOL mi_j2k_is_cinema_compliant(mi_image_t *image, mi_UINT16 rsiz, mi_event_mgr_t *p_manager);

/**
 * Checks for invalid number of tile-parts in SOT marker (TPsot==TNsot). See issue 254.
 *
 * @param       p_stream            the stream to read data from.
 * @param       tile_no             tile number we're looking for.
 * @param       p_correction_needed output value. if true, non conformant codestream needs TNsot correction.
 * @param       p_manager       the user event manager.
 *
 * @return true if the function was successful, false else.
 */
static mi_BOOL mi_j2k_need_nb_tile_parts_correction(mi_stream_private_t *p_stream, mi_UINT32 tile_no, mi_BOOL* p_correction_needed, mi_event_mgr_t * p_manager );

/*@}*/

/*@}*/

/* ----------------------------------------------------------------------- */
typedef struct j2k_prog_order{
        mi_PROG_ORDER enum_prog;
        char str_prog[5];
}j2k_prog_order_t;

static j2k_prog_order_t j2k_prog_order_list[] = {
        {mi_CPRL, "CPRL"},
        {mi_LRCP, "LRCP"},
        {mi_PCRL, "PCRL"},
        {mi_RLCP, "RLCP"},
        {mi_RPCL, "RPCL"},
        {(mi_PROG_ORDER)-1, ""}
};

/**
 * FIXME DOC
 */
static const mi_UINT32 MCT_ELEMENT_SIZE [] =
{
        2,
        4,
        4,
        8
};

typedef void (* mi_j2k_mct_function) (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem);

static const mi_j2k_mct_function j2k_mct_read_functions_to_float [] =
{
        mi_j2k_read_int16_to_float,
        mi_j2k_read_int32_to_float,
        mi_j2k_read_float32_to_float,
        mi_j2k_read_float64_to_float
};

static const mi_j2k_mct_function j2k_mct_read_functions_to_int32 [] =
{
        mi_j2k_read_int16_to_int32,
        mi_j2k_read_int32_to_int32,
        mi_j2k_read_float32_to_int32,
        mi_j2k_read_float64_to_int32
};

static const mi_j2k_mct_function j2k_mct_write_functions_from_float [] =
{
        mi_j2k_write_float_to_int16,
        mi_j2k_write_float_to_int32,
        mi_j2k_write_float_to_float,
        mi_j2k_write_float_to_float64
};

typedef struct mi_dec_memory_marker_handler
{
        /** marker value */
        mi_UINT32 id;
        /** value of the state when the marker can appear */
        mi_UINT32 states;
        /** action linked to the marker */
        mi_BOOL (*handler) (   mi_j2k_t *p_j2k,
                            mi_BYTE * p_header_data,
                            mi_UINT32 p_header_size,
                            mi_event_mgr_t * p_manager );
}
mi_dec_memory_marker_handler_t;

static const mi_dec_memory_marker_handler_t j2k_memory_marker_handler_tab [] =
{
  {J2K_MS_SOT, J2K_STATE_MH | J2K_STATE_TPHSOT, mi_j2k_read_sot},
  {J2K_MS_COD, J2K_STATE_MH | J2K_STATE_TPH, mi_j2k_read_cod},
  {J2K_MS_COC, J2K_STATE_MH | J2K_STATE_TPH, mi_j2k_read_coc},
  {J2K_MS_RGN, J2K_STATE_MH | J2K_STATE_TPH, mi_j2k_read_rgn},
  {J2K_MS_QCD, J2K_STATE_MH | J2K_STATE_TPH, mi_j2k_read_qcd},
  {J2K_MS_QCC, J2K_STATE_MH | J2K_STATE_TPH, mi_j2k_read_qcc},
  {J2K_MS_POC, J2K_STATE_MH | J2K_STATE_TPH, mi_j2k_read_poc},
  {J2K_MS_SIZ, J2K_STATE_MHSIZ, mi_j2k_read_siz},
  {J2K_MS_TLM, J2K_STATE_MH, mi_j2k_read_tlm},
  {J2K_MS_PLM, J2K_STATE_MH, mi_j2k_read_plm},
  {J2K_MS_PLT, J2K_STATE_TPH, mi_j2k_read_plt},
  {J2K_MS_PPM, J2K_STATE_MH, mi_j2k_read_ppm},
  {J2K_MS_PPT, J2K_STATE_TPH, mi_j2k_read_ppt},
  {J2K_MS_SOP, 0, 0},
  {J2K_MS_CRG, J2K_STATE_MH, mi_j2k_read_crg},
  {J2K_MS_COM, J2K_STATE_MH | J2K_STATE_TPH, mi_j2k_read_com},
  {J2K_MS_MCT, J2K_STATE_MH | J2K_STATE_TPH, mi_j2k_read_mct},
  {J2K_MS_CBD, J2K_STATE_MH , mi_j2k_read_cbd},
  {J2K_MS_MCC, J2K_STATE_MH | J2K_STATE_TPH, mi_j2k_read_mcc},
  {J2K_MS_MCO, J2K_STATE_MH | J2K_STATE_TPH, mi_j2k_read_mco},
  {J2K_MS_UNK, J2K_STATE_MH | J2K_STATE_TPH, 0}/*mi_j2k_read_unk is directly used*/
};

static void  mi_j2k_read_int16_to_float (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem)
{
        mi_BYTE * l_src_data = (mi_BYTE *) p_src_data;
        mi_FLOAT32 * l_dest_data = (mi_FLOAT32 *) p_dest_data;
        mi_UINT32 i;
        mi_UINT32 l_temp;

        for (i=0;i<p_nb_elem;++i) {
                mi_read_bytes(l_src_data,&l_temp,2);

                l_src_data+=sizeof(mi_INT16);

                *(l_dest_data++) = (mi_FLOAT32) l_temp;
        }
}

static void  mi_j2k_read_int32_to_float (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem)
{
        mi_BYTE * l_src_data = (mi_BYTE *) p_src_data;
        mi_FLOAT32 * l_dest_data = (mi_FLOAT32 *) p_dest_data;
        mi_UINT32 i;
        mi_UINT32 l_temp;

        for (i=0;i<p_nb_elem;++i) {
                mi_read_bytes(l_src_data,&l_temp,4);

                l_src_data+=sizeof(mi_INT32);

                *(l_dest_data++) = (mi_FLOAT32) l_temp;
        }
}

static void  mi_j2k_read_float32_to_float (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem)
{
        mi_BYTE * l_src_data = (mi_BYTE *) p_src_data;
        mi_FLOAT32 * l_dest_data = (mi_FLOAT32 *) p_dest_data;
        mi_UINT32 i;
        mi_FLOAT32 l_temp;

        for (i=0;i<p_nb_elem;++i) {
                mi_read_float(l_src_data,&l_temp);

                l_src_data+=sizeof(mi_FLOAT32);

                *(l_dest_data++) = l_temp;
        }
}

static void  mi_j2k_read_float64_to_float (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem)
{
        mi_BYTE * l_src_data = (mi_BYTE *) p_src_data;
        mi_FLOAT32 * l_dest_data = (mi_FLOAT32 *) p_dest_data;
        mi_UINT32 i;
        mi_FLOAT64 l_temp;

        for (i=0;i<p_nb_elem;++i) {
                mi_read_double(l_src_data,&l_temp);

                l_src_data+=sizeof(mi_FLOAT64);

                *(l_dest_data++) = (mi_FLOAT32) l_temp;
        }
}

static void  mi_j2k_read_int16_to_int32 (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem)
{
        mi_BYTE * l_src_data = (mi_BYTE *) p_src_data;
        mi_INT32 * l_dest_data = (mi_INT32 *) p_dest_data;
        mi_UINT32 i;
        mi_UINT32 l_temp;

        for (i=0;i<p_nb_elem;++i) {
                mi_read_bytes(l_src_data,&l_temp,2);

                l_src_data+=sizeof(mi_INT16);

                *(l_dest_data++) = (mi_INT32) l_temp;
        }
}

static void  mi_j2k_read_int32_to_int32 (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem)
{
        mi_BYTE * l_src_data = (mi_BYTE *) p_src_data;
        mi_INT32 * l_dest_data = (mi_INT32 *) p_dest_data;
        mi_UINT32 i;
        mi_UINT32 l_temp;

        for (i=0;i<p_nb_elem;++i) {
                mi_read_bytes(l_src_data,&l_temp,4);

                l_src_data+=sizeof(mi_INT32);

                *(l_dest_data++) = (mi_INT32) l_temp;
        }
}

static void  mi_j2k_read_float32_to_int32 (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem)
{
        mi_BYTE * l_src_data = (mi_BYTE *) p_src_data;
        mi_INT32 * l_dest_data = (mi_INT32 *) p_dest_data;
        mi_UINT32 i;
        mi_FLOAT32 l_temp;

        for (i=0;i<p_nb_elem;++i) {
                mi_read_float(l_src_data,&l_temp);

                l_src_data+=sizeof(mi_FLOAT32);

                *(l_dest_data++) = (mi_INT32) l_temp;
        }
}

static void  mi_j2k_read_float64_to_int32 (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem)
{
        mi_BYTE * l_src_data = (mi_BYTE *) p_src_data;
        mi_INT32 * l_dest_data = (mi_INT32 *) p_dest_data;
        mi_UINT32 i;
        mi_FLOAT64 l_temp;

        for (i=0;i<p_nb_elem;++i) {
                mi_read_double(l_src_data,&l_temp);

                l_src_data+=sizeof(mi_FLOAT64);

                *(l_dest_data++) = (mi_INT32) l_temp;
        }
}

static void  mi_j2k_write_float_to_int16 (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem)
{
        mi_BYTE * l_dest_data = (mi_BYTE *) p_dest_data;
        mi_FLOAT32 * l_src_data = (mi_FLOAT32 *) p_src_data;
        mi_UINT32 i;
        mi_UINT32 l_temp;

        for (i=0;i<p_nb_elem;++i) {
                l_temp = (mi_UINT32) *(l_src_data++);

                mi_write_bytes(l_dest_data,l_temp,sizeof(mi_INT16));

                l_dest_data+=sizeof(mi_INT16);
        }
}

static void mi_j2k_write_float_to_int32 (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem)
{
        mi_BYTE * l_dest_data = (mi_BYTE *) p_dest_data;
        mi_FLOAT32 * l_src_data = (mi_FLOAT32 *) p_src_data;
        mi_UINT32 i;
        mi_UINT32 l_temp;

        for (i=0;i<p_nb_elem;++i) {
                l_temp = (mi_UINT32) *(l_src_data++);

                mi_write_bytes(l_dest_data,l_temp,sizeof(mi_INT32));

                l_dest_data+=sizeof(mi_INT32);
        }
}

static void  mi_j2k_write_float_to_float (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem)
{
        mi_BYTE * l_dest_data = (mi_BYTE *) p_dest_data;
        mi_FLOAT32 * l_src_data = (mi_FLOAT32 *) p_src_data;
        mi_UINT32 i;
        mi_FLOAT32 l_temp;

        for (i=0;i<p_nb_elem;++i) {
                l_temp = (mi_FLOAT32) *(l_src_data++);

                mi_write_float(l_dest_data,l_temp);

                l_dest_data+=sizeof(mi_FLOAT32);
        }
}

static void  mi_j2k_write_float_to_float64 (const void * p_src_data, void * p_dest_data, mi_UINT32 p_nb_elem)
{
        mi_BYTE * l_dest_data = (mi_BYTE *) p_dest_data;
        mi_FLOAT32 * l_src_data = (mi_FLOAT32 *) p_src_data;
        mi_UINT32 i;
        mi_FLOAT64 l_temp;

        for (i=0;i<p_nb_elem;++i) {
                l_temp = (mi_FLOAT64) *(l_src_data++);

                mi_write_double(l_dest_data,l_temp);

                l_dest_data+=sizeof(mi_FLOAT64);
        }
}

char *mi_j2k_convert_progression_order(mi_PROG_ORDER prg_order){
        j2k_prog_order_t *po;
        for(po = j2k_prog_order_list; po->enum_prog != -1; po++ ){
                if(po->enum_prog == prg_order){
                        return po->str_prog;
                }
        }
        return po->str_prog;
}

static mi_BOOL mi_j2k_check_poc_val( const mi_poc_t *p_pocs,
                                                        mi_UINT32 p_nb_pocs,
                                                        mi_UINT32 p_nb_resolutions,
                                                        mi_UINT32 p_num_comps,
                                                        mi_UINT32 p_num_layers,
                                                        mi_event_mgr_t * p_manager)
{
        mi_UINT32* packet_array;
        mi_UINT32 index , resno, compno, layno;
        mi_UINT32 i;
        mi_UINT32 step_c = 1;
        mi_UINT32 step_r = p_num_comps * step_c;
        mi_UINT32 step_l = p_nb_resolutions * step_r;
        mi_BOOL loss = mi_FALSE;
        mi_UINT32 layno0 = 0;

        packet_array = (mi_UINT32*) mi_calloc(step_l * p_num_layers, sizeof(mi_UINT32));
        if (packet_array == 00) {
                mi_event_msg(p_manager , EVT_ERROR, "Not enough memory for checking the poc values.\n");
                return mi_FALSE;
        }

        if (p_nb_pocs == 0) {
        mi_free(packet_array);
                return mi_TRUE;
        }

        index = step_r * p_pocs->resno0;
        /* take each resolution for each poc */
        for (resno = p_pocs->resno0 ; resno < p_pocs->resno1 ; ++resno)
        {
                mi_UINT32 res_index = index + p_pocs->compno0 * step_c;

                /* take each comp of each resolution for each poc */
                for (compno = p_pocs->compno0 ; compno < p_pocs->compno1 ; ++compno) {
                        mi_UINT32 comp_index = res_index + layno0 * step_l;

                        /* and finally take each layer of each res of ... */
                        for (layno = layno0; layno < p_pocs->layno1 ; ++layno) {
                                /*index = step_r * resno + step_c * compno + step_l * layno;*/
                                packet_array[comp_index] = 1;
                                comp_index += step_l;
                        }

                        res_index += step_c;
                }

                index += step_r;
        }
        ++p_pocs;

        /* iterate through all the pocs */
        for (i = 1; i < p_nb_pocs ; ++i) {
                mi_UINT32 l_last_layno1 = (p_pocs-1)->layno1 ;

                layno0 = (p_pocs->layno1 > l_last_layno1)? l_last_layno1 : 0;
                index = step_r * p_pocs->resno0;

                /* take each resolution for each poc */
                for (resno = p_pocs->resno0 ; resno < p_pocs->resno1 ; ++resno) {
                        mi_UINT32 res_index = index + p_pocs->compno0 * step_c;

                        /* take each comp of each resolution for each poc */
                        for (compno = p_pocs->compno0 ; compno < p_pocs->compno1 ; ++compno) {
                                mi_UINT32 comp_index = res_index + layno0 * step_l;

                                /* and finally take each layer of each res of ... */
                                for (layno = layno0; layno < p_pocs->layno1 ; ++layno) {
                                        /*index = step_r * resno + step_c * compno + step_l * layno;*/
                                        packet_array[comp_index] = 1;
                                        comp_index += step_l;
                                }

                                res_index += step_c;
                        }

                        index += step_r;
                }

                ++p_pocs;
        }

        index = 0;
        for (layno = 0; layno < p_num_layers ; ++layno) {
                for (resno = 0; resno < p_nb_resolutions; ++resno) {
                        for (compno = 0; compno < p_num_comps; ++compno) {
                                loss |= (packet_array[index]!=1);
                                /*index = step_r * resno + step_c * compno + step_l * layno;*/
                                index += step_c;
                        }
                }
        }

        if (loss) {
                mi_event_msg(p_manager , EVT_ERROR, "Missing packets possible loss of data\n");
        }

        mi_free(packet_array);

        return !loss;
}

/* ----------------------------------------------------------------------- */

static mi_UINT32 mi_j2k_get_num_tp(mi_cp_t *cp, mi_UINT32 pino, mi_UINT32 tileno)
{
        const mi_CHAR *prog = 00;
        mi_INT32 i;
        mi_UINT32 tpnum = 1;
        mi_tcp_t *tcp = 00;
        mi_poc_t * l_current_poc = 00;

        /*  preconditions */
        assert(tileno < (cp->tw * cp->th));
        assert(pino < (cp->tcps[tileno].numpocs + 1));

        /* get the given tile coding parameter */
        tcp = &cp->tcps[tileno];
        assert(tcp != 00);

        l_current_poc = &(tcp->pocs[pino]);
        assert(l_current_poc != 0);

        /* get the progression order as a character string */
        prog = mi_j2k_convert_progression_order(tcp->prg);
        assert(strlen(prog) > 0);

        if (cp->m_specific_param.m_enc.m_tp_on == 1) {
                for (i=0;i<4;++i) {
                        switch (prog[i])
                        {
                                /* component wise */
                                case 'C':
                                        tpnum *= l_current_poc->compE;
                                        break;
                                /* resolution wise */
                                case 'R':
                                        tpnum *= l_current_poc->resE;
                                        break;
                                /* precinct wise */
                                case 'P':
                                        tpnum *= l_current_poc->prcE;
                                        break;
                                /* layer wise */
                                case 'L':
                                        tpnum *= l_current_poc->layE;
                                        break;
                        }
                        /* whould we split here ? */
                        if ( cp->m_specific_param.m_enc.m_tp_flag == prog[i] ) {
                                cp->m_specific_param.m_enc.m_tp_pos=i;
                                break;
                        }
                }
        }
        else {
                tpnum=1;
        }

        return tpnum;
}

static mi_BOOL mi_j2k_calculate_tp(  mi_j2k_t *p_j2k,
                                                        mi_cp_t *cp,
                                                        mi_UINT32 * p_nb_tiles,
                                                        mi_image_t *image,
                                                        mi_event_mgr_t * p_manager
                                )
{
        mi_UINT32 pino,tileno;
        mi_UINT32 l_nb_tiles;
        mi_tcp_t *tcp;

        /* preconditions */
        assert(p_nb_tiles != 00);
        assert(cp != 00);
        assert(image != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        l_nb_tiles = cp->tw * cp->th;
        * p_nb_tiles = 0;
        tcp = cp->tcps;

        /* INDEX >> */
        /* TODO mergeV2: check this part which use cstr_info */
        /*if (p_j2k->cstr_info) {
                mi_tile_info_t * l_info_tile_ptr = p_j2k->cstr_info->tile;

                for (tileno = 0; tileno < l_nb_tiles; ++tileno) {
                        mi_UINT32 cur_totnum_tp = 0;

                        mi_pi_update_encoding_parameters(image,cp,tileno);

                        for (pino = 0; pino <= tcp->numpocs; ++pino)
                        {
                                mi_UINT32 tp_num = mi_j2k_get_num_tp(cp,pino,tileno);

                                *p_nb_tiles = *p_nb_tiles + tp_num;

                                cur_totnum_tp += tp_num;
                        }

                        tcp->m_nb_tile_parts = cur_totnum_tp;

                        l_info_tile_ptr->tp = (mi_tp_info_t *) mi_malloc(cur_totnum_tp * sizeof(mi_tp_info_t));
                        if (l_info_tile_ptr->tp == 00) {
                                return mi_FALSE;
                        }

                        memset(l_info_tile_ptr->tp,0,cur_totnum_tp * sizeof(mi_tp_info_t));

                        l_info_tile_ptr->num_tps = cur_totnum_tp;

                        ++l_info_tile_ptr;
                        ++tcp;
                }
        }
        else */{
                for (tileno = 0; tileno < l_nb_tiles; ++tileno) {
                        mi_UINT32 cur_totnum_tp = 0;

                        mi_pi_update_encoding_parameters(image,cp,tileno);

                        for (pino = 0; pino <= tcp->numpocs; ++pino) {
                                mi_UINT32 tp_num = mi_j2k_get_num_tp(cp,pino,tileno);

                                *p_nb_tiles = *p_nb_tiles + tp_num;

                                cur_totnum_tp += tp_num;
                        }
                        tcp->m_nb_tile_parts = cur_totnum_tp;

                        ++tcp;
                }
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_soc(     mi_j2k_t *p_j2k,
                                                mi_stream_private_t *p_stream,
                                                    mi_event_mgr_t * p_manager )
{
        /* 2 bytes will be written */
        mi_BYTE * l_start_stream = 00;

        /* preconditions */
        assert(p_stream != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        l_start_stream = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

        /* write SOC identifier */
        mi_write_bytes(l_start_stream,J2K_MS_SOC,2);

        if (mi_stream_write_data(p_stream,l_start_stream,2,p_manager) != 2) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

/**
 * Reads a SOC marker (Start of Codestream)
 * @param       p_j2k           the jpeg2000 file codec.
 * @param       p_stream        FIXME DOC
 * @param       p_manager       the user event manager.
*/
static mi_BOOL mi_j2k_read_soc(   mi_j2k_t *p_j2k,
                                    mi_stream_private_t *p_stream,
                                    mi_event_mgr_t * p_manager
                                    )
{
        mi_BYTE l_data [2];
        mi_UINT32 l_marker;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        if (mi_stream_read_data(p_stream,l_data,2,p_manager) != 2) {
                return mi_FALSE;
        }

        mi_read_bytes(l_data,&l_marker,2);
        if (l_marker != J2K_MS_SOC) {
                return mi_FALSE;
        }

        /* Next marker should be a SIZ marker in the main header */
        p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_MHSIZ;

        /* FIXME move it in a index structure included in p_j2k*/
        p_j2k->cstr_index->main_head_start = mi_stream_tell(p_stream) - 2;

        mi_event_msg(p_manager, EVT_INFO, "Start to read j2k main header (%d).\n", p_j2k->cstr_index->main_head_start);

        /* Add the marker to the codestream index*/
        if (mi_FALSE == mi_j2k_add_mhmarker(p_j2k->cstr_index, J2K_MS_SOC, p_j2k->cstr_index->main_head_start, 2)) {
                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to add mh marker\n");
                return mi_FALSE;
        }
        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_siz(     mi_j2k_t *p_j2k,
                                                        mi_stream_private_t *p_stream,
                                                        mi_event_mgr_t * p_manager )
{
        mi_UINT32 i;
        mi_UINT32 l_size_len;
        mi_BYTE * l_current_ptr;
        mi_image_t * l_image = 00;
        mi_cp_t *cp = 00;
        mi_image_comp_t * l_img_comp = 00;

        /* preconditions */
        assert(p_stream != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        l_image = p_j2k->m_private_image;
        cp = &(p_j2k->m_cp);
        l_size_len = 40 + 3 * l_image->numcomps;
        l_img_comp = l_image->comps;

        if (l_size_len > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {

                mi_BYTE *new_header_tile_data = (mi_BYTE *) mi_realloc(p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_size_len);
                if (! new_header_tile_data) {
                        mi_free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory for the SIZ marker\n");
                        return mi_FALSE;
                }
                p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
                p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_size_len;
        }

        l_current_ptr = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

        /* write SOC identifier */
        mi_write_bytes(l_current_ptr,J2K_MS_SIZ,2);    /* SIZ */
        l_current_ptr+=2;

        mi_write_bytes(l_current_ptr,l_size_len-2,2); /* L_SIZ */
        l_current_ptr+=2;

        mi_write_bytes(l_current_ptr, cp->rsiz, 2);    /* Rsiz (capabilities) */
        l_current_ptr+=2;

        mi_write_bytes(l_current_ptr, l_image->x1, 4); /* Xsiz */
        l_current_ptr+=4;

        mi_write_bytes(l_current_ptr, l_image->y1, 4); /* Ysiz */
        l_current_ptr+=4;

        mi_write_bytes(l_current_ptr, l_image->x0, 4); /* X0siz */
        l_current_ptr+=4;

        mi_write_bytes(l_current_ptr, l_image->y0, 4); /* Y0siz */
        l_current_ptr+=4;

        mi_write_bytes(l_current_ptr, cp->tdx, 4);             /* XTsiz */
        l_current_ptr+=4;

        mi_write_bytes(l_current_ptr, cp->tdy, 4);             /* YTsiz */
        l_current_ptr+=4;

        mi_write_bytes(l_current_ptr, cp->tx0, 4);             /* XT0siz */
        l_current_ptr+=4;

        mi_write_bytes(l_current_ptr, cp->ty0, 4);             /* YT0siz */
        l_current_ptr+=4;

        mi_write_bytes(l_current_ptr, l_image->numcomps, 2);   /* Csiz */
        l_current_ptr+=2;

        for (i = 0; i < l_image->numcomps; ++i) {
                /* TODO here with MCT ? */
                mi_write_bytes(l_current_ptr, l_img_comp->prec - 1 + (l_img_comp->sgnd << 7), 1);      /* Ssiz_i */
                ++l_current_ptr;

                mi_write_bytes(l_current_ptr, l_img_comp->dx, 1);      /* XRsiz_i */
                ++l_current_ptr;

                mi_write_bytes(l_current_ptr, l_img_comp->dy, 1);      /* YRsiz_i */
                ++l_current_ptr;

                ++l_img_comp;
        }

        if (mi_stream_write_data(p_stream,p_j2k->m_specific_param.m_encoder.m_header_tile_data,l_size_len,p_manager) != l_size_len) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

/**
 * Reads a SIZ marker (image and tile size)
 * @param       p_j2k           the jpeg2000 file codec.
 * @param       p_header_data   the data contained in the SIZ box.
 * @param       p_header_size   the size of the data contained in the SIZ marker.
 * @param       p_manager       the user event manager.
*/
static mi_BOOL mi_j2k_read_siz(mi_j2k_t *p_j2k,
                                 mi_BYTE * p_header_data,
                                 mi_UINT32 p_header_size,
                                 mi_event_mgr_t * p_manager
                                 )
{
        mi_UINT32 i;
        mi_UINT32 l_nb_comp;
        mi_UINT32 l_nb_comp_remain;
        mi_UINT32 l_remaining_size;
        mi_UINT32 l_nb_tiles;
        mi_UINT32 l_tmp, l_tx1, l_ty1;
        mi_image_t *l_image = 00;
        mi_cp_t *l_cp = 00;
        mi_image_comp_t * l_img_comp = 00;
        mi_tcp_t * l_current_tile_param = 00;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_header_data != 00);

        l_image = p_j2k->m_private_image;
        l_cp = &(p_j2k->m_cp);

        /* minimum size == 39 - 3 (= minimum component parameter) */
        if (p_header_size < 36) {
                mi_event_msg(p_manager, EVT_ERROR, "Error with SIZ marker size\n");
                return mi_FALSE;
        }

        l_remaining_size = p_header_size - 36;
        l_nb_comp = l_remaining_size / 3;
        l_nb_comp_remain = l_remaining_size % 3;
        if (l_nb_comp_remain != 0){
                mi_event_msg(p_manager, EVT_ERROR, "Error with SIZ marker size\n");
                return mi_FALSE;
        }

        mi_read_bytes(p_header_data,&l_tmp ,2);                                                /* Rsiz (capabilities) */
        p_header_data+=2;
        l_cp->rsiz = (mi_UINT16) l_tmp;
        mi_read_bytes(p_header_data, (mi_UINT32*) &l_image->x1, 4);   /* Xsiz */
        p_header_data+=4;
        mi_read_bytes(p_header_data, (mi_UINT32*) &l_image->y1, 4);   /* Ysiz */
        p_header_data+=4;
        mi_read_bytes(p_header_data, (mi_UINT32*) &l_image->x0, 4);   /* X0siz */
        p_header_data+=4;
        mi_read_bytes(p_header_data, (mi_UINT32*) &l_image->y0, 4);   /* Y0siz */
        p_header_data+=4;
        mi_read_bytes(p_header_data, (mi_UINT32*) &l_cp->tdx, 4);             /* XTsiz */
        p_header_data+=4;
        mi_read_bytes(p_header_data, (mi_UINT32*) &l_cp->tdy, 4);             /* YTsiz */
        p_header_data+=4;
        mi_read_bytes(p_header_data, (mi_UINT32*) &l_cp->tx0, 4);             /* XT0siz */
        p_header_data+=4;
        mi_read_bytes(p_header_data, (mi_UINT32*) &l_cp->ty0, 4);             /* YT0siz */
        p_header_data+=4;
        mi_read_bytes(p_header_data, (mi_UINT32*) &l_tmp, 2);                 /* Csiz */
        p_header_data+=2;
        if (l_tmp < 16385)
                l_image->numcomps = (mi_UINT16) l_tmp;
        else {
                mi_event_msg(p_manager, EVT_ERROR, "Error with SIZ marker: number of component is illegal -> %d\n", l_tmp);
                return mi_FALSE;
        }

        if (l_image->numcomps != l_nb_comp) {
                mi_event_msg(p_manager, EVT_ERROR, "Error with SIZ marker: number of component is not compatible with the remaining number of parameters ( %d vs %d)\n", l_image->numcomps, l_nb_comp);
                return mi_FALSE;
        }

        /* testcase 4035.pdf.SIGSEGV.d8b.3375 */
        /* testcase issue427-null-image-size.jp2 */
        if ((l_image->x0 >= l_image->x1) || (l_image->y0 >= l_image->y1)) {
                mi_event_msg(p_manager, EVT_ERROR, "Error with SIZ marker: negative or zero image size (%" PRId64 " x %" PRId64 ")\n", (mi_INT64)l_image->x1 - l_image->x0, (mi_INT64)l_image->y1 - l_image->y0);
                return mi_FALSE;
        }
        /* testcase 2539.pdf.SIGFPE.706.1712 (also 3622.pdf.SIGFPE.706.2916 and 4008.pdf.SIGFPE.706.3345 and maybe more) */
        if ((l_cp->tdx == 0U) || (l_cp->tdy == 0U)) {
                mi_event_msg(p_manager, EVT_ERROR, "Error with SIZ marker: invalid tile size (tdx: %d, tdy: %d)\n", l_cp->tdx, l_cp->tdy);
                return mi_FALSE;
        }

        /* testcase 1610.pdf.SIGSEGV.59c.681 */
        if ((0xFFFFFFFFU / l_image->x1) < l_image->y1) {
                mi_event_msg(p_manager, EVT_ERROR, "Prevent buffer overflow (x1: %d, y1: %d)\n", l_image->x1, l_image->y1);
                return mi_FALSE;
        }

        /* testcase issue427-illegal-tile-offset.jp2 */
        l_tx1 = mi_uint_adds(l_cp->tx0, l_cp->tdx); /* manage overflow */
        l_ty1 = mi_uint_adds(l_cp->ty0, l_cp->tdy); /* manage overflow */
        if ((l_cp->tx0 > l_image->x0) || (l_cp->ty0 > l_image->y0) || (l_tx1 <= l_image->x0) || (l_ty1 <= l_image->y0) ) {
                mi_event_msg(p_manager, EVT_ERROR, "Error with SIZ marker: illegal tile offset\n");
                return mi_FALSE;
        }

        /* Allocate the resulting image components */
        l_image->comps = (mi_image_comp_t*) mi_calloc(l_image->numcomps, sizeof(mi_image_comp_t));
        if (l_image->comps == 00){
                l_image->numcomps = 0;
                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to take in charge SIZ marker\n");
                return mi_FALSE;
        }

        l_img_comp = l_image->comps;

        /* Read the component information */
        for (i = 0; i < l_image->numcomps; ++i){
                mi_UINT32 tmp;
                mi_read_bytes(p_header_data,&tmp,1);   /* Ssiz_i */
                ++p_header_data;
                l_img_comp->prec = (tmp & 0x7f) + 1;
                l_img_comp->sgnd = tmp >> 7;
                mi_read_bytes(p_header_data,&tmp,1);   /* XRsiz_i */
                ++p_header_data;
                l_img_comp->dx = (mi_UINT32)tmp; /* should be between 1 and 255 */
                mi_read_bytes(p_header_data,&tmp,1);   /* YRsiz_i */
                ++p_header_data;
                l_img_comp->dy = (mi_UINT32)tmp; /* should be between 1 and 255 */
                if( l_img_comp->dx < 1 || l_img_comp->dx > 255 ||
                    l_img_comp->dy < 1 || l_img_comp->dy > 255 ) {
                    mi_event_msg(p_manager, EVT_ERROR,
                                  "Invalid values for comp = %d : dx=%u dy=%u (should be between 1 and 255 according to the JPEG2000 norm)\n",
                                  i, l_img_comp->dx, l_img_comp->dy);
                    return mi_FALSE;
                }
                if( l_img_comp->prec > 38) { /* TODO openjpeg won't handle more than ? */
                    mi_event_msg(p_manager, EVT_ERROR,
                                  "Invalid values for comp = %d : prec=%u (should be between 1 and 38 according to the JPEG2000 norm)\n",
                                  i, l_img_comp->prec);
                    return mi_FALSE;
                }

                l_img_comp->resno_decoded = 0;                                                          /* number of resolution decoded */
                l_img_comp->factor = l_cp->m_specific_param.m_dec.m_reduce; /* reducing factor per component */
                ++l_img_comp;
        }

        /* Compute the number of tiles */
        l_cp->tw = (mi_UINT32)mi_int_ceildiv((mi_INT32)(l_image->x1 - l_cp->tx0), (mi_INT32)l_cp->tdx);
        l_cp->th = (mi_UINT32)mi_int_ceildiv((mi_INT32)(l_image->y1 - l_cp->ty0), (mi_INT32)l_cp->tdy);

        /* Check that the number of tiles is valid */
        if (l_cp->tw == 0 || l_cp->th == 0 || l_cp->tw > 65535 / l_cp->th) {
            mi_event_msg(  p_manager, EVT_ERROR, 
                            "Invalid number of tiles : %u x %u (maximum fixed by jpeg2000 norm is 65535 tiles)\n",
                            l_cp->tw, l_cp->th);
            return mi_FALSE;
        }
        l_nb_tiles = l_cp->tw * l_cp->th;

        /* Define the tiles which will be decoded */
        if (p_j2k->m_specific_param.m_decoder.m_discard_tiles) {
                p_j2k->m_specific_param.m_decoder.m_start_tile_x = (p_j2k->m_specific_param.m_decoder.m_start_tile_x - l_cp->tx0) / l_cp->tdx;
                p_j2k->m_specific_param.m_decoder.m_start_tile_y = (p_j2k->m_specific_param.m_decoder.m_start_tile_y - l_cp->ty0) / l_cp->tdy;
                p_j2k->m_specific_param.m_decoder.m_end_tile_x = (mi_UINT32)mi_int_ceildiv((mi_INT32)(p_j2k->m_specific_param.m_decoder.m_end_tile_x - l_cp->tx0), (mi_INT32)l_cp->tdx);
                p_j2k->m_specific_param.m_decoder.m_end_tile_y = (mi_UINT32)mi_int_ceildiv((mi_INT32)(p_j2k->m_specific_param.m_decoder.m_end_tile_y - l_cp->ty0), (mi_INT32)l_cp->tdy);
        }
        else {
                p_j2k->m_specific_param.m_decoder.m_start_tile_x = 0;
                p_j2k->m_specific_param.m_decoder.m_start_tile_y = 0;
                p_j2k->m_specific_param.m_decoder.m_end_tile_x = l_cp->tw;
                p_j2k->m_specific_param.m_decoder.m_end_tile_y = l_cp->th;
        }

        /* memory allocations */
        l_cp->tcps = (mi_tcp_t*) mi_calloc(l_nb_tiles, sizeof(mi_tcp_t));
        if (l_cp->tcps == 00) {
                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to take in charge SIZ marker\n");
                return mi_FALSE;
        }

        p_j2k->m_specific_param.m_decoder.m_default_tcp->tccps =
                        (mi_tccp_t*) mi_calloc(l_image->numcomps, sizeof(mi_tccp_t));
        if(p_j2k->m_specific_param.m_decoder.m_default_tcp->tccps  == 00) {
                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to take in charge SIZ marker\n");
                return mi_FALSE;
        }

        p_j2k->m_specific_param.m_decoder.m_default_tcp->m_mct_records =
                        (mi_mct_data_t*)mi_calloc(mi_J2K_MCT_DEFAULT_NB_RECORDS ,sizeof(mi_mct_data_t));

        if (! p_j2k->m_specific_param.m_decoder.m_default_tcp->m_mct_records) {
                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to take in charge SIZ marker\n");
                return mi_FALSE;
        }
        p_j2k->m_specific_param.m_decoder.m_default_tcp->m_nb_max_mct_records = mi_J2K_MCT_DEFAULT_NB_RECORDS;

        p_j2k->m_specific_param.m_decoder.m_default_tcp->m_mcc_records =
                        (mi_simple_mcc_decorrelation_data_t*)
                        mi_calloc(mi_J2K_MCC_DEFAULT_NB_RECORDS, sizeof(mi_simple_mcc_decorrelation_data_t));

        if (! p_j2k->m_specific_param.m_decoder.m_default_tcp->m_mcc_records) {
                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to take in charge SIZ marker\n");
                return mi_FALSE;
        }
        p_j2k->m_specific_param.m_decoder.m_default_tcp->m_nb_max_mcc_records = mi_J2K_MCC_DEFAULT_NB_RECORDS;

        /* set up default dc level shift */
        for (i=0;i<l_image->numcomps;++i) {
                if (! l_image->comps[i].sgnd) {
                        p_j2k->m_specific_param.m_decoder.m_default_tcp->tccps[i].m_dc_level_shift = 1 << (l_image->comps[i].prec - 1);
                }
        }

        l_current_tile_param = l_cp->tcps;
        for     (i = 0; i < l_nb_tiles; ++i) {
                l_current_tile_param->tccps = (mi_tccp_t*) mi_calloc(l_image->numcomps, sizeof(mi_tccp_t));
                if (l_current_tile_param->tccps == 00) {
                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to take in charge SIZ marker\n");
                        return mi_FALSE;
                }

                ++l_current_tile_param;
        }

        p_j2k->m_specific_param.m_decoder.m_state =  J2K_STATE_MH; /* FIXME J2K_DEC_STATE_MH; */
        mi_image_comp_header_update(l_image,l_cp);

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_com(     mi_j2k_t *p_j2k,
                                                        mi_stream_private_t *p_stream,
                                                        mi_event_mgr_t * p_manager
                            )
{
        mi_UINT32 l_comment_size;
        mi_UINT32 l_total_com_size;
        const mi_CHAR *l_comment;
        mi_BYTE * l_current_ptr = 00;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_stream != 00);
        assert(p_manager != 00);

        l_comment = p_j2k->m_cp.comment;
        l_comment_size = (mi_UINT32)strlen(l_comment);
        l_total_com_size = l_comment_size + 6;

        if (l_total_com_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
                mi_BYTE *new_header_tile_data = (mi_BYTE *) mi_realloc(p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_total_com_size);
                if (! new_header_tile_data) {
                        mi_free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to write the COM marker\n");
                        return mi_FALSE;
                }
                p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
                p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_total_com_size;
        }

        l_current_ptr = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

        mi_write_bytes(l_current_ptr,J2K_MS_COM , 2);  /* COM */
        l_current_ptr+=2;

        mi_write_bytes(l_current_ptr,l_total_com_size - 2 , 2);        /* L_COM */
        l_current_ptr+=2;

        mi_write_bytes(l_current_ptr,1 , 2);   /* General use (IS 8859-15:1999 (Latin) values) */
        l_current_ptr+=2;

        memcpy( l_current_ptr,l_comment,l_comment_size);

        if (mi_stream_write_data(p_stream,p_j2k->m_specific_param.m_encoder.m_header_tile_data,l_total_com_size,p_manager) != l_total_com_size) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

/**
 * Reads a COM marker (comments)
 * @param       p_j2k           the jpeg2000 file codec.
 * @param       p_header_data   the data contained in the COM box.
 * @param       p_header_size   the size of the data contained in the COM marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_com (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager
                                    )
{
        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_header_data != 00);
  (void)p_header_size;

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_cod(     mi_j2k_t *p_j2k,
                                                        mi_stream_private_t *p_stream,
                                                        mi_event_mgr_t * p_manager )
{
        mi_cp_t *l_cp = 00;
        mi_tcp_t *l_tcp = 00;
        mi_UINT32 l_code_size,l_remaining_size;
        mi_BYTE * l_current_data = 00;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        l_cp = &(p_j2k->m_cp);
        l_tcp = &l_cp->tcps[p_j2k->m_current_tile_number];
        l_code_size = 9 + mi_j2k_get_SPCod_SPCoc_size(p_j2k,p_j2k->m_current_tile_number,0);
        l_remaining_size = l_code_size;

        if (l_code_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
                mi_BYTE *new_header_tile_data = (mi_BYTE *) mi_realloc(p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_code_size);
                if (! new_header_tile_data) {
                        mi_free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to write COD marker\n");
                        return mi_FALSE;
                }
                p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
                p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_code_size;
        }

        l_current_data = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

        mi_write_bytes(l_current_data,J2K_MS_COD,2);             /* COD */
        l_current_data += 2;

        mi_write_bytes(l_current_data,l_code_size-2,2);          /* L_COD */
        l_current_data += 2;

        mi_write_bytes(l_current_data,l_tcp->csty,1);            /* Scod */
        ++l_current_data;

        mi_write_bytes(l_current_data,(mi_UINT32)l_tcp->prg,1); /* SGcod (A) */
        ++l_current_data;

        mi_write_bytes(l_current_data,l_tcp->numlayers,2);       /* SGcod (B) */
        l_current_data+=2;

        mi_write_bytes(l_current_data,l_tcp->mct,1);             /* SGcod (C) */
        ++l_current_data;

        l_remaining_size -= 9;

        if (! mi_j2k_write_SPCod_SPCoc(p_j2k,p_j2k->m_current_tile_number,0,l_current_data,&l_remaining_size,p_manager)) {
                mi_event_msg(p_manager, EVT_ERROR, "Error writing COD marker\n");
                return mi_FALSE;
        }

        if (l_remaining_size != 0) {
                mi_event_msg(p_manager, EVT_ERROR, "Error writing COD marker\n");
                return mi_FALSE;
        }

        if (mi_stream_write_data(p_stream,p_j2k->m_specific_param.m_encoder.m_header_tile_data,l_code_size,p_manager) != l_code_size) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

/**
 * Reads a COD marker (Coding Styke defaults)
 * @param       p_header_data   the data contained in the COD box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the COD marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_cod (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager
                                    )
{
        /* loop */
        mi_UINT32 i;
        mi_UINT32 l_tmp;
        mi_cp_t *l_cp = 00;
        mi_tcp_t *l_tcp = 00;
        mi_image_t *l_image = 00;

        /* preconditions */
        assert(p_header_data != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        l_image = p_j2k->m_private_image;
        l_cp = &(p_j2k->m_cp);

        /* If we are in the first tile-part header of the current tile */
        l_tcp = (p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_TPH) ?
                                &l_cp->tcps[p_j2k->m_current_tile_number] :
                                p_j2k->m_specific_param.m_decoder.m_default_tcp;
	
        /* Only one COD per tile */
        if (l_tcp->cod) {
                mi_event_msg(p_manager, EVT_ERROR, "COD marker already read. No more than one COD marker per tile.\n");
                return mi_FALSE;
        }
        l_tcp->cod = 1;
	
        /* Make sure room is sufficient */
        if (p_header_size < 5) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading COD marker\n");
                return mi_FALSE;
        }

        mi_read_bytes(p_header_data,&l_tcp->csty,1);           /* Scod */
        ++p_header_data;
        /* Make sure we know how to decode this */
        if ((l_tcp->csty & ~(mi_UINT32)(J2K_CP_CSTY_PRT | J2K_CP_CSTY_SOP | J2K_CP_CSTY_EPH)) != 0U) {
                mi_event_msg(p_manager, EVT_ERROR, "Unknown Scod value in COD marker\n");
                return mi_FALSE;
        }
        mi_read_bytes(p_header_data,&l_tmp,1);                         /* SGcod (A) */
        ++p_header_data;
        l_tcp->prg = (mi_PROG_ORDER) l_tmp;
        /* Make sure progression order is valid */
        if (l_tcp->prg > mi_CPRL ) {
                mi_event_msg(p_manager, EVT_ERROR, "Unknown progression order in COD marker\n");
                l_tcp->prg = mi_PROG_UNKNOWN;
        }
        mi_read_bytes(p_header_data,&l_tcp->numlayers,2);      /* SGcod (B) */
        p_header_data+=2;
	
        if ((l_tcp->numlayers < 1U) || (l_tcp->numlayers > 65535U)) {
                mi_event_msg(p_manager, EVT_ERROR, "Invalid number of layers in COD marker : %d not in range [1-65535]\n", l_tcp->numlayers);
                return mi_FALSE;
        }

        /* If user didn't set a number layer to decode take the max specify in the codestream. */
        if      (l_cp->m_specific_param.m_dec.m_layer) {
                l_tcp->num_layers_to_decode = l_cp->m_specific_param.m_dec.m_layer;
        }
        else {
                l_tcp->num_layers_to_decode = l_tcp->numlayers;
        }

        mi_read_bytes(p_header_data,&l_tcp->mct,1);            /* SGcod (C) */
        ++p_header_data;

        p_header_size -= 5;
        for     (i = 0; i < l_image->numcomps; ++i) {
                l_tcp->tccps[i].csty = l_tcp->csty & J2K_CCP_CSTY_PRT;
        }

        if (! mi_j2k_read_SPCod_SPCoc(p_j2k,0,p_header_data,&p_header_size,p_manager)) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading COD marker\n");
                return mi_FALSE;
        }

        if (p_header_size != 0) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading COD marker\n");
                return mi_FALSE;
        }

        /* Apply the coding style to other components of the current tile or the m_default_tcp*/
        mi_j2k_copy_tile_component_parameters(p_j2k);

        /* Index */

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_coc( mi_j2k_t *p_j2k,
                                                mi_UINT32 p_comp_no,
                                                mi_stream_private_t *p_stream,
                                                mi_event_mgr_t * p_manager )
{
        mi_UINT32 l_coc_size,l_remaining_size;
        mi_UINT32 l_comp_room;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        l_comp_room = (p_j2k->m_private_image->numcomps <= 256) ? 1 : 2;

        l_coc_size = 5 + l_comp_room + mi_j2k_get_SPCod_SPCoc_size(p_j2k,p_j2k->m_current_tile_number,p_comp_no);

        if (l_coc_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
                mi_BYTE *new_header_tile_data;
                /*p_j2k->m_specific_param.m_encoder.m_header_tile_data
                        = (mi_BYTE*)mi_realloc(
                                p_j2k->m_specific_param.m_encoder.m_header_tile_data,
                                l_coc_size);*/

                new_header_tile_data = (mi_BYTE *) mi_realloc(p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_coc_size);
                if (! new_header_tile_data) {
                        mi_free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to write COC marker\n");
                        return mi_FALSE;
                }
                p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
                p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_coc_size;
        }

        mi_j2k_write_coc_in_memory(p_j2k,p_comp_no,p_j2k->m_specific_param.m_encoder.m_header_tile_data,&l_remaining_size,p_manager);

        if (mi_stream_write_data(p_stream,p_j2k->m_specific_param.m_encoder.m_header_tile_data,l_coc_size,p_manager) != l_coc_size) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_compare_coc(mi_j2k_t *p_j2k, mi_UINT32 p_first_comp_no, mi_UINT32 p_second_comp_no)
{
	mi_cp_t *l_cp = NULL;
	mi_tcp_t *l_tcp = NULL;
	
	/* preconditions */
	assert(p_j2k != 00);
	
	l_cp = &(p_j2k->m_cp);
	l_tcp = &l_cp->tcps[p_j2k->m_current_tile_number];
	
	if (l_tcp->tccps[p_first_comp_no].csty != l_tcp->tccps[p_second_comp_no].csty) {
		return mi_FALSE;
	}
	
	
	return mi_j2k_compare_SPCod_SPCoc(p_j2k, p_j2k->m_current_tile_number, p_first_comp_no, p_second_comp_no);
}

static void mi_j2k_write_coc_in_memory(   mi_j2k_t *p_j2k,
                                                mi_UINT32 p_comp_no,
                                                mi_BYTE * p_data,
                                                mi_UINT32 * p_data_written,
                                                mi_event_mgr_t * p_manager
                                    )
{
        mi_cp_t *l_cp = 00;
        mi_tcp_t *l_tcp = 00;
        mi_UINT32 l_coc_size,l_remaining_size;
        mi_BYTE * l_current_data = 00;
        mi_image_t *l_image = 00;
        mi_UINT32 l_comp_room;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);

        l_cp = &(p_j2k->m_cp);
        l_tcp = &l_cp->tcps[p_j2k->m_current_tile_number];
        l_image = p_j2k->m_private_image;
        l_comp_room = (l_image->numcomps <= 256) ? 1 : 2;

        l_coc_size = 5 + l_comp_room + mi_j2k_get_SPCod_SPCoc_size(p_j2k,p_j2k->m_current_tile_number,p_comp_no);
        l_remaining_size = l_coc_size;

        l_current_data = p_data;

        mi_write_bytes(l_current_data,J2K_MS_COC,2);                           /* COC */
        l_current_data += 2;

        mi_write_bytes(l_current_data,l_coc_size-2,2);                         /* L_COC */
        l_current_data += 2;

        mi_write_bytes(l_current_data,p_comp_no, l_comp_room);         /* Ccoc */
        l_current_data+=l_comp_room;

        mi_write_bytes(l_current_data, l_tcp->tccps[p_comp_no].csty, 1);               /* Scoc */
        ++l_current_data;

        l_remaining_size -= (5 + l_comp_room);
        mi_j2k_write_SPCod_SPCoc(p_j2k,p_j2k->m_current_tile_number,0,l_current_data,&l_remaining_size,p_manager);
        * p_data_written = l_coc_size;
}

static mi_UINT32 mi_j2k_get_max_coc_size(mi_j2k_t *p_j2k)
{
        mi_UINT32 i,j;
        mi_UINT32 l_nb_comp;
        mi_UINT32 l_nb_tiles;
        mi_UINT32 l_max = 0;

        /* preconditions */

        l_nb_tiles = p_j2k->m_cp.tw * p_j2k->m_cp.th ;
        l_nb_comp = p_j2k->m_private_image->numcomps;

        for (i=0;i<l_nb_tiles;++i) {
                for (j=0;j<l_nb_comp;++j) {
                        l_max = mi_uint_max(l_max,mi_j2k_get_SPCod_SPCoc_size(p_j2k,i,j));
                }
        }

        return 6 + l_max;
}

/**
 * Reads a COC marker (Coding Style Component)
 * @param       p_header_data   the data contained in the COC box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the COC marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_coc (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager
                                    )
{
        mi_cp_t *l_cp = NULL;
        mi_tcp_t *l_tcp = NULL;
        mi_image_t *l_image = NULL;
        mi_UINT32 l_comp_room;
        mi_UINT32 l_comp_no;

        /* preconditions */
        assert(p_header_data != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        l_cp = &(p_j2k->m_cp);
        l_tcp = (p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_TPH ) ? /*FIXME J2K_DEC_STATE_TPH*/
                                &l_cp->tcps[p_j2k->m_current_tile_number] :
                                p_j2k->m_specific_param.m_decoder.m_default_tcp;
        l_image = p_j2k->m_private_image;

        l_comp_room = l_image->numcomps <= 256 ? 1 : 2;

        /* make sure room is sufficient*/
        if (p_header_size < l_comp_room + 1) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading COC marker\n");
                return mi_FALSE;
        }
        p_header_size -= l_comp_room + 1;

        mi_read_bytes(p_header_data,&l_comp_no,l_comp_room);                   /* Ccoc */
        p_header_data += l_comp_room;
        if (l_comp_no >= l_image->numcomps) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading COC marker (bad number of components)\n");
                return mi_FALSE;
        }

        mi_read_bytes(p_header_data,&l_tcp->tccps[l_comp_no].csty,1);                  /* Scoc */
        ++p_header_data ;

        if (! mi_j2k_read_SPCod_SPCoc(p_j2k,l_comp_no,p_header_data,&p_header_size,p_manager)) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading COC marker\n");
                return mi_FALSE;
        }

        if (p_header_size != 0) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading COC marker\n");
                return mi_FALSE;
        }
        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_qcd(     mi_j2k_t *p_j2k,
                                                        mi_stream_private_t *p_stream,
                                                        mi_event_mgr_t * p_manager
                            )
{
        mi_UINT32 l_qcd_size,l_remaining_size;
        mi_BYTE * l_current_data = 00;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        l_qcd_size = 4 + mi_j2k_get_SQcd_SQcc_size(p_j2k,p_j2k->m_current_tile_number,0);
        l_remaining_size = l_qcd_size;

        if (l_qcd_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
                mi_BYTE *new_header_tile_data = (mi_BYTE *) mi_realloc(p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_qcd_size);
                if (! new_header_tile_data) {
                        mi_free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to write QCD marker\n");
                        return mi_FALSE;
                }
                p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
                p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_qcd_size;
        }

        l_current_data = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

        mi_write_bytes(l_current_data,J2K_MS_QCD,2);           /* QCD */
        l_current_data += 2;

        mi_write_bytes(l_current_data,l_qcd_size-2,2);         /* L_QCD */
        l_current_data += 2;

        l_remaining_size -= 4;

        if (! mi_j2k_write_SQcd_SQcc(p_j2k,p_j2k->m_current_tile_number,0,l_current_data,&l_remaining_size,p_manager)) {
                mi_event_msg(p_manager, EVT_ERROR, "Error writing QCD marker\n");
                return mi_FALSE;
        }

        if (l_remaining_size != 0) {
                mi_event_msg(p_manager, EVT_ERROR, "Error writing QCD marker\n");
                return mi_FALSE;
        }

        if (mi_stream_write_data(p_stream, p_j2k->m_specific_param.m_encoder.m_header_tile_data,l_qcd_size,p_manager) != l_qcd_size) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

/**
 * Reads a QCD marker (Quantization defaults)
 * @param       p_header_data   the data contained in the QCD box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the QCD marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_qcd (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager
                                    )
{
        /* preconditions */
        assert(p_header_data != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        if (! mi_j2k_read_SQcd_SQcc(p_j2k,0,p_header_data,&p_header_size,p_manager)) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading QCD marker\n");
                return mi_FALSE;
        }

        if (p_header_size != 0) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading QCD marker\n");
                return mi_FALSE;
        }

        /* Apply the quantization parameters to other components of the current tile or the m_default_tcp */
        mi_j2k_copy_tile_quantization_parameters(p_j2k);

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_qcc(     mi_j2k_t *p_j2k,
                                                mi_UINT32 p_comp_no,
                                                mi_stream_private_t *p_stream,
                                                mi_event_mgr_t * p_manager
                            )
{
        mi_UINT32 l_qcc_size,l_remaining_size;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        l_qcc_size = 5 + mi_j2k_get_SQcd_SQcc_size(p_j2k,p_j2k->m_current_tile_number,p_comp_no);
        l_qcc_size += p_j2k->m_private_image->numcomps <= 256 ? 0:1;
        l_remaining_size = l_qcc_size;

        if (l_qcc_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
                mi_BYTE *new_header_tile_data = (mi_BYTE *) mi_realloc(p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_qcc_size);
                if (! new_header_tile_data) {
                        mi_free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to write QCC marker\n");
                        return mi_FALSE;
                }
                p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
                p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_qcc_size;
        }

        mi_j2k_write_qcc_in_memory(p_j2k,p_comp_no,p_j2k->m_specific_param.m_encoder.m_header_tile_data,&l_remaining_size,p_manager);

        if (mi_stream_write_data(p_stream,p_j2k->m_specific_param.m_encoder.m_header_tile_data,l_qcc_size,p_manager) != l_qcc_size) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_compare_qcc(mi_j2k_t *p_j2k, mi_UINT32 p_first_comp_no, mi_UINT32 p_second_comp_no)
{
	return mi_j2k_compare_SQcd_SQcc(p_j2k,p_j2k->m_current_tile_number,p_first_comp_no, p_second_comp_no);
}

static void mi_j2k_write_qcc_in_memory(   mi_j2k_t *p_j2k,
                                                                mi_UINT32 p_comp_no,
                                                                mi_BYTE * p_data,
                                                                mi_UINT32 * p_data_written,
                                                                mi_event_mgr_t * p_manager
                                    )
{
        mi_UINT32 l_qcc_size,l_remaining_size;
        mi_BYTE * l_current_data = 00;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);

        l_qcc_size = 6 + mi_j2k_get_SQcd_SQcc_size(p_j2k,p_j2k->m_current_tile_number,p_comp_no);
        l_remaining_size = l_qcc_size;

        l_current_data = p_data;

        mi_write_bytes(l_current_data,J2K_MS_QCC,2);           /* QCC */
        l_current_data += 2;

        if (p_j2k->m_private_image->numcomps <= 256) {
                --l_qcc_size;

                mi_write_bytes(l_current_data,l_qcc_size-2,2);         /* L_QCC */
                l_current_data += 2;

                mi_write_bytes(l_current_data, p_comp_no, 1);  /* Cqcc */
                ++l_current_data;

                /* in the case only one byte is sufficient the last byte allocated is useless -> still do -6 for available */
                l_remaining_size -= 6;
        }
        else {
                mi_write_bytes(l_current_data,l_qcc_size-2,2);         /* L_QCC */
                l_current_data += 2;

                mi_write_bytes(l_current_data, p_comp_no, 2);  /* Cqcc */
                l_current_data+=2;

                l_remaining_size -= 6;
        }

        mi_j2k_write_SQcd_SQcc(p_j2k,p_j2k->m_current_tile_number,p_comp_no,l_current_data,&l_remaining_size,p_manager);

        *p_data_written = l_qcc_size;
}

static mi_UINT32 mi_j2k_get_max_qcc_size (mi_j2k_t *p_j2k)
{
        return mi_j2k_get_max_coc_size(p_j2k);
}

/**
 * Reads a QCC marker (Quantization component)
 * @param       p_header_data   the data contained in the QCC box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the QCC marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_qcc(   mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager
                                    )
{
        mi_UINT32 l_num_comp,l_comp_no;

        /* preconditions */
        assert(p_header_data != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        l_num_comp = p_j2k->m_private_image->numcomps;

        if (l_num_comp <= 256) {
                if (p_header_size < 1) {
                        mi_event_msg(p_manager, EVT_ERROR, "Error reading QCC marker\n");
                        return mi_FALSE;
                }
                mi_read_bytes(p_header_data,&l_comp_no,1);
                ++p_header_data;
                --p_header_size;
        }
        else {
                if (p_header_size < 2) {
                        mi_event_msg(p_manager, EVT_ERROR, "Error reading QCC marker\n");
                        return mi_FALSE;
                }
                mi_read_bytes(p_header_data,&l_comp_no,2);
                p_header_data+=2;
                p_header_size-=2;
        }

        if (l_comp_no >= p_j2k->m_private_image->numcomps) {
                mi_event_msg(p_manager, EVT_ERROR,
                              "Invalid component number: %d, regarding the number of components %d\n",
                              l_comp_no, p_j2k->m_private_image->numcomps);
                return mi_FALSE;
        }

        if (! mi_j2k_read_SQcd_SQcc(p_j2k,l_comp_no,p_header_data,&p_header_size,p_manager)) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading QCC marker\n");
                return mi_FALSE;
        }

        if (p_header_size != 0) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading QCC marker\n");
                return mi_FALSE;
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_poc(     mi_j2k_t *p_j2k,
                                                        mi_stream_private_t *p_stream,
                                                        mi_event_mgr_t * p_manager
                            )
{
        mi_UINT32 l_nb_comp;
        mi_UINT32 l_nb_poc;
        mi_UINT32 l_poc_size;
        mi_UINT32 l_written_size = 0;
        mi_tcp_t *l_tcp = 00;
        mi_UINT32 l_poc_room;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        l_tcp = &p_j2k->m_cp.tcps[p_j2k->m_current_tile_number];
        l_nb_comp = p_j2k->m_private_image->numcomps;
        l_nb_poc = 1 + l_tcp->numpocs;

        if (l_nb_comp <= 256) {
                l_poc_room = 1;
        }
        else {
                l_poc_room = 2;
        }
        l_poc_size = 4 + (5 + 2 * l_poc_room) * l_nb_poc;

        if (l_poc_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
                mi_BYTE *new_header_tile_data = (mi_BYTE *) mi_realloc(p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_poc_size);
                if (! new_header_tile_data) {
                        mi_free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to write POC marker\n");
                        return mi_FALSE;
                }
                p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
                p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_poc_size;
        }

        mi_j2k_write_poc_in_memory(p_j2k,p_j2k->m_specific_param.m_encoder.m_header_tile_data,&l_written_size,p_manager);

        if (mi_stream_write_data(p_stream,p_j2k->m_specific_param.m_encoder.m_header_tile_data,l_poc_size,p_manager) != l_poc_size) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

static void mi_j2k_write_poc_in_memory(   mi_j2k_t *p_j2k,
                                                                mi_BYTE * p_data,
                                                                mi_UINT32 * p_data_written,
                                                                mi_event_mgr_t * p_manager
                                    )
{
        mi_UINT32 i;
        mi_BYTE * l_current_data = 00;
        mi_UINT32 l_nb_comp;
        mi_UINT32 l_nb_poc;
        mi_UINT32 l_poc_size;
        mi_image_t *l_image = 00;
        mi_tcp_t *l_tcp = 00;
        mi_tccp_t *l_tccp = 00;
        mi_poc_t *l_current_poc = 00;
        mi_UINT32 l_poc_room;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);

        l_tcp = &p_j2k->m_cp.tcps[p_j2k->m_current_tile_number];
        l_tccp = &l_tcp->tccps[0];
        l_image = p_j2k->m_private_image;
        l_nb_comp = l_image->numcomps;
        l_nb_poc = 1 + l_tcp->numpocs;

        if (l_nb_comp <= 256) {
                l_poc_room = 1;
        }
        else {
                l_poc_room = 2;
        }

        l_poc_size = 4 + (5 + 2 * l_poc_room) * l_nb_poc;

        l_current_data = p_data;

        mi_write_bytes(l_current_data,J2K_MS_POC,2);                                   /* POC  */
        l_current_data += 2;

        mi_write_bytes(l_current_data,l_poc_size-2,2);                                 /* Lpoc */
        l_current_data += 2;

        l_current_poc =  l_tcp->pocs;
        for (i = 0; i < l_nb_poc; ++i) {
                mi_write_bytes(l_current_data,l_current_poc->resno0,1);                                /* RSpoc_i */
                ++l_current_data;

                mi_write_bytes(l_current_data,l_current_poc->compno0,l_poc_room);              /* CSpoc_i */
                l_current_data+=l_poc_room;

                mi_write_bytes(l_current_data,l_current_poc->layno1,2);                                /* LYEpoc_i */
                l_current_data+=2;

                mi_write_bytes(l_current_data,l_current_poc->resno1,1);                                /* REpoc_i */
                ++l_current_data;

                mi_write_bytes(l_current_data,l_current_poc->compno1,l_poc_room);              /* CEpoc_i */
                l_current_data+=l_poc_room;

                mi_write_bytes(l_current_data, (mi_UINT32)l_current_poc->prg,1);    /* Ppoc_i */
                ++l_current_data;

                /* change the value of the max layer according to the actual number of layers in the file, components and resolutions*/
                l_current_poc->layno1 = (mi_UINT32)mi_int_min((mi_INT32)l_current_poc->layno1, (mi_INT32)l_tcp->numlayers);
                l_current_poc->resno1 = (mi_UINT32)mi_int_min((mi_INT32)l_current_poc->resno1, (mi_INT32)l_tccp->numresolutions);
                l_current_poc->compno1 = (mi_UINT32)mi_int_min((mi_INT32)l_current_poc->compno1, (mi_INT32)l_nb_comp);

                ++l_current_poc;
        }

        *p_data_written = l_poc_size;
}

static mi_UINT32 mi_j2k_get_max_poc_size(mi_j2k_t *p_j2k)
{
        mi_tcp_t * l_tcp = 00;
        mi_UINT32 l_nb_tiles = 0;
        mi_UINT32 l_max_poc = 0;
        mi_UINT32 i;

        l_tcp = p_j2k->m_cp.tcps;
        l_nb_tiles = p_j2k->m_cp.th * p_j2k->m_cp.tw;

        for (i=0;i<l_nb_tiles;++i) {
                l_max_poc = mi_uint_max(l_max_poc,l_tcp->numpocs);
                ++l_tcp;
        }

        ++l_max_poc;

        return 4 + 9 * l_max_poc;
}

static mi_UINT32 mi_j2k_get_max_toc_size (mi_j2k_t *p_j2k)
{
        mi_UINT32 i;
        mi_UINT32 l_nb_tiles;
        mi_UINT32 l_max = 0;
        mi_tcp_t * l_tcp = 00;

        l_tcp = p_j2k->m_cp.tcps;
        l_nb_tiles = p_j2k->m_cp.tw * p_j2k->m_cp.th ;

        for (i=0;i<l_nb_tiles;++i) {
                l_max = mi_uint_max(l_max,l_tcp->m_nb_tile_parts);

                ++l_tcp;
        }

        return 12 * l_max;
}

static mi_UINT32 mi_j2k_get_specific_header_sizes(mi_j2k_t *p_j2k)
{
        mi_UINT32 l_nb_bytes = 0;
        mi_UINT32 l_nb_comps;
        mi_UINT32 l_coc_bytes,l_qcc_bytes;

        l_nb_comps = p_j2k->m_private_image->numcomps - 1;
        l_nb_bytes += mi_j2k_get_max_toc_size(p_j2k);

        if (!(mi_IS_CINEMA(p_j2k->m_cp.rsiz))) {
                l_coc_bytes = mi_j2k_get_max_coc_size(p_j2k);
                l_nb_bytes += l_nb_comps * l_coc_bytes;

                l_qcc_bytes = mi_j2k_get_max_qcc_size(p_j2k);
                l_nb_bytes += l_nb_comps * l_qcc_bytes;
        }

        l_nb_bytes += mi_j2k_get_max_poc_size(p_j2k);

        /*** DEVELOPER CORNER, Add room for your headers ***/

        return l_nb_bytes;
}

/**
 * Reads a POC marker (Progression Order Change)
 *
 * @param       p_header_data   the data contained in the POC box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the POC marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_poc (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager
                                    )
{
        mi_UINT32 i, l_nb_comp, l_tmp;
        mi_image_t * l_image = 00;
        mi_UINT32 l_old_poc_nb, l_current_poc_nb, l_current_poc_remaining;
        mi_UINT32 l_chunk_size, l_comp_room;

        mi_cp_t *l_cp = 00;
        mi_tcp_t *l_tcp = 00;
        mi_poc_t *l_current_poc = 00;

        /* preconditions */
        assert(p_header_data != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        l_image = p_j2k->m_private_image;
        l_nb_comp = l_image->numcomps;
        if (l_nb_comp <= 256) {
                l_comp_room = 1;
        }
        else {
                l_comp_room = 2;
        }
        l_chunk_size = 5 + 2 * l_comp_room;
        l_current_poc_nb = p_header_size / l_chunk_size;
        l_current_poc_remaining = p_header_size % l_chunk_size;

        if ((l_current_poc_nb <= 0) || (l_current_poc_remaining != 0)) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading POC marker\n");
                return mi_FALSE;
        }

        l_cp = &(p_j2k->m_cp);
        l_tcp = (p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_TPH) ?
                                &l_cp->tcps[p_j2k->m_current_tile_number] :
                                p_j2k->m_specific_param.m_decoder.m_default_tcp;
        l_old_poc_nb = l_tcp->POC ? l_tcp->numpocs + 1 : 0;
        l_current_poc_nb += l_old_poc_nb;

        if(l_current_poc_nb >= 32)
          {
          mi_event_msg(p_manager, EVT_ERROR, "Too many POCs %d\n", l_current_poc_nb);
          return mi_FALSE;
          }
        assert(l_current_poc_nb < 32);

        /* now poc is in use.*/
        l_tcp->POC = 1;

        l_current_poc = &l_tcp->pocs[l_old_poc_nb];
        for     (i = l_old_poc_nb; i < l_current_poc_nb; ++i) {
                mi_read_bytes(p_header_data,&(l_current_poc->resno0),1);                               /* RSpoc_i */
                ++p_header_data;
                mi_read_bytes(p_header_data,&(l_current_poc->compno0),l_comp_room);    /* CSpoc_i */
                p_header_data+=l_comp_room;
                mi_read_bytes(p_header_data,&(l_current_poc->layno1),2);                               /* LYEpoc_i */
                /* make sure layer end is in acceptable bounds */
                l_current_poc->layno1 = mi_uint_min(l_current_poc->layno1, l_tcp->numlayers);
                p_header_data+=2;
                mi_read_bytes(p_header_data,&(l_current_poc->resno1),1);                               /* REpoc_i */
                ++p_header_data;
                mi_read_bytes(p_header_data,&(l_current_poc->compno1),l_comp_room);    /* CEpoc_i */
                p_header_data+=l_comp_room;
                mi_read_bytes(p_header_data,&l_tmp,1);                                                                 /* Ppoc_i */
                ++p_header_data;
                l_current_poc->prg = (mi_PROG_ORDER) l_tmp;
                /* make sure comp is in acceptable bounds */
                l_current_poc->compno1 = mi_uint_min(l_current_poc->compno1, l_nb_comp);
                ++l_current_poc;
        }

        l_tcp->numpocs = l_current_poc_nb - 1;
        return mi_TRUE;
}

/**
 * Reads a CRG marker (Component registration)
 *
 * @param       p_header_data   the data contained in the TLM box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the TLM marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_crg (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager
                                    )
{
        mi_UINT32 l_nb_comp;
        /* preconditions */
        assert(p_header_data != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        l_nb_comp = p_j2k->m_private_image->numcomps;

        if (p_header_size != l_nb_comp *4) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading CRG marker\n");
                return mi_FALSE;
        }
        /* Do not care of this at the moment since only local variables are set here */
        /*
        for
                (i = 0; i < l_nb_comp; ++i)
        {
                mi_read_bytes(p_header_data,&l_Xcrg_i,2);                              // Xcrg_i
                p_header_data+=2;
                mi_read_bytes(p_header_data,&l_Ycrg_i,2);                              // Xcrg_i
                p_header_data+=2;
        }
        */
        return mi_TRUE;
}

/**
 * Reads a TLM marker (Tile Length Marker)
 *
 * @param       p_header_data   the data contained in the TLM box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the TLM marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_tlm (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager
                                    )
{
        mi_UINT32 l_Ztlm, l_Stlm, l_ST, l_SP, l_tot_num_tp_remaining, l_quotient, l_Ptlm_size;
        /* preconditions */
        assert(p_header_data != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        if (p_header_size < 2) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading TLM marker\n");
                return mi_FALSE;
        }
        p_header_size -= 2;

        mi_read_bytes(p_header_data,&l_Ztlm,1);                                /* Ztlm */
        ++p_header_data;
        mi_read_bytes(p_header_data,&l_Stlm,1);                                /* Stlm */
        ++p_header_data;

        l_ST = ((l_Stlm >> 4) & 0x3);
        l_SP = (l_Stlm >> 6) & 0x1;

        l_Ptlm_size = (l_SP + 1) * 2;
        l_quotient = l_Ptlm_size + l_ST;

        l_tot_num_tp_remaining = p_header_size % l_quotient;

        if (l_tot_num_tp_remaining != 0) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading TLM marker\n");
                return mi_FALSE;
        }
        /* FIXME Do not care of this at the moment since only local variables are set here */
        /*
        for
                (i = 0; i < l_tot_num_tp; ++i)
        {
                mi_read_bytes(p_header_data,&l_Ttlm_i,l_ST);                           // Ttlm_i
                p_header_data += l_ST;
                mi_read_bytes(p_header_data,&l_Ptlm_i,l_Ptlm_size);            // Ptlm_i
                p_header_data += l_Ptlm_size;
        }*/
        return mi_TRUE;
}

/**
 * Reads a PLM marker (Packet length, main header marker)
 *
 * @param       p_header_data   the data contained in the TLM box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the TLM marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_plm (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager
                                    )
{
        /* preconditions */
        assert(p_header_data != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        if (p_header_size < 1) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading PLM marker\n");
                return mi_FALSE;
        }
        /* Do not care of this at the moment since only local variables are set here */
        /*
        mi_read_bytes(p_header_data,&l_Zplm,1);                                        // Zplm
        ++p_header_data;
        --p_header_size;

        while
                (p_header_size > 0)
        {
                mi_read_bytes(p_header_data,&l_Nplm,1);                                // Nplm
                ++p_header_data;
                p_header_size -= (1+l_Nplm);
                if
                        (p_header_size < 0)
                {
                        mi_event_msg(p_manager, EVT_ERROR, "Error reading PLM marker\n");
                        return false;
                }
                for
                        (i = 0; i < l_Nplm; ++i)
                {
                        mi_read_bytes(p_header_data,&l_tmp,1);                         // Iplm_ij
                        ++p_header_data;
                        // take only the last seven bytes
                        l_packet_len |= (l_tmp & 0x7f);
                        if
                                (l_tmp & 0x80)
                        {
                                l_packet_len <<= 7;
                        }
                        else
                        {
                // store packet length and proceed to next packet
                                l_packet_len = 0;
                        }
                }
                if
                        (l_packet_len != 0)
                {
                        mi_event_msg(p_manager, EVT_ERROR, "Error reading PLM marker\n");
                        return false;
                }
        }
        */
        return mi_TRUE;
}

/**
 * Reads a PLT marker (Packet length, tile-part header)
 *
 * @param       p_header_data   the data contained in the PLT box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the PLT marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_plt (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager
                                    )
{
        mi_UINT32 l_Zplt, l_tmp, l_packet_len = 0, i;

        /* preconditions */
        assert(p_header_data != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        if (p_header_size < 1) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading PLT marker\n");
                return mi_FALSE;
        }

        mi_read_bytes(p_header_data,&l_Zplt,1);                /* Zplt */
        ++p_header_data;
        --p_header_size;

        for (i = 0; i < p_header_size; ++i) {
                mi_read_bytes(p_header_data,&l_tmp,1);         /* Iplt_ij */
                ++p_header_data;
                /* take only the last seven bytes */
                l_packet_len |= (l_tmp & 0x7f);
                if (l_tmp & 0x80) {
                        l_packet_len <<= 7;
                }
                else {
            /* store packet length and proceed to next packet */
                        l_packet_len = 0;
                }
        }

        if (l_packet_len != 0) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading PLT marker\n");
                return mi_FALSE;
        }

        return mi_TRUE;
}

/**
 * Reads a PPM marker (Packed packet headers, main header)
 *
 * @param       p_header_data   the data contained in the POC box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the POC marker.
 * @param       p_manager               the user event manager.
 */

static mi_BOOL mi_j2k_read_ppm (
																	mi_j2k_t *p_j2k,
																	mi_BYTE * p_header_data,
																	mi_UINT32 p_header_size,
																	mi_event_mgr_t * p_manager )
{
	mi_cp_t *l_cp = 00;
	mi_UINT32 l_Z_ppm;
	
	/* preconditions */
	assert(p_header_data != 00);
	assert(p_j2k != 00);
	assert(p_manager != 00);
	
	/* We need to have the Z_ppm element + 1 byte of Nppm/Ippm at minimum */
	if (p_header_size < 2) {
		mi_event_msg(p_manager, EVT_ERROR, "Error reading PPM marker\n");
		return mi_FALSE;
	}
	
	l_cp = &(p_j2k->m_cp);
	l_cp->ppm = 1;
	
	mi_read_bytes(p_header_data,&l_Z_ppm,1);               /* Z_ppm */
	++p_header_data;
	--p_header_size;
	
	/* check allocation needed */
	if (l_cp->ppm_markers == NULL) { /* first PPM marker */
		mi_UINT32 l_newCount = l_Z_ppm + 1U; /* can't overflow, l_Z_ppm is UINT8 */
		assert(l_cp->ppm_markers_count == 0U);
		
		l_cp->ppm_markers = (mi_ppx *) mi_calloc(l_newCount, sizeof(mi_ppx));
		if (l_cp->ppm_markers == NULL) {
			mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to read PPM marker\n");
			return mi_FALSE;
		}
		l_cp->ppm_markers_count = l_newCount;
	} else if (l_cp->ppm_markers_count <= l_Z_ppm) {
		mi_UINT32 l_newCount = l_Z_ppm + 1U; /* can't overflow, l_Z_ppm is UINT8 */
		mi_ppx *new_ppm_markers;
		new_ppm_markers = (mi_ppx *) mi_realloc(l_cp->ppm_markers, l_newCount * sizeof(mi_ppx));
		if (new_ppm_markers == NULL) {
			/* clean up to be done on l_cp destruction */
			mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to read PPM marker\n");
			return mi_FALSE;
		}
		l_cp->ppm_markers = new_ppm_markers;
		memset(l_cp->ppm_markers + l_cp->ppm_markers_count, 0, (l_newCount - l_cp->ppm_markers_count) * sizeof(mi_ppx));
		l_cp->ppm_markers_count = l_newCount;
	}
	
	if (l_cp->ppm_markers[l_Z_ppm].m_data != NULL) {
		/* clean up to be done on l_cp destruction */
		mi_event_msg(p_manager, EVT_ERROR, "Zppm %u already read\n", l_Z_ppm);
		return mi_FALSE;
	}
	
	l_cp->ppm_markers[l_Z_ppm].m_data = (mi_BYTE *) mi_malloc(p_header_size);
	if (l_cp->ppm_markers[l_Z_ppm].m_data == NULL) {
		/* clean up to be done on l_cp destruction */
		mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to read PPM marker\n");
		return mi_FALSE;
	}
	l_cp->ppm_markers[l_Z_ppm].m_data_size = p_header_size;
	memcpy(l_cp->ppm_markers[l_Z_ppm].m_data, p_header_data, p_header_size);

	return mi_TRUE;
}

/**
 * Merges all PPM markers read (Packed headers, main header)
 *
 * @param       p_cp      main coding parameters.
 * @param       p_manager the user event manager.
 */
static mi_BOOL mi_j2k_merge_ppm ( mi_cp_t *p_cp, mi_event_mgr_t * p_manager )
{
	mi_UINT32 i, l_ppm_data_size, l_N_ppm_remaining;
	
	/* preconditions */
	assert(p_cp != 00);
	assert(p_manager != 00);
	assert(p_cp->ppm_buffer == NULL);
	
	if (p_cp->ppm == 0U) {
		return mi_TRUE;
	}
	
	l_ppm_data_size = 0U;
	l_N_ppm_remaining = 0U;
	for (i = 0U; i < p_cp->ppm_markers_count; ++i) {
		if (p_cp->ppm_markers[i].m_data != NULL) { /* standard doesn't seem to require contiguous Zppm */
			mi_UINT32 l_N_ppm;
			mi_UINT32 l_data_size = p_cp->ppm_markers[i].m_data_size;
			const mi_BYTE* l_data = p_cp->ppm_markers[i].m_data;
			
			if (l_N_ppm_remaining >= l_data_size) {
				l_N_ppm_remaining -= l_data_size;
				l_data_size = 0U;
			} else {
				l_data += l_N_ppm_remaining;
				l_data_size -= l_N_ppm_remaining;
				l_N_ppm_remaining = 0U;
			}
			
			if (l_data_size > 0U) {
				do
				{
					/* read Nppm */
					if (l_data_size < 4U) {
						/* clean up to be done on l_cp destruction */
						mi_event_msg(p_manager, EVT_ERROR, "Not enough bytes to read Nppm\n");
						return mi_FALSE;
					}
					mi_read_bytes(l_data, &l_N_ppm, 4);
					l_data+=4;
					l_data_size-=4;
					l_ppm_data_size += l_N_ppm; /* can't overflow, max 256 markers of max 65536 bytes, that is when PPM markers are not corrupted which is checked elsewhere */
					
					if (l_data_size >= l_N_ppm) {
						l_data_size -= l_N_ppm;
						l_data += l_N_ppm;
					} else {
						l_N_ppm_remaining = l_N_ppm - l_data_size;
						l_data_size = 0U;
					}
				} while (l_data_size > 0U);
			}
		}
	}
	
	if (l_N_ppm_remaining != 0U) {
		/* clean up to be done on l_cp destruction */
		mi_event_msg(p_manager, EVT_ERROR, "Corrupted PPM markers\n");
		return mi_FALSE;
	}
	
	p_cp->ppm_buffer = (mi_BYTE *) mi_malloc(l_ppm_data_size);
	if (p_cp->ppm_buffer == 00) {
		mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to read PPM marker\n");
		return mi_FALSE;
	}
	p_cp->ppm_len = l_ppm_data_size;
	l_ppm_data_size = 0U;
	l_N_ppm_remaining = 0U;
	for (i = 0U; i < p_cp->ppm_markers_count; ++i) {
		if (p_cp->ppm_markers[i].m_data != NULL) { /* standard doesn't seem to require contiguous Zppm */
			mi_UINT32 l_N_ppm;
			mi_UINT32 l_data_size = p_cp->ppm_markers[i].m_data_size;
			const mi_BYTE* l_data = p_cp->ppm_markers[i].m_data;
			
			if (l_N_ppm_remaining >= l_data_size) {
				memcpy(p_cp->ppm_buffer + l_ppm_data_size, l_data, l_data_size);
				l_ppm_data_size += l_data_size;
				l_N_ppm_remaining -= l_data_size;
				l_data_size = 0U;
			} else {
				memcpy(p_cp->ppm_buffer + l_ppm_data_size, l_data, l_N_ppm_remaining);
				l_ppm_data_size += l_N_ppm_remaining;
				l_data += l_N_ppm_remaining;
				l_data_size -= l_N_ppm_remaining;
				l_N_ppm_remaining = 0U;
			}

			if (l_data_size > 0U) {
				do
				{
					/* read Nppm */
					if (l_data_size < 4U) {
						/* clean up to be done on l_cp destruction */
						mi_event_msg(p_manager, EVT_ERROR, "Not enough bytes to read Nppm\n");
						return mi_FALSE;
					}
					mi_read_bytes(l_data, &l_N_ppm, 4);
					l_data+=4;
					l_data_size-=4;
					
					if (l_data_size >= l_N_ppm) {
						memcpy(p_cp->ppm_buffer + l_ppm_data_size, l_data, l_N_ppm);
						l_ppm_data_size += l_N_ppm;
						l_data_size -= l_N_ppm;
						l_data += l_N_ppm;
					} else {
						memcpy(p_cp->ppm_buffer + l_ppm_data_size, l_data, l_data_size);
						l_ppm_data_size += l_data_size;
						l_N_ppm_remaining = l_N_ppm - l_data_size;
						l_data_size = 0U;
					}
				} while (l_data_size > 0U);
			}
			mi_free(p_cp->ppm_markers[i].m_data);
			p_cp->ppm_markers[i].m_data = NULL;
			p_cp->ppm_markers[i].m_data_size = 0U;
		}
	}
	
	p_cp->ppm_data = p_cp->ppm_buffer;
	p_cp->ppm_data_size = p_cp->ppm_len;
	
	p_cp->ppm_markers_count = 0U;
	mi_free(p_cp->ppm_markers);
	p_cp->ppm_markers = NULL;
	
	return mi_TRUE;
}

/**
 * Reads a PPT marker (Packed packet headers, tile-part header)
 *
 * @param       p_header_data   the data contained in the PPT box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the PPT marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_ppt (  mi_j2k_t *p_j2k,
                                    mi_BYTE * p_header_data,
                                    mi_UINT32 p_header_size,
                                    mi_event_mgr_t * p_manager
                                    )
{
	mi_cp_t *l_cp = 00;
	mi_tcp_t *l_tcp = 00;
	mi_UINT32 l_Z_ppt;

	/* preconditions */
	assert(p_header_data != 00);
	assert(p_j2k != 00);
	assert(p_manager != 00);

	/* We need to have the Z_ppt element + 1 byte of Ippt at minimum */
	if (p_header_size < 2) {
		mi_event_msg(p_manager, EVT_ERROR, "Error reading PPT marker\n");
		return mi_FALSE;
	}

	l_cp = &(p_j2k->m_cp);
	if (l_cp->ppm){
		mi_event_msg(p_manager, EVT_ERROR, "Error reading PPT marker: packet header have been previously found in the main header (PPM marker).\n");
		return mi_FALSE;
	}

	l_tcp = &(l_cp->tcps[p_j2k->m_current_tile_number]);
	l_tcp->ppt = 1;

	mi_read_bytes(p_header_data,&l_Z_ppt,1);               /* Z_ppt */
	++p_header_data;
	--p_header_size;
	
	/* check allocation needed */
	if (l_tcp->ppt_markers == NULL) { /* first PPT marker */
		mi_UINT32 l_newCount = l_Z_ppt + 1U; /* can't overflow, l_Z_ppt is UINT8 */
		assert(l_tcp->ppt_markers_count == 0U);
		
		l_tcp->ppt_markers = (mi_ppx *) mi_calloc(l_newCount, sizeof(mi_ppx));
		if (l_tcp->ppt_markers == NULL) {
			mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to read PPT marker\n");
			return mi_FALSE;
		}
		l_tcp->ppt_markers_count = l_newCount;
	} else if (l_tcp->ppt_markers_count <= l_Z_ppt) {
		mi_UINT32 l_newCount = l_Z_ppt + 1U; /* can't overflow, l_Z_ppt is UINT8 */
		mi_ppx *new_ppt_markers;
		new_ppt_markers = (mi_ppx *) mi_realloc(l_tcp->ppt_markers, l_newCount * sizeof(mi_ppx));
		if (new_ppt_markers == NULL) {
			/* clean up to be done on l_tcp destruction */
			mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to read PPT marker\n");
			return mi_FALSE;
		}
		l_tcp->ppt_markers = new_ppt_markers;
		memset(l_tcp->ppt_markers + l_tcp->ppt_markers_count, 0, (l_newCount - l_tcp->ppt_markers_count) * sizeof(mi_ppx));
		l_tcp->ppt_markers_count = l_newCount;
	}
	
	if (l_tcp->ppt_markers[l_Z_ppt].m_data != NULL) {
		/* clean up to be done on l_tcp destruction */
		mi_event_msg(p_manager, EVT_ERROR, "Zppt %u already read\n", l_Z_ppt);
		return mi_FALSE;
	}
	
	l_tcp->ppt_markers[l_Z_ppt].m_data = (mi_BYTE *) mi_malloc(p_header_size);
	if (l_tcp->ppt_markers[l_Z_ppt].m_data == NULL) {
		/* clean up to be done on l_tcp destruction */
		mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to read PPT marker\n");
		return mi_FALSE;
	}
	l_tcp->ppt_markers[l_Z_ppt].m_data_size = p_header_size;
	memcpy(l_tcp->ppt_markers[l_Z_ppt].m_data, p_header_data, p_header_size);
	return mi_TRUE;
}

/**
 * Merges all PPT markers read (Packed packet headers, tile-part header)
 *
 * @param       p_tcp   the tile.
 * @param       p_manager               the user event manager.
 */
static mi_BOOL mi_j2k_merge_ppt(mi_tcp_t *p_tcp, mi_event_mgr_t * p_manager)
{
	mi_UINT32 i, l_ppt_data_size;
	/* preconditions */
	assert(p_tcp != 00);
	assert(p_manager != 00);
	assert(p_tcp->ppt_buffer == NULL);
	
	if (p_tcp->ppt == 0U) {
		return mi_TRUE;
	}
	
	l_ppt_data_size = 0U;
	for (i = 0U; i < p_tcp->ppt_markers_count; ++i) {
		l_ppt_data_size += p_tcp->ppt_markers[i].m_data_size; /* can't overflow, max 256 markers of max 65536 bytes */
	}
	
	p_tcp->ppt_buffer = (mi_BYTE *) mi_malloc(l_ppt_data_size);
	if (p_tcp->ppt_buffer == 00) {
		mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to read PPT marker\n");
		return mi_FALSE;
	}
	p_tcp->ppt_len = l_ppt_data_size;
	l_ppt_data_size = 0U;
	for (i = 0U; i < p_tcp->ppt_markers_count; ++i) {
		if (p_tcp->ppt_markers[i].m_data != NULL) { /* standard doesn't seem to require contiguous Zppt */
			memcpy(p_tcp->ppt_buffer + l_ppt_data_size, p_tcp->ppt_markers[i].m_data, p_tcp->ppt_markers[i].m_data_size);
			l_ppt_data_size += p_tcp->ppt_markers[i].m_data_size; /* can't overflow, max 256 markers of max 65536 bytes */
			
			mi_free(p_tcp->ppt_markers[i].m_data);
			p_tcp->ppt_markers[i].m_data = NULL;
			p_tcp->ppt_markers[i].m_data_size = 0U;
		}
	}
	
	p_tcp->ppt_markers_count = 0U;
	mi_free(p_tcp->ppt_markers);
	p_tcp->ppt_markers = NULL;
	
	p_tcp->ppt_data = p_tcp->ppt_buffer;
	p_tcp->ppt_data_size = p_tcp->ppt_len;
	return mi_TRUE;
}

static mi_BOOL mi_j2k_write_tlm(     mi_j2k_t *p_j2k,
                                                        mi_stream_private_t *p_stream,
                                                        mi_event_mgr_t * p_manager
                            )
{
        mi_BYTE * l_current_data = 00;
        mi_UINT32 l_tlm_size;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        l_tlm_size = 6 + (5*p_j2k->m_specific_param.m_encoder.m_total_tile_parts);

        if (l_tlm_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
                mi_BYTE *new_header_tile_data = (mi_BYTE *) mi_realloc(p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_tlm_size);
                if (! new_header_tile_data) {
                        mi_free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to write TLM marker\n");
                        return mi_FALSE;
                }
                p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
                p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_tlm_size;
        }

        l_current_data = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

        /* change the way data is written to avoid seeking if possible */
        /* TODO */
        p_j2k->m_specific_param.m_encoder.m_tlm_start = mi_stream_tell(p_stream);

        mi_write_bytes(l_current_data,J2K_MS_TLM,2);                                   /* TLM */
        l_current_data += 2;

        mi_write_bytes(l_current_data,l_tlm_size-2,2);                                 /* Lpoc */
        l_current_data += 2;

        mi_write_bytes(l_current_data,0,1);                                                    /* Ztlm=0*/
        ++l_current_data;

        mi_write_bytes(l_current_data,0x50,1);                                                 /* Stlm ST=1(8bits-255 tiles max),SP=1(Ptlm=32bits) */
        ++l_current_data;

        /* do nothing on the 5 * l_j2k->m_specific_param.m_encoder.m_total_tile_parts remaining data */
        if (mi_stream_write_data(p_stream,p_j2k->m_specific_param.m_encoder.m_header_tile_data,l_tlm_size,p_manager) != l_tlm_size) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_sot(     mi_j2k_t *p_j2k,
                                                        mi_BYTE * p_data,
                                                        mi_UINT32 * p_data_written,
                                                        const mi_stream_private_t *p_stream,
                                                        mi_event_mgr_t * p_manager
                            )
{
        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        mi_write_bytes(p_data,J2K_MS_SOT,2);                                   /* SOT */
        p_data += 2;

        mi_write_bytes(p_data,10,2);                                                   /* Lsot */
        p_data += 2;

        mi_write_bytes(p_data, p_j2k->m_current_tile_number,2);                        /* Isot */
        p_data += 2;

        /* Psot  */
        p_data += 4;

        mi_write_bytes(p_data, p_j2k->m_specific_param.m_encoder.m_current_tile_part_number,1);                        /* TPsot */
        ++p_data;

        mi_write_bytes(p_data, p_j2k->m_cp.tcps[p_j2k->m_current_tile_number].m_nb_tile_parts,1);                      /* TNsot */
        ++p_data;

        * p_data_written = 12;

        return mi_TRUE;
}

static mi_BOOL mi_j2k_get_sot_values(mi_BYTE *  p_header_data,
																			 mi_UINT32  p_header_size,
																			 mi_UINT32* p_tile_no,
																			 mi_UINT32* p_tot_len,
																			 mi_UINT32* p_current_part,
																			 mi_UINT32* p_num_parts,
																			 mi_event_mgr_t * p_manager )
{
	/* preconditions */
	assert(p_header_data != 00);
	assert(p_manager != 00);
	
	/* Size of this marker is fixed = 12 (we have already read marker and its size)*/
	if (p_header_size != 8) {
		mi_event_msg(p_manager, EVT_ERROR, "Error reading SOT marker\n");
		return mi_FALSE;
	}
	
	mi_read_bytes(p_header_data,p_tile_no,2);      /* Isot */
	p_header_data+=2;
	mi_read_bytes(p_header_data,p_tot_len,4);      /* Psot */
	p_header_data+=4;
	mi_read_bytes(p_header_data,p_current_part,1); /* TPsot */
	++p_header_data;
	mi_read_bytes(p_header_data,p_num_parts ,1);   /* TNsot */
	++p_header_data;
	return mi_TRUE;
}

static mi_BOOL mi_j2k_read_sot ( mi_j2k_t *p_j2k,
                            mi_BYTE * p_header_data,
                            mi_UINT32 p_header_size,
                            mi_event_mgr_t * p_manager )
{
        mi_cp_t *l_cp = 00;
        mi_tcp_t *l_tcp = 00;
        mi_UINT32 l_tot_len, l_num_parts = 0;
        mi_UINT32 l_current_part;
        mi_UINT32 l_tile_x,l_tile_y;

        /* preconditions */
	
        assert(p_j2k != 00);
        assert(p_manager != 00);
	
        if (! mi_j2k_get_sot_values(p_header_data, p_header_size, &(p_j2k->m_current_tile_number), &l_tot_len, &l_current_part, &l_num_parts, p_manager)) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading SOT marker\n");
                return mi_FALSE;
        }

        l_cp = &(p_j2k->m_cp);

        /* testcase 2.pdf.SIGFPE.706.1112 */
        if (p_j2k->m_current_tile_number >= l_cp->tw * l_cp->th) {
                mi_event_msg(p_manager, EVT_ERROR, "Invalid tile number %d\n", p_j2k->m_current_tile_number);
                return mi_FALSE;
        }

        l_tcp = &l_cp->tcps[p_j2k->m_current_tile_number];
        l_tile_x = p_j2k->m_current_tile_number % l_cp->tw;
        l_tile_y = p_j2k->m_current_tile_number / l_cp->tw;

        /* look for the tile in the list of already processed tile (in parts). */
        /* Optimization possible here with a more complex data structure and with the removing of tiles */
        /* since the time taken by this function can only grow at the time */

        /* PSot should be equal to zero or >=14 or <= 2^32-1 */
        if ((l_tot_len !=0 ) && (l_tot_len < 14) )
        {
            if (l_tot_len == 12 ) /* MSD: Special case for the PHR data which are read by kakadu*/
            {
                mi_event_msg(p_manager, EVT_WARNING, "Empty SOT marker detected: Psot=%d.\n", l_tot_len);
            }
            else
            {
                mi_event_msg(p_manager, EVT_ERROR, "Psot value is not correct regards to the JPEG2000 norm: %d.\n", l_tot_len);
                return mi_FALSE;
            }
        }

                /* Ref A.4.2: Psot could be equal zero if it is the last tile-part of the codestream.*/
                if (!l_tot_len) {
                        mi_event_msg(p_manager, EVT_INFO, "Psot value of the current tile-part is equal to zero, "
                                        "we assuming it is the last tile-part of the codestream.\n");
                        p_j2k->m_specific_param.m_decoder.m_last_tile_part = 1;
                }

                if (l_num_parts != 0) { /* Number of tile-part header is provided by this tile-part header */
                        l_num_parts += p_j2k->m_specific_param.m_decoder.m_nb_tile_parts_correction;
                        /* Useful to manage the case of textGBR.jp2 file because two values of TNSot are allowed: the correct numbers of
                         * tile-parts for that tile and zero (A.4.2 of 15444-1 : 2002). */
                        if (l_tcp->m_nb_tile_parts) {
                                if (l_current_part >= l_tcp->m_nb_tile_parts){
                                        mi_event_msg(p_manager, EVT_ERROR, "In SOT marker, TPSot (%d) is not valid regards to the current "
                                                        "number of tile-part (%d), giving up\n", l_current_part, l_tcp->m_nb_tile_parts );
                                        p_j2k->m_specific_param.m_decoder.m_last_tile_part = 1;
                                        return mi_FALSE;
                                }
                        }
                        if( l_current_part >= l_num_parts ) {
                          /* testcase 451.pdf.SIGSEGV.ce9.3723 */
                          mi_event_msg(p_manager, EVT_ERROR, "In SOT marker, TPSot (%d) is not valid regards to the current "
                            "number of tile-part (header) (%d), giving up\n", l_current_part, l_num_parts );
                          p_j2k->m_specific_param.m_decoder.m_last_tile_part = 1;
                          return mi_FALSE;
                        }
                        l_tcp->m_nb_tile_parts = l_num_parts;
                }

                /* If know the number of tile part header we will check if we didn't read the last*/
                if (l_tcp->m_nb_tile_parts) {
                        if (l_tcp->m_nb_tile_parts == (l_current_part+1)) {
                                p_j2k->m_specific_param.m_decoder.m_can_decode = 1; /* Process the last tile-part header*/
                        }
                }

                if (!p_j2k->m_specific_param.m_decoder.m_last_tile_part){
                        /* Keep the size of data to skip after this marker */
                        p_j2k->m_specific_param.m_decoder.m_sot_length = l_tot_len - 12; /* SOT_marker_size = 12 */
                }
                else {
                        /* FIXME: need to be computed from the number of bytes remaining in the codestream */
                        p_j2k->m_specific_param.m_decoder.m_sot_length = 0;
                }

                p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_TPH;

                /* Check if the current tile is outside the area we want decode or not corresponding to the tile index*/
                if (p_j2k->m_specific_param.m_decoder.m_tile_ind_to_dec == -1) {
                        p_j2k->m_specific_param.m_decoder.m_skip_data =
                                (l_tile_x < p_j2k->m_specific_param.m_decoder.m_start_tile_x)
                                ||      (l_tile_x >= p_j2k->m_specific_param.m_decoder.m_end_tile_x)
                                ||  (l_tile_y < p_j2k->m_specific_param.m_decoder.m_start_tile_y)
                                ||      (l_tile_y >= p_j2k->m_specific_param.m_decoder.m_end_tile_y);
                }
                else {
                        assert( p_j2k->m_specific_param.m_decoder.m_tile_ind_to_dec >= 0 );
                        p_j2k->m_specific_param.m_decoder.m_skip_data =
                                (p_j2k->m_current_tile_number != (mi_UINT32)p_j2k->m_specific_param.m_decoder.m_tile_ind_to_dec);
                }

                /* Index */
                if (p_j2k->cstr_index)
                {
                        assert(p_j2k->cstr_index->tile_index != 00);
                        p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].tileno = p_j2k->m_current_tile_number;
                        p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].current_tpsno = l_current_part;

                        if (l_num_parts != 0){
                                p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].nb_tps = l_num_parts;
                                p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].current_nb_tps = l_num_parts;

                                if (!p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].tp_index) {
                                        p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].tp_index =
                                                (mi_tp_index_t*)mi_calloc(l_num_parts, sizeof(mi_tp_index_t));
                                        if (!p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].tp_index) {
                                                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to read SOT marker. Tile index allocation failed\n");
                                                return mi_FALSE;
																				}
                                }
                                else {
                                        mi_tp_index_t *new_tp_index = (mi_tp_index_t *) mi_realloc(
                                                        p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].tp_index, l_num_parts* sizeof(mi_tp_index_t));
                                        if (! new_tp_index) {
                                                mi_free(p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].tp_index);
                                                p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].tp_index = NULL;
                                                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to read SOT marker. Tile index allocation failed\n");
                                                return mi_FALSE;
                                        }
                                        p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].tp_index = new_tp_index;
                                }
                        }
                        else{
                                /*if (!p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].tp_index)*/ {

                                        if (!p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].tp_index) {
                                                p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].current_nb_tps = 10;
                                                p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].tp_index =
                                                        (mi_tp_index_t*)mi_calloc( p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].current_nb_tps,
                                                                        sizeof(mi_tp_index_t));
																								if (!p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].tp_index) {
                                                        p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].current_nb_tps = 0;
                                                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to read SOT marker. Tile index allocation failed\n");
                                                        return mi_FALSE;
																				        }
                                        }

                                        if ( l_current_part >= p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].current_nb_tps ){
                                                mi_tp_index_t *new_tp_index;
                                                p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].current_nb_tps = l_current_part + 1;
                                                new_tp_index = (mi_tp_index_t *) mi_realloc(
                                                                p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].tp_index,
                                                                p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].current_nb_tps * sizeof(mi_tp_index_t));
                                                if (! new_tp_index) {
                                                        mi_free(p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].tp_index);
                                                        p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].tp_index = NULL;
                                                        p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].current_nb_tps = 0;
                                                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to read SOT marker. Tile index allocation failed\n");
                                                        return mi_FALSE;
                                                }
                                                p_j2k->cstr_index->tile_index[p_j2k->m_current_tile_number].tp_index = new_tp_index;
                                        }
                                }

                        }

                }

                /* FIXME move this onto a separate method to call before reading any SOT, remove part about main_end header, use a index struct inside p_j2k */
                /* if (p_j2k->cstr_info) {
                   if (l_tcp->first) {
                   if (tileno == 0) {
                   p_j2k->cstr_info->main_head_end = p_stream_tell(p_stream) - 13;
                   }

                   p_j2k->cstr_info->tile[tileno].tileno = tileno;
                   p_j2k->cstr_info->tile[tileno].start_pos = p_stream_tell(p_stream) - 12;
                   p_j2k->cstr_info->tile[tileno].end_pos = p_j2k->cstr_info->tile[tileno].start_pos + totlen - 1;
                   p_j2k->cstr_info->tile[tileno].num_tps = numparts;

                   if (numparts) {
                   p_j2k->cstr_info->tile[tileno].tp = (mi_tp_info_t *) mi_malloc(numparts * sizeof(mi_tp_info_t));
                   }
                   else {
                   p_j2k->cstr_info->tile[tileno].tp = (mi_tp_info_t *) mi_malloc(10 * sizeof(mi_tp_info_t)); // Fixme (10)
                   }
                   }
                   else {
                   p_j2k->cstr_info->tile[tileno].end_pos += totlen;
                   }

                   p_j2k->cstr_info->tile[tileno].tp[partno].tp_start_pos = p_stream_tell(p_stream) - 12;
                   p_j2k->cstr_info->tile[tileno].tp[partno].tp_end_pos =
                   p_j2k->cstr_info->tile[tileno].tp[partno].tp_start_pos + totlen - 1;
                   }*/
                return mi_TRUE;
        }

static mi_BOOL mi_j2k_write_sod(     mi_j2k_t *p_j2k,
                                                        mi_tcd_t * p_tile_coder,
                                                        mi_BYTE * p_data,
                                                        mi_UINT32 * p_data_written,
                                                        mi_UINT32 p_total_data_size,
                                                        const mi_stream_private_t *p_stream,
                                                        mi_event_mgr_t * p_manager
                            )
{
        mi_codestream_info_t *l_cstr_info = 00;
        mi_UINT32 l_remaining_data;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        mi_write_bytes(p_data,J2K_MS_SOD,2);                                   /* SOD */
        p_data += 2;

        /* make room for the EOF marker */
        l_remaining_data =  p_total_data_size - 4;

        /* update tile coder */
        p_tile_coder->tp_num = p_j2k->m_specific_param.m_encoder.m_current_poc_tile_part_number ;
        p_tile_coder->cur_tp_num = p_j2k->m_specific_param.m_encoder.m_current_tile_part_number;

         /* INDEX >> */
        /* TODO mergeV2: check this part which use cstr_info */
        /*l_cstr_info = p_j2k->cstr_info;
        if (l_cstr_info) {
                if (!p_j2k->m_specific_param.m_encoder.m_current_tile_part_number ) {
                        //TODO cstr_info->tile[p_j2k->m_current_tile_number].end_header = p_stream_tell(p_stream) + p_j2k->pos_correction - 1;
                        l_cstr_info->tile[p_j2k->m_current_tile_number].tileno = p_j2k->m_current_tile_number;
                }
                else {*/
                        /*
                        TODO
                        if
                                (cstr_info->tile[p_j2k->m_current_tile_number].packet[cstr_info->packno - 1].end_pos < p_stream_tell(p_stream))
                        {
                                cstr_info->tile[p_j2k->m_current_tile_number].packet[cstr_info->packno].start_pos = p_stream_tell(p_stream);
                        }*/
                /*}*/
        /*}*/
        /* << INDEX */

        if (p_j2k->m_specific_param.m_encoder.m_current_tile_part_number == 0) {
                p_tile_coder->tcd_image->tiles->packno = 0;
                if (l_cstr_info) {
                        l_cstr_info->packno = 0;
                }
        }

        *p_data_written = 0;

        if (! mi_tcd_encode_tile(p_tile_coder, p_j2k->m_current_tile_number, p_data, p_data_written, l_remaining_data , l_cstr_info)) {
                mi_event_msg(p_manager, EVT_ERROR, "Cannot encode tile\n");
                return mi_FALSE;
        }

        *p_data_written += 2;

        return mi_TRUE;
}

static mi_BOOL mi_j2k_read_sod (mi_j2k_t *p_j2k,
                           mi_stream_private_t *p_stream,
                                                   mi_event_mgr_t * p_manager
                           )
{
        mi_SIZE_T l_current_read_size;
        mi_codestream_index_t * l_cstr_index = 00;
        mi_BYTE ** l_current_data = 00;
        mi_tcp_t * l_tcp = 00;
        mi_UINT32 * l_tile_len = 00;
        mi_BOOL l_sot_length_pb_detected = mi_FALSE;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        l_tcp = &(p_j2k->m_cp.tcps[p_j2k->m_current_tile_number]);

        if (p_j2k->m_specific_param.m_decoder.m_last_tile_part) {
                /* mi_stream_get_number_byte_left returns mi_OFF_T
                // but we are in the last tile part,
                // so its result will fit on mi_UINT32 unless we find
                // a file with a single tile part of more than 4 GB...*/
                p_j2k->m_specific_param.m_decoder.m_sot_length = (mi_UINT32)(mi_stream_get_number_byte_left(p_stream) - 2);
        }
        else {
            /* Check to avoid pass the limit of mi_UINT32 */
            if (p_j2k->m_specific_param.m_decoder.m_sot_length >= 2 )
                p_j2k->m_specific_param.m_decoder.m_sot_length -= 2;
            else {
                /* MSD: case commented to support empty SOT marker (PHR data) */
            }
        }

        l_current_data = &(l_tcp->m_data);
        l_tile_len = &l_tcp->m_data_size;

        /* Patch to support new PHR data */
        if (p_j2k->m_specific_param.m_decoder.m_sot_length) {
            /* If we are here, we'll try to read the data after allocation */
            /* Check enough bytes left in stream before allocation */
            if ((mi_OFF_T)p_j2k->m_specific_param.m_decoder.m_sot_length > mi_stream_get_number_byte_left(p_stream)) {
                mi_event_msg(p_manager, EVT_ERROR, "Tile part length size inconsistent with stream length\n");
                return mi_FALSE;
            }
            if (! *l_current_data) {
                /* LH: oddly enough, in this path, l_tile_len!=0.
                 * TODO: If this was consistent, we could simplify the code to only use realloc(), as realloc(0,...) default to malloc(0,...).
                 */
                *l_current_data = (mi_BYTE*) mi_malloc(p_j2k->m_specific_param.m_decoder.m_sot_length);
            }
            else {
                mi_BYTE *l_new_current_data = (mi_BYTE *) mi_realloc(*l_current_data, *l_tile_len + p_j2k->m_specific_param.m_decoder.m_sot_length);
                if (! l_new_current_data) {
                        mi_free(*l_current_data);
                        /*nothing more is done as l_current_data will be set to null, and just
                          afterward we enter in the error path
                          and the actual tile_len is updated (committed) at the end of the
                          function. */
                }
                *l_current_data = l_new_current_data;
            }
            
            if (*l_current_data == 00) {
                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to decode tile\n");
                return mi_FALSE;
            }
        }
        else {
            l_sot_length_pb_detected = mi_TRUE;
        }

        /* Index */
        l_cstr_index = p_j2k->cstr_index;
        if (l_cstr_index) {
                mi_OFF_T l_current_pos = mi_stream_tell(p_stream) - 2;

                mi_UINT32 l_current_tile_part = l_cstr_index->tile_index[p_j2k->m_current_tile_number].current_tpsno;
                l_cstr_index->tile_index[p_j2k->m_current_tile_number].tp_index[l_current_tile_part].end_header =
                                l_current_pos;
                l_cstr_index->tile_index[p_j2k->m_current_tile_number].tp_index[l_current_tile_part].end_pos =
                                l_current_pos + p_j2k->m_specific_param.m_decoder.m_sot_length + 2;

                if (mi_FALSE == mi_j2k_add_tlmarker(p_j2k->m_current_tile_number,
                                        l_cstr_index,
                                        J2K_MS_SOD,
                                        l_current_pos,
                                        p_j2k->m_specific_param.m_decoder.m_sot_length + 2)) {
                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to add tl marker\n");
                        return mi_FALSE;
                }

                /*l_cstr_index->packno = 0;*/
        }

        /* Patch to support new PHR data */
        if (!l_sot_length_pb_detected) {
            l_current_read_size = mi_stream_read_data(
                        p_stream,
                        *l_current_data + *l_tile_len,
                        p_j2k->m_specific_param.m_decoder.m_sot_length,
                        p_manager);
        }
        else
        {
            l_current_read_size = 0;
        }

        if (l_current_read_size != p_j2k->m_specific_param.m_decoder.m_sot_length) {
                p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_NEOC;
        }
        else {
                p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_TPHSOT;
        }

        *l_tile_len += (mi_UINT32)l_current_read_size;

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_rgn(mi_j2k_t *p_j2k,
                            mi_UINT32 p_tile_no,
                            mi_UINT32 p_comp_no,
                            mi_UINT32 nb_comps,
                            mi_stream_private_t *p_stream,
                            mi_event_mgr_t * p_manager
                            )
{
        mi_BYTE * l_current_data = 00;
        mi_UINT32 l_rgn_size;
        mi_cp_t *l_cp = 00;
        mi_tcp_t *l_tcp = 00;
        mi_tccp_t *l_tccp = 00;
        mi_UINT32 l_comp_room;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        l_cp = &(p_j2k->m_cp);
        l_tcp = &l_cp->tcps[p_tile_no];
        l_tccp = &l_tcp->tccps[p_comp_no];

        if (nb_comps <= 256) {
                l_comp_room = 1;
        }
        else {
                l_comp_room = 2;
        }

        l_rgn_size = 6 + l_comp_room;

        l_current_data = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

        mi_write_bytes(l_current_data,J2K_MS_RGN,2);                                   /* RGN  */
        l_current_data += 2;

        mi_write_bytes(l_current_data,l_rgn_size-2,2);                                 /* Lrgn */
        l_current_data += 2;

        mi_write_bytes(l_current_data,p_comp_no,l_comp_room);                          /* Crgn */
        l_current_data+=l_comp_room;

        mi_write_bytes(l_current_data, 0,1);                                           /* Srgn */
        ++l_current_data;

        mi_write_bytes(l_current_data, (mi_UINT32)l_tccp->roishift,1);                            /* SPrgn */
        ++l_current_data;

        if (mi_stream_write_data(p_stream,p_j2k->m_specific_param.m_encoder.m_header_tile_data,l_rgn_size,p_manager) != l_rgn_size) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_eoc(     mi_j2k_t *p_j2k,
                            mi_stream_private_t *p_stream,
                            mi_event_mgr_t * p_manager
                            )
{
        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        mi_write_bytes(p_j2k->m_specific_param.m_encoder.m_header_tile_data,J2K_MS_EOC,2);                                     /* EOC */

        if ( mi_stream_write_data(p_stream,p_j2k->m_specific_param.m_encoder.m_header_tile_data,2,p_manager) != 2) {
                return mi_FALSE;
        }

        if ( ! mi_stream_flush(p_stream,p_manager) ) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

/**
 * Reads a RGN marker (Region Of Interest)
 *
 * @param       p_header_data   the data contained in the POC box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the POC marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_rgn (mi_j2k_t *p_j2k,
                                  mi_BYTE * p_header_data,
                                  mi_UINT32 p_header_size,
                                  mi_event_mgr_t * p_manager
                                  )
{
        mi_UINT32 l_nb_comp;
        mi_image_t * l_image = 00;

        mi_cp_t *l_cp = 00;
        mi_tcp_t *l_tcp = 00;
        mi_UINT32 l_comp_room, l_comp_no, l_roi_sty;

        /* preconditions*/
        assert(p_header_data != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        l_image = p_j2k->m_private_image;
        l_nb_comp = l_image->numcomps;

        if (l_nb_comp <= 256) {
                l_comp_room = 1; }
        else {
                l_comp_room = 2; }

        if (p_header_size != 2 + l_comp_room) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading RGN marker\n");
                return mi_FALSE;
        }

        l_cp = &(p_j2k->m_cp);
        l_tcp = (p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_TPH) ?
                                &l_cp->tcps[p_j2k->m_current_tile_number] :
                                p_j2k->m_specific_param.m_decoder.m_default_tcp;

        mi_read_bytes(p_header_data,&l_comp_no,l_comp_room);           /* Crgn */
        p_header_data+=l_comp_room;
        mi_read_bytes(p_header_data,&l_roi_sty,1);                                     /* Srgn */
        ++p_header_data;

        /* testcase 3635.pdf.asan.77.2930 */
        if (l_comp_no >= l_nb_comp) {
                mi_event_msg(p_manager, EVT_ERROR,
                        "bad component number in RGN (%d when there are only %d)\n",
                        l_comp_no, l_nb_comp);
                return mi_FALSE;
        }

        mi_read_bytes(p_header_data,(mi_UINT32 *) (&(l_tcp->tccps[l_comp_no].roishift)),1);   /* SPrgn */
        ++p_header_data;

        return mi_TRUE;

}

static mi_FLOAT32 mi_j2k_get_tp_stride (mi_tcp_t * p_tcp)
{
        return (mi_FLOAT32) ((p_tcp->m_nb_tile_parts - 1) * 14);
}

static mi_FLOAT32 mi_j2k_get_default_stride (mi_tcp_t * p_tcp)
{
    (void)p_tcp;
    return 0;
}

static mi_BOOL mi_j2k_update_rates(  mi_j2k_t *p_j2k,
                                                            mi_stream_private_t *p_stream,
                                                            mi_event_mgr_t * p_manager )
{
        mi_cp_t * l_cp = 00;
        mi_image_t * l_image = 00;
        mi_tcp_t * l_tcp = 00;
        mi_image_comp_t * l_img_comp = 00;

        mi_UINT32 i,j,k;
        mi_INT32 l_x0,l_y0,l_x1,l_y1;
        mi_FLOAT32 * l_rates = 0;
        mi_FLOAT32 l_sot_remove;
        mi_UINT32 l_bits_empty, l_size_pixel;
        mi_UINT32 l_tile_size = 0;
        mi_UINT32 l_last_res;
        mi_FLOAT32 (* l_tp_stride_func)(mi_tcp_t *) = 00;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        l_cp = &(p_j2k->m_cp);
        l_image = p_j2k->m_private_image;
        l_tcp = l_cp->tcps;

        l_bits_empty = 8 * l_image->comps->dx * l_image->comps->dy;
        l_size_pixel = l_image->numcomps * l_image->comps->prec;
        l_sot_remove = (mi_FLOAT32) mi_stream_tell(p_stream) / (mi_FLOAT32)(l_cp->th * l_cp->tw);

        if (l_cp->m_specific_param.m_enc.m_tp_on) {
                l_tp_stride_func = mi_j2k_get_tp_stride;
        }
        else {
                l_tp_stride_func = mi_j2k_get_default_stride;
        }

        for (i=0;i<l_cp->th;++i) {
                for (j=0;j<l_cp->tw;++j) {
                        mi_FLOAT32 l_offset = (mi_FLOAT32)(*l_tp_stride_func)(l_tcp) / (mi_FLOAT32)l_tcp->numlayers;

                        /* 4 borders of the tile rescale on the image if necessary */
                        l_x0 = mi_int_max((mi_INT32)(l_cp->tx0 + j * l_cp->tdx), (mi_INT32)l_image->x0);
                        l_y0 = mi_int_max((mi_INT32)(l_cp->ty0 + i * l_cp->tdy), (mi_INT32)l_image->y0);
                        l_x1 = mi_int_min((mi_INT32)(l_cp->tx0 + (j + 1) * l_cp->tdx), (mi_INT32)l_image->x1);
                        l_y1 = mi_int_min((mi_INT32)(l_cp->ty0 + (i + 1) * l_cp->tdy), (mi_INT32)l_image->y1);

                        l_rates = l_tcp->rates;

                        /* Modification of the RATE >> */
                        if (*l_rates > 0.0f) {
                                *l_rates =              (( (mi_FLOAT32) (l_size_pixel * (mi_UINT32)(l_x1 - l_x0) * (mi_UINT32)(l_y1 - l_y0)))
                                                                /
                                                                ((*l_rates) * (mi_FLOAT32)l_bits_empty)
                                                                )
                                                                -
                                                                l_offset;
                        }

                        ++l_rates;

                        for (k = 1; k < l_tcp->numlayers; ++k) {
                                if (*l_rates > 0.0f) {
                                        *l_rates =              (( (mi_FLOAT32) (l_size_pixel * (mi_UINT32)(l_x1 - l_x0) * (mi_UINT32)(l_y1 - l_y0)))
                                                                        /
                                                                                ((*l_rates) * (mi_FLOAT32)l_bits_empty)
                                                                        )
                                                                        -
                                                                        l_offset;
                                }

                                ++l_rates;
                        }

                        ++l_tcp;

                }
        }

        l_tcp = l_cp->tcps;

        for (i=0;i<l_cp->th;++i) {
                for     (j=0;j<l_cp->tw;++j) {
                        l_rates = l_tcp->rates;

                        if (*l_rates > 0.0f) {
                                *l_rates -= l_sot_remove;

                                if (*l_rates < 30.0f) {
                                        *l_rates = 30.0f;
                                }
                        }

                        ++l_rates;

                        l_last_res = l_tcp->numlayers - 1;

                        for (k = 1; k < l_last_res; ++k) {

                                if (*l_rates > 0.0f) {
                                        *l_rates -= l_sot_remove;

                                        if (*l_rates < *(l_rates - 1) + 10.0f) {
                                                *l_rates  = (*(l_rates - 1)) + 20.0f;
                                        }
                                }

                                ++l_rates;
                        }

                        if (*l_rates > 0.0f) {
                                *l_rates -= (l_sot_remove + 2.f);

                                if (*l_rates < *(l_rates - 1) + 10.0f) {
                                        *l_rates  = (*(l_rates - 1)) + 20.0f;
                                }
                        }

                        ++l_tcp;
                }
        }

        l_img_comp = l_image->comps;
        l_tile_size = 0;

        for (i=0;i<l_image->numcomps;++i) {
                l_tile_size += (        mi_uint_ceildiv(l_cp->tdx,l_img_comp->dx)
                                                        *
                                                        mi_uint_ceildiv(l_cp->tdy,l_img_comp->dy)
                                                        *
                                                        l_img_comp->prec
                                                );

                ++l_img_comp;
        }

        l_tile_size = (mi_UINT32) (l_tile_size * 0.1625); /* 1.3/8 = 0.1625 */

        l_tile_size += mi_j2k_get_specific_header_sizes(p_j2k);

        p_j2k->m_specific_param.m_encoder.m_encoded_tile_size = l_tile_size;
        p_j2k->m_specific_param.m_encoder.m_encoded_tile_data =
                        (mi_BYTE *) mi_malloc(p_j2k->m_specific_param.m_encoder.m_encoded_tile_size);
        if (p_j2k->m_specific_param.m_encoder.m_encoded_tile_data == 00) {
                return mi_FALSE;
        }

        if (mi_IS_CINEMA(l_cp->rsiz)) {
                p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer =
                                (mi_BYTE *) mi_malloc(5*p_j2k->m_specific_param.m_encoder.m_total_tile_parts);
                if (! p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer) {
                        return mi_FALSE;
                }

                p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_current =
                                p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer;
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_get_end_header(mi_j2k_t *p_j2k,
                                                        struct mi_stream_private *p_stream,
                                                        struct mi_event_mgr * p_manager )
{
        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        p_j2k->cstr_index->main_head_end = mi_stream_tell(p_stream);

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_mct_data_group(  mi_j2k_t *p_j2k,
                                                                        struct mi_stream_private *p_stream,
                                                                        struct mi_event_mgr * p_manager )
{
        mi_UINT32 i;
        mi_simple_mcc_decorrelation_data_t * l_mcc_record;
        mi_mct_data_t * l_mct_record;
        mi_tcp_t * l_tcp;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_stream != 00);
        assert(p_manager != 00);

        if (! mi_j2k_write_cbd(p_j2k,p_stream,p_manager)) {
                return mi_FALSE;
        }

        l_tcp = &(p_j2k->m_cp.tcps[p_j2k->m_current_tile_number]);
        l_mct_record = l_tcp->m_mct_records;

        for (i=0;i<l_tcp->m_nb_mct_records;++i) {

                if (! mi_j2k_write_mct_record(p_j2k,l_mct_record,p_stream,p_manager)) {
                        return mi_FALSE;
                }

                ++l_mct_record;
        }

        l_mcc_record = l_tcp->m_mcc_records;

        for     (i=0;i<l_tcp->m_nb_mcc_records;++i) {

                if (! mi_j2k_write_mcc_record(p_j2k,l_mcc_record,p_stream,p_manager)) {
                        return mi_FALSE;
                }

                ++l_mcc_record;
        }

        if (! mi_j2k_write_mco(p_j2k,p_stream,p_manager)) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_all_coc(
	mi_j2k_t *p_j2k,
	struct mi_stream_private *p_stream,
	struct mi_event_mgr * p_manager )
{
	mi_UINT32 compno;
	
	/* preconditions */
	assert(p_j2k != 00);
	assert(p_manager != 00);
	assert(p_stream != 00);
	
	for (compno = 1; compno < p_j2k->m_private_image->numcomps; ++compno)
	{
		/* cod is first component of first tile */
		if (! mi_j2k_compare_coc(p_j2k, 0, compno)) {
			if (! mi_j2k_write_coc(p_j2k,compno,p_stream, p_manager)) {
				return mi_FALSE;
			}
		}
	}
	
	return mi_TRUE;
}

static mi_BOOL mi_j2k_write_all_qcc(
	mi_j2k_t *p_j2k,
	struct mi_stream_private *p_stream,
	struct mi_event_mgr * p_manager )
{
	mi_UINT32 compno;
	
	/* preconditions */
	assert(p_j2k != 00);
	assert(p_manager != 00);
	assert(p_stream != 00);
	
	for (compno = 1; compno < p_j2k->m_private_image->numcomps; ++compno)
	{
		/* qcd is first component of first tile */
		if (! mi_j2k_compare_qcc(p_j2k, 0, compno)) {
			if (! mi_j2k_write_qcc(p_j2k,compno,p_stream, p_manager)) {
				return mi_FALSE;
			}
		}
	}
	return mi_TRUE;
}

static mi_BOOL mi_j2k_write_regions( mi_j2k_t *p_j2k,
                                                        struct mi_stream_private *p_stream,
                                                        struct mi_event_mgr * p_manager )
{
        mi_UINT32 compno;
        const mi_tccp_t *l_tccp = 00;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        l_tccp = p_j2k->m_cp.tcps->tccps;

        for (compno = 0; compno < p_j2k->m_private_image->numcomps; ++compno)  {
                if (l_tccp->roishift) {

                        if (! mi_j2k_write_rgn(p_j2k,0,compno,p_j2k->m_private_image->numcomps,p_stream,p_manager)) {
                                return mi_FALSE;
                        }
                }

                ++l_tccp;
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_epc(     mi_j2k_t *p_j2k,
                                                struct mi_stream_private *p_stream,
                                                struct mi_event_mgr * p_manager )
{
        mi_codestream_index_t * l_cstr_index = 00;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        l_cstr_index = p_j2k->cstr_index;
        if (l_cstr_index) {
                l_cstr_index->codestream_size = (mi_UINT64)mi_stream_tell(p_stream);
                /* UniPG>> */
                /* The following adjustment is done to adjust the codestream size */
                /* if SOD is not at 0 in the buffer. Useful in case of JP2, where */
                /* the first bunch of bytes is not in the codestream              */
                l_cstr_index->codestream_size -= (mi_UINT64)l_cstr_index->main_head_start;
                /* <<UniPG */
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_read_unk (     mi_j2k_t *p_j2k,
                                                        mi_stream_private_t *p_stream,
                                                        mi_UINT32 *output_marker,
                                                        mi_event_mgr_t * p_manager
                                                        )
{
        mi_UINT32 l_unknown_marker;
        const mi_dec_memory_marker_handler_t * l_marker_handler;
        mi_UINT32 l_size_unk = 2;

        /* preconditions*/
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        mi_event_msg(p_manager, EVT_WARNING, "Unknown marker\n");

		for (;;) {
                /* Try to read 2 bytes (the next marker ID) from stream and copy them into the buffer*/
                if (mi_stream_read_data(p_stream,p_j2k->m_specific_param.m_decoder.m_header_data,2,p_manager) != 2) {
                        mi_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                        return mi_FALSE;
                }

                /* read 2 bytes as the new marker ID*/
                mi_read_bytes(p_j2k->m_specific_param.m_decoder.m_header_data,&l_unknown_marker,2);

                if (!(l_unknown_marker < 0xff00)) {

                        /* Get the marker handler from the marker ID*/
                        l_marker_handler = mi_j2k_get_marker_handler(l_unknown_marker);

                        if (!(p_j2k->m_specific_param.m_decoder.m_state & l_marker_handler->states)) {
                                mi_event_msg(p_manager, EVT_ERROR, "Marker is not compliant with its position\n");
                                return mi_FALSE;
                        }
                        else {
                                if (l_marker_handler->id != J2K_MS_UNK) {
                                        /* Add the marker to the codestream index*/
                                        if (l_marker_handler->id != J2K_MS_SOT)
                                        {
                                                mi_BOOL res = mi_j2k_add_mhmarker(p_j2k->cstr_index, J2K_MS_UNK,
                                                                (mi_UINT32) mi_stream_tell(p_stream) - l_size_unk,
                                                                l_size_unk);
                                                if (res == mi_FALSE) {
                                                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to add mh marker\n");
                                                        return mi_FALSE;
                                                }
                                        }
                                        break; /* next marker is known and well located */
                                }
                                else
                                        l_size_unk += 2;
                        }
                }
        }

        *output_marker = l_marker_handler->id ;

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_mct_record(      mi_j2k_t *p_j2k,
                                                                mi_mct_data_t * p_mct_record,
                                                                struct mi_stream_private *p_stream,
                                                                struct mi_event_mgr * p_manager )
{
        mi_UINT32 l_mct_size;
        mi_BYTE * l_current_data = 00;
        mi_UINT32 l_tmp;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        l_mct_size = 10 + p_mct_record->m_data_size;

        if (l_mct_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
                mi_BYTE *new_header_tile_data = (mi_BYTE *) mi_realloc(p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_mct_size);
                if (! new_header_tile_data) {
                        mi_free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to write MCT marker\n");
                        return mi_FALSE;
                }
                p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
                p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_mct_size;
        }

        l_current_data = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

        mi_write_bytes(l_current_data,J2K_MS_MCT,2);                                   /* MCT */
        l_current_data += 2;

        mi_write_bytes(l_current_data,l_mct_size-2,2);                                 /* Lmct */
        l_current_data += 2;

        mi_write_bytes(l_current_data,0,2);                                                    /* Zmct */
        l_current_data += 2;

        /* only one marker atm */
        l_tmp = (p_mct_record->m_index & 0xff) | (p_mct_record->m_array_type << 8) | (p_mct_record->m_element_type << 10);

        mi_write_bytes(l_current_data,l_tmp,2);
        l_current_data += 2;

        mi_write_bytes(l_current_data,0,2);                                                    /* Ymct */
        l_current_data+=2;

        memcpy(l_current_data,p_mct_record->m_data,p_mct_record->m_data_size);

        if (mi_stream_write_data(p_stream,p_j2k->m_specific_param.m_encoder.m_header_tile_data,l_mct_size,p_manager) != l_mct_size) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

/**
 * Reads a MCT marker (Multiple Component Transform)
 *
 * @param       p_header_data   the data contained in the MCT box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the MCT marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_mct (      mi_j2k_t *p_j2k,
                                                                    mi_BYTE * p_header_data,
                                                                    mi_UINT32 p_header_size,
                                                                    mi_event_mgr_t * p_manager
                                    )
{
        mi_UINT32 i;
        mi_tcp_t *l_tcp = 00;
        mi_UINT32 l_tmp;
        mi_UINT32 l_indix;
        mi_mct_data_t * l_mct_data;

        /* preconditions */
        assert(p_header_data != 00);
        assert(p_j2k != 00);

        l_tcp = p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_TPH ?
                        &p_j2k->m_cp.tcps[p_j2k->m_current_tile_number] :
                        p_j2k->m_specific_param.m_decoder.m_default_tcp;

        if (p_header_size < 2) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading MCT marker\n");
                return mi_FALSE;
        }

        /* first marker */
        mi_read_bytes(p_header_data,&l_tmp,2);                         /* Zmct */
        p_header_data += 2;
        if (l_tmp != 0) {
                mi_event_msg(p_manager, EVT_WARNING, "Cannot take in charge mct data within multiple MCT records\n");
                return mi_TRUE;
        }

        if(p_header_size <= 6) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading MCT marker\n");
                return mi_FALSE;
        }

        /* Imct -> no need for other values, take the first, type is double with decorrelation x0000 1101 0000 0000*/
        mi_read_bytes(p_header_data,&l_tmp,2);                         /* Imct */
        p_header_data += 2;

        l_indix = l_tmp & 0xff;
        l_mct_data = l_tcp->m_mct_records;

        for (i=0;i<l_tcp->m_nb_mct_records;++i) {
                if (l_mct_data->m_index == l_indix) {
                        break;
                }
                ++l_mct_data;
        }

        /* NOT FOUND */
        if (i == l_tcp->m_nb_mct_records) {
                if (l_tcp->m_nb_mct_records == l_tcp->m_nb_max_mct_records) {
                        mi_mct_data_t *new_mct_records;
                        l_tcp->m_nb_max_mct_records += mi_J2K_MCT_DEFAULT_NB_RECORDS;

                        new_mct_records = (mi_mct_data_t *) mi_realloc(l_tcp->m_mct_records, l_tcp->m_nb_max_mct_records * sizeof(mi_mct_data_t));
                        if (! new_mct_records) {
                                mi_free(l_tcp->m_mct_records);
                                l_tcp->m_mct_records = NULL;
                                l_tcp->m_nb_max_mct_records = 0;
                                l_tcp->m_nb_mct_records = 0;
                                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to read MCT marker\n");
                                return mi_FALSE;
                        }
                        l_tcp->m_mct_records = new_mct_records;
                        l_mct_data = l_tcp->m_mct_records + l_tcp->m_nb_mct_records;
                        memset(l_mct_data ,0,(l_tcp->m_nb_max_mct_records - l_tcp->m_nb_mct_records) * sizeof(mi_mct_data_t));
                }

                l_mct_data = l_tcp->m_mct_records + l_tcp->m_nb_mct_records;
                ++l_tcp->m_nb_mct_records;
        }

        if (l_mct_data->m_data) {
                mi_free(l_mct_data->m_data);
                l_mct_data->m_data = 00;
        }

        l_mct_data->m_index = l_indix;
        l_mct_data->m_array_type = (J2K_MCT_ARRAY_TYPE)((l_tmp  >> 8) & 3);
        l_mct_data->m_element_type = (J2K_MCT_ELEMENT_TYPE)((l_tmp  >> 10) & 3);

        mi_read_bytes(p_header_data,&l_tmp,2);                         /* Ymct */
        p_header_data+=2;
        if (l_tmp != 0) {
                mi_event_msg(p_manager, EVT_WARNING, "Cannot take in charge multiple MCT markers\n");
                return mi_TRUE;
        }

        p_header_size -= 6;

        l_mct_data->m_data = (mi_BYTE*)mi_malloc(p_header_size);
        if (! l_mct_data->m_data) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading MCT marker\n");
                return mi_FALSE;
        }
        memcpy(l_mct_data->m_data,p_header_data,p_header_size);

        l_mct_data->m_data_size = p_header_size;

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_mcc_record(      mi_j2k_t *p_j2k,
                                                                struct mi_simple_mcc_decorrelation_data * p_mcc_record,
                                                                struct mi_stream_private *p_stream,
                                                                struct mi_event_mgr * p_manager )
{
        mi_UINT32 i;
        mi_UINT32 l_mcc_size;
        mi_BYTE * l_current_data = 00;
        mi_UINT32 l_nb_bytes_for_comp;
        mi_UINT32 l_mask;
        mi_UINT32 l_tmcc;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        if (p_mcc_record->m_nb_comps > 255 ) {
        l_nb_bytes_for_comp = 2;
                l_mask = 0x8000;
        }
        else {
                l_nb_bytes_for_comp = 1;
                l_mask = 0;
        }

        l_mcc_size = p_mcc_record->m_nb_comps * 2 * l_nb_bytes_for_comp + 19;
        if (l_mcc_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size)
        {
                mi_BYTE *new_header_tile_data = (mi_BYTE *) mi_realloc(p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_mcc_size);
                if (! new_header_tile_data) {
                        mi_free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to write MCC marker\n");
                        return mi_FALSE;
                }
                p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
                p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_mcc_size;
        }

        l_current_data = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

        mi_write_bytes(l_current_data,J2K_MS_MCC,2);                                   /* MCC */
        l_current_data += 2;

        mi_write_bytes(l_current_data,l_mcc_size-2,2);                                 /* Lmcc */
        l_current_data += 2;

        /* first marker */
        mi_write_bytes(l_current_data,0,2);                                    /* Zmcc */
        l_current_data += 2;

        mi_write_bytes(l_current_data,p_mcc_record->m_index,1);                                        /* Imcc -> no need for other values, take the first */
        ++l_current_data;

        /* only one marker atm */
        mi_write_bytes(l_current_data,0,2);                                    /* Ymcc */
        l_current_data+=2;

        mi_write_bytes(l_current_data,1,2);                                    /* Qmcc -> number of collections -> 1 */
        l_current_data+=2;

        mi_write_bytes(l_current_data,0x1,1);                                  /* Xmcci type of component transformation -> array based decorrelation */
        ++l_current_data;

        mi_write_bytes(l_current_data,p_mcc_record->m_nb_comps | l_mask,2);    /* Nmcci number of input components involved and size for each component offset = 8 bits */
        l_current_data+=2;

        for (i=0;i<p_mcc_record->m_nb_comps;++i) {
                mi_write_bytes(l_current_data,i,l_nb_bytes_for_comp);                          /* Cmccij Component offset*/
                l_current_data+=l_nb_bytes_for_comp;
        }

        mi_write_bytes(l_current_data,p_mcc_record->m_nb_comps|l_mask,2);      /* Mmcci number of output components involved and size for each component offset = 8 bits */
        l_current_data+=2;

        for (i=0;i<p_mcc_record->m_nb_comps;++i)
        {
                mi_write_bytes(l_current_data,i,l_nb_bytes_for_comp);                          /* Wmccij Component offset*/
                l_current_data+=l_nb_bytes_for_comp;
        }

        l_tmcc = ((!p_mcc_record->m_is_irreversible) & 1U) << 16;

        if (p_mcc_record->m_decorrelation_array) {
                l_tmcc |= p_mcc_record->m_decorrelation_array->m_index;
        }

        if (p_mcc_record->m_offset_array) {
                l_tmcc |= ((p_mcc_record->m_offset_array->m_index)<<8);
        }

        mi_write_bytes(l_current_data,l_tmcc,3);       /* Tmcci : use MCT defined as number 1 and irreversible array based. */
        l_current_data+=3;

        if (mi_stream_write_data(p_stream,p_j2k->m_specific_param.m_encoder.m_header_tile_data,l_mcc_size,p_manager) != l_mcc_size) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_read_mcc (     mi_j2k_t *p_j2k,
                                                mi_BYTE * p_header_data,
                                                mi_UINT32 p_header_size,
                                                mi_event_mgr_t * p_manager )
{
        mi_UINT32 i,j;
        mi_UINT32 l_tmp;
        mi_UINT32 l_indix;
        mi_tcp_t * l_tcp;
        mi_simple_mcc_decorrelation_data_t * l_mcc_record;
        mi_mct_data_t * l_mct_data;
        mi_UINT32 l_nb_collections;
        mi_UINT32 l_nb_comps;
        mi_UINT32 l_nb_bytes_by_comp;

        /* preconditions */
        assert(p_header_data != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        l_tcp = p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_TPH ?
                        &p_j2k->m_cp.tcps[p_j2k->m_current_tile_number] :
                        p_j2k->m_specific_param.m_decoder.m_default_tcp;

        if (p_header_size < 2) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading MCC marker\n");
                return mi_FALSE;
        }

        /* first marker */
        mi_read_bytes(p_header_data,&l_tmp,2);                         /* Zmcc */
        p_header_data += 2;
        if (l_tmp != 0) {
                mi_event_msg(p_manager, EVT_WARNING, "Cannot take in charge multiple data spanning\n");
                return mi_TRUE;
        }

        if (p_header_size < 7) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading MCC marker\n");
                return mi_FALSE;
        }

        mi_read_bytes(p_header_data,&l_indix,1); /* Imcc -> no need for other values, take the first */
        ++p_header_data;

        l_mcc_record = l_tcp->m_mcc_records;

        for(i=0;i<l_tcp->m_nb_mcc_records;++i) {
                if (l_mcc_record->m_index == l_indix) {
                        break;
                }
                ++l_mcc_record;
        }

        /** NOT FOUND */
        if (i == l_tcp->m_nb_mcc_records) {
                if (l_tcp->m_nb_mcc_records == l_tcp->m_nb_max_mcc_records) {
                        mi_simple_mcc_decorrelation_data_t *new_mcc_records;
                        l_tcp->m_nb_max_mcc_records += mi_J2K_MCC_DEFAULT_NB_RECORDS;

                        new_mcc_records = (mi_simple_mcc_decorrelation_data_t *) mi_realloc(
                                        l_tcp->m_mcc_records, l_tcp->m_nb_max_mcc_records * sizeof(mi_simple_mcc_decorrelation_data_t));
                        if (! new_mcc_records) {
                                mi_free(l_tcp->m_mcc_records);
                                l_tcp->m_mcc_records = NULL;
                                l_tcp->m_nb_max_mcc_records = 0;
                                l_tcp->m_nb_mcc_records = 0;
                                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to read MCC marker\n");
                                return mi_FALSE;
                        }
                        l_tcp->m_mcc_records = new_mcc_records;
                        l_mcc_record = l_tcp->m_mcc_records + l_tcp->m_nb_mcc_records;
                        memset(l_mcc_record,0,(l_tcp->m_nb_max_mcc_records-l_tcp->m_nb_mcc_records) * sizeof(mi_simple_mcc_decorrelation_data_t));
                }
                l_mcc_record = l_tcp->m_mcc_records + l_tcp->m_nb_mcc_records;
        }
        l_mcc_record->m_index = l_indix;

        /* only one marker atm */
        mi_read_bytes(p_header_data,&l_tmp,2);                         /* Ymcc */
        p_header_data+=2;
        if (l_tmp != 0) {
                mi_event_msg(p_manager, EVT_WARNING, "Cannot take in charge multiple data spanning\n");
                return mi_TRUE;
        }

        mi_read_bytes(p_header_data,&l_nb_collections,2);                              /* Qmcc -> number of collections -> 1 */
        p_header_data+=2;

        if (l_nb_collections > 1) {
                mi_event_msg(p_manager, EVT_WARNING, "Cannot take in charge multiple collections\n");
                return mi_TRUE;
        }

        p_header_size -= 7;

        for (i=0;i<l_nb_collections;++i) {
                if (p_header_size < 3) {
                        mi_event_msg(p_manager, EVT_ERROR, "Error reading MCC marker\n");
                        return mi_FALSE;
                }

                mi_read_bytes(p_header_data,&l_tmp,1); /* Xmcci type of component transformation -> array based decorrelation */
                ++p_header_data;

                if (l_tmp != 1) {
                        mi_event_msg(p_manager, EVT_WARNING, "Cannot take in charge collections other than array decorrelation\n");
                        return mi_TRUE;
                }

                mi_read_bytes(p_header_data,&l_nb_comps,2);

                p_header_data+=2;
                p_header_size-=3;

                l_nb_bytes_by_comp = 1 + (l_nb_comps>>15);
                l_mcc_record->m_nb_comps = l_nb_comps & 0x7fff;

                if (p_header_size < (l_nb_bytes_by_comp * l_mcc_record->m_nb_comps + 2)) {
                        mi_event_msg(p_manager, EVT_ERROR, "Error reading MCC marker\n");
                        return mi_FALSE;
                }

                p_header_size -= (l_nb_bytes_by_comp * l_mcc_record->m_nb_comps + 2);

                for (j=0;j<l_mcc_record->m_nb_comps;++j) {
                        mi_read_bytes(p_header_data,&l_tmp,l_nb_bytes_by_comp);        /* Cmccij Component offset*/
                        p_header_data+=l_nb_bytes_by_comp;

                        if (l_tmp != j) {
                                mi_event_msg(p_manager, EVT_WARNING, "Cannot take in charge collections with indix shuffle\n");
                                return mi_TRUE;
                        }
                }

                mi_read_bytes(p_header_data,&l_nb_comps,2);
                p_header_data+=2;

                l_nb_bytes_by_comp = 1 + (l_nb_comps>>15);
                l_nb_comps &= 0x7fff;

                if (l_nb_comps != l_mcc_record->m_nb_comps) {
                        mi_event_msg(p_manager, EVT_WARNING, "Cannot take in charge collections without same number of indixes\n");
                        return mi_TRUE;
                }

                if (p_header_size < (l_nb_bytes_by_comp * l_mcc_record->m_nb_comps + 3)) {
                        mi_event_msg(p_manager, EVT_ERROR, "Error reading MCC marker\n");
                        return mi_FALSE;
                }

                p_header_size -= (l_nb_bytes_by_comp * l_mcc_record->m_nb_comps + 3);

                for (j=0;j<l_mcc_record->m_nb_comps;++j) {
                        mi_read_bytes(p_header_data,&l_tmp,l_nb_bytes_by_comp);        /* Wmccij Component offset*/
                        p_header_data+=l_nb_bytes_by_comp;

                        if (l_tmp != j) {
                                mi_event_msg(p_manager, EVT_WARNING, "Cannot take in charge collections with indix shuffle\n");
                                return mi_TRUE;
                        }
                }

                mi_read_bytes(p_header_data,&l_tmp,3); /* Wmccij Component offset*/
                p_header_data += 3;

                l_mcc_record->m_is_irreversible = ! ((l_tmp>>16) & 1);
                l_mcc_record->m_decorrelation_array = 00;
                l_mcc_record->m_offset_array = 00;

                l_indix = l_tmp & 0xff;
                if (l_indix != 0) {
                        l_mct_data = l_tcp->m_mct_records;
                        for (j=0;j<l_tcp->m_nb_mct_records;++j) {
                                if (l_mct_data->m_index == l_indix) {
                                        l_mcc_record->m_decorrelation_array = l_mct_data;
                                        break;
                                }
                                ++l_mct_data;
                        }

                        if (l_mcc_record->m_decorrelation_array == 00) {
                                mi_event_msg(p_manager, EVT_ERROR, "Error reading MCC marker\n");
                                return mi_FALSE;
                        }
                }

                l_indix = (l_tmp >> 8) & 0xff;
                if (l_indix != 0) {
                        l_mct_data = l_tcp->m_mct_records;
                        for (j=0;j<l_tcp->m_nb_mct_records;++j) {
                                if (l_mct_data->m_index == l_indix) {
                                        l_mcc_record->m_offset_array = l_mct_data;
                                        break;
                                }
                                ++l_mct_data;
                        }

                        if (l_mcc_record->m_offset_array == 00) {
                                mi_event_msg(p_manager, EVT_ERROR, "Error reading MCC marker\n");
                                return mi_FALSE;
                        }
                }
        }

        if (p_header_size != 0) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading MCC marker\n");
                return mi_FALSE;
        }

        ++l_tcp->m_nb_mcc_records;

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_mco(     mi_j2k_t *p_j2k,
                                                struct mi_stream_private *p_stream,
                                                struct mi_event_mgr * p_manager
                                  )
{
        mi_BYTE * l_current_data = 00;
        mi_UINT32 l_mco_size;
        mi_tcp_t * l_tcp = 00;
        mi_simple_mcc_decorrelation_data_t * l_mcc_record;
        mi_UINT32 i;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        l_tcp =&(p_j2k->m_cp.tcps[p_j2k->m_current_tile_number]);
	
        l_mco_size = 5 + l_tcp->m_nb_mcc_records;
        if (l_mco_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {

                mi_BYTE *new_header_tile_data = (mi_BYTE *) mi_realloc(p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_mco_size);
                if (! new_header_tile_data) {
                        mi_free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to write MCO marker\n");
                        return mi_FALSE;
                }
                p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
                p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_mco_size;
        }
        l_current_data = p_j2k->m_specific_param.m_encoder.m_header_tile_data;


        mi_write_bytes(l_current_data,J2K_MS_MCO,2);                   /* MCO */
        l_current_data += 2;

        mi_write_bytes(l_current_data,l_mco_size-2,2);                 /* Lmco */
        l_current_data += 2;

        mi_write_bytes(l_current_data,l_tcp->m_nb_mcc_records,1);      /* Nmco : only one transform stage*/
        ++l_current_data;

        l_mcc_record = l_tcp->m_mcc_records;
        for (i=0;i<l_tcp->m_nb_mcc_records;++i) {
                mi_write_bytes(l_current_data,l_mcc_record->m_index,1);/* Imco -> use the mcc indicated by 1*/
                ++l_current_data;
                ++l_mcc_record;
        }

        if (mi_stream_write_data(p_stream,p_j2k->m_specific_param.m_encoder.m_header_tile_data,l_mco_size,p_manager) != l_mco_size) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

/**
 * Reads a MCO marker (Multiple Component Transform Ordering)
 *
 * @param       p_header_data   the data contained in the MCO box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the MCO marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_mco (      mi_j2k_t *p_j2k,
                                                                    mi_BYTE * p_header_data,
                                                                    mi_UINT32 p_header_size,
                                                                    mi_event_mgr_t * p_manager
                                    )
{
        mi_UINT32 l_tmp, i;
        mi_UINT32 l_nb_stages;
        mi_tcp_t * l_tcp;
        mi_tccp_t * l_tccp;
        mi_image_t * l_image;

        /* preconditions */
        assert(p_header_data != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        l_image = p_j2k->m_private_image;
        l_tcp = p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_TPH ?
                        &p_j2k->m_cp.tcps[p_j2k->m_current_tile_number] :
                        p_j2k->m_specific_param.m_decoder.m_default_tcp;

        if (p_header_size < 1) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading MCO marker\n");
                return mi_FALSE;
        }

        mi_read_bytes(p_header_data,&l_nb_stages,1);                           /* Nmco : only one transform stage*/
        ++p_header_data;

        if (l_nb_stages > 1) {
                mi_event_msg(p_manager, EVT_WARNING, "Cannot take in charge multiple transformation stages.\n");
                return mi_TRUE;
        }

        if (p_header_size != l_nb_stages + 1) {
                mi_event_msg(p_manager, EVT_WARNING, "Error reading MCO marker\n");
                return mi_FALSE;
        }

        l_tccp = l_tcp->tccps;

        for (i=0;i<l_image->numcomps;++i) {
                l_tccp->m_dc_level_shift = 0;
                ++l_tccp;
        }

        if (l_tcp->m_mct_decoding_matrix) {
                mi_free(l_tcp->m_mct_decoding_matrix);
                l_tcp->m_mct_decoding_matrix = 00;
        }

        for (i=0;i<l_nb_stages;++i) {
                mi_read_bytes(p_header_data,&l_tmp,1);
                ++p_header_data;

                if (! mi_j2k_add_mct(l_tcp,p_j2k->m_private_image,l_tmp)) {
                        return mi_FALSE;
                }
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_add_mct(mi_tcp_t * p_tcp, mi_image_t * p_image, mi_UINT32 p_index)
{
        mi_UINT32 i;
        mi_simple_mcc_decorrelation_data_t * l_mcc_record;
        mi_mct_data_t * l_deco_array, * l_offset_array;
        mi_UINT32 l_data_size,l_mct_size, l_offset_size;
        mi_UINT32 l_nb_elem;
        mi_UINT32 * l_offset_data, * l_current_offset_data;
        mi_tccp_t * l_tccp;

        /* preconditions */
        assert(p_tcp != 00);

        l_mcc_record = p_tcp->m_mcc_records;

        for (i=0;i<p_tcp->m_nb_mcc_records;++i) {
                if (l_mcc_record->m_index == p_index) {
                        break;
                }
        }

        if (i==p_tcp->m_nb_mcc_records) {
                /** element discarded **/
                return mi_TRUE;
        }

        if (l_mcc_record->m_nb_comps != p_image->numcomps) {
                /** do not support number of comps != image */
                return mi_TRUE;
        }

        l_deco_array = l_mcc_record->m_decorrelation_array;

        if (l_deco_array) {
                l_data_size = MCT_ELEMENT_SIZE[l_deco_array->m_element_type] * p_image->numcomps * p_image->numcomps;
                if (l_deco_array->m_data_size != l_data_size) {
                        return mi_FALSE;
                }

                l_nb_elem = p_image->numcomps * p_image->numcomps;
                l_mct_size = l_nb_elem * (mi_UINT32)sizeof(mi_FLOAT32);
                p_tcp->m_mct_decoding_matrix = (mi_FLOAT32*)mi_malloc(l_mct_size);

                if (! p_tcp->m_mct_decoding_matrix ) {
                        return mi_FALSE;
                }

                j2k_mct_read_functions_to_float[l_deco_array->m_element_type](l_deco_array->m_data,p_tcp->m_mct_decoding_matrix,l_nb_elem);
        }

        l_offset_array = l_mcc_record->m_offset_array;

        if (l_offset_array) {
                l_data_size = MCT_ELEMENT_SIZE[l_offset_array->m_element_type] * p_image->numcomps;
                if (l_offset_array->m_data_size != l_data_size) {
                        return mi_FALSE;
                }

                l_nb_elem = p_image->numcomps;
                l_offset_size = l_nb_elem * (mi_UINT32)sizeof(mi_UINT32);
                l_offset_data = (mi_UINT32*)mi_malloc(l_offset_size);

                if (! l_offset_data ) {
                        return mi_FALSE;
                }

                j2k_mct_read_functions_to_int32[l_offset_array->m_element_type](l_offset_array->m_data,l_offset_data,l_nb_elem);

                l_tccp = p_tcp->tccps;
                l_current_offset_data = l_offset_data;

                for (i=0;i<p_image->numcomps;++i) {
                        l_tccp->m_dc_level_shift = (mi_INT32)*(l_current_offset_data++);
                        ++l_tccp;
                }

                mi_free(l_offset_data);
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_cbd( mi_j2k_t *p_j2k,
                                                struct mi_stream_private *p_stream,
                                                struct mi_event_mgr * p_manager )
{
        mi_UINT32 i;
        mi_UINT32 l_cbd_size;
        mi_BYTE * l_current_data = 00;
        mi_image_t *l_image = 00;
        mi_image_comp_t * l_comp = 00;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        l_image = p_j2k->m_private_image;
        l_cbd_size = 6 + p_j2k->m_private_image->numcomps;

        if (l_cbd_size > p_j2k->m_specific_param.m_encoder.m_header_tile_data_size) {
                mi_BYTE *new_header_tile_data = (mi_BYTE *) mi_realloc(p_j2k->m_specific_param.m_encoder.m_header_tile_data, l_cbd_size);
                if (! new_header_tile_data) {
                        mi_free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data = NULL;
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to write CBD marker\n");
                        return mi_FALSE;
                }
                p_j2k->m_specific_param.m_encoder.m_header_tile_data = new_header_tile_data;
                p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = l_cbd_size;
        }

        l_current_data = p_j2k->m_specific_param.m_encoder.m_header_tile_data;

        mi_write_bytes(l_current_data,J2K_MS_CBD,2);                   /* CBD */
        l_current_data += 2;

        mi_write_bytes(l_current_data,l_cbd_size-2,2);                 /* L_CBD */
        l_current_data += 2;

        mi_write_bytes(l_current_data,l_image->numcomps, 2);           /* Ncbd */
        l_current_data+=2;

        l_comp = l_image->comps;

        for (i=0;i<l_image->numcomps;++i) {
                mi_write_bytes(l_current_data, (l_comp->sgnd << 7) | (l_comp->prec - 1), 1);           /* Component bit depth */
                ++l_current_data;

                ++l_comp;
        }

        if (mi_stream_write_data(p_stream,p_j2k->m_specific_param.m_encoder.m_header_tile_data,l_cbd_size,p_manager) != l_cbd_size) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

/**
 * Reads a CBD marker (Component bit depth definition)
 * @param       p_header_data   the data contained in the CBD box.
 * @param       p_j2k                   the jpeg2000 codec.
 * @param       p_header_size   the size of the data contained in the CBD marker.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_read_cbd (      mi_j2k_t *p_j2k,
                                                                mi_BYTE * p_header_data,
                                                                mi_UINT32 p_header_size,
                                                                mi_event_mgr_t * p_manager
                                    )
{
        mi_UINT32 l_nb_comp,l_num_comp;
        mi_UINT32 l_comp_def;
        mi_UINT32 i;
        mi_image_comp_t * l_comp = 00;

        /* preconditions */
        assert(p_header_data != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        l_num_comp = p_j2k->m_private_image->numcomps;

        if (p_header_size != (p_j2k->m_private_image->numcomps + 2)) {
                mi_event_msg(p_manager, EVT_ERROR, "Crror reading CBD marker\n");
                return mi_FALSE;
        }

        mi_read_bytes(p_header_data,&l_nb_comp,2);                             /* Ncbd */
        p_header_data+=2;

        if (l_nb_comp != l_num_comp) {
                mi_event_msg(p_manager, EVT_ERROR, "Crror reading CBD marker\n");
                return mi_FALSE;
        }

        l_comp = p_j2k->m_private_image->comps;
        for (i=0;i<l_num_comp;++i) {
                mi_read_bytes(p_header_data,&l_comp_def,1);                    /* Component bit depth */
                ++p_header_data;
        l_comp->sgnd = (l_comp_def>>7) & 1;
                l_comp->prec = (l_comp_def&0x7f) + 1;
                ++l_comp;
        }

        return mi_TRUE;
}

/* ----------------------------------------------------------------------- */
/* J2K / JPT decoder interface                                             */
/* ----------------------------------------------------------------------- */

void mi_j2k_setup_decoder(mi_j2k_t *j2k, mi_dparameters_t *parameters)
{
        if(j2k && parameters) {
                j2k->m_cp.m_specific_param.m_dec.m_layer = parameters->cp_layer;
                j2k->m_cp.m_specific_param.m_dec.m_reduce = parameters->cp_reduce;
        }
}

/* ----------------------------------------------------------------------- */
/* J2K encoder interface                                                       */
/* ----------------------------------------------------------------------- */

mi_j2k_t* mi_j2k_create_compress(void)
{
        mi_j2k_t *l_j2k = (mi_j2k_t*) mi_calloc(1,sizeof(mi_j2k_t));
        if (!l_j2k) {
                return NULL;
        }


        l_j2k->m_is_decoder = 0;
        l_j2k->m_cp.m_is_decoder = 0;

        l_j2k->m_specific_param.m_encoder.m_header_tile_data = (mi_BYTE *) mi_malloc(mi_J2K_DEFAULT_HEADER_SIZE);
        if (! l_j2k->m_specific_param.m_encoder.m_header_tile_data) {
                mi_j2k_destroy(l_j2k);
                return NULL;
        }

        l_j2k->m_specific_param.m_encoder.m_header_tile_data_size = mi_J2K_DEFAULT_HEADER_SIZE;

        /* validation list creation*/
        l_j2k->m_validation_list = mi_procedure_list_create();
        if (! l_j2k->m_validation_list) {
                mi_j2k_destroy(l_j2k);
                return NULL;
        }

        /* execution list creation*/
        l_j2k->m_procedure_list = mi_procedure_list_create();
        if (! l_j2k->m_procedure_list) {
                mi_j2k_destroy(l_j2k);
                return NULL;
        }

        return l_j2k;
}

static int mi_j2k_initialise_4K_poc(mi_poc_t *POC, int numres){
    POC[0].tile  = 1;
    POC[0].resno0  = 0;
    POC[0].compno0 = 0;
    POC[0].layno1  = 1;
    POC[0].resno1  = (mi_UINT32)(numres-1);
    POC[0].compno1 = 3;
    POC[0].prg1 = mi_CPRL;
    POC[1].tile  = 1;
    POC[1].resno0  = (mi_UINT32)(numres-1);
    POC[1].compno0 = 0;
    POC[1].layno1  = 1;
    POC[1].resno1  = (mi_UINT32)numres;
    POC[1].compno1 = 3;
    POC[1].prg1 = mi_CPRL;
    return 2;
}

static void mi_j2k_set_cinema_parameters(mi_cparameters_t *parameters, mi_image_t *image, mi_event_mgr_t *p_manager)
{
    /* Configure cinema parameters */
    int i;

    /* No tiling */
    parameters->tile_size_on = mi_FALSE;
    parameters->cp_tdx=1;
    parameters->cp_tdy=1;

    /* One tile part for each component */
    parameters->tp_flag = 'C';
    parameters->tp_on = 1;

    /* Tile and Image shall be at (0,0) */
    parameters->cp_tx0 = 0;
    parameters->cp_ty0 = 0;
    parameters->image_offset_x0 = 0;
    parameters->image_offset_y0 = 0;

    /* Codeblock size= 32*32 */
    parameters->cblockw_init = 32;
    parameters->cblockh_init = 32;

    /* Codeblock style: no mode switch enabled */
    parameters->mode = 0;

    /* No ROI */
    parameters->roi_compno = -1;

    /* No subsampling */
    parameters->subsampling_dx = 1;
    parameters->subsampling_dy = 1;

    /* 9-7 transform */
    parameters->irreversible = 1;

    /* Number of layers */
    if (parameters->tcp_numlayers > 1){
        mi_event_msg(p_manager, EVT_WARNING,
                "JPEG 2000 Profile-3 and 4 (2k/4k dc profile) requires:\n"
                "1 single quality layer"
                "-> Number of layers forced to 1 (rather than %d)\n"
                "-> Rate of the last layer (%3.1f) will be used",
                parameters->tcp_numlayers, parameters->tcp_rates[parameters->tcp_numlayers-1]);
        parameters->tcp_rates[0] = parameters->tcp_rates[parameters->tcp_numlayers-1];
        parameters->tcp_numlayers = 1;
    }

    /* Resolution levels */
    switch (parameters->rsiz){
    case mi_PROFILE_CINEMA_2K:
        if(parameters->numresolution > 6){
            mi_event_msg(p_manager, EVT_WARNING,
                    "JPEG 2000 Profile-3 (2k dc profile) requires:\n"
                    "Number of decomposition levels <= 5\n"
                    "-> Number of decomposition levels forced to 5 (rather than %d)\n",
                    parameters->numresolution+1);
            parameters->numresolution = 6;
        }
        break;
    case mi_PROFILE_CINEMA_4K:
        if(parameters->numresolution < 2){
            mi_event_msg(p_manager, EVT_WARNING,
                    "JPEG 2000 Profile-4 (4k dc profile) requires:\n"
                    "Number of decomposition levels >= 1 && <= 6\n"
                    "-> Number of decomposition levels forced to 1 (rather than %d)\n",
                    parameters->numresolution+1);
            parameters->numresolution = 1;
        }else if(parameters->numresolution > 7){
            mi_event_msg(p_manager, EVT_WARNING,
                    "JPEG 2000 Profile-4 (4k dc profile) requires:\n"
                    "Number of decomposition levels >= 1 && <= 6\n"
                    "-> Number of decomposition levels forced to 6 (rather than %d)\n",
                    parameters->numresolution+1);
            parameters->numresolution = 7;
        }
        break;
    default :
        break;
    }

    /* Precincts */
    parameters->csty |= 0x01;
    parameters->res_spec = parameters->numresolution-1;
    for (i = 0; i<parameters->res_spec; i++) {
        parameters->prcw_init[i] = 256;
        parameters->prch_init[i] = 256;
    }

    /* The progression order shall be CPRL */
    parameters->prog_order = mi_CPRL;

    /* Progression order changes for 4K, disallowed for 2K */
    if (parameters->rsiz == mi_PROFILE_CINEMA_4K) {
        parameters->numpocs = (mi_UINT32)mi_j2k_initialise_4K_poc(parameters->POC,parameters->numresolution);
    } else {
        parameters->numpocs = 0;
    }

    /* Limited bit-rate */
    parameters->cp_disto_alloc = 1;
    if (parameters->max_cs_size <= 0) {
        /* No rate has been introduced, 24 fps is assumed */
        parameters->max_cs_size = mi_CINEMA_24_CS;
        mi_event_msg(p_manager, EVT_WARNING,
                      "JPEG 2000 Profile-3 and 4 (2k/4k dc profile) requires:\n"
                      "Maximum 1302083 compressed bytes @ 24fps\n"
                      "As no rate has been given, this limit will be used.\n");
    } else if (parameters->max_cs_size > mi_CINEMA_24_CS) {
        mi_event_msg(p_manager, EVT_WARNING,
                      "JPEG 2000 Profile-3 and 4 (2k/4k dc profile) requires:\n"
                      "Maximum 1302083 compressed bytes @ 24fps\n"
                      "-> Specified rate exceeds this limit. Rate will be forced to 1302083 bytes.\n");
        parameters->max_cs_size = mi_CINEMA_24_CS;
    }

    if (parameters->max_comp_size <= 0) {
        /* No rate has been introduced, 24 fps is assumed */
        parameters->max_comp_size = mi_CINEMA_24_COMP;
        mi_event_msg(p_manager, EVT_WARNING,
                      "JPEG 2000 Profile-3 and 4 (2k/4k dc profile) requires:\n"
                      "Maximum 1041666 compressed bytes @ 24fps\n"
                      "As no rate has been given, this limit will be used.\n");
    } else if (parameters->max_comp_size > mi_CINEMA_24_COMP) {
        mi_event_msg(p_manager, EVT_WARNING,
                      "JPEG 2000 Profile-3 and 4 (2k/4k dc profile) requires:\n"
                      "Maximum 1041666 compressed bytes @ 24fps\n"
                      "-> Specified rate exceeds this limit. Rate will be forced to 1041666 bytes.\n");
        parameters->max_comp_size = mi_CINEMA_24_COMP;
    }

    parameters->tcp_rates[0] = (mi_FLOAT32) (image->numcomps * image->comps[0].w * image->comps[0].h * image->comps[0].prec)/
            (mi_FLOAT32)(((mi_UINT32)parameters->max_cs_size) * 8 * image->comps[0].dx * image->comps[0].dy);

}

static mi_BOOL mi_j2k_is_cinema_compliant(mi_image_t *image, mi_UINT16 rsiz, mi_event_mgr_t *p_manager)
{
    mi_UINT32 i;

    /* Number of components */
    if (image->numcomps != 3){
        mi_event_msg(p_manager, EVT_WARNING,
                "JPEG 2000 Profile-3 (2k dc profile) requires:\n"
                "3 components"
                "-> Number of components of input image (%d) is not compliant\n"
                "-> Non-profile-3 codestream will be generated\n",
                image->numcomps);
        return mi_FALSE;
    }

    /* Bitdepth */
    for (i = 0; i < image->numcomps; i++) {
        if ((image->comps[i].bpp != 12) | (image->comps[i].sgnd)){
            char signed_str[] = "signed";
            char unsigned_str[] = "unsigned";
            char *tmp_str = image->comps[i].sgnd?signed_str:unsigned_str;
            mi_event_msg(p_manager, EVT_WARNING,
                    "JPEG 2000 Profile-3 (2k dc profile) requires:\n"
                    "Precision of each component shall be 12 bits unsigned"
                    "-> At least component %d of input image (%d bits, %s) is not compliant\n"
                    "-> Non-profile-3 codestream will be generated\n",
                    i,image->comps[i].bpp, tmp_str);
            return mi_FALSE;
        }
    }

    /* Image size */
    switch (rsiz){
    case mi_PROFILE_CINEMA_2K:
        if (((image->comps[0].w > 2048) | (image->comps[0].h > 1080))){
            mi_event_msg(p_manager, EVT_WARNING,
                    "JPEG 2000 Profile-3 (2k dc profile) requires:\n"
                    "width <= 2048 and height <= 1080\n"
                    "-> Input image size %d x %d is not compliant\n"
                    "-> Non-profile-3 codestream will be generated\n",
                    image->comps[0].w,image->comps[0].h);
            return mi_FALSE;
        }
        break;
    case mi_PROFILE_CINEMA_4K:
        if (((image->comps[0].w > 4096) | (image->comps[0].h > 2160))){
            mi_event_msg(p_manager, EVT_WARNING,
                    "JPEG 2000 Profile-4 (4k dc profile) requires:\n"
                    "width <= 4096 and height <= 2160\n"
                    "-> Image size %d x %d is not compliant\n"
                    "-> Non-profile-4 codestream will be generated\n",
                    image->comps[0].w,image->comps[0].h);
            return mi_FALSE;
        }
        break;
    default :
        break;
    }

    return mi_TRUE;
}

mi_BOOL mi_j2k_setup_encoder(     mi_j2k_t *p_j2k,
                                                    mi_cparameters_t *parameters,
                                                    mi_image_t *image,
                                                    mi_event_mgr_t * p_manager)
{
        mi_UINT32 i, j, tileno, numpocs_tile;
        mi_cp_t *cp = 00;

        if(!p_j2k || !parameters || ! image) {
                return mi_FALSE;
        }

        if ((parameters->numresolution <= 0) || (parameters->numresolution > mi_J2K_MAXRLVLS)) {
            mi_event_msg(p_manager, EVT_ERROR, "Invalid number of resolutions : %d not in range [1,%d]\n", parameters->numresolution, mi_J2K_MAXRLVLS);
            return mi_FALSE;
        }

        /* keep a link to cp so that we can destroy it later in j2k_destroy_compress */
        cp = &(p_j2k->m_cp);

        /* set default values for cp */
        cp->tw = 1;
        cp->th = 1;
		//================从这里开始与电影画面配置有关，跟标准JPEG2000实现无关系===========
        /* FIXME ADE: to be removed once deprecated cp_cinema and cp_rsiz have been removed */
        if (parameters->rsiz == mi_PROFILE_NONE) { /* consider deprecated fields only if RSIZ has not been set */
            mi_BOOL deprecated_used = mi_FALSE;
            switch (parameters->cp_cinema){
            case mi_CINEMA2K_24:
                parameters->rsiz = mi_PROFILE_CINEMA_2K;
                parameters->max_cs_size = mi_CINEMA_24_CS;
                parameters->max_comp_size = mi_CINEMA_24_COMP;
                deprecated_used = mi_TRUE;
                break;
            case mi_CINEMA2K_48:
                parameters->rsiz = mi_PROFILE_CINEMA_2K;
                parameters->max_cs_size = mi_CINEMA_48_CS;
                parameters->max_comp_size = mi_CINEMA_48_COMP;
                deprecated_used = mi_TRUE;
                break;
            case mi_CINEMA4K_24:
                parameters->rsiz = mi_PROFILE_CINEMA_4K;
                parameters->max_cs_size = mi_CINEMA_24_CS;
                parameters->max_comp_size = mi_CINEMA_24_COMP;
                deprecated_used = mi_TRUE;
                break;
            case mi_OFF:
            default:
                break;
            }
            switch (parameters->cp_rsiz){
            case mi_CINEMA2K:
                parameters->rsiz = mi_PROFILE_CINEMA_2K;
                deprecated_used = mi_TRUE;
                break;
            case mi_CINEMA4K:
                parameters->rsiz = mi_PROFILE_CINEMA_4K;
                deprecated_used = mi_TRUE;
                break;
            case mi_MCT:
                parameters->rsiz = mi_PROFILE_PART2 | mi_EXTENSION_MCT;
                deprecated_used = mi_TRUE;
            case mi_STD_RSIZ:
            default:
                break;
            }
            if (deprecated_used) {
                mi_event_msg(p_manager, EVT_WARNING,
                        "Deprecated fields cp_cinema or cp_rsiz are used\n"
                        "Please consider using only the rsiz field\n"
                        "See openjpeg.h documentation for more details\n");
            }
        }

        /* see if max_codestream_size does limit input rate */
        if (parameters->max_cs_size <= 0) {
            if (parameters->tcp_rates[parameters->tcp_numlayers-1] > 0) {
                mi_FLOAT32 temp_size;
                temp_size =(mi_FLOAT32)(image->numcomps * image->comps[0].w * image->comps[0].h * image->comps[0].prec)/
                        (parameters->tcp_rates[parameters->tcp_numlayers-1] * 8 * (mi_FLOAT32)image->comps[0].dx * (mi_FLOAT32)image->comps[0].dy);
                parameters->max_cs_size = (int) floor(temp_size);
            } else {
                parameters->max_cs_size = 0;
            }
        } else {
            mi_FLOAT32 temp_rate;
            mi_BOOL cap = mi_FALSE;
            temp_rate = (mi_FLOAT32) (image->numcomps * image->comps[0].w * image->comps[0].h * image->comps[0].prec)/
                    (mi_FLOAT32)(((mi_UINT32)parameters->max_cs_size) * 8 * image->comps[0].dx * image->comps[0].dy);
            for (i = 0; i < (mi_UINT32) parameters->tcp_numlayers; i++) {
                if (parameters->tcp_rates[i] < temp_rate) {
                    parameters->tcp_rates[i] = temp_rate;
                    cap = mi_TRUE;
                }
            }
            if (cap) {
                mi_event_msg(p_manager, EVT_WARNING,
                        "The desired maximum codestream size has limited\n"
                        "at least one of the desired quality layers\n");
            }
        }

        /* Manage profiles and applications and set RSIZ */
        /* set cinema parameters if required */
        if (mi_IS_CINEMA(parameters->rsiz)){
            if ((parameters->rsiz == mi_PROFILE_CINEMA_S2K)
                    || (parameters->rsiz == mi_PROFILE_CINEMA_S4K)){
                mi_event_msg(p_manager, EVT_WARNING,
                        "JPEG 2000 Scalable Digital Cinema profiles not yet supported\n");
                parameters->rsiz = mi_PROFILE_NONE;
            } else {
                mi_j2k_set_cinema_parameters(parameters,image,p_manager);
                if (!mi_j2k_is_cinema_compliant(image,parameters->rsiz,p_manager)) {
                    parameters->rsiz = mi_PROFILE_NONE;
                }
            }
        } else if (mi_IS_STORAGE(parameters->rsiz)) {
            mi_event_msg(p_manager, EVT_WARNING,
                    "JPEG 2000 Long Term Storage profile not yet supported\n");
            parameters->rsiz = mi_PROFILE_NONE;
        } else if (mi_IS_BROADCAST(parameters->rsiz)) {
            mi_event_msg(p_manager, EVT_WARNING,
                    "JPEG 2000 Broadcast profiles not yet supported\n");
            parameters->rsiz = mi_PROFILE_NONE;
        } else if (mi_IS_IMF(parameters->rsiz)) {
            mi_event_msg(p_manager, EVT_WARNING,
                    "JPEG 2000 IMF profiles not yet supported\n");
            parameters->rsiz = mi_PROFILE_NONE;
        } else if (mi_IS_PART2(parameters->rsiz)) {
            if (parameters->rsiz == ((mi_PROFILE_PART2) | (mi_EXTENSION_NONE))) {
                mi_event_msg(p_manager, EVT_WARNING,
                              "JPEG 2000 Part-2 profile defined\n"
                              "but no Part-2 extension enabled.\n"
                              "Profile set to NONE.\n");
                parameters->rsiz = mi_PROFILE_NONE;
            } else if (parameters->rsiz != ((mi_PROFILE_PART2) | (mi_EXTENSION_MCT))) {
                mi_event_msg(p_manager, EVT_WARNING,
                              "Unsupported Part-2 extension enabled\n"
                              "Profile set to NONE.\n");
                parameters->rsiz = mi_PROFILE_NONE;
            }
        }
		//===================电影画面配置结束行=====================
#pragma region 重点（图片分量编码参数设置与切片设置）
        /*
        copy user encoding parameters
        */
        cp->m_specific_param.m_enc.m_max_comp_size = (mi_UINT32)parameters->max_comp_size;
        cp->rsiz = parameters->rsiz;
        cp->m_specific_param.m_enc.m_disto_alloc = (mi_UINT32)parameters->cp_disto_alloc & 1u;
        cp->m_specific_param.m_enc.m_fixed_alloc = (mi_UINT32)parameters->cp_fixed_alloc & 1u;
        cp->m_specific_param.m_enc.m_fixed_quality = (mi_UINT32)parameters->cp_fixed_quality & 1u;

        /* mod fixed_quality */
        if (parameters->cp_fixed_alloc && parameters->cp_matrice) {
                size_t array_size = (size_t)parameters->tcp_numlayers * (size_t)parameters->numresolution * 3 * sizeof(mi_INT32);
                cp->m_specific_param.m_enc.m_matrice = (mi_INT32 *) mi_malloc(array_size);
								if (!cp->m_specific_param.m_enc.m_matrice) {
								        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to allocate copy of user encoding parameters matrix \n");
								        return mi_FALSE;
								}
                memcpy(cp->m_specific_param.m_enc.m_matrice, parameters->cp_matrice, array_size);
        }
		
        /* tiles */
        cp->tdx = (mi_UINT32)parameters->cp_tdx;
        cp->tdy = (mi_UINT32)parameters->cp_tdy;

        /* tile offset */
        cp->tx0 = (mi_UINT32)parameters->cp_tx0;
        cp->ty0 = (mi_UINT32)parameters->cp_ty0;
#pragma endregion
        /* comment string */
        if(parameters->cp_comment) {
                cp->comment = (char*)mi_malloc(strlen(parameters->cp_comment) + 1U);
								if(!cp->comment) {
								        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to allocate copy of comment string\n");
								        return mi_FALSE;
								}
                strcpy(cp->comment, parameters->cp_comment);
        } else {
                /* Create default comment for codestream */
                const char comment[] = "Created by OpenJPEG version ";
                const size_t clen = strlen(comment);
                const char *version = mi_version();


                cp->comment = (char*)mi_malloc(clen+strlen(version)+1);
								if(!cp->comment) {
								        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to allocate comment string\n");
								        return mi_FALSE;
								}
                sprintf(cp->comment,"%s%s", comment, version);
				}

        /*
        calculate other encoding parameters
        */
#pragma region 重点（切片坐标）
        if (parameters->tile_size_on) {
                cp->tw = (mi_UINT32)mi_int_ceildiv((mi_INT32)(image->x1 - cp->tx0), (mi_INT32)cp->tdx);
                cp->th = (mi_UINT32)mi_int_ceildiv((mi_INT32)(image->y1 - cp->ty0), (mi_INT32)cp->tdy);
        } else {
                cp->tdx = image->x1 - cp->tx0;
                cp->tdy = image->y1 - cp->ty0;
        }

        if (parameters->tp_on) {
                cp->m_specific_param.m_enc.m_tp_flag = (mi_BYTE)parameters->tp_flag;
                cp->m_specific_param.m_enc.m_tp_on = 1;
        }
#pragma endregion

#pragma region 重点（切片分量设置）
        /* initialize the mutiple tiles */
        /* ---------------------------- */
        cp->tcps = (mi_tcp_t*) mi_calloc(cp->tw * cp->th, sizeof(mi_tcp_t));
        if (!cp->tcps) {
                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to allocate tile coding parameters\n");
                return mi_FALSE;
        }
        if (parameters->numpocs) {
                /* initialisation of POC */
                mi_j2k_check_poc_val(parameters->POC,parameters->numpocs, (mi_UINT32)parameters->numresolution, image->numcomps, (mi_UINT32)parameters->tcp_numlayers, p_manager);
                /* TODO MSD use the return value*/
        }

        for (tileno = 0; tileno < cp->tw * cp->th; tileno++) {
                mi_tcp_t *tcp = &cp->tcps[tileno];
                tcp->numlayers = (mi_UINT32)parameters->tcp_numlayers;

                for (j = 0; j < tcp->numlayers; j++) {
                        if(mi_IS_CINEMA(cp->rsiz)){
                                if (cp->m_specific_param.m_enc.m_fixed_quality) {
                                        tcp->distoratio[j] = parameters->tcp_distoratio[j];
                                }
                                tcp->rates[j] = parameters->tcp_rates[j];
                        }else{
                                if (cp->m_specific_param.m_enc.m_fixed_quality) {       /* add fixed_quality */
                                        tcp->distoratio[j] = parameters->tcp_distoratio[j];
                                } else {
                                        tcp->rates[j] = parameters->tcp_rates[j];
                                }
                        }
                }

                tcp->csty = (mi_UINT32)parameters->csty;
                tcp->prg = parameters->prog_order;
                tcp->mct = (mi_UINT32)parameters->tcp_mct;

                numpocs_tile = 0;
                tcp->POC = 0;
#pragma endregion 
                if (parameters->numpocs) {
                        /* initialisation of POC */
                        tcp->POC = 1;
                        for (i = 0; i < parameters->numpocs; i++) {
                                if (tileno + 1 == parameters->POC[i].tile )  {
                                        mi_poc_t *tcp_poc = &tcp->pocs[numpocs_tile];

                                        tcp_poc->resno0         = parameters->POC[numpocs_tile].resno0;
                                        tcp_poc->compno0        = parameters->POC[numpocs_tile].compno0;
                                        tcp_poc->layno1         = parameters->POC[numpocs_tile].layno1;
                                        tcp_poc->resno1         = parameters->POC[numpocs_tile].resno1;
                                        tcp_poc->compno1        = parameters->POC[numpocs_tile].compno1;
                                        tcp_poc->prg1           = parameters->POC[numpocs_tile].prg1;
                                        tcp_poc->tile           = parameters->POC[numpocs_tile].tile;

                                        numpocs_tile++;
                                }
                        }

                        tcp->numpocs = numpocs_tile -1 ;
                }else{
                        tcp->numpocs = 0;
                }
#pragma region 重点（MCT）
                tcp->tccps = (mi_tccp_t*) mi_calloc(image->numcomps, sizeof(mi_tccp_t));
                if (!tcp->tccps) {
                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to allocate tile component coding parameters\n");
                        return mi_FALSE;
                }

                if (parameters->mct_data) {                      
                    mi_UINT32 lMctSize = image->numcomps * image->numcomps * (mi_UINT32)sizeof(mi_FLOAT32);
                    mi_FLOAT32 * lTmpBuf = (mi_FLOAT32*)mi_malloc(lMctSize);
                    mi_INT32 * l_dc_shift = (mi_INT32 *) ((mi_BYTE *) parameters->mct_data + lMctSize);

										if (!lTmpBuf) {
                            mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to allocate temp buffer\n");
                            return mi_FALSE;
                    }

                    tcp->mct = 2;
                    tcp->m_mct_coding_matrix = (mi_FLOAT32*)mi_malloc(lMctSize);
										if (! tcp->m_mct_coding_matrix) {
                            mi_free(lTmpBuf);
														lTmpBuf = NULL;
                            mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to allocate encoder MCT coding matrix \n");
                            return mi_FALSE;
                    }
                    memcpy(tcp->m_mct_coding_matrix,parameters->mct_data,lMctSize);
                    memcpy(lTmpBuf,parameters->mct_data,lMctSize);

                    tcp->m_mct_decoding_matrix = (mi_FLOAT32*)mi_malloc(lMctSize);
										if (! tcp->m_mct_decoding_matrix) {
														mi_free(lTmpBuf);
														lTmpBuf = NULL;
                            mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to allocate encoder MCT decoding matrix \n");
                            return mi_FALSE;
                    }
					//这里的MCT用的是矩阵求逆的LUP分解法				
                    if(mi_matrix_inversion_f(lTmpBuf,(tcp->m_mct_decoding_matrix),image->numcomps) == mi_FALSE) {
                            mi_free(lTmpBuf);
														lTmpBuf = NULL;
                            mi_event_msg(p_manager, EVT_ERROR, "Failed to inverse encoder MCT decoding matrix \n");
                            return mi_FALSE;
										}

                    tcp->mct_norms = (mi_FLOAT64*)
                                    mi_malloc(image->numcomps * sizeof(mi_FLOAT64));
										if (! tcp->mct_norms) {
                            mi_free(lTmpBuf);
														lTmpBuf = NULL;
                            mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to allocate encoder MCT norms \n");
                            return mi_FALSE;
                    }
                    mi_calculate_norms(tcp->mct_norms,image->numcomps,tcp->m_mct_decoding_matrix);
                    mi_free(lTmpBuf);

                    for (i = 0; i < image->numcomps; i++) {
                            mi_tccp_t *tccp = &tcp->tccps[i];
                            tccp->m_dc_level_shift = l_dc_shift[i];
                    }

                    if (mi_j2k_setup_mct_encoding(tcp,image) == mi_FALSE) {
                        /* free will be handled by mi_j2k_destroy */
												mi_event_msg(p_manager, EVT_ERROR, "Failed to setup j2k mct encoding\n");
                        return mi_FALSE;
                    }
                }
#pragma endregion
#pragma region 重点（直流电平移）	
                else {
                    if(tcp->mct==1 && image->numcomps >= 3) { /* RGB->YCC MCT is enabled */
                        if ((image->comps[0].dx != image->comps[1].dx) ||
                                (image->comps[0].dx != image->comps[2].dx) ||
                                (image->comps[0].dy != image->comps[1].dy) ||
                                (image->comps[0].dy != image->comps[2].dy)) {
                            mi_event_msg(p_manager, EVT_WARNING, "Cannot perform MCT on components with different sizes. Disabling MCT.\n");
                            tcp->mct = 0;
                        }
                    }
                        for (i = 0; i < image->numcomps; i++) {
                                mi_tccp_t *tccp = &tcp->tccps[i];
                                mi_image_comp_t * l_comp = &(image->comps[i]);

                                if (! l_comp->sgnd) {
                                        tccp->m_dc_level_shift = 1 << (l_comp->prec - 1);
                                }
                        }
                }
#pragma endregion
#pragma region codeblock and quatization
                for (i = 0; i < image->numcomps; i++) {
                        mi_tccp_t *tccp = &tcp->tccps[i];

                        tccp->csty = parameters->csty & 0x01;   /* 0 => one precinct || 1 => custom precinct  */
                        tccp->numresolutions = (mi_UINT32)parameters->numresolution;
                        tccp->cblkw = (mi_UINT32)mi_int_floorlog2(parameters->cblockw_init);
                        tccp->cblkh = (mi_UINT32)mi_int_floorlog2(parameters->cblockh_init);
                        tccp->cblksty = (mi_UINT32)parameters->mode;
                        tccp->qmfbid = parameters->irreversible ? 0 : 1;
                        tccp->qntsty = parameters->irreversible ? J2K_CCP_QNTSTY_SEQNT : J2K_CCP_QNTSTY_NOQNT;
                        tccp->numgbits = 2;

                        if ((mi_INT32)i == parameters->roi_compno) {
                                tccp->roishift = parameters->roi_shift;
                        } else {
                                tccp->roishift = 0;
                        }

						if (parameters->csty & J2K_CCP_CSTY_PRT) {
							mi_INT32 p = 0, it_res;
							assert(tccp->numresolutions > 0);
							for (it_res = (mi_INT32)tccp->numresolutions - 1; it_res >= 0; it_res--) {
								if (p < parameters->res_spec) {

									if (parameters->prcw_init[p] < 1) {//若用户指定的尺寸的位数·小于1则设置为1
										tccp->prcw[it_res] = 1;
									}
									else {
										tccp->prcw[it_res] = (mi_UINT32)mi_int_floorlog2(parameters->prcw_init[p]);
									}

									if (parameters->prch_init[p] < 1) {
										tccp->prch[it_res] = 1;
									}
									else {
										tccp->prch[it_res] = (mi_UINT32)mi_int_floorlog2(parameters->prch_init[p]);
									}

								}
								else {//若这个else触发则说明用户没有为当前图片的分辨率级数设定区尺寸，则遇到一次尺寸减半一次
									mi_INT32 res_spec = parameters->res_spec;
									mi_INT32 size_prcw = 0;
									mi_INT32 size_prch = 0;

									assert(res_spec > 0); /* issue 189 */
									size_prcw = parameters->prcw_init[res_spec - 1] >> (p - (res_spec - 1));//遇到一次尺寸减半一次
									size_prch = parameters->prch_init[res_spec - 1] >> (p - (res_spec - 1));//遇到一次尺寸减半一次


									if (size_prcw < 1) {
										tccp->prcw[it_res] = 1;
									}
									else {
										tccp->prcw[it_res] = (mi_UINT32)mi_int_floorlog2(size_prcw);
									}

									if (size_prch < 1) {
										tccp->prch[it_res] = 1;
									}
									else {
										tccp->prch[it_res] = (mi_UINT32)mi_int_floorlog2(size_prch);
									}
								}
								p++;
								/*printf("\nsize precinct for level %d : %d,%d\n", it_res,tccp->prcw[it_res], tccp->prch[it_res]); */
							}       /*end for*/
						}
						else {
                                        for (j = 0; j < tccp->numresolutions; j++) {
                                                tccp->prcw[j] = 15;
                                                tccp->prch[j] = 15;
                                        }
                                }
						//计算量化步长（显式量化，9/7小波）
                        mi_dwt_calc_explicit_stepsizes(tccp, image->comps[i].prec);
                }
        }
#pragma endregion
        if (parameters->mct_data) {
                mi_free(parameters->mct_data);
                parameters->mct_data = 00;
        }
        return mi_TRUE;
}

static mi_BOOL mi_j2k_add_mhmarker(mi_codestream_index_t *cstr_index, mi_UINT32 type, mi_OFF_T pos, mi_UINT32 len)
{
        assert(cstr_index != 00);

        /* expand the list? */
        if ((cstr_index->marknum + 1) > cstr_index->maxmarknum) {
                mi_marker_info_t *new_marker;
                cstr_index->maxmarknum = (mi_UINT32)(100 + (mi_FLOAT32) cstr_index->maxmarknum);
                new_marker = (mi_marker_info_t *) mi_realloc(cstr_index->marker, cstr_index->maxmarknum *sizeof(mi_marker_info_t));
                if (! new_marker) {
                        mi_free(cstr_index->marker);
                        cstr_index->marker = NULL;
                        cstr_index->maxmarknum = 0;
                        cstr_index->marknum = 0;
                        /* mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to add mh marker\n"); */
                        return mi_FALSE;
                }
                cstr_index->marker = new_marker;
        }

        /* add the marker */
        cstr_index->marker[cstr_index->marknum].type = (mi_UINT16)type;
        cstr_index->marker[cstr_index->marknum].pos = (mi_INT32)pos;
        cstr_index->marker[cstr_index->marknum].len = (mi_INT32)len;
        cstr_index->marknum++;
        return mi_TRUE;
}

static mi_BOOL mi_j2k_add_tlmarker(mi_UINT32 tileno, mi_codestream_index_t *cstr_index, mi_UINT32 type, mi_OFF_T pos, mi_UINT32 len)
{
        assert(cstr_index != 00);
        assert(cstr_index->tile_index != 00);

        /* expand the list? */
        if ((cstr_index->tile_index[tileno].marknum + 1) > cstr_index->tile_index[tileno].maxmarknum) {
                mi_marker_info_t *new_marker;
                cstr_index->tile_index[tileno].maxmarknum = (mi_UINT32)(100 + (mi_FLOAT32) cstr_index->tile_index[tileno].maxmarknum);
                new_marker = (mi_marker_info_t *) mi_realloc(
                                cstr_index->tile_index[tileno].marker,
                                cstr_index->tile_index[tileno].maxmarknum *sizeof(mi_marker_info_t));
                if (! new_marker) {
                        mi_free(cstr_index->tile_index[tileno].marker);
                        cstr_index->tile_index[tileno].marker = NULL;
                        cstr_index->tile_index[tileno].maxmarknum = 0;
                        cstr_index->tile_index[tileno].marknum = 0;
                        /* mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to add tl marker\n"); */
                        return mi_FALSE;
                }
                cstr_index->tile_index[tileno].marker = new_marker;
        }

        /* add the marker */
        cstr_index->tile_index[tileno].marker[cstr_index->tile_index[tileno].marknum].type = (mi_UINT16)type;
        cstr_index->tile_index[tileno].marker[cstr_index->tile_index[tileno].marknum].pos = (mi_INT32)pos;
        cstr_index->tile_index[tileno].marker[cstr_index->tile_index[tileno].marknum].len = (mi_INT32)len;
        cstr_index->tile_index[tileno].marknum++;

        if (type == J2K_MS_SOT) {
                mi_UINT32 l_current_tile_part = cstr_index->tile_index[tileno].current_tpsno;

                if (cstr_index->tile_index[tileno].tp_index)
                        cstr_index->tile_index[tileno].tp_index[l_current_tile_part].start_pos = pos;

        }
        return mi_TRUE;
}

/*
 * -----------------------------------------------------------------------
 * -----------------------------------------------------------------------
 * -----------------------------------------------------------------------
 */

mi_BOOL mi_j2k_end_decompress(mi_j2k_t *p_j2k,
                                mi_stream_private_t *p_stream,
                                mi_event_mgr_t * p_manager
                                )
{
    (void)p_j2k;
    (void)p_stream;
    (void)p_manager;
    return mi_TRUE;
}

mi_BOOL mi_j2k_read_header(   mi_stream_private_t *p_stream,
                                                            mi_j2k_t* p_j2k,
                                                            mi_image_t** p_image,
                                                            mi_event_mgr_t* p_manager )
{
        /* preconditions */
        assert(p_j2k != 00);
        assert(p_stream != 00);
        assert(p_manager != 00);

        /* create an empty image header */
        p_j2k->m_private_image = mi_image_create0();
        if (! p_j2k->m_private_image) {
                return mi_FALSE;
        }

        /* customization of the validation */
        if (! mi_j2k_setup_decoding_validation(p_j2k, p_manager)) {
                mi_image_destroy(p_j2k->m_private_image);
                p_j2k->m_private_image = NULL;
                return mi_FALSE;
        }

        /* validation of the parameters codec */
        if (! mi_j2k_exec(p_j2k, p_j2k->m_validation_list, p_stream,p_manager)) {
                mi_image_destroy(p_j2k->m_private_image);
                p_j2k->m_private_image = NULL;
                return mi_FALSE;
        }

        /* customization of the encoding */
        if (! mi_j2k_setup_header_reading(p_j2k, p_manager)) {
                mi_image_destroy(p_j2k->m_private_image);
                p_j2k->m_private_image = NULL;
                return mi_FALSE;
        }

        /* read header */
        if (! mi_j2k_exec (p_j2k,p_j2k->m_procedure_list,p_stream,p_manager)) {
                mi_image_destroy(p_j2k->m_private_image);
                p_j2k->m_private_image = NULL;
                return mi_FALSE;
        }

        *p_image = mi_image_create0();
        if (! (*p_image)) {
                return mi_FALSE;
        }

        /* Copy codestream image information to the output image */
        mi_copy_image_header(p_j2k->m_private_image, *p_image);

    /*Allocate and initialize some elements of codestrem index*/
        if (!mi_j2k_allocate_tile_element_cstr_index(p_j2k)){
                return mi_FALSE;
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_setup_header_reading (mi_j2k_t *p_j2k, mi_event_mgr_t * p_manager)
{
        /* preconditions*/
        assert(p_j2k != 00);
        assert(p_manager != 00);

        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_read_header_procedure, p_manager)) {
                return mi_FALSE;
        }

        /* DEVELOPER CORNER, add your custom procedures */
        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_copy_default_tcp_and_create_tcd, p_manager))  {
                return mi_FALSE;
        }
	
        return mi_TRUE;
}

static mi_BOOL mi_j2k_setup_decoding_validation (mi_j2k_t *p_j2k, mi_event_mgr_t * p_manager)
{
        /* preconditions*/
        assert(p_j2k != 00);
        assert(p_manager != 00);

        if (! mi_procedure_list_add_procedure(p_j2k->m_validation_list,(mi_procedure)mi_j2k_build_decoder, p_manager)) {
                return mi_FALSE;
        }
        if (! mi_procedure_list_add_procedure(p_j2k->m_validation_list,(mi_procedure)mi_j2k_decoding_validation, p_manager)) {
                return mi_FALSE;
        }

        /* DEVELOPER CORNER, add your custom validation procedure */
        return mi_TRUE;
}

static mi_BOOL mi_j2k_mct_validation (       mi_j2k_t * p_j2k,
                                                                mi_stream_private_t *p_stream,
                                                                mi_event_mgr_t * p_manager )
{
        mi_BOOL l_is_valid = mi_TRUE;
        mi_UINT32 i,j;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_stream != 00);
        assert(p_manager != 00);

        if ((p_j2k->m_cp.rsiz & 0x8200) == 0x8200) {
                mi_UINT32 l_nb_tiles = p_j2k->m_cp.th * p_j2k->m_cp.tw;
                mi_tcp_t * l_tcp = p_j2k->m_cp.tcps;

                for (i=0;i<l_nb_tiles;++i) {
                        if (l_tcp->mct == 2) {
                                mi_tccp_t * l_tccp = l_tcp->tccps;
                                l_is_valid &= (l_tcp->m_mct_coding_matrix != 00);

                                for (j=0;j<p_j2k->m_private_image->numcomps;++j) {
                                        l_is_valid &= ! (l_tccp->qmfbid & 1);
                                        ++l_tccp;
                                }
                        }
                        ++l_tcp;
                }
        }

        return l_is_valid;
}

mi_BOOL mi_j2k_setup_mct_encoding(mi_tcp_t * p_tcp, mi_image_t * p_image)
{
        mi_UINT32 i;
        mi_UINT32 l_indix = 1;
        mi_mct_data_t * l_mct_deco_data = 00,* l_mct_offset_data = 00;
        mi_simple_mcc_decorrelation_data_t * l_mcc_data;
        mi_UINT32 l_mct_size,l_nb_elem;
        mi_FLOAT32 * l_data, * l_current_data;
        mi_tccp_t * l_tccp;

        /* preconditions */
        assert(p_tcp != 00);

        if (p_tcp->mct != 2) {
                return mi_TRUE;
        }

        if (p_tcp->m_mct_decoding_matrix) {
                if (p_tcp->m_nb_mct_records == p_tcp->m_nb_max_mct_records) {
                        mi_mct_data_t *new_mct_records;
                        p_tcp->m_nb_max_mct_records += mi_J2K_MCT_DEFAULT_NB_RECORDS;

                        new_mct_records = (mi_mct_data_t *) mi_realloc(p_tcp->m_mct_records, p_tcp->m_nb_max_mct_records * sizeof(mi_mct_data_t));
                        if (! new_mct_records) {
                                mi_free(p_tcp->m_mct_records);
                                p_tcp->m_mct_records = NULL;
                                p_tcp->m_nb_max_mct_records = 0;
                                p_tcp->m_nb_mct_records = 0;
                                /* mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to setup mct encoding\n"); */
                                return mi_FALSE;
                        }
                        p_tcp->m_mct_records = new_mct_records;
                        l_mct_deco_data = p_tcp->m_mct_records + p_tcp->m_nb_mct_records;

                        memset(l_mct_deco_data ,0,(p_tcp->m_nb_max_mct_records - p_tcp->m_nb_mct_records) * sizeof(mi_mct_data_t));
                }
                l_mct_deco_data = p_tcp->m_mct_records + p_tcp->m_nb_mct_records;

                if (l_mct_deco_data->m_data) {
                        mi_free(l_mct_deco_data->m_data);
                        l_mct_deco_data->m_data = 00;
                }

                l_mct_deco_data->m_index = l_indix++;
                l_mct_deco_data->m_array_type = MCT_TYPE_DECORRELATION;
                l_mct_deco_data->m_element_type = MCT_TYPE_FLOAT;
                l_nb_elem = p_image->numcomps * p_image->numcomps;
                l_mct_size = l_nb_elem * MCT_ELEMENT_SIZE[l_mct_deco_data->m_element_type];
                l_mct_deco_data->m_data = (mi_BYTE*)mi_malloc(l_mct_size );

                if (! l_mct_deco_data->m_data) {
                        return mi_FALSE;
                }

                j2k_mct_write_functions_from_float[l_mct_deco_data->m_element_type](p_tcp->m_mct_decoding_matrix,l_mct_deco_data->m_data,l_nb_elem);

                l_mct_deco_data->m_data_size = l_mct_size;
                ++p_tcp->m_nb_mct_records;
        }

        if (p_tcp->m_nb_mct_records == p_tcp->m_nb_max_mct_records) {
                mi_mct_data_t *new_mct_records;
                p_tcp->m_nb_max_mct_records += mi_J2K_MCT_DEFAULT_NB_RECORDS;
                new_mct_records = (mi_mct_data_t *) mi_realloc(p_tcp->m_mct_records, p_tcp->m_nb_max_mct_records * sizeof(mi_mct_data_t));
                if (! new_mct_records) {
                        mi_free(p_tcp->m_mct_records);
                        p_tcp->m_mct_records = NULL;
                        p_tcp->m_nb_max_mct_records = 0;
                        p_tcp->m_nb_mct_records = 0;
                        /* mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to setup mct encoding\n"); */
                        return mi_FALSE;
                }
                p_tcp->m_mct_records = new_mct_records;
                l_mct_offset_data = p_tcp->m_mct_records + p_tcp->m_nb_mct_records;

                memset(l_mct_offset_data ,0,(p_tcp->m_nb_max_mct_records - p_tcp->m_nb_mct_records) * sizeof(mi_mct_data_t));

                if (l_mct_deco_data) {
                        l_mct_deco_data = l_mct_offset_data - 1;
                }
        }

        l_mct_offset_data = p_tcp->m_mct_records + p_tcp->m_nb_mct_records;

        if (l_mct_offset_data->m_data) {
                mi_free(l_mct_offset_data->m_data);
                l_mct_offset_data->m_data = 00;
        }

        l_mct_offset_data->m_index = l_indix++;
        l_mct_offset_data->m_array_type = MCT_TYPE_OFFSET;
        l_mct_offset_data->m_element_type = MCT_TYPE_FLOAT;
        l_nb_elem = p_image->numcomps;
        l_mct_size = l_nb_elem * MCT_ELEMENT_SIZE[l_mct_offset_data->m_element_type];
        l_mct_offset_data->m_data = (mi_BYTE*)mi_malloc(l_mct_size );

        if (! l_mct_offset_data->m_data) {
                return mi_FALSE;
        }

        l_data = (mi_FLOAT32*)mi_malloc(l_nb_elem * sizeof(mi_FLOAT32));
        if (! l_data) {
                mi_free(l_mct_offset_data->m_data);
                l_mct_offset_data->m_data = 00;
                return mi_FALSE;
        }

        l_tccp = p_tcp->tccps;
        l_current_data = l_data;

        for (i=0;i<l_nb_elem;++i) {
                *(l_current_data++) = (mi_FLOAT32) (l_tccp->m_dc_level_shift);
                ++l_tccp;
        }

        j2k_mct_write_functions_from_float[l_mct_offset_data->m_element_type](l_data,l_mct_offset_data->m_data,l_nb_elem);

        mi_free(l_data);

        l_mct_offset_data->m_data_size = l_mct_size;

        ++p_tcp->m_nb_mct_records;

        if (p_tcp->m_nb_mcc_records == p_tcp->m_nb_max_mcc_records) {
                mi_simple_mcc_decorrelation_data_t *new_mcc_records;
                p_tcp->m_nb_max_mcc_records += mi_J2K_MCT_DEFAULT_NB_RECORDS;
                new_mcc_records = (mi_simple_mcc_decorrelation_data_t *) mi_realloc(
                                p_tcp->m_mcc_records, p_tcp->m_nb_max_mcc_records * sizeof(mi_simple_mcc_decorrelation_data_t));
                if (! new_mcc_records) {
                        mi_free(p_tcp->m_mcc_records);
                        p_tcp->m_mcc_records = NULL;
                        p_tcp->m_nb_max_mcc_records = 0;
                        p_tcp->m_nb_mcc_records = 0;
                        /* mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to setup mct encoding\n"); */
                        return mi_FALSE;
                }
                p_tcp->m_mcc_records = new_mcc_records;
                l_mcc_data = p_tcp->m_mcc_records + p_tcp->m_nb_mcc_records;
                memset(l_mcc_data ,0,(p_tcp->m_nb_max_mcc_records - p_tcp->m_nb_mcc_records) * sizeof(mi_simple_mcc_decorrelation_data_t));

        }

        l_mcc_data = p_tcp->m_mcc_records + p_tcp->m_nb_mcc_records;
        l_mcc_data->m_decorrelation_array = l_mct_deco_data;
        l_mcc_data->m_is_irreversible = 1;
        l_mcc_data->m_nb_comps = p_image->numcomps;
        l_mcc_data->m_index = l_indix++;
        l_mcc_data->m_offset_array = l_mct_offset_data;
        ++p_tcp->m_nb_mcc_records;

        return mi_TRUE;
}

static mi_BOOL mi_j2k_build_decoder (mi_j2k_t * p_j2k,
                                                            mi_stream_private_t *p_stream,
                                                            mi_event_mgr_t * p_manager )
{
        /* add here initialization of cp
           copy paste of setup_decoder */
  (void)p_j2k;
  (void)p_stream;
  (void)p_manager;
        return mi_TRUE;
}

static mi_BOOL mi_j2k_build_encoder (mi_j2k_t * p_j2k,
                                                        mi_stream_private_t *p_stream,
                                                        mi_event_mgr_t * p_manager )
{
        /* add here initialization of cp
           copy paste of setup_encoder */
  (void)p_j2k;
  (void)p_stream;
  (void)p_manager;
        return mi_TRUE;
}

static mi_BOOL mi_j2k_encoding_validation (  mi_j2k_t * p_j2k,
                                                                            mi_stream_private_t *p_stream,
                                                                            mi_event_mgr_t * p_manager )
{
        mi_BOOL l_is_valid = mi_TRUE;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_stream != 00);
        assert(p_manager != 00);

        /* STATE checking */
        /* make sure the state is at 0 */
        l_is_valid &= (p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_NONE);

        /* POINTER validation */
        /* make sure a p_j2k codec is present */
        l_is_valid &= (p_j2k->m_procedure_list != 00);
        /* make sure a validation list is present */
        l_is_valid &= (p_j2k->m_validation_list != 00);

        /* ISO 15444-1:2004 states between 1 & 33 (0 -> 32) */
        /* 33 (32) would always fail the check below (if a cast to 64bits was done) */
        /* FIXME Shall we change mi_J2K_MAXRLVLS to 32 ? */
        if ((p_j2k->m_cp.tcps->tccps->numresolutions <= 0) || (p_j2k->m_cp.tcps->tccps->numresolutions > 32)) {
                mi_event_msg(p_manager, EVT_ERROR, "Number of resolutions is too high in comparison to the size of tiles\n");
                return mi_FALSE;
        }

        if ((p_j2k->m_cp.tdx) < (mi_UINT32) (1 << (p_j2k->m_cp.tcps->tccps->numresolutions - 1U))) {
                mi_event_msg(p_manager, EVT_ERROR, "Number of resolutions is too high in comparison to the size of tiles\n");
                return mi_FALSE;
        }

        if ((p_j2k->m_cp.tdy) < (mi_UINT32) (1 << (p_j2k->m_cp.tcps->tccps->numresolutions - 1U))) {
                mi_event_msg(p_manager, EVT_ERROR, "Number of resolutions is too high in comparison to the size of tiles\n");
                return mi_FALSE;
        }

        /* PARAMETER VALIDATION */
        return l_is_valid;
}

static mi_BOOL mi_j2k_decoding_validation (  mi_j2k_t *p_j2k,
                                        mi_stream_private_t *p_stream,
                                        mi_event_mgr_t * p_manager
                                        )
{
        mi_BOOL l_is_valid = mi_TRUE;

        /* preconditions*/
        assert(p_j2k != 00);
        assert(p_stream != 00);
        assert(p_manager != 00);

        /* STATE checking */
        /* make sure the state is at 0 */
        l_is_valid &= (p_j2k->m_specific_param.m_decoder.m_state == 0x0000);

        /* POINTER validation */
        /* make sure a p_j2k codec is present */
        /* make sure a procedure list is present */
        l_is_valid &= (p_j2k->m_procedure_list != 00);
        /* make sure a validation list is present */
        l_is_valid &= (p_j2k->m_validation_list != 00);

        /* PARAMETER VALIDATION */
        return l_is_valid;
}

static mi_BOOL mi_j2k_read_header_procedure( mi_j2k_t *p_j2k,
                                                                            mi_stream_private_t *p_stream,
                                                                            mi_event_mgr_t * p_manager)
{
        mi_UINT32 l_current_marker;
        mi_UINT32 l_marker_size;
        const mi_dec_memory_marker_handler_t * l_marker_handler = 00;
        mi_BOOL l_has_siz = 0;
        mi_BOOL l_has_cod = 0;
        mi_BOOL l_has_qcd = 0;

        /* preconditions */
        assert(p_stream != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        /*  We enter in the main header */
        p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_MHSOC;

        /* Try to read the SOC marker, the codestream must begin with SOC marker */
        if (! mi_j2k_read_soc(p_j2k,p_stream,p_manager)) {
                mi_event_msg(p_manager, EVT_ERROR, "Expected a SOC marker \n");
                return mi_FALSE;
        }

        /* Try to read 2 bytes (the next marker ID) from stream and copy them into the buffer */
        if (mi_stream_read_data(p_stream,p_j2k->m_specific_param.m_decoder.m_header_data,2,p_manager) != 2) {
                mi_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                return mi_FALSE;
        }

        /* Read 2 bytes as the new marker ID */
        mi_read_bytes(p_j2k->m_specific_param.m_decoder.m_header_data,&l_current_marker,2);

        /* Try to read until the SOT is detected */
        while (l_current_marker != J2K_MS_SOT) {

                /* Check if the current marker ID is valid */
                if (l_current_marker < 0xff00) {
                        mi_event_msg(p_manager, EVT_ERROR, "A marker ID was expected (0xff--) instead of %.8x\n", l_current_marker);
                        return mi_FALSE;
                }

                /* Get the marker handler from the marker ID */
                l_marker_handler = mi_j2k_get_marker_handler(l_current_marker);

                /* Manage case where marker is unknown */
                if (l_marker_handler->id == J2K_MS_UNK) {
                        if (! mi_j2k_read_unk(p_j2k, p_stream, &l_current_marker, p_manager)){
                                mi_event_msg(p_manager, EVT_ERROR, "Unknow marker have been detected and generated error.\n");
                                return mi_FALSE;
                        }

                        if (l_current_marker == J2K_MS_SOT)
                                break; /* SOT marker is detected main header is completely read */
                        else    /* Get the marker handler from the marker ID */
                                l_marker_handler = mi_j2k_get_marker_handler(l_current_marker);
                }

                if (l_marker_handler->id == J2K_MS_SIZ) {
                    /* Mark required SIZ marker as found */
                    l_has_siz = 1;
                }
                if (l_marker_handler->id == J2K_MS_COD) {
                    /* Mark required COD marker as found */
                    l_has_cod = 1;
                }
                if (l_marker_handler->id == J2K_MS_QCD) {
                    /* Mark required QCD marker as found */
                    l_has_qcd = 1;
                }

                /* Check if the marker is known and if it is the right place to find it */
                if (! (p_j2k->m_specific_param.m_decoder.m_state & l_marker_handler->states) ) {
                        mi_event_msg(p_manager, EVT_ERROR, "Marker is not compliant with its position\n");
                        return mi_FALSE;
                }

                /* Try to read 2 bytes (the marker size) from stream and copy them into the buffer */
                if (mi_stream_read_data(p_stream,p_j2k->m_specific_param.m_decoder.m_header_data,2,p_manager) != 2) {
                        mi_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                        return mi_FALSE;
                }

                /* read 2 bytes as the marker size */
                mi_read_bytes(p_j2k->m_specific_param.m_decoder.m_header_data,&l_marker_size,2);
                l_marker_size -= 2; /* Subtract the size of the marker ID already read */

                /* Check if the marker size is compatible with the header data size */
                if (l_marker_size > p_j2k->m_specific_param.m_decoder.m_header_data_size) {
                        mi_BYTE *new_header_data = (mi_BYTE *) mi_realloc(p_j2k->m_specific_param.m_decoder.m_header_data, l_marker_size);
                        if (! new_header_data) {
                                mi_free(p_j2k->m_specific_param.m_decoder.m_header_data);
                                p_j2k->m_specific_param.m_decoder.m_header_data = NULL;
                                p_j2k->m_specific_param.m_decoder.m_header_data_size = 0;
                                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to read header\n");
                                return mi_FALSE;
                        }
                        p_j2k->m_specific_param.m_decoder.m_header_data = new_header_data;
                        p_j2k->m_specific_param.m_decoder.m_header_data_size = l_marker_size;
                }

                /* Try to read the rest of the marker segment from stream and copy them into the buffer */
                if (mi_stream_read_data(p_stream,p_j2k->m_specific_param.m_decoder.m_header_data,l_marker_size,p_manager) != l_marker_size) {
                        mi_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                        return mi_FALSE;
                }

                /* Read the marker segment with the correct marker handler */
                if (! (*(l_marker_handler->handler))(p_j2k,p_j2k->m_specific_param.m_decoder.m_header_data,l_marker_size,p_manager)) {
                        mi_event_msg(p_manager, EVT_ERROR, "Marker handler function failed to read the marker segment\n");
                        return mi_FALSE;
                }

                /* Add the marker to the codestream index*/
                if (mi_FALSE == mi_j2k_add_mhmarker(
                                        p_j2k->cstr_index,
                                        l_marker_handler->id,
                                        (mi_UINT32) mi_stream_tell(p_stream) - l_marker_size - 4,
                                        l_marker_size + 4 )) {
                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to add mh marker\n");
                        return mi_FALSE;
                }

                /* Try to read 2 bytes (the next marker ID) from stream and copy them into the buffer */
                if (mi_stream_read_data(p_stream,p_j2k->m_specific_param.m_decoder.m_header_data,2,p_manager) != 2) {
                        mi_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                        return mi_FALSE;
                }

                /* read 2 bytes as the new marker ID */
                mi_read_bytes(p_j2k->m_specific_param.m_decoder.m_header_data,&l_current_marker,2);
        }

        if (l_has_siz == 0) {
            mi_event_msg(p_manager, EVT_ERROR, "required SIZ marker not found in main header\n");
            return mi_FALSE;
        }
        if (l_has_cod == 0) {
            mi_event_msg(p_manager, EVT_ERROR, "required COD marker not found in main header\n");
            return mi_FALSE;
        }
        if (l_has_qcd == 0) {
            mi_event_msg(p_manager, EVT_ERROR, "required QCD marker not found in main header\n");
            return mi_FALSE;
        }
	
        if (! mi_j2k_merge_ppm(&(p_j2k->m_cp), p_manager)) {
            mi_event_msg(p_manager, EVT_ERROR, "Failed to merge PPM data\n");
            return mi_FALSE;
        }

        mi_event_msg(p_manager, EVT_INFO, "Main header has been correctly decoded.\n");

        /* Position of the last element if the main header */
        p_j2k->cstr_index->main_head_end = (mi_UINT32) mi_stream_tell(p_stream) - 2;

        /* Next step: read a tile-part header */
        p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_TPHSOT;

        return mi_TRUE;
}

static mi_BOOL mi_j2k_exec ( mi_j2k_t * p_j2k,
                                        mi_procedure_list_t * p_procedure_list,
                                        mi_stream_private_t *p_stream,
                                        mi_event_mgr_t * p_manager )
{
        mi_BOOL (** l_procedure) (mi_j2k_t * ,mi_stream_private_t *,mi_event_mgr_t *) = 00;
        mi_BOOL l_result = mi_TRUE;
        mi_UINT32 l_nb_proc, i;

        /* preconditions*/
        assert(p_procedure_list != 00);
        assert(p_j2k != 00);
        assert(p_stream != 00);
        assert(p_manager != 00);

        l_nb_proc = mi_procedure_list_get_nb_procedures(p_procedure_list);
        l_procedure = (mi_BOOL (**) (mi_j2k_t * ,mi_stream_private_t *,mi_event_mgr_t *)) mi_procedure_list_get_first_procedure(p_procedure_list);

        for     (i=0;i<l_nb_proc;++i) {
                l_result = l_result && ((*l_procedure) (p_j2k,p_stream,p_manager));
                ++l_procedure;
        }

        /* and clear the procedure list at the end.*/
        mi_procedure_list_clear(p_procedure_list);
        return l_result;
}

/* FIXME DOC*/
static mi_BOOL mi_j2k_copy_default_tcp_and_create_tcd (       mi_j2k_t * p_j2k,
                                                            mi_stream_private_t *p_stream,
                                                            mi_event_mgr_t * p_manager
                                                            )
{
        mi_tcp_t * l_tcp = 00;
        mi_tcp_t * l_default_tcp = 00;
        mi_UINT32 l_nb_tiles;
        mi_UINT32 i,j;
        mi_tccp_t *l_current_tccp = 00;
        mi_UINT32 l_tccp_size;
        mi_UINT32 l_mct_size;
        mi_image_t * l_image;
        mi_UINT32 l_mcc_records_size,l_mct_records_size;
        mi_mct_data_t * l_src_mct_rec, *l_dest_mct_rec;
        mi_simple_mcc_decorrelation_data_t * l_src_mcc_rec, *l_dest_mcc_rec;
        mi_UINT32 l_offset;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_stream != 00);
        assert(p_manager != 00);

        l_image = p_j2k->m_private_image;
        l_nb_tiles = p_j2k->m_cp.th * p_j2k->m_cp.tw;
        l_tcp = p_j2k->m_cp.tcps;
        l_tccp_size = l_image->numcomps * (mi_UINT32)sizeof(mi_tccp_t);
        l_default_tcp = p_j2k->m_specific_param.m_decoder.m_default_tcp;
        l_mct_size = l_image->numcomps * l_image->numcomps * (mi_UINT32)sizeof(mi_FLOAT32);

        /* For each tile */
        for (i=0; i<l_nb_tiles; ++i) {
                /* keep the tile-compo coding parameters pointer of the current tile coding parameters*/
                l_current_tccp = l_tcp->tccps;
                /*Copy default coding parameters into the current tile coding parameters*/
                memcpy(l_tcp, l_default_tcp, sizeof(mi_tcp_t));
                /* Initialize some values of the current tile coding parameters*/
                l_tcp->cod = 0;
                l_tcp->ppt = 0;
                l_tcp->ppt_data = 00;
                /* Remove memory not owned by this tile in case of early error return. */
                l_tcp->m_mct_decoding_matrix = 00;
                l_tcp->m_nb_max_mct_records = 0;
                l_tcp->m_mct_records = 00;
                l_tcp->m_nb_max_mcc_records = 0;
                l_tcp->m_mcc_records = 00;
                /* Reconnect the tile-compo coding parameters pointer to the current tile coding parameters*/
                l_tcp->tccps = l_current_tccp;

                /* Get the mct_decoding_matrix of the dflt_tile_cp and copy them into the current tile cp*/
                if (l_default_tcp->m_mct_decoding_matrix) {
                        l_tcp->m_mct_decoding_matrix = (mi_FLOAT32*)mi_malloc(l_mct_size);
                        if (! l_tcp->m_mct_decoding_matrix ) {
                                return mi_FALSE;
                        }
                        memcpy(l_tcp->m_mct_decoding_matrix,l_default_tcp->m_mct_decoding_matrix,l_mct_size);
                }

                /* Get the mct_record of the dflt_tile_cp and copy them into the current tile cp*/
                l_mct_records_size = l_default_tcp->m_nb_max_mct_records * (mi_UINT32)sizeof(mi_mct_data_t);
                l_tcp->m_mct_records = (mi_mct_data_t*)mi_malloc(l_mct_records_size);
                if (! l_tcp->m_mct_records) {
                        return mi_FALSE;
                }
                memcpy(l_tcp->m_mct_records, l_default_tcp->m_mct_records,l_mct_records_size);

                /* Copy the mct record data from dflt_tile_cp to the current tile*/
                l_src_mct_rec = l_default_tcp->m_mct_records;
                l_dest_mct_rec = l_tcp->m_mct_records;

                for (j=0;j<l_default_tcp->m_nb_mct_records;++j) {

                        if (l_src_mct_rec->m_data) {

                                l_dest_mct_rec->m_data = (mi_BYTE*) mi_malloc(l_src_mct_rec->m_data_size);
                                if(! l_dest_mct_rec->m_data) {
                                        return mi_FALSE;
                                }
                                memcpy(l_dest_mct_rec->m_data,l_src_mct_rec->m_data,l_src_mct_rec->m_data_size);
                        }

                        ++l_src_mct_rec;
                        ++l_dest_mct_rec;
                        /* Update with each pass to free exactly what has been allocated on early return. */
                        l_tcp->m_nb_max_mct_records += 1;
                }

                /* Get the mcc_record of the dflt_tile_cp and copy them into the current tile cp*/
                l_mcc_records_size = l_default_tcp->m_nb_max_mcc_records * (mi_UINT32)sizeof(mi_simple_mcc_decorrelation_data_t);
                l_tcp->m_mcc_records = (mi_simple_mcc_decorrelation_data_t*) mi_malloc(l_mcc_records_size);
                if (! l_tcp->m_mcc_records) {
                        return mi_FALSE;
                }
                memcpy(l_tcp->m_mcc_records,l_default_tcp->m_mcc_records,l_mcc_records_size);
                l_tcp->m_nb_max_mcc_records = l_default_tcp->m_nb_max_mcc_records;

                /* Copy the mcc record data from dflt_tile_cp to the current tile*/
                l_src_mcc_rec = l_default_tcp->m_mcc_records;
                l_dest_mcc_rec = l_tcp->m_mcc_records;

                for (j=0;j<l_default_tcp->m_nb_max_mcc_records;++j) {

                        if (l_src_mcc_rec->m_decorrelation_array) {
                                l_offset = (mi_UINT32)(l_src_mcc_rec->m_decorrelation_array - l_default_tcp->m_mct_records);
                                l_dest_mcc_rec->m_decorrelation_array = l_tcp->m_mct_records + l_offset;
                        }

                        if (l_src_mcc_rec->m_offset_array) {
                                l_offset = (mi_UINT32)(l_src_mcc_rec->m_offset_array - l_default_tcp->m_mct_records);
                                l_dest_mcc_rec->m_offset_array = l_tcp->m_mct_records + l_offset;
                        }

                        ++l_src_mcc_rec;
                        ++l_dest_mcc_rec;
                }

                /* Copy all the dflt_tile_compo_cp to the current tile cp */
                memcpy(l_current_tccp,l_default_tcp->tccps,l_tccp_size);

                /* Move to next tile cp*/
                ++l_tcp;
        }

        /* Create the current tile decoder*/
        p_j2k->m_tcd = (mi_tcd_t*)mi_tcd_create(mi_TRUE); /* FIXME why a cast ? */
        if (! p_j2k->m_tcd ) {
                return mi_FALSE;
        }

        if ( !mi_tcd_init(p_j2k->m_tcd, l_image, &(p_j2k->m_cp)) ) {
                mi_tcd_destroy(p_j2k->m_tcd);
                p_j2k->m_tcd = 00;
                mi_event_msg(p_manager, EVT_ERROR, "Cannot decode tile, memory error\n");
                return mi_FALSE;
        }

        return mi_TRUE;
}

static const mi_dec_memory_marker_handler_t * mi_j2k_get_marker_handler (mi_UINT32 p_id)
{
        const mi_dec_memory_marker_handler_t *e;
        for (e = j2k_memory_marker_handler_tab; e->id != 0; ++e) {
                if (e->id == p_id) {
                        break; /* we find a handler corresponding to the marker ID*/
                }
        }
        return e;
}

void mi_j2k_destroy (mi_j2k_t *p_j2k)
{
        if (p_j2k == 00) {
                return;
        }

        if (p_j2k->m_is_decoder) {

                if (p_j2k->m_specific_param.m_decoder.m_default_tcp != 00) {
                        mi_j2k_tcp_destroy(p_j2k->m_specific_param.m_decoder.m_default_tcp);
                        mi_free(p_j2k->m_specific_param.m_decoder.m_default_tcp);
                        p_j2k->m_specific_param.m_decoder.m_default_tcp = 00;
                }

                if (p_j2k->m_specific_param.m_decoder.m_header_data != 00) {
                        mi_free(p_j2k->m_specific_param.m_decoder.m_header_data);
                        p_j2k->m_specific_param.m_decoder.m_header_data = 00;
                        p_j2k->m_specific_param.m_decoder.m_header_data_size = 0;
                }
        }
        else {

                if (p_j2k->m_specific_param.m_encoder.m_encoded_tile_data) {
                        mi_free(p_j2k->m_specific_param.m_encoder.m_encoded_tile_data);
                        p_j2k->m_specific_param.m_encoder.m_encoded_tile_data = 00;
                }

                if (p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer) {
                        mi_free(p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer);
                        p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer = 00;
                        p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_current = 00;
                }

                if (p_j2k->m_specific_param.m_encoder.m_header_tile_data) {
                        mi_free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data = 00;
                        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;
                }
        }

        mi_tcd_destroy(p_j2k->m_tcd);

        mi_j2k_cp_destroy(&(p_j2k->m_cp));
        memset(&(p_j2k->m_cp),0,sizeof(mi_cp_t));

        mi_procedure_list_destroy(p_j2k->m_procedure_list);
        p_j2k->m_procedure_list = 00;

        mi_procedure_list_destroy(p_j2k->m_validation_list);
        p_j2k->m_procedure_list = 00;

        j2k_destroy_cstr_index(p_j2k->cstr_index);
        p_j2k->cstr_index = NULL;

        mi_image_destroy(p_j2k->m_private_image);
        p_j2k->m_private_image = NULL;

        mi_image_destroy(p_j2k->m_output_image);
        p_j2k->m_output_image = NULL;

        mi_free(p_j2k);
}

void j2k_destroy_cstr_index (mi_codestream_index_t *p_cstr_ind)
{
        if (p_cstr_ind) {

                if (p_cstr_ind->marker) {
                        mi_free(p_cstr_ind->marker);
                        p_cstr_ind->marker = NULL;
                }

                if (p_cstr_ind->tile_index) {
                        mi_UINT32 it_tile = 0;

                        for (it_tile=0; it_tile < p_cstr_ind->nb_of_tiles; it_tile++) {

                                if(p_cstr_ind->tile_index[it_tile].packet_index) {
                                        mi_free(p_cstr_ind->tile_index[it_tile].packet_index);
                                        p_cstr_ind->tile_index[it_tile].packet_index = NULL;
                                }

                                if(p_cstr_ind->tile_index[it_tile].tp_index){
                                        mi_free(p_cstr_ind->tile_index[it_tile].tp_index);
                                        p_cstr_ind->tile_index[it_tile].tp_index = NULL;
                                }

                                if(p_cstr_ind->tile_index[it_tile].marker){
                                        mi_free(p_cstr_ind->tile_index[it_tile].marker);
                                        p_cstr_ind->tile_index[it_tile].marker = NULL;

                                }
                        }

                        mi_free( p_cstr_ind->tile_index);
                        p_cstr_ind->tile_index = NULL;
                }

                mi_free(p_cstr_ind);
        }
}

static void mi_j2k_tcp_destroy (mi_tcp_t *p_tcp)
{
	if (p_tcp == 00) {
		return;
	}
	
	if (p_tcp->ppt_markers != 00) {
		mi_UINT32 i;
		for (i = 0U; i < p_tcp->ppt_markers_count; ++i) {
			if (p_tcp->ppt_markers[i].m_data != NULL) {
				mi_free(p_tcp->ppt_markers[i].m_data);
			}
		}
		p_tcp->ppt_markers_count = 0U;
		mi_free(p_tcp->ppt_markers);
		p_tcp->ppt_markers = NULL;
	}
	
	if (p_tcp->ppt_buffer != 00) {
		mi_free(p_tcp->ppt_buffer);
		p_tcp->ppt_buffer = 00;
	}
	
	if (p_tcp->tccps != 00) {
		mi_free(p_tcp->tccps);
		p_tcp->tccps = 00;
	}
	
	if (p_tcp->m_mct_coding_matrix != 00) {
		mi_free(p_tcp->m_mct_coding_matrix);
		p_tcp->m_mct_coding_matrix = 00;
	}
	
	if (p_tcp->m_mct_decoding_matrix != 00) {
		mi_free(p_tcp->m_mct_decoding_matrix);
		p_tcp->m_mct_decoding_matrix = 00;
	}
	
	if (p_tcp->m_mcc_records) {
		mi_free(p_tcp->m_mcc_records);
		p_tcp->m_mcc_records = 00;
		p_tcp->m_nb_max_mcc_records = 0;
		p_tcp->m_nb_mcc_records = 0;
	}
	
	if (p_tcp->m_mct_records) {
		mi_mct_data_t * l_mct_data = p_tcp->m_mct_records;
		mi_UINT32 i;
		
		for (i=0;i<p_tcp->m_nb_mct_records;++i) {
			if (l_mct_data->m_data) {
				mi_free(l_mct_data->m_data);
				l_mct_data->m_data = 00;
			}
			
			++l_mct_data;
		}
		
		mi_free(p_tcp->m_mct_records);
		p_tcp->m_mct_records = 00;
	}

	if (p_tcp->mct_norms != 00) {
		mi_free(p_tcp->mct_norms);
		p_tcp->mct_norms = 00;
	}

	mi_j2k_tcp_data_destroy(p_tcp);

}

static void mi_j2k_tcp_data_destroy (mi_tcp_t *p_tcp)
{
        if (p_tcp->m_data) {
                mi_free(p_tcp->m_data);
                p_tcp->m_data = NULL;
                p_tcp->m_data_size = 0;
        }
}

static void mi_j2k_cp_destroy (mi_cp_t *p_cp)
{
	mi_UINT32 l_nb_tiles;
	mi_tcp_t * l_current_tile = 00;

	if (p_cp == 00)
	{
		return;
	}
	if (p_cp->tcps != 00)
	{
		mi_UINT32 i;
		l_current_tile = p_cp->tcps;
		l_nb_tiles = p_cp->th * p_cp->tw;
		
		for (i = 0U; i < l_nb_tiles; ++i)
		{
			mi_j2k_tcp_destroy(l_current_tile);
			++l_current_tile;
		}
		mi_free(p_cp->tcps);
		p_cp->tcps = 00;
	}
	if (p_cp->ppm_markers != 00) {
		mi_UINT32 i;
		for (i = 0U; i < p_cp->ppm_markers_count; ++i) {
			if (p_cp->ppm_markers[i].m_data != NULL) {
				mi_free(p_cp->ppm_markers[i].m_data);
			}
		}
		p_cp->ppm_markers_count = 0U;
		mi_free(p_cp->ppm_markers);
		p_cp->ppm_markers = NULL;
	}
	mi_free(p_cp->ppm_buffer);
	p_cp->ppm_buffer = 00;
	p_cp->ppm_data = NULL; /* ppm_data belongs to the allocated buffer pointed by ppm_buffer */
	mi_free(p_cp->comment);
	p_cp->comment = 00;
	if (! p_cp->m_is_decoder)
	{
		mi_free(p_cp->m_specific_param.m_enc.m_matrice);
		p_cp->m_specific_param.m_enc.m_matrice = 00;
	}
}

static mi_BOOL mi_j2k_need_nb_tile_parts_correction(mi_stream_private_t *p_stream, mi_UINT32 tile_no, mi_BOOL* p_correction_needed, mi_event_mgr_t * p_manager )
{
	mi_BYTE   l_header_data[10];
	mi_OFF_T  l_stream_pos_backup;
	mi_UINT32 l_current_marker;
	mi_UINT32 l_marker_size;
	mi_UINT32 l_tile_no, l_tot_len, l_current_part, l_num_parts;
	
	/* initialize to no correction needed */
	*p_correction_needed = mi_FALSE;
	
	if (!mi_stream_has_seek(p_stream)) {
		/* We can't do much in this case, seek is needed */
		return mi_TRUE;
	}
	
	l_stream_pos_backup = mi_stream_tell(p_stream);
	if (l_stream_pos_backup == -1) {
		/* let's do nothing */
		return mi_TRUE;
	}
	
	for (;;) {
		/* Try to read 2 bytes (the next marker ID) from stream and copy them into the buffer */
		if (mi_stream_read_data(p_stream,l_header_data, 2, p_manager) != 2) {
			/* assume all is OK */
			if (! mi_stream_seek(p_stream, l_stream_pos_backup, p_manager)) {
				return mi_FALSE;
			}
			return mi_TRUE;
		}
		
		/* Read 2 bytes from buffer as the new marker ID */
		mi_read_bytes(l_header_data, &l_current_marker, 2);
		
		if (l_current_marker != J2K_MS_SOT) {
			/* assume all is OK */
			if (! mi_stream_seek(p_stream, l_stream_pos_backup, p_manager)) {
				return mi_FALSE;
			}
			return mi_TRUE;
		}
		
		/* Try to read 2 bytes (the marker size) from stream and copy them into the buffer */
		if (mi_stream_read_data(p_stream, l_header_data, 2, p_manager) != 2) {
			mi_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
			return mi_FALSE;
		}
		
		/* Read 2 bytes from the buffer as the marker size */
		mi_read_bytes(l_header_data, &l_marker_size, 2);
		
		/* Check marker size for SOT Marker */
		if (l_marker_size != 10) {
			mi_event_msg(p_manager, EVT_ERROR, "Inconsistent marker size\n");
			return mi_FALSE;
		}
		l_marker_size -= 2;
		
		if (mi_stream_read_data(p_stream, l_header_data, l_marker_size, p_manager) != l_marker_size) {
			mi_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
			return mi_FALSE;
		}
		
		if (! mi_j2k_get_sot_values(l_header_data, l_marker_size, &l_tile_no, &l_tot_len, &l_current_part, &l_num_parts, p_manager)) {
			return mi_FALSE;
		}
		
		if (l_tile_no == tile_no) {
			/* we found what we were looking for */
			break;
		}
		
		if ((l_tot_len == 0U) || (l_tot_len < 14U)) {
			/* last SOT until EOC or invalid Psot value */
			/* assume all is OK */
			if (! mi_stream_seek(p_stream, l_stream_pos_backup, p_manager)) {
				return mi_FALSE;
			}
			return mi_TRUE;
		}
		l_tot_len -= 12U;
		/* look for next SOT marker */
		if (mi_stream_skip(p_stream, (mi_OFF_T)(l_tot_len), p_manager) != (mi_OFF_T)(l_tot_len)) {
			/* assume all is OK */
			if (! mi_stream_seek(p_stream, l_stream_pos_backup, p_manager)) {
				return mi_FALSE;
			}
			return mi_TRUE;
		}
	}
	
	/* check for correction */
	if (l_current_part == l_num_parts) {
		*p_correction_needed = mi_TRUE;
	}
	
	if (! mi_stream_seek(p_stream, l_stream_pos_backup, p_manager)) {
		return mi_FALSE;
	}
	return mi_TRUE;
}

mi_BOOL mi_j2k_read_tile_header(      mi_j2k_t * p_j2k,
                                                                    mi_UINT32 * p_tile_index,
                                                                    mi_UINT32 * p_data_size,
                                                                    mi_INT32 * p_tile_x0, mi_INT32 * p_tile_y0,
                                                                    mi_INT32 * p_tile_x1, mi_INT32 * p_tile_y1,
                                                                    mi_UINT32 * p_nb_comps,
                                                                    mi_BOOL * p_go_on,
                                                                    mi_stream_private_t *p_stream,
                                                                    mi_event_mgr_t * p_manager )
{
        mi_UINT32 l_current_marker = J2K_MS_SOT;
        mi_UINT32 l_marker_size;
        const mi_dec_memory_marker_handler_t * l_marker_handler = 00;
        mi_tcp_t * l_tcp = NULL;

        /* preconditions */
        assert(p_stream != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        /* Reach the End Of Codestream ?*/
        if (p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_EOC){
                l_current_marker = J2K_MS_EOC;
        }
        /* We need to encounter a SOT marker (a new tile-part header) */
        else if (p_j2k->m_specific_param.m_decoder.m_state != J2K_STATE_TPHSOT){
                return mi_FALSE;
        }

        /* Read into the codestream until reach the EOC or ! can_decode ??? FIXME */
        while ( (!p_j2k->m_specific_param.m_decoder.m_can_decode) && (l_current_marker != J2K_MS_EOC) ) {

                /* Try to read until the Start Of Data is detected */
                while (l_current_marker != J2K_MS_SOD) {
                    
                    if(mi_stream_get_number_byte_left(p_stream) == 0)
                    {
                        p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_NEOC;
                        break;
                    }

                        /* Try to read 2 bytes (the marker size) from stream and copy them into the buffer */
                        if (mi_stream_read_data(p_stream,p_j2k->m_specific_param.m_decoder.m_header_data,2,p_manager) != 2) {
                                mi_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                                return mi_FALSE;
                        }

                        /* Read 2 bytes from the buffer as the marker size */
                        mi_read_bytes(p_j2k->m_specific_param.m_decoder.m_header_data,&l_marker_size,2);

                        /* Check marker size (does not include marker ID but includes marker size) */
                        if (l_marker_size < 2) {
                                mi_event_msg(p_manager, EVT_ERROR, "Inconsistent marker size\n");
                                return mi_FALSE;
                        }

                        /* cf. https://code.google.com/p/openjpeg/issues/detail?id=226 */
                        if (l_current_marker == 0x8080 && mi_stream_get_number_byte_left(p_stream) == 0) {
                                p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_NEOC;
                                break;
                        }

                        /* Why this condition? FIXME */
                        if (p_j2k->m_specific_param.m_decoder.m_state & J2K_STATE_TPH){
                                p_j2k->m_specific_param.m_decoder.m_sot_length -= (l_marker_size + 2);
                        }
                        l_marker_size -= 2; /* Subtract the size of the marker ID already read */

                        /* Get the marker handler from the marker ID */
                        l_marker_handler = mi_j2k_get_marker_handler(l_current_marker);

                        /* Check if the marker is known and if it is the right place to find it */
                        if (! (p_j2k->m_specific_param.m_decoder.m_state & l_marker_handler->states) ) {
                                mi_event_msg(p_manager, EVT_ERROR, "Marker is not compliant with its position\n");
                                return mi_FALSE;
                        }
/* FIXME manage case of unknown marker as in the main header ? */

                        /* Check if the marker size is compatible with the header data size */
                        if (l_marker_size > p_j2k->m_specific_param.m_decoder.m_header_data_size) {
                                mi_BYTE *new_header_data = NULL;
                                /* If we are here, this means we consider this marker as known & we will read it */
                                /* Check enough bytes left in stream before allocation */
                                if ((mi_OFF_T)l_marker_size >  mi_stream_get_number_byte_left(p_stream)) {
                                        mi_event_msg(p_manager, EVT_ERROR, "Marker size inconsistent with stream length\n");
                                        return mi_FALSE;
                                }
                                new_header_data = (mi_BYTE *) mi_realloc(p_j2k->m_specific_param.m_decoder.m_header_data, l_marker_size);
                                if (! new_header_data) {
                                        mi_free(p_j2k->m_specific_param.m_decoder.m_header_data);
                                        p_j2k->m_specific_param.m_decoder.m_header_data = NULL;
                                        p_j2k->m_specific_param.m_decoder.m_header_data_size = 0;
                                        mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to read header\n");
                                        return mi_FALSE;
                                }
                                p_j2k->m_specific_param.m_decoder.m_header_data = new_header_data;
                                p_j2k->m_specific_param.m_decoder.m_header_data_size = l_marker_size;
                        }

                        /* Try to read the rest of the marker segment from stream and copy them into the buffer */
                        if (mi_stream_read_data(p_stream,p_j2k->m_specific_param.m_decoder.m_header_data,l_marker_size,p_manager) != l_marker_size) {
                                mi_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                                return mi_FALSE;
                        }

                        if (!l_marker_handler->handler) {
                                /* See issue #175 */
                                mi_event_msg(p_manager, EVT_ERROR, "Not sure how that happened.\n");
                                return mi_FALSE;
                        }
                        /* Read the marker segment with the correct marker handler */
                        if (! (*(l_marker_handler->handler))(p_j2k,p_j2k->m_specific_param.m_decoder.m_header_data,l_marker_size,p_manager)) {
                                mi_event_msg(p_manager, EVT_ERROR, "Fail to read the current marker segment (%#x)\n", l_current_marker);
                                return mi_FALSE;
                        }

                        /* Add the marker to the codestream index*/
                        if (mi_FALSE == mi_j2k_add_tlmarker(p_j2k->m_current_tile_number,
                                                p_j2k->cstr_index,
                                                l_marker_handler->id,
                                                (mi_UINT32) mi_stream_tell(p_stream) - l_marker_size - 4,
                                                l_marker_size + 4 )) {
                                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to add tl marker\n");
                                return mi_FALSE;
                        }

                        /* Keep the position of the last SOT marker read */
                        if ( l_marker_handler->id == J2K_MS_SOT ) {
                                mi_UINT32 sot_pos = (mi_UINT32) mi_stream_tell(p_stream) - l_marker_size - 4 ;
                                if (sot_pos > p_j2k->m_specific_param.m_decoder.m_last_sot_read_pos)
                                {
                                        p_j2k->m_specific_param.m_decoder.m_last_sot_read_pos = sot_pos;
                                }
                        }

                        if (p_j2k->m_specific_param.m_decoder.m_skip_data) {
                                /* Skip the rest of the tile part header*/
                                if (mi_stream_skip(p_stream,p_j2k->m_specific_param.m_decoder.m_sot_length,p_manager) != p_j2k->m_specific_param.m_decoder.m_sot_length) {
                                        mi_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                                        return mi_FALSE;
                                }
                                l_current_marker = J2K_MS_SOD; /* Normally we reached a SOD */
                        }
                        else {
                                /* Try to read 2 bytes (the next marker ID) from stream and copy them into the buffer*/
                                if (mi_stream_read_data(p_stream,p_j2k->m_specific_param.m_decoder.m_header_data,2,p_manager) != 2) {
                                        mi_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                                        return mi_FALSE;
                                }
                                /* Read 2 bytes from the buffer as the new marker ID */
                                mi_read_bytes(p_j2k->m_specific_param.m_decoder.m_header_data,&l_current_marker,2);
                        }
                }
                if(mi_stream_get_number_byte_left(p_stream) == 0
                    && p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_NEOC)
                    break;

                /* If we didn't skip data before, we need to read the SOD marker*/
                if (! p_j2k->m_specific_param.m_decoder.m_skip_data) {
                        /* Try to read the SOD marker and skip data ? FIXME */
                        if (! mi_j2k_read_sod(p_j2k, p_stream, p_manager)) {
                                return mi_FALSE;
                        }
                        if (p_j2k->m_specific_param.m_decoder.m_can_decode && !p_j2k->m_specific_param.m_decoder.m_nb_tile_parts_correction_checked) {
                                /* Issue 254 */
                                mi_BOOL l_correction_needed;
													
                                p_j2k->m_specific_param.m_decoder.m_nb_tile_parts_correction_checked = 1;
                                if(!mi_j2k_need_nb_tile_parts_correction(p_stream, p_j2k->m_current_tile_number, &l_correction_needed, p_manager)) {
                                        mi_event_msg(p_manager, EVT_ERROR, "mi_j2k_apply_nb_tile_parts_correction error\n");
                                        return mi_FALSE;
                                }
                                if (l_correction_needed) {
                                        mi_UINT32 l_nb_tiles = p_j2k->m_cp.tw * p_j2k->m_cp.th;
                                        mi_UINT32 l_tile_no;

                                        p_j2k->m_specific_param.m_decoder.m_can_decode = 0;
                                        p_j2k->m_specific_param.m_decoder.m_nb_tile_parts_correction = 1;
                                        /* correct tiles */
                                        for (l_tile_no = 0U; l_tile_no < l_nb_tiles; ++l_tile_no) {
                                                if (p_j2k->m_cp.tcps[l_tile_no].m_nb_tile_parts != 0U) {
                                                        p_j2k->m_cp.tcps[l_tile_no].m_nb_tile_parts+=1;
                                                }
                                        }
                                        mi_event_msg(p_manager, EVT_WARNING, "Non conformant codestream TPsot==TNsot.\n");
                                }
                        }
                        if (! p_j2k->m_specific_param.m_decoder.m_can_decode){
                                /* Try to read 2 bytes (the next marker ID) from stream and copy them into the buffer */
                                if (mi_stream_read_data(p_stream,p_j2k->m_specific_param.m_decoder.m_header_data,2,p_manager) != 2) {
                                        mi_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                                        return mi_FALSE;
                                }

                                /* Read 2 bytes from buffer as the new marker ID */
                                mi_read_bytes(p_j2k->m_specific_param.m_decoder.m_header_data,&l_current_marker,2);
                        }
                }
                else {
                        /* Indicate we will try to read a new tile-part header*/
                        p_j2k->m_specific_param.m_decoder.m_skip_data = 0;
                        p_j2k->m_specific_param.m_decoder.m_can_decode = 0;
                        p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_TPHSOT;

                        /* Try to read 2 bytes (the next marker ID) from stream and copy them into the buffer */
                        if (mi_stream_read_data(p_stream,p_j2k->m_specific_param.m_decoder.m_header_data,2,p_manager) != 2) {
                                mi_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                                return mi_FALSE;
                        }

                        /* Read 2 bytes from buffer as the new marker ID */
                        mi_read_bytes(p_j2k->m_specific_param.m_decoder.m_header_data,&l_current_marker,2);
                }
        }

        /* Current marker is the EOC marker ?*/
        if (l_current_marker == J2K_MS_EOC) {
                if (p_j2k->m_specific_param.m_decoder.m_state != J2K_STATE_EOC ){
                        p_j2k->m_current_tile_number = 0;
                        p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_EOC;
                }
        }

        /* FIXME DOC ???*/
        if ( ! p_j2k->m_specific_param.m_decoder.m_can_decode) {
                mi_UINT32 l_nb_tiles = p_j2k->m_cp.th * p_j2k->m_cp.tw;
                l_tcp = p_j2k->m_cp.tcps + p_j2k->m_current_tile_number;

                while( (p_j2k->m_current_tile_number < l_nb_tiles) && (l_tcp->m_data == 00) ) {
                        ++p_j2k->m_current_tile_number;
                        ++l_tcp;
                }

                if (p_j2k->m_current_tile_number == l_nb_tiles) {
                        *p_go_on = mi_FALSE;
                        return mi_TRUE;
                }
        }

        if (! mi_j2k_merge_ppt(p_j2k->m_cp.tcps + p_j2k->m_current_tile_number, p_manager)) {
                mi_event_msg(p_manager, EVT_ERROR, "Failed to merge PPT data\n");
                return mi_FALSE;
        }
        /*FIXME ???*/
        if (! mi_tcd_init_decode_tile(p_j2k->m_tcd, p_j2k->m_current_tile_number, p_manager)) {
                mi_event_msg(p_manager, EVT_ERROR, "Cannot decode tile, memory error\n");
                return mi_FALSE;
        }

        mi_event_msg(p_manager, EVT_INFO, "Header of tile %d / %d has been read.\n",
                        p_j2k->m_current_tile_number+1, (p_j2k->m_cp.th * p_j2k->m_cp.tw));

        *p_tile_index = p_j2k->m_current_tile_number;
        *p_go_on = mi_TRUE;
        *p_data_size = mi_tcd_get_decoded_tile_size(p_j2k->m_tcd);
        *p_tile_x0 = p_j2k->m_tcd->tcd_image->tiles->x0;
        *p_tile_y0 = p_j2k->m_tcd->tcd_image->tiles->y0;
        *p_tile_x1 = p_j2k->m_tcd->tcd_image->tiles->x1;
        *p_tile_y1 = p_j2k->m_tcd->tcd_image->tiles->y1;
        *p_nb_comps = p_j2k->m_tcd->tcd_image->tiles->numcomps;

         p_j2k->m_specific_param.m_decoder.m_state |= 0x0080;/* FIXME J2K_DEC_STATE_DATA;*/

        return mi_TRUE;
}

mi_BOOL mi_j2k_decode_tile (  mi_j2k_t * p_j2k,
                                                        mi_UINT32 p_tile_index,
                                                        mi_BYTE * p_data,
                                                        mi_UINT32 p_data_size,
                                                        mi_stream_private_t *p_stream,
                                                        mi_event_mgr_t * p_manager )
{
        mi_UINT32 l_current_marker;
        mi_BYTE l_data [2];
        mi_tcp_t * l_tcp;

        /* preconditions */
        assert(p_stream != 00);
        assert(p_j2k != 00);
        assert(p_manager != 00);

        if ( !(p_j2k->m_specific_param.m_decoder.m_state & 0x0080/*FIXME J2K_DEC_STATE_DATA*/)
                || (p_tile_index != p_j2k->m_current_tile_number) ) {
                return mi_FALSE;
        }

        l_tcp = &(p_j2k->m_cp.tcps[p_tile_index]);
        if (! l_tcp->m_data) {
                mi_j2k_tcp_destroy(l_tcp);
                return mi_FALSE;
        }

        if (! mi_tcd_decode_tile(      p_j2k->m_tcd,
                                                                l_tcp->m_data,
                                                                l_tcp->m_data_size,
                                                                p_tile_index,
                                                                p_j2k->cstr_index, p_manager) ) {
                mi_j2k_tcp_destroy(l_tcp);
                p_j2k->m_specific_param.m_decoder.m_state |= 0x8000;/*FIXME J2K_DEC_STATE_ERR;*/
                mi_event_msg(p_manager, EVT_ERROR, "Failed to decode.\n");
                return mi_FALSE;
        }

        if (! mi_tcd_update_tile_data(p_j2k->m_tcd,p_data,p_data_size)) {
                return mi_FALSE;
        }

        /* To avoid to destroy the tcp which can be useful when we try to decode a tile decoded before (cf j2k_random_tile_access)
         * we destroy just the data which will be re-read in read_tile_header*/
        /*mi_j2k_tcp_destroy(l_tcp);
        p_j2k->m_tcd->tcp = 0;*/
        mi_j2k_tcp_data_destroy(l_tcp);

        p_j2k->m_specific_param.m_decoder.m_can_decode = 0;
        p_j2k->m_specific_param.m_decoder.m_state &= (~ (0x0080u));/* FIXME J2K_DEC_STATE_DATA);*/

        if(mi_stream_get_number_byte_left(p_stream) == 0 
            && p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_NEOC){
            return mi_TRUE;
        }

        if (p_j2k->m_specific_param.m_decoder.m_state != 0x0100){ /*FIXME J2K_DEC_STATE_EOC)*/
                if (mi_stream_read_data(p_stream,l_data,2,p_manager) != 2) {
                        mi_event_msg(p_manager, EVT_ERROR, "Stream too short\n");
                        return mi_FALSE;
                }

                mi_read_bytes(l_data,&l_current_marker,2);

                if (l_current_marker == J2K_MS_EOC) {
                        p_j2k->m_current_tile_number = 0;
                        p_j2k->m_specific_param.m_decoder.m_state =  0x0100;/*FIXME J2K_DEC_STATE_EOC;*/
                }
                else if (l_current_marker != J2K_MS_SOT)
                {       
                        if(mi_stream_get_number_byte_left(p_stream) == 0) {
                            p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_NEOC;
                            mi_event_msg(p_manager, EVT_WARNING, "Stream does not end with EOC\n");
                            return mi_TRUE;
                        }
                        mi_event_msg(p_manager, EVT_ERROR, "Stream too short, expected SOT\n");
                        return mi_FALSE;
                }
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_update_image_data (mi_tcd_t * p_tcd, mi_BYTE * p_data, mi_image_t* p_output_image)
{
        mi_UINT32 i,j,k = 0;
        mi_UINT32 l_width_src,l_height_src;
        mi_UINT32 l_width_dest,l_height_dest;
        mi_INT32 l_offset_x0_src, l_offset_y0_src, l_offset_x1_src, l_offset_y1_src;
        mi_SIZE_T l_start_offset_src, l_line_offset_src, l_end_offset_src ;
        mi_UINT32 l_start_x_dest , l_start_y_dest;
        mi_UINT32 l_x0_dest, l_y0_dest, l_x1_dest, l_y1_dest;
        mi_SIZE_T l_start_offset_dest, l_line_offset_dest;

        mi_image_comp_t * l_img_comp_src = 00;
        mi_image_comp_t * l_img_comp_dest = 00;

        mi_tcd_tilecomp_t * l_tilec = 00;
        mi_image_t * l_image_src = 00;
        mi_UINT32 l_size_comp, l_remaining;
        mi_INT32 * l_dest_ptr;
        mi_tcd_resolution_t* l_res= 00;

        l_tilec = p_tcd->tcd_image->tiles->comps;
        l_image_src = p_tcd->image;
        l_img_comp_src = l_image_src->comps;

        l_img_comp_dest = p_output_image->comps;

        for (i=0; i<l_image_src->numcomps; i++) {

                /* Allocate output component buffer if necessary */
                if (!l_img_comp_dest->data) {

                        l_img_comp_dest->data = (mi_INT32*) mi_calloc((mi_SIZE_T)l_img_comp_dest->w * (mi_SIZE_T)l_img_comp_dest->h, sizeof(mi_INT32));
                        if (! l_img_comp_dest->data) {
                                return mi_FALSE;
                        }
                }

                /* Copy info from decoded comp image to output image */
                l_img_comp_dest->resno_decoded = l_img_comp_src->resno_decoded;

                /*-----*/
                /* Compute the precision of the output buffer */
                l_size_comp = l_img_comp_src->prec >> 3; /*(/ 8)*/
                l_remaining = l_img_comp_src->prec & 7;  /* (%8) */
                l_res = l_tilec->resolutions + l_img_comp_src->resno_decoded;

                if (l_remaining) {
                        ++l_size_comp;
                }

                if (l_size_comp == 3) {
                        l_size_comp = 4;
                }
                /*-----*/

                /* Current tile component size*/
                /*if (i == 0) {
                fprintf(stdout, "SRC: l_res_x0=%d, l_res_x1=%d, l_res_y0=%d, l_res_y1=%d\n",
                                l_res->x0, l_res->x1, l_res->y0, l_res->y1);
                }*/

                l_width_src = (mi_UINT32)(l_res->x1 - l_res->x0);
                l_height_src = (mi_UINT32)(l_res->y1 - l_res->y0);

                /* Border of the current output component*/
                l_x0_dest = mi_uint_ceildivpow2(l_img_comp_dest->x0, l_img_comp_dest->factor);
                l_y0_dest = mi_uint_ceildivpow2(l_img_comp_dest->y0, l_img_comp_dest->factor);
                l_x1_dest = l_x0_dest + l_img_comp_dest->w; /* can't overflow given that image->x1 is uint32 */
                l_y1_dest = l_y0_dest + l_img_comp_dest->h;

                /*if (i == 0) {
                fprintf(stdout, "DEST: l_x0_dest=%d, l_x1_dest=%d, l_y0_dest=%d, l_y1_dest=%d (%d)\n",
                                l_x0_dest, l_x1_dest, l_y0_dest, l_y1_dest, l_img_comp_dest->factor );
                }*/

                /*-----*/
                /* Compute the area (l_offset_x0_src, l_offset_y0_src, l_offset_x1_src, l_offset_y1_src)
                 * of the input buffer (decoded tile component) which will be move
                 * in the output buffer. Compute the area of the output buffer (l_start_x_dest,
                 * l_start_y_dest, l_width_dest, l_height_dest)  which will be modified
                 * by this input area.
                 * */
                assert( l_res->x0 >= 0);
                assert( l_res->x1 >= 0);
                if ( l_x0_dest < (mi_UINT32)l_res->x0 ) {
                        l_start_x_dest = (mi_UINT32)l_res->x0 - l_x0_dest;
                        l_offset_x0_src = 0;

                        if ( l_x1_dest >= (mi_UINT32)l_res->x1 ) {
                                l_width_dest = l_width_src;
                                l_offset_x1_src = 0;
                        }
                        else {
                                l_width_dest = l_x1_dest - (mi_UINT32)l_res->x0 ;
                                l_offset_x1_src = (mi_INT32)(l_width_src - l_width_dest);
                        }
                }
                else {
                        l_start_x_dest = 0U;
                        l_offset_x0_src = (mi_INT32)l_x0_dest - l_res->x0;

                        if ( l_x1_dest >= (mi_UINT32)l_res->x1 ) {
                                l_width_dest = l_width_src - (mi_UINT32)l_offset_x0_src;
                                l_offset_x1_src = 0;
                        }
                        else {
                                l_width_dest = l_img_comp_dest->w ;
                                l_offset_x1_src = l_res->x1 - (mi_INT32)l_x1_dest;
                        }
                }

                if ( l_y0_dest < (mi_UINT32)l_res->y0 ) {
                        l_start_y_dest = (mi_UINT32)l_res->y0 - l_y0_dest;
                        l_offset_y0_src = 0;

                        if ( l_y1_dest >= (mi_UINT32)l_res->y1 ) {
                                l_height_dest = l_height_src;
                                l_offset_y1_src = 0;
                        }
                        else {
                                l_height_dest = l_y1_dest - (mi_UINT32)l_res->y0 ;
                                l_offset_y1_src =  (mi_INT32)(l_height_src - l_height_dest);
                        }
                }
                else {
                        l_start_y_dest = 0U;
                        l_offset_y0_src = (mi_INT32)l_y0_dest - l_res->y0;

                        if ( l_y1_dest >= (mi_UINT32)l_res->y1 ) {
                                l_height_dest = l_height_src - (mi_UINT32)l_offset_y0_src;
                                l_offset_y1_src = 0;
                        }
                        else {
                                l_height_dest = l_img_comp_dest->h ;
                                l_offset_y1_src = l_res->y1 - (mi_INT32)l_y1_dest;
                        }
                }

                if( (l_offset_x0_src < 0 ) || (l_offset_y0_src < 0 ) || (l_offset_x1_src < 0 ) || (l_offset_y1_src < 0 ) ){
                        return mi_FALSE;
                }
                /* testcase 2977.pdf.asan.67.2198 */
                if ((mi_INT32)l_width_dest < 0 || (mi_INT32)l_height_dest < 0) {
                        return mi_FALSE;
                }
                /*-----*/

                /* Compute the input buffer offset */
                l_start_offset_src = (mi_SIZE_T)l_offset_x0_src + (mi_SIZE_T)l_offset_y0_src * (mi_SIZE_T)l_width_src;
                l_line_offset_src  = (mi_SIZE_T)l_offset_x1_src + (mi_SIZE_T)l_offset_x0_src;
                l_end_offset_src   = (mi_SIZE_T)l_offset_y1_src * (mi_SIZE_T)l_width_src - (mi_SIZE_T)l_offset_x0_src;

                /* Compute the output buffer offset */
                l_start_offset_dest = (mi_SIZE_T)l_start_x_dest + (mi_SIZE_T)l_start_y_dest * (mi_SIZE_T)l_img_comp_dest->w;
                l_line_offset_dest  = (mi_SIZE_T)l_img_comp_dest->w - (mi_SIZE_T)l_width_dest;

                /* Move the output buffer to the first place where we will write*/
                l_dest_ptr = l_img_comp_dest->data + l_start_offset_dest;

                /*if (i == 0) {
                        fprintf(stdout, "COMPO[%d]:\n",i);
                        fprintf(stdout, "SRC: l_start_x_src=%d, l_start_y_src=%d, l_width_src=%d, l_height_src=%d\n"
                                        "\t tile offset:%d, %d, %d, %d\n"
                                        "\t buffer offset: %d; %d, %d\n",
                                        l_res->x0, l_res->y0, l_width_src, l_height_src,
                                        l_offset_x0_src, l_offset_y0_src, l_offset_x1_src, l_offset_y1_src,
                                        l_start_offset_src, l_line_offset_src, l_end_offset_src);

                        fprintf(stdout, "DEST: l_start_x_dest=%d, l_start_y_dest=%d, l_width_dest=%d, l_height_dest=%d\n"
                                        "\t start offset: %d, line offset= %d\n",
                                        l_start_x_dest, l_start_y_dest, l_width_dest, l_height_dest, l_start_offset_dest, l_line_offset_dest);
                }*/

                switch (l_size_comp) {
                        case 1:
                                {
                                        mi_CHAR * l_src_ptr = (mi_CHAR*) p_data;
                                        l_src_ptr += l_start_offset_src; /* Move to the first place where we will read*/

                                        if (l_img_comp_src->sgnd) {
                                                for (j = 0 ; j < l_height_dest ; ++j) {
                                                        for ( k = 0 ; k < l_width_dest ; ++k) {
                                                                *(l_dest_ptr++) = (mi_INT32) (*(l_src_ptr++)); /* Copy only the data needed for the output image */
                                                        }

                                                        l_dest_ptr+= l_line_offset_dest; /* Move to the next place where we will write */
                                                        l_src_ptr += l_line_offset_src ; /* Move to the next place where we will read */
                                                }
                                        }
                                        else {
                                                for ( j = 0 ; j < l_height_dest ; ++j ) {
                                                        for ( k = 0 ; k < l_width_dest ; ++k) {
                                                                *(l_dest_ptr++) = (mi_INT32) ((*(l_src_ptr++))&0xff);
                                                        }

                                                        l_dest_ptr+= l_line_offset_dest;
                                                        l_src_ptr += l_line_offset_src;
                                                }
                                        }

                                        l_src_ptr += l_end_offset_src; /* Move to the end of this component-part of the input buffer */
                                        p_data = (mi_BYTE*) l_src_ptr; /* Keep the current position for the next component-part */
                                }
                                break;
                        case 2:
                                {
                                        mi_INT16 * l_src_ptr = (mi_INT16 *) p_data;
                                        l_src_ptr += l_start_offset_src;

                                        if (l_img_comp_src->sgnd) {
                                                for (j=0;j<l_height_dest;++j) {
                                                        for (k=0;k<l_width_dest;++k) {
                                                                *(l_dest_ptr++) = *(l_src_ptr++);
                                                        }

                                                        l_dest_ptr+= l_line_offset_dest;
                                                        l_src_ptr += l_line_offset_src ;
                                                }
                                        }
                                        else {
                                                for (j=0;j<l_height_dest;++j) {
                                                        for (k=0;k<l_width_dest;++k) {
                                                                *(l_dest_ptr++) = (*(l_src_ptr++))&0xffff;
                                                        }

                                                        l_dest_ptr+= l_line_offset_dest;
                                                        l_src_ptr += l_line_offset_src ;
                                                }
                                        }

                                        l_src_ptr += l_end_offset_src;
                                        p_data = (mi_BYTE*) l_src_ptr;
                                }
                                break;
                        case 4:
                                {
                                        mi_INT32 * l_src_ptr = (mi_INT32 *) p_data;
                                        l_src_ptr += l_start_offset_src;

                                        for (j=0;j<l_height_dest;++j) {
                                                for (k=0;k<l_width_dest;++k) {
                                                        *(l_dest_ptr++) = (*(l_src_ptr++));
                                                }

                                                l_dest_ptr+= l_line_offset_dest;
                                                l_src_ptr += l_line_offset_src ;
                                        }

                                        l_src_ptr += l_end_offset_src;
                                        p_data = (mi_BYTE*) l_src_ptr;
                                }
                                break;
                }

                ++l_img_comp_dest;
                ++l_img_comp_src;
                ++l_tilec;
        }

        return mi_TRUE;
}

mi_BOOL mi_j2k_set_decode_area(       mi_j2k_t *p_j2k,
                                                                    mi_image_t* p_image,
                                                                    mi_INT32 p_start_x, mi_INT32 p_start_y,
                                                                    mi_INT32 p_end_x, mi_INT32 p_end_y,
                                                                    mi_event_mgr_t * p_manager )
{
        mi_cp_t * l_cp = &(p_j2k->m_cp);
        mi_image_t * l_image = p_j2k->m_private_image;

        mi_UINT32 it_comp;
        mi_INT32 l_comp_x1, l_comp_y1;
        mi_image_comp_t* l_img_comp = NULL;

        /* Check if we are read the main header */
        if (p_j2k->m_specific_param.m_decoder.m_state != J2K_STATE_TPHSOT) { /* FIXME J2K_DEC_STATE_TPHSOT)*/
                mi_event_msg(p_manager, EVT_ERROR, "Need to decode the main header before begin to decode the remaining codestream");
                return mi_FALSE;
        }

        if ( !p_start_x && !p_start_y && !p_end_x && !p_end_y){
                mi_event_msg(p_manager, EVT_INFO, "No decoded area parameters, set the decoded area to the whole image\n");

                p_j2k->m_specific_param.m_decoder.m_start_tile_x = 0;
                p_j2k->m_specific_param.m_decoder.m_start_tile_y = 0;
                p_j2k->m_specific_param.m_decoder.m_end_tile_x = l_cp->tw;
                p_j2k->m_specific_param.m_decoder.m_end_tile_y = l_cp->th;

                return mi_TRUE;
        }

        /* ----- */
        /* Check if the positions provided by the user are correct */

        /* Left */
        assert(p_start_x >= 0 );
        assert(p_start_y >= 0 );

        if ((mi_UINT32)p_start_x > l_image->x1 ) {
                mi_event_msg(p_manager, EVT_ERROR,
                        "Left position of the decoded area (region_x0=%d) is outside the image area (Xsiz=%d).\n",
                        p_start_x, l_image->x1);
                return mi_FALSE;
        }
        else if ((mi_UINT32)p_start_x < l_image->x0){
                mi_event_msg(p_manager, EVT_WARNING,
                                "Left position of the decoded area (region_x0=%d) is outside the image area (XOsiz=%d).\n",
                                p_start_x, l_image->x0);
                p_j2k->m_specific_param.m_decoder.m_start_tile_x = 0;
                p_image->x0 = l_image->x0;
        }
        else {
                p_j2k->m_specific_param.m_decoder.m_start_tile_x = ((mi_UINT32)p_start_x - l_cp->tx0) / l_cp->tdx;
                p_image->x0 = (mi_UINT32)p_start_x;
        }

        /* Up */
        if ((mi_UINT32)p_start_y > l_image->y1){
                mi_event_msg(p_manager, EVT_ERROR,
                                "Up position of the decoded area (region_y0=%d) is outside the image area (Ysiz=%d).\n",
                                p_start_y, l_image->y1);
                return mi_FALSE;
        }
        else if ((mi_UINT32)p_start_y < l_image->y0){
                mi_event_msg(p_manager, EVT_WARNING,
                                "Up position of the decoded area (region_y0=%d) is outside the image area (YOsiz=%d).\n",
                                p_start_y, l_image->y0);
                p_j2k->m_specific_param.m_decoder.m_start_tile_y = 0;
                p_image->y0 = l_image->y0;
        }
        else {
                p_j2k->m_specific_param.m_decoder.m_start_tile_y = ((mi_UINT32)p_start_y - l_cp->ty0) / l_cp->tdy;
                p_image->y0 = (mi_UINT32)p_start_y;
        }

        /* Right */
        assert((mi_UINT32)p_end_x > 0);
        assert((mi_UINT32)p_end_y > 0);
        if ((mi_UINT32)p_end_x < l_image->x0) {
                mi_event_msg(p_manager, EVT_ERROR,
                        "Right position of the decoded area (region_x1=%d) is outside the image area (XOsiz=%d).\n",
                        p_end_x, l_image->x0);
                return mi_FALSE;
        }
        else if ((mi_UINT32)p_end_x > l_image->x1) {
                mi_event_msg(p_manager, EVT_WARNING,
                        "Right position of the decoded area (region_x1=%d) is outside the image area (Xsiz=%d).\n",
                        p_end_x, l_image->x1);
                p_j2k->m_specific_param.m_decoder.m_end_tile_x = l_cp->tw;
                p_image->x1 = l_image->x1;
        }
        else {
                p_j2k->m_specific_param.m_decoder.m_end_tile_x = (mi_UINT32)mi_int_ceildiv(p_end_x - (mi_INT32)l_cp->tx0, (mi_INT32)l_cp->tdx);
                p_image->x1 = (mi_UINT32)p_end_x;
        }

        /* Bottom */
        if ((mi_UINT32)p_end_y < l_image->y0) {
                mi_event_msg(p_manager, EVT_ERROR,
                        "Bottom position of the decoded area (region_y1=%d) is outside the image area (YOsiz=%d).\n",
                        p_end_y, l_image->y0);
                return mi_FALSE;
        }
        if ((mi_UINT32)p_end_y > l_image->y1){
                mi_event_msg(p_manager, EVT_WARNING,
                        "Bottom position of the decoded area (region_y1=%d) is outside the image area (Ysiz=%d).\n",
                        p_end_y, l_image->y1);
                p_j2k->m_specific_param.m_decoder.m_end_tile_y = l_cp->th;
                p_image->y1 = l_image->y1;
        }
        else{
                p_j2k->m_specific_param.m_decoder.m_end_tile_y = (mi_UINT32)mi_int_ceildiv(p_end_y - (mi_INT32)l_cp->ty0, (mi_INT32)l_cp->tdy);
                p_image->y1 = (mi_UINT32)p_end_y;
        }
        /* ----- */

        p_j2k->m_specific_param.m_decoder.m_discard_tiles = 1;

        l_img_comp = p_image->comps;
        for (it_comp=0; it_comp < p_image->numcomps; ++it_comp)
        {
                mi_INT32 l_h,l_w;

                l_img_comp->x0 = (mi_UINT32)mi_int_ceildiv((mi_INT32)p_image->x0, (mi_INT32)l_img_comp->dx);
                l_img_comp->y0 = (mi_UINT32)mi_int_ceildiv((mi_INT32)p_image->y0, (mi_INT32)l_img_comp->dy);
                l_comp_x1 = mi_int_ceildiv((mi_INT32)p_image->x1, (mi_INT32)l_img_comp->dx);
                l_comp_y1 = mi_int_ceildiv((mi_INT32)p_image->y1, (mi_INT32)l_img_comp->dy);

                l_w = mi_int_ceildivpow2(l_comp_x1, (mi_INT32)l_img_comp->factor)
                                - mi_int_ceildivpow2((mi_INT32)l_img_comp->x0, (mi_INT32)l_img_comp->factor);
                if (l_w < 0){
                        mi_event_msg(p_manager, EVT_ERROR,
                                "Size x of the decoded component image is incorrect (comp[%d].w=%d).\n",
                                it_comp, l_w);
                        return mi_FALSE;
                }
                l_img_comp->w = (mi_UINT32)l_w;

                l_h = mi_int_ceildivpow2(l_comp_y1, (mi_INT32)l_img_comp->factor)
                                - mi_int_ceildivpow2((mi_INT32)l_img_comp->y0, (mi_INT32)l_img_comp->factor);
                if (l_h < 0){
                        mi_event_msg(p_manager, EVT_ERROR,
                                "Size y of the decoded component image is incorrect (comp[%d].h=%d).\n",
                                it_comp, l_h);
                        return mi_FALSE;
                }
                l_img_comp->h = (mi_UINT32)l_h;

                l_img_comp++;
        }

        mi_event_msg( p_manager, EVT_INFO,"Setting decoding area to %d,%d,%d,%d\n",
                        p_image->x0, p_image->y0, p_image->x1, p_image->y1);

        return mi_TRUE;
}

mi_j2k_t* mi_j2k_create_decompress(void)
{
        mi_j2k_t *l_j2k = (mi_j2k_t*) mi_calloc(1,sizeof(mi_j2k_t));
        if (!l_j2k) {
                return 00;
        }

        l_j2k->m_is_decoder = 1;
        l_j2k->m_cp.m_is_decoder = 1;

#ifdef mi_DISABLE_TPSOT_FIX
        l_j2k->m_specific_param.m_decoder.m_nb_tile_parts_correction_checked = 1;
#endif

        l_j2k->m_specific_param.m_decoder.m_default_tcp = (mi_tcp_t*) mi_calloc(1,sizeof(mi_tcp_t));
        if (!l_j2k->m_specific_param.m_decoder.m_default_tcp) {
                mi_j2k_destroy(l_j2k);
                return 00;
        }

        l_j2k->m_specific_param.m_decoder.m_header_data = (mi_BYTE *) mi_calloc(1,mi_J2K_DEFAULT_HEADER_SIZE);
        if (! l_j2k->m_specific_param.m_decoder.m_header_data) {
                mi_j2k_destroy(l_j2k);
                return 00;
        }

        l_j2k->m_specific_param.m_decoder.m_header_data_size = mi_J2K_DEFAULT_HEADER_SIZE;

        l_j2k->m_specific_param.m_decoder.m_tile_ind_to_dec = -1 ;

        l_j2k->m_specific_param.m_decoder.m_last_sot_read_pos = 0 ;

        /* codestream index creation */
        l_j2k->cstr_index = mi_j2k_create_cstr_index();
        if (!l_j2k->cstr_index){
                mi_j2k_destroy(l_j2k);
                return 00;
        }

        /* validation list creation */
        l_j2k->m_validation_list = mi_procedure_list_create();
        if (! l_j2k->m_validation_list) {
                mi_j2k_destroy(l_j2k);
                return 00;
        }

        /* execution list creation */
        l_j2k->m_procedure_list = mi_procedure_list_create();
        if (! l_j2k->m_procedure_list) {
                mi_j2k_destroy(l_j2k);
                return 00;
        }

        return l_j2k;
}

static mi_codestream_index_t* mi_j2k_create_cstr_index(void)
{
        mi_codestream_index_t* cstr_index = (mi_codestream_index_t*)
                        mi_calloc(1,sizeof(mi_codestream_index_t));
        if (!cstr_index)
                return NULL;

        cstr_index->maxmarknum = 100;
        cstr_index->marknum = 0;
        cstr_index->marker = (mi_marker_info_t*)
                        mi_calloc(cstr_index->maxmarknum, sizeof(mi_marker_info_t));
        if (!cstr_index-> marker) {
                mi_free(cstr_index);
                return NULL;
        }

        cstr_index->tile_index = NULL;

        return cstr_index;
}

static mi_UINT32 mi_j2k_get_SPCod_SPCoc_size (       mi_j2k_t *p_j2k,
                                                                                mi_UINT32 p_tile_no,
                                                                                mi_UINT32 p_comp_no )
{
        mi_cp_t *l_cp = 00;
        mi_tcp_t *l_tcp = 00;
        mi_tccp_t *l_tccp = 00;

        /* preconditions */
        assert(p_j2k != 00);

        l_cp = &(p_j2k->m_cp);
        l_tcp = &l_cp->tcps[p_tile_no];
        l_tccp = &l_tcp->tccps[p_comp_no];

        /* preconditions again */
        assert(p_tile_no < (l_cp->tw * l_cp->th));
        assert(p_comp_no < p_j2k->m_private_image->numcomps);

        if (l_tccp->csty & J2K_CCP_CSTY_PRT) {
                return 5 + l_tccp->numresolutions;
        }
        else {
                return 5;
        }
}

static mi_BOOL mi_j2k_compare_SPCod_SPCoc(mi_j2k_t *p_j2k, mi_UINT32 p_tile_no, mi_UINT32 p_first_comp_no, mi_UINT32 p_second_comp_no)
{
	mi_UINT32 i;
	mi_cp_t *l_cp = NULL;
	mi_tcp_t *l_tcp = NULL;
	mi_tccp_t *l_tccp0 = NULL;
	mi_tccp_t *l_tccp1 = NULL;
	
	/* preconditions */
	assert(p_j2k != 00);
	
	l_cp = &(p_j2k->m_cp);
	l_tcp = &l_cp->tcps[p_tile_no];
	l_tccp0 = &l_tcp->tccps[p_first_comp_no];
	l_tccp1 = &l_tcp->tccps[p_second_comp_no];
	
	if (l_tccp0->numresolutions != l_tccp1->numresolutions) {
		return mi_FALSE;
	}
	if (l_tccp0->cblkw != l_tccp1->cblkw) {
		return mi_FALSE;
	}
	if (l_tccp0->cblkh != l_tccp1->cblkh) {
		return mi_FALSE;
	}
	if (l_tccp0->cblksty != l_tccp1->cblksty) {
		return mi_FALSE;
	}
	if (l_tccp0->qmfbid != l_tccp1->qmfbid) {
		return mi_FALSE;
	}
	if ((l_tccp0->csty & J2K_CCP_CSTY_PRT) != (l_tccp1->csty & J2K_CCP_CSTY_PRT)) {
		return mi_FALSE;
	}
	
	for (i = 0U; i < l_tccp0->numresolutions; ++i) {
		if (l_tccp0->prcw[i] != l_tccp1->prcw[i]) {
			return mi_FALSE;
		}
		if (l_tccp0->prch[i] != l_tccp1->prch[i]) {
			return mi_FALSE;
		}
	}
	return mi_TRUE;
}

static mi_BOOL mi_j2k_write_SPCod_SPCoc(     mi_j2k_t *p_j2k,
                                                                    mi_UINT32 p_tile_no,
                                                                    mi_UINT32 p_comp_no,
                                                                    mi_BYTE * p_data,
                                                                    mi_UINT32 * p_header_size,
                                                                    struct mi_event_mgr * p_manager )
{
        mi_UINT32 i;
        mi_cp_t *l_cp = 00;
        mi_tcp_t *l_tcp = 00;
        mi_tccp_t *l_tccp = 00;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_header_size != 00);
        assert(p_manager != 00);
        assert(p_data != 00);

        l_cp = &(p_j2k->m_cp);
        l_tcp = &l_cp->tcps[p_tile_no];
        l_tccp = &l_tcp->tccps[p_comp_no];

        /* preconditions again */
        assert(p_tile_no < (l_cp->tw * l_cp->th));
        assert(p_comp_no <(p_j2k->m_private_image->numcomps));

        if (*p_header_size < 5) {
                mi_event_msg(p_manager, EVT_ERROR, "Error writing SPCod SPCoc element\n");
                return mi_FALSE;
        }

        mi_write_bytes(p_data,l_tccp->numresolutions - 1, 1);  /* SPcoc (D) */
        ++p_data;

        mi_write_bytes(p_data,l_tccp->cblkw - 2, 1);                   /* SPcoc (E) */
        ++p_data;

        mi_write_bytes(p_data,l_tccp->cblkh - 2, 1);                   /* SPcoc (F) */
        ++p_data;

        mi_write_bytes(p_data,l_tccp->cblksty, 1);                             /* SPcoc (G) */
        ++p_data;

        mi_write_bytes(p_data,l_tccp->qmfbid, 1);                              /* SPcoc (H) */
        ++p_data;

        *p_header_size = *p_header_size - 5;

        if (l_tccp->csty & J2K_CCP_CSTY_PRT) {

                if (*p_header_size < l_tccp->numresolutions) {
                        mi_event_msg(p_manager, EVT_ERROR, "Error writing SPCod SPCoc element\n");
                        return mi_FALSE;
                }

                for (i = 0; i < l_tccp->numresolutions; ++i) {
                        mi_write_bytes(p_data,l_tccp->prcw[i] + (l_tccp->prch[i] << 4), 1);    /* SPcoc (I_i) */
                        ++p_data;
                }

                *p_header_size = *p_header_size - l_tccp->numresolutions;
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_read_SPCod_SPCoc(  mi_j2k_t *p_j2k,
                                                                mi_UINT32 compno,
                                                                mi_BYTE * p_header_data,
                                                                mi_UINT32 * p_header_size,
                                                                mi_event_mgr_t * p_manager)
{
        mi_UINT32 i, l_tmp;
        mi_cp_t *l_cp = NULL;
        mi_tcp_t *l_tcp = NULL;
        mi_tccp_t *l_tccp = NULL;
        mi_BYTE * l_current_ptr = NULL;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_header_data != 00);

        l_cp = &(p_j2k->m_cp);
        l_tcp = (p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_TPH) ?
                                &l_cp->tcps[p_j2k->m_current_tile_number] :
                                p_j2k->m_specific_param.m_decoder.m_default_tcp;

        /* precondition again */
        assert(compno < p_j2k->m_private_image->numcomps);

        l_tccp = &l_tcp->tccps[compno];
        l_current_ptr = p_header_data;

        /* make sure room is sufficient */
        if (*p_header_size < 5) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading SPCod SPCoc element\n");
                return mi_FALSE;
        }

        mi_read_bytes(l_current_ptr, &l_tccp->numresolutions ,1);              /* SPcox (D) */
        ++l_tccp->numresolutions;                                                                               /* tccp->numresolutions = read() + 1 */
        if (l_tccp->numresolutions > mi_J2K_MAXRLVLS) {
                mi_event_msg(p_manager, EVT_ERROR,
                              "Invalid value for numresolutions : %d, max value is set in openjpeg.h at %d\n",
                              l_tccp->numresolutions, mi_J2K_MAXRLVLS);
                return mi_FALSE;
        }
        ++l_current_ptr;

        /* If user wants to remove more resolutions than the codestream contains, return error */
        if (l_cp->m_specific_param.m_dec.m_reduce >= l_tccp->numresolutions) {
                mi_event_msg(p_manager, EVT_ERROR, "Error decoding component %d.\nThe number of resolutions to remove is higher than the number "
                                        "of resolutions of this component\nModify the cp_reduce parameter.\n\n", compno);
                p_j2k->m_specific_param.m_decoder.m_state |= 0x8000;/* FIXME J2K_DEC_STATE_ERR;*/
                return mi_FALSE;
        }

        mi_read_bytes(l_current_ptr,&l_tccp->cblkw ,1);                /* SPcoc (E) */
        ++l_current_ptr;
        l_tccp->cblkw += 2;

        mi_read_bytes(l_current_ptr,&l_tccp->cblkh ,1);                /* SPcoc (F) */
        ++l_current_ptr;
        l_tccp->cblkh += 2;

        if ((l_tccp->cblkw > 10) || (l_tccp->cblkh > 10) || ((l_tccp->cblkw + l_tccp->cblkh) > 12)) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading SPCod SPCoc element, Invalid cblkw/cblkh combination\n");
                return mi_FALSE;
        }
	

        mi_read_bytes(l_current_ptr,&l_tccp->cblksty ,1);              /* SPcoc (G) */
        ++l_current_ptr;
        if (l_tccp->cblksty & 0xC0U) { /* 2 msb are reserved, assume we can't read */
                mi_event_msg(p_manager, EVT_ERROR, "Error reading SPCod SPCoc element, Invalid code-block style found\n");
                return mi_FALSE;
        }

        mi_read_bytes(l_current_ptr,&l_tccp->qmfbid ,1);               /* SPcoc (H) */
        ++l_current_ptr;

        *p_header_size = *p_header_size - 5;

        /* use custom precinct size ? */
        if (l_tccp->csty & J2K_CCP_CSTY_PRT) {
                if (*p_header_size < l_tccp->numresolutions) {
                        mi_event_msg(p_manager, EVT_ERROR, "Error reading SPCod SPCoc element\n");
                        return mi_FALSE;
                }

                for     (i = 0; i < l_tccp->numresolutions; ++i) {
                        mi_read_bytes(l_current_ptr,&l_tmp ,1);                /* SPcoc (I_i) */
                        ++l_current_ptr;
                        /* Precinct exponent 0 is only allowed for lowest resolution level (Table A.21) */
                        if ((i != 0) && (((l_tmp & 0xf) == 0) || ((l_tmp >> 4) == 0))) {
                                mi_event_msg(p_manager, EVT_ERROR, "Invalid precinct size\n");
                                return mi_FALSE;
                        }
                        l_tccp->prcw[i] = l_tmp & 0xf;
                        l_tccp->prch[i] = l_tmp >> 4;
                }

                *p_header_size = *p_header_size - l_tccp->numresolutions;
        }
        else {
                /* set default size for the precinct width and height */
                for     (i = 0; i < l_tccp->numresolutions; ++i) {
                        l_tccp->prcw[i] = 15;
                        l_tccp->prch[i] = 15;
                }
        }

        return mi_TRUE;
}

static void mi_j2k_copy_tile_component_parameters( mi_j2k_t *p_j2k )
{
        /* loop */
        mi_UINT32 i;
        mi_cp_t *l_cp = NULL;
        mi_tcp_t *l_tcp = NULL;
        mi_tccp_t *l_ref_tccp = NULL, *l_copied_tccp = NULL;
        mi_UINT32 l_prc_size;

        /* preconditions */
        assert(p_j2k != 00);

        l_cp = &(p_j2k->m_cp);
        l_tcp = (p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_TPH) ? /* FIXME J2K_DEC_STATE_TPH*/
                                &l_cp->tcps[p_j2k->m_current_tile_number] :
                                p_j2k->m_specific_param.m_decoder.m_default_tcp;

        l_ref_tccp = &l_tcp->tccps[0];
        l_copied_tccp = l_ref_tccp + 1;
        l_prc_size = l_ref_tccp->numresolutions * (mi_UINT32)sizeof(mi_UINT32);

        for     (i=1; i<p_j2k->m_private_image->numcomps; ++i) {
                l_copied_tccp->numresolutions = l_ref_tccp->numresolutions;
                l_copied_tccp->cblkw = l_ref_tccp->cblkw;
                l_copied_tccp->cblkh = l_ref_tccp->cblkh;
                l_copied_tccp->cblksty = l_ref_tccp->cblksty;
                l_copied_tccp->qmfbid = l_ref_tccp->qmfbid;
                memcpy(l_copied_tccp->prcw,l_ref_tccp->prcw,l_prc_size);
                memcpy(l_copied_tccp->prch,l_ref_tccp->prch,l_prc_size);
                ++l_copied_tccp;
        }
}

static mi_UINT32 mi_j2k_get_SQcd_SQcc_size ( mi_j2k_t *p_j2k,
                                                                        mi_UINT32 p_tile_no,
                                                                        mi_UINT32 p_comp_no )
{
        mi_UINT32 l_num_bands;

        mi_cp_t *l_cp = 00;
        mi_tcp_t *l_tcp = 00;
        mi_tccp_t *l_tccp = 00;

        /* preconditions */
        assert(p_j2k != 00);

        l_cp = &(p_j2k->m_cp);
        l_tcp = &l_cp->tcps[p_tile_no];
        l_tccp = &l_tcp->tccps[p_comp_no];

        /* preconditions again */
        assert(p_tile_no < l_cp->tw * l_cp->th);
        assert(p_comp_no < p_j2k->m_private_image->numcomps);

        l_num_bands = (l_tccp->qntsty == J2K_CCP_QNTSTY_SIQNT) ? 1 : (l_tccp->numresolutions * 3 - 2);

        if (l_tccp->qntsty == J2K_CCP_QNTSTY_NOQNT)  {
                return 1 + l_num_bands;
        }
        else {
                return 1 + 2*l_num_bands;
        }
}

static mi_BOOL mi_j2k_compare_SQcd_SQcc(mi_j2k_t *p_j2k, mi_UINT32 p_tile_no, mi_UINT32 p_first_comp_no, mi_UINT32 p_second_comp_no)
{
	mi_cp_t *l_cp = NULL;
	mi_tcp_t *l_tcp = NULL;
	mi_tccp_t *l_tccp0 = NULL;
	mi_tccp_t *l_tccp1 = NULL;
	mi_UINT32 l_band_no, l_num_bands;
	
	/* preconditions */
	assert(p_j2k != 00);
	
	l_cp = &(p_j2k->m_cp);
	l_tcp = &l_cp->tcps[p_tile_no];
	l_tccp0 = &l_tcp->tccps[p_first_comp_no];
	l_tccp1 = &l_tcp->tccps[p_second_comp_no];
	
	if (l_tccp0->qntsty != l_tccp1->qntsty ) {
		return mi_FALSE;
	}
	if (l_tccp0->numgbits != l_tccp1->numgbits ) {
		return mi_FALSE;
	}
	if (l_tccp0->qntsty == J2K_CCP_QNTSTY_SIQNT) {
		l_num_bands = 1U;
	} else {
		l_num_bands = l_tccp0->numresolutions * 3U - 2U;
		if (l_num_bands != (l_tccp1->numresolutions * 3U - 2U)) {
			return mi_FALSE;
		}
	}
	
	for (l_band_no = 0; l_band_no < l_num_bands; ++l_band_no) {
		if (l_tccp0->stepsizes[l_band_no].expn != l_tccp1->stepsizes[l_band_no].expn ) {
			return mi_FALSE;
		}
	}
	if (l_tccp0->qntsty != J2K_CCP_QNTSTY_NOQNT)
	{
		for (l_band_no = 0; l_band_no < l_num_bands; ++l_band_no) {
			if (l_tccp0->stepsizes[l_band_no].mant != l_tccp1->stepsizes[l_band_no].mant ) {
				return mi_FALSE;
			}
		}
	}
	return mi_TRUE;
}


static mi_BOOL mi_j2k_write_SQcd_SQcc(       mi_j2k_t *p_j2k,
                                                                mi_UINT32 p_tile_no,
                                                                mi_UINT32 p_comp_no,
                                                                mi_BYTE * p_data,
                                                                mi_UINT32 * p_header_size,
                                                                struct mi_event_mgr * p_manager )
{
        mi_UINT32 l_header_size;
        mi_UINT32 l_band_no, l_num_bands;
        mi_UINT32 l_expn,l_mant;

        mi_cp_t *l_cp = 00;
        mi_tcp_t *l_tcp = 00;
        mi_tccp_t *l_tccp = 00;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_header_size != 00);
        assert(p_manager != 00);
        assert(p_data != 00);

        l_cp = &(p_j2k->m_cp);
        l_tcp = &l_cp->tcps[p_tile_no];
        l_tccp = &l_tcp->tccps[p_comp_no];

        /* preconditions again */
        assert(p_tile_no < l_cp->tw * l_cp->th);
        assert(p_comp_no <p_j2k->m_private_image->numcomps);

        l_num_bands = (l_tccp->qntsty == J2K_CCP_QNTSTY_SIQNT) ? 1 : (l_tccp->numresolutions * 3 - 2);

        if (l_tccp->qntsty == J2K_CCP_QNTSTY_NOQNT)  {
                l_header_size = 1 + l_num_bands;

                if (*p_header_size < l_header_size) {
                        mi_event_msg(p_manager, EVT_ERROR, "Error writing SQcd SQcc element\n");
                        return mi_FALSE;
                }

                mi_write_bytes(p_data,l_tccp->qntsty + (l_tccp->numgbits << 5), 1);    /* Sqcx */
                ++p_data;

                for (l_band_no = 0; l_band_no < l_num_bands; ++l_band_no) {
                        l_expn = (mi_UINT32)l_tccp->stepsizes[l_band_no].expn;
                        mi_write_bytes(p_data, l_expn << 3, 1);        /* SPqcx_i */
                        ++p_data;
                }
        }
        else {
                l_header_size = 1 + 2*l_num_bands;

                if (*p_header_size < l_header_size) {
                        mi_event_msg(p_manager, EVT_ERROR, "Error writing SQcd SQcc element\n");
                        return mi_FALSE;
                }

                mi_write_bytes(p_data,l_tccp->qntsty + (l_tccp->numgbits << 5), 1);    /* Sqcx */
                ++p_data;

                for (l_band_no = 0; l_band_no < l_num_bands; ++l_band_no) {
                        l_expn = (mi_UINT32)l_tccp->stepsizes[l_band_no].expn;
                        l_mant = (mi_UINT32)l_tccp->stepsizes[l_band_no].mant;

                        mi_write_bytes(p_data, (l_expn << 11) + l_mant, 2);    /* SPqcx_i */
                        p_data += 2;
                }
        }

        *p_header_size = *p_header_size - l_header_size;

        return mi_TRUE;
}

static mi_BOOL mi_j2k_read_SQcd_SQcc(mi_j2k_t *p_j2k,
                                                            mi_UINT32 p_comp_no,
                                                            mi_BYTE* p_header_data,
                                                            mi_UINT32 * p_header_size,
                                                            mi_event_mgr_t * p_manager
                                                            )
{
        /* loop*/
        mi_UINT32 l_band_no;
        mi_cp_t *l_cp = 00;
        mi_tcp_t *l_tcp = 00;
        mi_tccp_t *l_tccp = 00;
        mi_BYTE * l_current_ptr = 00;
        mi_UINT32 l_tmp, l_num_band;

        /* preconditions*/
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_header_data != 00);

        l_cp = &(p_j2k->m_cp);
        /* come from tile part header or main header ?*/
        l_tcp = (p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_TPH) ? /*FIXME J2K_DEC_STATE_TPH*/
                                &l_cp->tcps[p_j2k->m_current_tile_number] :
                                p_j2k->m_specific_param.m_decoder.m_default_tcp;

        /* precondition again*/
        assert(p_comp_no <  p_j2k->m_private_image->numcomps);

        l_tccp = &l_tcp->tccps[p_comp_no];
        l_current_ptr = p_header_data;

        if (*p_header_size < 1) {
                mi_event_msg(p_manager, EVT_ERROR, "Error reading SQcd or SQcc element\n");
                return mi_FALSE;
        }
        *p_header_size -= 1;

        mi_read_bytes(l_current_ptr, &l_tmp ,1);                       /* Sqcx */
        ++l_current_ptr;

        l_tccp->qntsty = l_tmp & 0x1f;
        l_tccp->numgbits = l_tmp >> 5;
        if (l_tccp->qntsty == J2K_CCP_QNTSTY_SIQNT) {
        l_num_band = 1;
        }
        else {
                l_num_band = (l_tccp->qntsty == J2K_CCP_QNTSTY_NOQNT) ?
                        (*p_header_size) :
                        (*p_header_size) / 2;

                if( l_num_band > mi_J2K_MAXBANDS ) {
                        mi_event_msg(p_manager, EVT_WARNING, "While reading CCP_QNTSTY element inside QCD or QCC marker segment, "
                                "number of subbands (%d) is greater to mi_J2K_MAXBANDS (%d). So we limit the number of elements stored to "
                                "mi_J2K_MAXBANDS (%d) and skip the rest. \n", l_num_band, mi_J2K_MAXBANDS, mi_J2K_MAXBANDS);
                        /*return mi_FALSE;*/
                }
        }

        if (l_tccp->qntsty == J2K_CCP_QNTSTY_NOQNT) {
                for     (l_band_no = 0; l_band_no < l_num_band; l_band_no++) {
                        mi_read_bytes(l_current_ptr, &l_tmp ,1);                       /* SPqcx_i */
                        ++l_current_ptr;
                        if (l_band_no < mi_J2K_MAXBANDS){
                                l_tccp->stepsizes[l_band_no].expn = (mi_INT32)(l_tmp >> 3);
                                l_tccp->stepsizes[l_band_no].mant = 0;
                        }
                }
                *p_header_size = *p_header_size - l_num_band;
        }
        else {
                for     (l_band_no = 0; l_band_no < l_num_band; l_band_no++) {
                        mi_read_bytes(l_current_ptr, &l_tmp ,2);                       /* SPqcx_i */
                        l_current_ptr+=2;
                        if (l_band_no < mi_J2K_MAXBANDS){
                                l_tccp->stepsizes[l_band_no].expn = (mi_INT32)(l_tmp >> 11);
                                l_tccp->stepsizes[l_band_no].mant = l_tmp & 0x7ff;
                        }
                }
                *p_header_size = *p_header_size - 2*l_num_band;
        }

        /* Add Antonin : if scalar_derived -> compute other stepsizes */
        if (l_tccp->qntsty == J2K_CCP_QNTSTY_SIQNT) {
                for (l_band_no = 1; l_band_no < mi_J2K_MAXBANDS; l_band_no++) {
                        l_tccp->stepsizes[l_band_no].expn =
                                ((mi_INT32)(l_tccp->stepsizes[0].expn) - (mi_INT32)((l_band_no - 1) / 3) > 0) ?
                                        (mi_INT32)(l_tccp->stepsizes[0].expn) - (mi_INT32)((l_band_no - 1) / 3) : 0;
                        l_tccp->stepsizes[l_band_no].mant = l_tccp->stepsizes[0].mant;
                }
        }

        return mi_TRUE;
}

static void mi_j2k_copy_tile_quantization_parameters( mi_j2k_t *p_j2k )
{
        mi_UINT32 i;
        mi_cp_t *l_cp = NULL;
        mi_tcp_t *l_tcp = NULL;
        mi_tccp_t *l_ref_tccp = NULL;
        mi_tccp_t *l_copied_tccp = NULL;
        mi_UINT32 l_size;

        /* preconditions */
        assert(p_j2k != 00);

        l_cp = &(p_j2k->m_cp);
        l_tcp = p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_TPH ?
                        &l_cp->tcps[p_j2k->m_current_tile_number] :
                        p_j2k->m_specific_param.m_decoder.m_default_tcp;

        l_ref_tccp = &l_tcp->tccps[0];
        l_copied_tccp = l_ref_tccp + 1;
        l_size = mi_J2K_MAXBANDS * sizeof(mi_stepsize_t);

        for     (i=1;i<p_j2k->m_private_image->numcomps;++i) {
                l_copied_tccp->qntsty = l_ref_tccp->qntsty;
                l_copied_tccp->numgbits = l_ref_tccp->numgbits;
                memcpy(l_copied_tccp->stepsizes,l_ref_tccp->stepsizes,l_size);
                ++l_copied_tccp;
        }
}

static void mi_j2k_dump_tile_info( mi_tcp_t * l_default_tile,mi_INT32 numcomps,FILE* out_stream)
{
        if (l_default_tile)
        {
                mi_INT32 compno;

                fprintf(out_stream, "\t default tile {\n");
                fprintf(out_stream, "\t\t csty=%#x\n", l_default_tile->csty);
                fprintf(out_stream, "\t\t prg=%#x\n", l_default_tile->prg);
                fprintf(out_stream, "\t\t numlayers=%d\n", l_default_tile->numlayers);
                fprintf(out_stream, "\t\t mct=%x\n", l_default_tile->mct);

                for (compno = 0; compno < numcomps; compno++) {
                        mi_tccp_t *l_tccp = &(l_default_tile->tccps[compno]);
                        mi_UINT32 resno;
      mi_INT32 bandno, numbands;

                        /* coding style*/
                        fprintf(out_stream, "\t\t comp %d {\n", compno);
                        fprintf(out_stream, "\t\t\t csty=%#x\n", l_tccp->csty);
                        fprintf(out_stream, "\t\t\t numresolutions=%d\n", l_tccp->numresolutions);
                        fprintf(out_stream, "\t\t\t cblkw=2^%d\n", l_tccp->cblkw);
                        fprintf(out_stream, "\t\t\t cblkh=2^%d\n", l_tccp->cblkh);
                        fprintf(out_stream, "\t\t\t cblksty=%#x\n", l_tccp->cblksty);
                        fprintf(out_stream, "\t\t\t qmfbid=%d\n", l_tccp->qmfbid);

                        fprintf(out_stream, "\t\t\t preccintsize (w,h)=");
                        for (resno = 0; resno < l_tccp->numresolutions; resno++) {
                                fprintf(out_stream, "(%d,%d) ", l_tccp->prcw[resno], l_tccp->prch[resno]);
                        }
                        fprintf(out_stream, "\n");

                        /* quantization style*/
                        fprintf(out_stream, "\t\t\t qntsty=%d\n", l_tccp->qntsty);
                        fprintf(out_stream, "\t\t\t numgbits=%d\n", l_tccp->numgbits);
                        fprintf(out_stream, "\t\t\t stepsizes (m,e)=");
                        numbands = (l_tccp->qntsty == J2K_CCP_QNTSTY_SIQNT) ? 1 : (mi_INT32)l_tccp->numresolutions * 3 - 2;
                        for (bandno = 0; bandno < numbands; bandno++) {
                                fprintf(out_stream, "(%d,%d) ", l_tccp->stepsizes[bandno].mant,
                                        l_tccp->stepsizes[bandno].expn);
                        }
                        fprintf(out_stream, "\n");

                        /* RGN value*/
                        fprintf(out_stream, "\t\t\t roishift=%d\n", l_tccp->roishift);

                        fprintf(out_stream, "\t\t }\n");
                } /*end of component of default tile*/
                fprintf(out_stream, "\t }\n"); /*end of default tile*/
            }
}

void j2k_dump (mi_j2k_t* p_j2k, mi_INT32 flag, FILE* out_stream)
{
        /* Check if the flag is compatible with j2k file*/
        if ( (flag & mi_JP2_INFO) || (flag & mi_JP2_IND)){
                fprintf(out_stream, "Wrong flag\n");
                return;
        }

        /* Dump the image_header */
        if (flag & mi_IMG_INFO){
                if (p_j2k->m_private_image)
                        j2k_dump_image_header(p_j2k->m_private_image, 0, out_stream);
        }

        /* Dump the codestream info from main header */
        if (flag & mi_J2K_MH_INFO){
                if (p_j2k->m_private_image)
                        mi_j2k_dump_MH_info(p_j2k, out_stream);
        }
        /* Dump all tile/codestream info */
        if (flag & mi_J2K_TCH_INFO){
          mi_UINT32 l_nb_tiles = p_j2k->m_cp.th * p_j2k->m_cp.tw;
          mi_UINT32 i;
          mi_tcp_t * l_tcp = p_j2k->m_cp.tcps;
          if (p_j2k->m_private_image) {
            for (i=0;i<l_nb_tiles;++i) {
              mi_j2k_dump_tile_info( l_tcp,(mi_INT32)p_j2k->m_private_image->numcomps, out_stream);
              ++l_tcp;
            }
          }
        }

        /* Dump the codestream info of the current tile */
        if (flag & mi_J2K_TH_INFO){

        }

        /* Dump the codestream index from main header */
        if (flag & mi_J2K_MH_IND){
                mi_j2k_dump_MH_index(p_j2k, out_stream);
        }

        /* Dump the codestream index of the current tile */
        if (flag & mi_J2K_TH_IND){

        }

}

static void mi_j2k_dump_MH_index(mi_j2k_t* p_j2k, FILE* out_stream)
{
        mi_codestream_index_t* cstr_index = p_j2k->cstr_index;
        mi_UINT32 it_marker, it_tile, it_tile_part;

        fprintf(out_stream, "Codestream index from main header: {\n");

        fprintf(out_stream, "\t Main header start position=%" PRIi64 "\n"
                                    "\t Main header end position=%" PRIi64 "\n",
                        cstr_index->main_head_start, cstr_index->main_head_end);

        fprintf(out_stream, "\t Marker list: {\n");

        if (cstr_index->marker){
                for (it_marker=0; it_marker < cstr_index->marknum ; it_marker++){
                        fprintf(out_stream, "\t\t type=%#x, pos=%" PRIi64 ", len=%d\n",
                                        cstr_index->marker[it_marker].type,
                                        cstr_index->marker[it_marker].pos,
                                        cstr_index->marker[it_marker].len );
                }
        }

        fprintf(out_stream, "\t }\n");

        if (cstr_index->tile_index){

        /* Simple test to avoid to write empty information*/
        mi_UINT32 l_acc_nb_of_tile_part = 0;
        for (it_tile=0; it_tile < cstr_index->nb_of_tiles ; it_tile++){
                        l_acc_nb_of_tile_part += cstr_index->tile_index[it_tile].nb_tps;
        }

        if (l_acc_nb_of_tile_part)
        {
            fprintf(out_stream, "\t Tile index: {\n");

                    for (it_tile=0; it_tile < cstr_index->nb_of_tiles ; it_tile++){
                            mi_UINT32 nb_of_tile_part = cstr_index->tile_index[it_tile].nb_tps;

                            fprintf(out_stream, "\t\t nb of tile-part in tile [%d]=%d\n", it_tile, nb_of_tile_part);

                            if (cstr_index->tile_index[it_tile].tp_index){
                                    for (it_tile_part =0; it_tile_part < nb_of_tile_part; it_tile_part++){
                                            fprintf(out_stream, "\t\t\t tile-part[%d]: star_pos=%" PRIi64 ", end_header=%" PRIi64 ", end_pos=%" PRIi64 ".\n",
                                                            it_tile_part,
                                                            cstr_index->tile_index[it_tile].tp_index[it_tile_part].start_pos,
                                                            cstr_index->tile_index[it_tile].tp_index[it_tile_part].end_header,
                                                            cstr_index->tile_index[it_tile].tp_index[it_tile_part].end_pos);
                                    }
                            }

                            if (cstr_index->tile_index[it_tile].marker){
                                    for (it_marker=0; it_marker < cstr_index->tile_index[it_tile].marknum ; it_marker++){
                                            fprintf(out_stream, "\t\t type=%#x, pos=%" PRIi64 ", len=%d\n",
                                                            cstr_index->tile_index[it_tile].marker[it_marker].type,
                                                            cstr_index->tile_index[it_tile].marker[it_marker].pos,
                                                            cstr_index->tile_index[it_tile].marker[it_marker].len );
                                    }
                            }
                    }
                    fprintf(out_stream,"\t }\n");
        }
        }

        fprintf(out_stream,"}\n");

}


static void mi_j2k_dump_MH_info(mi_j2k_t* p_j2k, FILE* out_stream)
{

        fprintf(out_stream, "Codestream info from main header: {\n");

        fprintf(out_stream, "\t tx0=%d, ty0=%d\n", p_j2k->m_cp.tx0, p_j2k->m_cp.ty0);
        fprintf(out_stream, "\t tdx=%d, tdy=%d\n", p_j2k->m_cp.tdx, p_j2k->m_cp.tdy);
        fprintf(out_stream, "\t tw=%d, th=%d\n", p_j2k->m_cp.tw, p_j2k->m_cp.th);
        mi_j2k_dump_tile_info(p_j2k->m_specific_param.m_decoder.m_default_tcp,(mi_INT32)p_j2k->m_private_image->numcomps, out_stream);
        fprintf(out_stream, "}\n");
}

void j2k_dump_image_header(mi_image_t* img_header, mi_BOOL dev_dump_flag, FILE* out_stream)
{
        char tab[2];

        if (dev_dump_flag){
                fprintf(stdout, "[DEV] Dump an image_header struct {\n");
                tab[0] = '\0';
        }
        else {
                fprintf(out_stream, "Image info {\n");
                tab[0] = '\t';tab[1] = '\0';
        }

        fprintf(out_stream, "%s x0=%d, y0=%d\n", tab, img_header->x0, img_header->y0);
        fprintf(out_stream,     "%s x1=%d, y1=%d\n", tab, img_header->x1, img_header->y1);
        fprintf(out_stream, "%s numcomps=%d\n", tab, img_header->numcomps);

        if (img_header->comps){
                mi_UINT32 compno;
                for (compno = 0; compno < img_header->numcomps; compno++) {
                        fprintf(out_stream, "%s\t component %d {\n", tab, compno);
                        j2k_dump_image_comp_header(&(img_header->comps[compno]), dev_dump_flag, out_stream);
                        fprintf(out_stream,"%s}\n",tab);
                }
        }

        fprintf(out_stream, "}\n");
}

void j2k_dump_image_comp_header(mi_image_comp_t* comp_header, mi_BOOL dev_dump_flag, FILE* out_stream)
{
        char tab[3];

        if (dev_dump_flag){
                fprintf(stdout, "[DEV] Dump an image_comp_header struct {\n");
                tab[0] = '\0';
        }       else {
                tab[0] = '\t';tab[1] = '\t';tab[2] = '\0';
        }

        fprintf(out_stream, "%s dx=%d, dy=%d\n", tab, comp_header->dx, comp_header->dy);
        fprintf(out_stream, "%s prec=%d\n", tab, comp_header->prec);
        fprintf(out_stream, "%s sgnd=%d\n", tab, comp_header->sgnd);

        if (dev_dump_flag)
                fprintf(out_stream, "}\n");
}

mi_codestream_info_v2_t* j2k_get_cstr_info(mi_j2k_t* p_j2k)
{
        mi_UINT32 compno;
        mi_UINT32 numcomps = p_j2k->m_private_image->numcomps;
        mi_tcp_t *l_default_tile;
        mi_codestream_info_v2_t* cstr_info = (mi_codestream_info_v2_t*) mi_calloc(1,sizeof(mi_codestream_info_v2_t));
		if (!cstr_info)
			return NULL;

        cstr_info->nbcomps = p_j2k->m_private_image->numcomps;

        cstr_info->tx0 = p_j2k->m_cp.tx0;
        cstr_info->ty0 = p_j2k->m_cp.ty0;
        cstr_info->tdx = p_j2k->m_cp.tdx;
        cstr_info->tdy = p_j2k->m_cp.tdy;
        cstr_info->tw = p_j2k->m_cp.tw;
        cstr_info->th = p_j2k->m_cp.th;

        cstr_info->tile_info = NULL; /* Not fill from the main header*/

        l_default_tile = p_j2k->m_specific_param.m_decoder.m_default_tcp;

        cstr_info->m_default_tile_info.csty = l_default_tile->csty;
        cstr_info->m_default_tile_info.prg = l_default_tile->prg;
        cstr_info->m_default_tile_info.numlayers = l_default_tile->numlayers;
        cstr_info->m_default_tile_info.mct = l_default_tile->mct;

        cstr_info->m_default_tile_info.tccp_info = (mi_tccp_info_t*) mi_calloc(cstr_info->nbcomps, sizeof(mi_tccp_info_t));
		if (!cstr_info->m_default_tile_info.tccp_info)
		{
			mi_destroy_cstr_info(&cstr_info);
			return NULL;
		}

        for (compno = 0; compno < numcomps; compno++) {
                mi_tccp_t *l_tccp = &(l_default_tile->tccps[compno]);
                mi_tccp_info_t *l_tccp_info = &(cstr_info->m_default_tile_info.tccp_info[compno]);
                mi_INT32 bandno, numbands;

                /* coding style*/
                l_tccp_info->csty = l_tccp->csty;
                l_tccp_info->numresolutions = l_tccp->numresolutions;
                l_tccp_info->cblkw = l_tccp->cblkw;
                l_tccp_info->cblkh = l_tccp->cblkh;
                l_tccp_info->cblksty = l_tccp->cblksty;
                l_tccp_info->qmfbid = l_tccp->qmfbid;
                if (l_tccp->numresolutions < mi_J2K_MAXRLVLS)
                {
                        memcpy(l_tccp_info->prch, l_tccp->prch, l_tccp->numresolutions);
                        memcpy(l_tccp_info->prcw, l_tccp->prcw, l_tccp->numresolutions);
                }

                /* quantization style*/
                l_tccp_info->qntsty = l_tccp->qntsty;
                l_tccp_info->numgbits = l_tccp->numgbits;

                numbands = (l_tccp->qntsty == J2K_CCP_QNTSTY_SIQNT) ? 1 : (mi_INT32)l_tccp->numresolutions * 3 - 2;
                if (numbands < mi_J2K_MAXBANDS) {
                        for (bandno = 0; bandno < numbands; bandno++) {
                                l_tccp_info->stepsizes_mant[bandno] = (mi_UINT32)l_tccp->stepsizes[bandno].mant;
                                l_tccp_info->stepsizes_expn[bandno] = (mi_UINT32)l_tccp->stepsizes[bandno].expn;
                        }
                }

                /* RGN value*/
                l_tccp_info->roishift = l_tccp->roishift;
        }

        return cstr_info;
}

mi_codestream_index_t* j2k_get_cstr_index(mi_j2k_t* p_j2k)
{
        mi_codestream_index_t* l_cstr_index = (mi_codestream_index_t*)
                        mi_calloc(1,sizeof(mi_codestream_index_t));
        if (!l_cstr_index)
                return NULL;

        l_cstr_index->main_head_start = p_j2k->cstr_index->main_head_start;
        l_cstr_index->main_head_end = p_j2k->cstr_index->main_head_end;
        l_cstr_index->codestream_size = p_j2k->cstr_index->codestream_size;

        l_cstr_index->marknum = p_j2k->cstr_index->marknum;
        l_cstr_index->marker = (mi_marker_info_t*)mi_malloc(l_cstr_index->marknum*sizeof(mi_marker_info_t));
        if (!l_cstr_index->marker){
                mi_free( l_cstr_index);
                return NULL;
        }

        if (p_j2k->cstr_index->marker)
                memcpy(l_cstr_index->marker, p_j2k->cstr_index->marker, l_cstr_index->marknum * sizeof(mi_marker_info_t) );
        else{
                mi_free(l_cstr_index->marker);
                l_cstr_index->marker = NULL;
        }

        l_cstr_index->nb_of_tiles = p_j2k->cstr_index->nb_of_tiles;
        l_cstr_index->tile_index = (mi_tile_index_t*)mi_calloc(l_cstr_index->nb_of_tiles, sizeof(mi_tile_index_t) );
        if (!l_cstr_index->tile_index){
                mi_free( l_cstr_index->marker);
                mi_free( l_cstr_index);
                return NULL;
        }

        if (!p_j2k->cstr_index->tile_index){
                mi_free(l_cstr_index->tile_index);
                l_cstr_index->tile_index = NULL;
        }
        else {
                mi_UINT32 it_tile = 0;
                for (it_tile = 0; it_tile < l_cstr_index->nb_of_tiles; it_tile++ ){

                        /* Tile Marker*/
                        l_cstr_index->tile_index[it_tile].marknum = p_j2k->cstr_index->tile_index[it_tile].marknum;

                        l_cstr_index->tile_index[it_tile].marker =
                                (mi_marker_info_t*)mi_malloc(l_cstr_index->tile_index[it_tile].marknum*sizeof(mi_marker_info_t));

                        if (!l_cstr_index->tile_index[it_tile].marker) {
                                mi_UINT32 it_tile_free;

                                for (it_tile_free=0; it_tile_free < it_tile; it_tile_free++){
                                        mi_free(l_cstr_index->tile_index[it_tile_free].marker);
                                }

                                mi_free( l_cstr_index->tile_index);
                                mi_free( l_cstr_index->marker);
                                mi_free( l_cstr_index);
                                return NULL;
                        }

                        if (p_j2k->cstr_index->tile_index[it_tile].marker)
                                memcpy( l_cstr_index->tile_index[it_tile].marker,
                                                p_j2k->cstr_index->tile_index[it_tile].marker,
                                                l_cstr_index->tile_index[it_tile].marknum * sizeof(mi_marker_info_t) );
                        else{
                                mi_free(l_cstr_index->tile_index[it_tile].marker);
                                l_cstr_index->tile_index[it_tile].marker = NULL;
                        }

                        /* Tile part index*/
                        l_cstr_index->tile_index[it_tile].nb_tps = p_j2k->cstr_index->tile_index[it_tile].nb_tps;

                        l_cstr_index->tile_index[it_tile].tp_index =
                                (mi_tp_index_t*)mi_malloc(l_cstr_index->tile_index[it_tile].nb_tps*sizeof(mi_tp_index_t));

                        if(!l_cstr_index->tile_index[it_tile].tp_index){
                                mi_UINT32 it_tile_free;

                                for (it_tile_free=0; it_tile_free < it_tile; it_tile_free++){
                                        mi_free(l_cstr_index->tile_index[it_tile_free].marker);
                                        mi_free(l_cstr_index->tile_index[it_tile_free].tp_index);
                                }

                                mi_free( l_cstr_index->tile_index);
                                mi_free( l_cstr_index->marker);
                                mi_free( l_cstr_index);
                                return NULL;
                        }

                        if (p_j2k->cstr_index->tile_index[it_tile].tp_index){
                                memcpy( l_cstr_index->tile_index[it_tile].tp_index,
                                                p_j2k->cstr_index->tile_index[it_tile].tp_index,
                                                l_cstr_index->tile_index[it_tile].nb_tps * sizeof(mi_tp_index_t) );
                        }
                        else{
                                mi_free(l_cstr_index->tile_index[it_tile].tp_index);
                                l_cstr_index->tile_index[it_tile].tp_index = NULL;
                        }

                        /* Packet index (NOT USED)*/
                        l_cstr_index->tile_index[it_tile].nb_packet = 0;
                        l_cstr_index->tile_index[it_tile].packet_index = NULL;

                }
        }

        return l_cstr_index;
}

static mi_BOOL mi_j2k_allocate_tile_element_cstr_index(mi_j2k_t *p_j2k)
{
        mi_UINT32 it_tile=0;

        p_j2k->cstr_index->nb_of_tiles = p_j2k->m_cp.tw * p_j2k->m_cp.th;
        p_j2k->cstr_index->tile_index = (mi_tile_index_t*)mi_calloc(p_j2k->cstr_index->nb_of_tiles, sizeof(mi_tile_index_t));
        if (!p_j2k->cstr_index->tile_index)
                return mi_FALSE;

        for (it_tile=0; it_tile < p_j2k->cstr_index->nb_of_tiles; it_tile++){
                p_j2k->cstr_index->tile_index[it_tile].maxmarknum = 100;
                p_j2k->cstr_index->tile_index[it_tile].marknum = 0;
                p_j2k->cstr_index->tile_index[it_tile].marker = (mi_marker_info_t*)
                                mi_calloc(p_j2k->cstr_index->tile_index[it_tile].maxmarknum, sizeof(mi_marker_info_t));
                if (!p_j2k->cstr_index->tile_index[it_tile].marker)
                        return mi_FALSE;
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_decode_tiles ( mi_j2k_t *p_j2k,
                                                            mi_stream_private_t *p_stream,
                                                            mi_event_mgr_t * p_manager)
{
        mi_BOOL l_go_on = mi_TRUE;
        mi_UINT32 l_current_tile_no;
        mi_UINT32 l_data_size,l_max_data_size;
        mi_INT32 l_tile_x0,l_tile_y0,l_tile_x1,l_tile_y1;
        mi_UINT32 l_nb_comps;
        mi_BYTE * l_current_data;
        mi_UINT32 nr_tiles = 0;

        l_current_data = (mi_BYTE*)mi_malloc(1000);
        if (! l_current_data) {
                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to decode tiles\n");
                return mi_FALSE;
        }
        l_max_data_size = 1000;

		for (;;) {
                if (! mi_j2k_read_tile_header( p_j2k,
                                        &l_current_tile_no,
                                        &l_data_size,
                                        &l_tile_x0, &l_tile_y0,
                                        &l_tile_x1, &l_tile_y1,
                                        &l_nb_comps,
                                        &l_go_on,
                                        p_stream,
                                        p_manager)) {
                        mi_free(l_current_data);
                        return mi_FALSE;
                }

                if (! l_go_on) {
                        break;
                }

                if (l_data_size > l_max_data_size) {
                        mi_BYTE *l_new_current_data = (mi_BYTE *) mi_realloc(l_current_data, l_data_size);
                        if (! l_new_current_data) {
                                mi_free(l_current_data);
                                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to decode tile %d/%d\n", l_current_tile_no +1, p_j2k->m_cp.th * p_j2k->m_cp.tw);
                                return mi_FALSE;
                        }
                        l_current_data = l_new_current_data;
                        l_max_data_size = l_data_size;
                }

                if (! mi_j2k_decode_tile(p_j2k,l_current_tile_no,l_current_data,l_data_size,p_stream,p_manager)) {
                        mi_free(l_current_data);
                        mi_event_msg(p_manager, EVT_ERROR, "Failed to decode tile %d/%d\n", l_current_tile_no +1, p_j2k->m_cp.th * p_j2k->m_cp.tw);
                        return mi_FALSE;
                }
                mi_event_msg(p_manager, EVT_INFO, "Tile %d/%d has been decoded.\n", l_current_tile_no +1, p_j2k->m_cp.th * p_j2k->m_cp.tw);

                if (! mi_j2k_update_image_data(p_j2k->m_tcd,l_current_data, p_j2k->m_output_image)) {
                        mi_free(l_current_data);
                        return mi_FALSE;
                }
                mi_event_msg(p_manager, EVT_INFO, "Image data has been updated with tile %d.\n\n", l_current_tile_no + 1);
                
                if(mi_stream_get_number_byte_left(p_stream) == 0  
                    && p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_NEOC)
                    break;
                if(++nr_tiles ==  p_j2k->m_cp.th * p_j2k->m_cp.tw) 
                    break;
        }

        mi_free(l_current_data);

        return mi_TRUE;
}

/**
 * Sets up the procedures to do on decoding data. Developpers wanting to extend the library can add their own reading procedures.
 */
static mi_BOOL mi_j2k_setup_decoding (mi_j2k_t *p_j2k, mi_event_mgr_t * p_manager)
{
        /* preconditions*/
        assert(p_j2k != 00);
        assert(p_manager != 00);

        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_decode_tiles, p_manager)) {
                return mi_FALSE;
        }
        /* DEVELOPER CORNER, add your custom procedures */

        return mi_TRUE;
}

/*
 * Read and decode one tile.
 */
static mi_BOOL mi_j2k_decode_one_tile (       mi_j2k_t *p_j2k,
                                                                            mi_stream_private_t *p_stream,
                                                                            mi_event_mgr_t * p_manager)
{
        mi_BOOL l_go_on = mi_TRUE;
        mi_UINT32 l_current_tile_no;
        mi_UINT32 l_tile_no_to_dec;
        mi_UINT32 l_data_size,l_max_data_size;
        mi_INT32 l_tile_x0,l_tile_y0,l_tile_x1,l_tile_y1;
        mi_UINT32 l_nb_comps;
        mi_BYTE * l_current_data;

        l_current_data = (mi_BYTE*)mi_malloc(1000);
        if (! l_current_data) {
                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to decode one tile\n");
                return mi_FALSE;
        }
        l_max_data_size = 1000;

        /*Allocate and initialize some elements of codestrem index if not already done*/
        if( !p_j2k->cstr_index->tile_index)
        {
                if (!mi_j2k_allocate_tile_element_cstr_index(p_j2k)){
                        mi_free(l_current_data);
                        return mi_FALSE;
                }
        }
        /* Move into the codestream to the first SOT used to decode the desired tile */
        l_tile_no_to_dec = (mi_UINT32)p_j2k->m_specific_param.m_decoder.m_tile_ind_to_dec;
        if (p_j2k->cstr_index->tile_index)
                if(p_j2k->cstr_index->tile_index->tp_index)
                {
                        if ( ! p_j2k->cstr_index->tile_index[l_tile_no_to_dec].nb_tps) {
                                /* the index for this tile has not been built,
                                 *  so move to the last SOT read */
                                if ( !(mi_stream_read_seek(p_stream, p_j2k->m_specific_param.m_decoder.m_last_sot_read_pos+2, p_manager)) ){
                                        mi_event_msg(p_manager, EVT_ERROR, "Problem with seek function\n");
                                        mi_free(l_current_data);
                                        return mi_FALSE;
                                }
                        }
                        else{
                                if ( !(mi_stream_read_seek(p_stream, p_j2k->cstr_index->tile_index[l_tile_no_to_dec].tp_index[0].start_pos+2, p_manager)) ) {
                                        mi_event_msg(p_manager, EVT_ERROR, "Problem with seek function\n");
                                        mi_free(l_current_data);
                                        return mi_FALSE;
                                }
                        }
                        /* Special case if we have previously read the EOC marker (if the previous tile getted is the last ) */
                        if(p_j2k->m_specific_param.m_decoder.m_state == J2K_STATE_EOC)
                                p_j2k->m_specific_param.m_decoder.m_state = J2K_STATE_TPHSOT;
                }

		for (;;) {
                if (! mi_j2k_read_tile_header( p_j2k,
                                        &l_current_tile_no,
                                        &l_data_size,
                                        &l_tile_x0, &l_tile_y0,
                                        &l_tile_x1, &l_tile_y1,
                                        &l_nb_comps,
                                        &l_go_on,
                                        p_stream,
                                        p_manager)) {
                        mi_free(l_current_data);
                        return mi_FALSE;
                }

                if (! l_go_on) {
                        break;
                }

                if (l_data_size > l_max_data_size) {
                        mi_BYTE *l_new_current_data = (mi_BYTE *) mi_realloc(l_current_data, l_data_size);
                        if (! l_new_current_data) {
                                mi_free(l_current_data);
                                l_current_data = NULL;
                                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to decode tile %d/%d\n", l_current_tile_no+1, p_j2k->m_cp.th * p_j2k->m_cp.tw);
                                return mi_FALSE;
                        }
                        l_current_data = l_new_current_data;
                        l_max_data_size = l_data_size;
                }

                if (! mi_j2k_decode_tile(p_j2k,l_current_tile_no,l_current_data,l_data_size,p_stream,p_manager)) {
                        mi_free(l_current_data);
                        return mi_FALSE;
                }
                mi_event_msg(p_manager, EVT_INFO, "Tile %d/%d has been decoded.\n", l_current_tile_no+1, p_j2k->m_cp.th * p_j2k->m_cp.tw);

                if (! mi_j2k_update_image_data(p_j2k->m_tcd,l_current_data, p_j2k->m_output_image)) {
                        mi_free(l_current_data);
                        return mi_FALSE;
                }
                mi_event_msg(p_manager, EVT_INFO, "Image data has been updated with tile %d.\n\n", l_current_tile_no+1);

                if(l_current_tile_no == l_tile_no_to_dec)
                {
                        /* move into the codestream to the first SOT (FIXME or not move?)*/
                        if (!(mi_stream_read_seek(p_stream, p_j2k->cstr_index->main_head_end + 2, p_manager) ) ) {
                                mi_event_msg(p_manager, EVT_ERROR, "Problem with seek function\n");
                                mi_free(l_current_data);
                                return mi_FALSE;
                        }
                        break;
                }
                else {
                        mi_event_msg(p_manager, EVT_WARNING, "Tile read, decoded and updated is not the desired one (%d vs %d).\n", l_current_tile_no+1, l_tile_no_to_dec+1);
                }

        }

        mi_free(l_current_data);

        return mi_TRUE;
}

/**
 * Sets up the procedures to do on decoding one tile. Developpers wanting to extend the library can add their own reading procedures.
 */
static mi_BOOL mi_j2k_setup_decoding_tile (mi_j2k_t *p_j2k, mi_event_mgr_t * p_manager)
{
        /* preconditions*/
        assert(p_j2k != 00);
        assert(p_manager != 00);

        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_decode_one_tile, p_manager)) {
                return mi_FALSE;
        }
        /* DEVELOPER CORNER, add your custom procedures */

        return mi_TRUE;
}

mi_BOOL mi_j2k_decode(mi_j2k_t * p_j2k,
                                                mi_stream_private_t * p_stream,
                                                mi_image_t * p_image,
                                                mi_event_mgr_t * p_manager)
{
        mi_UINT32 compno;

        if (!p_image)
                return mi_FALSE;
	
        p_j2k->m_output_image = mi_image_create0();
        if (! (p_j2k->m_output_image)) {
                return mi_FALSE;
        }
        mi_copy_image_header(p_image, p_j2k->m_output_image);

        /* customization of the decoding */
        mi_j2k_setup_decoding(p_j2k, p_manager);

        /* Decode the codestream */
        if (! mi_j2k_exec (p_j2k,p_j2k->m_procedure_list,p_stream,p_manager)) {
                mi_image_destroy(p_j2k->m_private_image);
                p_j2k->m_private_image = NULL;
                return mi_FALSE;
        }

        /* Move data and copy one information from codec to output image*/
        for (compno = 0; compno < p_image->numcomps; compno++) {
                p_image->comps[compno].resno_decoded = p_j2k->m_output_image->comps[compno].resno_decoded;
                p_image->comps[compno].data = p_j2k->m_output_image->comps[compno].data;
                p_j2k->m_output_image->comps[compno].data = NULL;
        }

        return mi_TRUE;
}

mi_BOOL mi_j2k_get_tile(      mi_j2k_t *p_j2k,
                                                    mi_stream_private_t *p_stream,
                                                    mi_image_t* p_image,
                                                    mi_event_mgr_t * p_manager,
                                                    mi_UINT32 tile_index )
{
        mi_UINT32 compno;
        mi_UINT32 l_tile_x, l_tile_y;
        mi_image_comp_t* l_img_comp;

        if (!p_image) {
                mi_event_msg(p_manager, EVT_ERROR, "We need an image previously created.\n");
                return mi_FALSE;
        }

        if ( /*(tile_index < 0) &&*/ (tile_index >= p_j2k->m_cp.tw * p_j2k->m_cp.th) ){
                mi_event_msg(p_manager, EVT_ERROR, "Tile index provided by the user is incorrect %d (max = %d) \n", tile_index, (p_j2k->m_cp.tw * p_j2k->m_cp.th) - 1);
                return mi_FALSE;
        }

        /* Compute the dimension of the desired tile*/
        l_tile_x = tile_index % p_j2k->m_cp.tw;
        l_tile_y = tile_index / p_j2k->m_cp.tw;

        p_image->x0 = l_tile_x * p_j2k->m_cp.tdx + p_j2k->m_cp.tx0;
        if (p_image->x0 < p_j2k->m_private_image->x0)
                p_image->x0 = p_j2k->m_private_image->x0;
        p_image->x1 = (l_tile_x + 1) * p_j2k->m_cp.tdx + p_j2k->m_cp.tx0;
        if (p_image->x1 > p_j2k->m_private_image->x1)
                p_image->x1 = p_j2k->m_private_image->x1;

        p_image->y0 = l_tile_y * p_j2k->m_cp.tdy + p_j2k->m_cp.ty0;
        if (p_image->y0 < p_j2k->m_private_image->y0)
                p_image->y0 = p_j2k->m_private_image->y0;
        p_image->y1 = (l_tile_y + 1) * p_j2k->m_cp.tdy + p_j2k->m_cp.ty0;
        if (p_image->y1 > p_j2k->m_private_image->y1)
                p_image->y1 = p_j2k->m_private_image->y1;

        l_img_comp = p_image->comps;
        for (compno=0; compno < p_image->numcomps; ++compno)
        {
                mi_INT32 l_comp_x1, l_comp_y1;

                l_img_comp->factor = p_j2k->m_private_image->comps[compno].factor;

                l_img_comp->x0 = (mi_UINT32)mi_int_ceildiv((mi_INT32)p_image->x0, (mi_INT32)l_img_comp->dx);
                l_img_comp->y0 = (mi_UINT32)mi_int_ceildiv((mi_INT32)p_image->y0, (mi_INT32)l_img_comp->dy);
                l_comp_x1 = mi_int_ceildiv((mi_INT32)p_image->x1, (mi_INT32)l_img_comp->dx);
                l_comp_y1 = mi_int_ceildiv((mi_INT32)p_image->y1, (mi_INT32)l_img_comp->dy);

                l_img_comp->w = (mi_UINT32)(mi_int_ceildivpow2(l_comp_x1, (mi_INT32)l_img_comp->factor) - mi_int_ceildivpow2((mi_INT32)l_img_comp->x0, (mi_INT32)l_img_comp->factor));
                l_img_comp->h = (mi_UINT32)(mi_int_ceildivpow2(l_comp_y1, (mi_INT32)l_img_comp->factor) - mi_int_ceildivpow2((mi_INT32)l_img_comp->y0, (mi_INT32)l_img_comp->factor));

                l_img_comp++;
        }

        /* Destroy the previous output image*/
        if (p_j2k->m_output_image)
                mi_image_destroy(p_j2k->m_output_image);

        /* Create the ouput image from the information previously computed*/
        p_j2k->m_output_image = mi_image_create0();
        if (! (p_j2k->m_output_image)) {
                return mi_FALSE;
        }
        mi_copy_image_header(p_image, p_j2k->m_output_image);

        p_j2k->m_specific_param.m_decoder.m_tile_ind_to_dec = (mi_INT32)tile_index;

        /* customization of the decoding */
        mi_j2k_setup_decoding_tile(p_j2k, p_manager);

        /* Decode the codestream */
        if (! mi_j2k_exec (p_j2k,p_j2k->m_procedure_list,p_stream,p_manager)) {
                mi_image_destroy(p_j2k->m_private_image);
                p_j2k->m_private_image = NULL;
                return mi_FALSE;
        }

        /* Move data and copy one information from codec to output image*/
        for (compno = 0; compno < p_image->numcomps; compno++) {
                p_image->comps[compno].resno_decoded = p_j2k->m_output_image->comps[compno].resno_decoded;

                if (p_image->comps[compno].data)
                        mi_free(p_image->comps[compno].data);

                p_image->comps[compno].data = p_j2k->m_output_image->comps[compno].data;

                p_j2k->m_output_image->comps[compno].data = NULL;
        }

        return mi_TRUE;
}

mi_BOOL mi_j2k_set_decoded_resolution_factor(mi_j2k_t *p_j2k,
                                               mi_UINT32 res_factor,
                                               mi_event_mgr_t * p_manager)
{
        mi_UINT32 it_comp;

        p_j2k->m_cp.m_specific_param.m_dec.m_reduce = res_factor;

        if (p_j2k->m_private_image) {
                if (p_j2k->m_private_image->comps) {
                        if (p_j2k->m_specific_param.m_decoder.m_default_tcp) {
                                if (p_j2k->m_specific_param.m_decoder.m_default_tcp->tccps) {
                                        for (it_comp = 0 ; it_comp < p_j2k->m_private_image->numcomps; it_comp++) {
                                                mi_UINT32 max_res = p_j2k->m_specific_param.m_decoder.m_default_tcp->tccps[it_comp].numresolutions;
                                                if ( res_factor >= max_res){
                                                        mi_event_msg(p_manager, EVT_ERROR, "Resolution factor is greater than the maximum resolution in the component.\n");
                                                        return mi_FALSE;
                                                }
                                                p_j2k->m_private_image->comps[it_comp].factor = res_factor;
                                        }
                                        return mi_TRUE;
                                }
                        }
                }
        }

        return mi_FALSE;
}

mi_BOOL mi_j2k_encode(mi_j2k_t * p_j2k,
                        mi_stream_private_t *p_stream,
                        mi_event_mgr_t * p_manager )
{
        mi_UINT32 i, j;
        mi_UINT32 l_nb_tiles;
        mi_UINT32 l_max_tile_size = 0, l_current_tile_size;
        mi_BYTE * l_current_data = 00;
        mi_BOOL l_reuse_data = mi_FALSE;
        mi_tcd_t* p_tcd = 00;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_stream != 00);
        assert(p_manager != 00);
	
        p_tcd = p_j2k->m_tcd;

        l_nb_tiles = p_j2k->m_cp.th * p_j2k->m_cp.tw;
        if (l_nb_tiles == 1) {
                l_reuse_data = mi_TRUE;
        }
        for (i=0;i<l_nb_tiles;++i) {
                if (! mi_j2k_pre_write_tile(p_j2k,i,p_stream,p_manager)) {
                        if (l_current_data) {
                                mi_free(l_current_data);
                        }
                        return mi_FALSE;
                }

                /* if we only have one tile, then simply set tile component data equal to image component data */
                /* otherwise, allocate the data */
                for (j=0;j<p_j2k->m_tcd->image->numcomps;++j) {
                        mi_tcd_tilecomp_t* l_tilec = p_tcd->tcd_image->tiles->comps + j;
                        if (l_reuse_data) {
												        mi_image_comp_t * l_img_comp = p_tcd->image->comps + j;
												        l_tilec->data  =  l_img_comp->data;
												        l_tilec->ownsData = mi_FALSE;
                        } else {
												        if(! mi_alloc_tile_component_data(l_tilec)) {
												                mi_event_msg(p_manager, EVT_ERROR, "Error allocating tile component data." );
												                if (l_current_data) {
												                        mi_free(l_current_data);
												                }
												                return mi_FALSE;
												        }
                        }
                }
                l_current_tile_size = mi_tcd_get_encoded_tile_size(p_j2k->m_tcd);
                if (!l_reuse_data) {
                        if (l_current_tile_size > l_max_tile_size) {
												        mi_BYTE *l_new_current_data = (mi_BYTE *) mi_realloc(l_current_data, l_current_tile_size);
												        if (! l_new_current_data) {
												                if (l_current_data) {
												                        mi_free(l_current_data);
												                }
												                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to encode all tiles\n");
												                return mi_FALSE;
																}
																l_current_data = l_new_current_data;
																l_max_tile_size = l_current_tile_size;
                        }

                        /* copy image data (32 bit) to l_current_data as contiguous, all-component, zero offset buffer */
                        /* 32 bit components @ 8 bit precision get converted to 8 bit */
                        /* 32 bit components @ 16 bit precision get converted to 16 bit */
                        mi_j2k_get_tile_data(p_j2k->m_tcd,l_current_data);

                        /* now copy this data into the tile component */
                        if (! mi_tcd_copy_tile_data(p_j2k->m_tcd,l_current_data,l_current_tile_size)) {
																mi_event_msg(p_manager, EVT_ERROR, "Size mismatch between tile data and sent data." );
																mi_free(l_current_data);
																return mi_FALSE;
                        }
                }

                if (! mi_j2k_post_write_tile (p_j2k,p_stream,p_manager)) {
                        if (l_current_data) {
                                mi_free(l_current_data);
                        }
                        return mi_FALSE;
                }
        }

        if (l_current_data) {
                mi_free(l_current_data);
        }
        return mi_TRUE;
}

mi_BOOL mi_j2k_end_compress(  mi_j2k_t *p_j2k,
                                                        mi_stream_private_t *p_stream,
                                                        mi_event_mgr_t * p_manager)
{
        /* customization of the encoding */
        if (! mi_j2k_setup_end_compress(p_j2k, p_manager)) {
                return mi_FALSE;
        }

        if (! mi_j2k_exec (p_j2k, p_j2k->m_procedure_list, p_stream, p_manager))
        {
                return mi_FALSE;
        }

        return mi_TRUE;
}

mi_BOOL mi_j2k_start_compress(mi_j2k_t *p_j2k,
                                                            mi_stream_private_t *p_stream,
                                                            mi_image_t * p_image,
                                                            mi_event_mgr_t * p_manager)
{
        /* preconditions */
        assert(p_j2k != 00);
        assert(p_stream != 00);
        assert(p_manager != 00);

        p_j2k->m_private_image = mi_image_create0();
        if (! p_j2k->m_private_image) {
                mi_event_msg(p_manager, EVT_ERROR, "Failed to allocate image header." );
                return mi_FALSE;
        }
        mi_copy_image_header(p_image, p_j2k->m_private_image);

        /* TODO_MSD: Find a better way */
        if (p_image->comps) {
                mi_UINT32 it_comp;
                for (it_comp = 0 ; it_comp < p_image->numcomps; it_comp++) {
                        if (p_image->comps[it_comp].data) {
                                p_j2k->m_private_image->comps[it_comp].data =p_image->comps[it_comp].data;
                                p_image->comps[it_comp].data = NULL;

                        }
                }
        }

        /* customization of the validation */
        if (! mi_j2k_setup_encoding_validation (p_j2k, p_manager)) {
                return mi_FALSE;
        }

        /* validation of the parameters codec */
        if (! mi_j2k_exec(p_j2k,p_j2k->m_validation_list,p_stream,p_manager)) {
                return mi_FALSE;
        }

        /* customization of the encoding */
        if (! mi_j2k_setup_header_writing(p_j2k, p_manager)) {
                return mi_FALSE;
        }

        /* write header */
        if (! mi_j2k_exec (p_j2k,p_j2k->m_procedure_list,p_stream,p_manager)) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_pre_write_tile (       mi_j2k_t * p_j2k,
                                                                mi_UINT32 p_tile_index,
                                                                mi_stream_private_t *p_stream,
                                                                mi_event_mgr_t * p_manager )
{
  (void)p_stream;
        if (p_tile_index != p_j2k->m_current_tile_number) {
                mi_event_msg(p_manager, EVT_ERROR, "The given tile index does not match." );
                return mi_FALSE;
        }

        mi_event_msg(p_manager, EVT_INFO, "tile number %d / %d\n", p_j2k->m_current_tile_number + 1, p_j2k->m_cp.tw * p_j2k->m_cp.th);

        p_j2k->m_specific_param.m_encoder.m_current_tile_part_number = 0;
        p_j2k->m_tcd->cur_totnum_tp = p_j2k->m_cp.tcps[p_tile_index].m_nb_tile_parts;
        p_j2k->m_specific_param.m_encoder.m_current_poc_tile_part_number = 0;

        /* initialisation before tile encoding  */
        if (! mi_tcd_init_encode_tile(p_j2k->m_tcd, p_j2k->m_current_tile_number, p_manager)) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

static void mi_get_tile_dimensions(mi_image_t * l_image,
                             mi_tcd_tilecomp_t * l_tilec,
                             mi_image_comp_t * l_img_comp,
                             mi_UINT32* l_size_comp,
                             mi_UINT32* l_width,
                             mi_UINT32* l_height,
                             mi_UINT32* l_offset_x,
                             mi_UINT32* l_offset_y,
                             mi_UINT32* l_image_width,
                             mi_UINT32* l_stride,
                             mi_UINT32* l_tile_offset) {
	mi_UINT32 l_remaining;
	*l_size_comp = l_img_comp->prec >> 3; /* (/8) */
	l_remaining = l_img_comp->prec & 7;  /* (%8) */
	if (l_remaining) {
		*l_size_comp += 1;
	}

	if (*l_size_comp == 3) {
		*l_size_comp = 4;
	}

	*l_width  = (mi_UINT32)(l_tilec->x1 - l_tilec->x0);
	*l_height = (mi_UINT32)(l_tilec->y1 - l_tilec->y0);
	*l_offset_x = (mi_UINT32)mi_int_ceildiv((mi_INT32)l_image->x0, (mi_INT32)l_img_comp->dx);
	*l_offset_y = (mi_UINT32)mi_int_ceildiv((mi_INT32)l_image->y0, (mi_INT32)l_img_comp->dy);
	*l_image_width = (mi_UINT32)mi_int_ceildiv((mi_INT32)l_image->x1 - (mi_INT32)l_image->x0, (mi_INT32)l_img_comp->dx);
	*l_stride = *l_image_width - *l_width;
	*l_tile_offset = ((mi_UINT32)l_tilec->x0 - *l_offset_x) + ((mi_UINT32)l_tilec->y0 - *l_offset_y) * *l_image_width;
}

static void mi_j2k_get_tile_data (mi_tcd_t * p_tcd, mi_BYTE * p_data)
{
        mi_UINT32 i,j,k = 0;

        for (i=0;i<p_tcd->image->numcomps;++i) {
                mi_image_t * l_image =  p_tcd->image;
                mi_INT32 * l_src_ptr;
                mi_tcd_tilecomp_t * l_tilec = p_tcd->tcd_image->tiles->comps + i;
                mi_image_comp_t * l_img_comp = l_image->comps + i;
                mi_UINT32 l_size_comp,l_width,l_height,l_offset_x,l_offset_y, l_image_width,l_stride,l_tile_offset;

                mi_get_tile_dimensions(l_image,
                                        l_tilec,
                                        l_img_comp,
                                        &l_size_comp,
                                        &l_width,
                                        &l_height,
                                        &l_offset_x,
                                        &l_offset_y,
                                        &l_image_width,
                                        &l_stride,
                                        &l_tile_offset);

                l_src_ptr = l_img_comp->data + l_tile_offset;

                switch (l_size_comp) {
                        case 1:
                                {
                                        mi_CHAR * l_dest_ptr = (mi_CHAR*) p_data;
                                        if (l_img_comp->sgnd) {
                                                for     (j=0;j<l_height;++j) {
                                                        for (k=0;k<l_width;++k) {
                                                                *(l_dest_ptr) = (mi_CHAR) (*l_src_ptr);
                                                                ++l_dest_ptr;
                                                                ++l_src_ptr;
                                                        }
                                                        l_src_ptr += l_stride;
                                                }
                                        }
                                        else {
                                                for (j=0;j<l_height;++j) {
                                                        for (k=0;k<l_width;++k) {
                                                                *(l_dest_ptr) = (mi_CHAR)((*l_src_ptr)&0xff);
                                                                ++l_dest_ptr;
                                                                ++l_src_ptr;
                                                        }
                                                        l_src_ptr += l_stride;
                                                }
                                        }

                                        p_data = (mi_BYTE*) l_dest_ptr;
                                }
                                break;
                        case 2:
                                {
                                        mi_INT16 * l_dest_ptr = (mi_INT16 *) p_data;
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
                                                                *(l_dest_ptr++) = (mi_INT16)((*(l_src_ptr++)) & 0xffff);
                                                        }
                                                        l_src_ptr += l_stride;
                                                }
                                        }

                                        p_data = (mi_BYTE*) l_dest_ptr;
                                }
                                break;
                        case 4:
                                {
                                        mi_INT32 * l_dest_ptr = (mi_INT32 *) p_data;
                                        for (j=0;j<l_height;++j) {
                                                for (k=0;k<l_width;++k) {
                                                        *(l_dest_ptr++) = *(l_src_ptr++);
                                                }
                                                l_src_ptr += l_stride;
                                        }

                                        p_data = (mi_BYTE*) l_dest_ptr;
                                }
                                break;
                }
        }
}

static mi_BOOL mi_j2k_post_write_tile (      mi_j2k_t * p_j2k,
                                                                mi_stream_private_t *p_stream,
                                                                mi_event_mgr_t * p_manager )
{
        mi_UINT32 l_nb_bytes_written;
        mi_BYTE * l_current_data = 00;
        mi_UINT32 l_tile_size = 0;
        mi_UINT32 l_available_data;

        /* preconditions */
        assert(p_j2k->m_specific_param.m_encoder.m_encoded_tile_data);

        l_tile_size = p_j2k->m_specific_param.m_encoder.m_encoded_tile_size;
        l_available_data = l_tile_size;
        l_current_data = p_j2k->m_specific_param.m_encoder.m_encoded_tile_data;

        l_nb_bytes_written = 0;
        if (! mi_j2k_write_first_tile_part(p_j2k,l_current_data,&l_nb_bytes_written,l_available_data,p_stream,p_manager)) {
                return mi_FALSE;
        }
        l_current_data += l_nb_bytes_written;
        l_available_data -= l_nb_bytes_written;

        l_nb_bytes_written = 0;
        if (! mi_j2k_write_all_tile_parts(p_j2k,l_current_data,&l_nb_bytes_written,l_available_data,p_stream,p_manager)) {
                return mi_FALSE;
        }

        l_available_data -= l_nb_bytes_written;
        l_nb_bytes_written = l_tile_size - l_available_data;

        if ( mi_stream_write_data(     p_stream,
                                                                p_j2k->m_specific_param.m_encoder.m_encoded_tile_data,
                                                                l_nb_bytes_written,p_manager) != l_nb_bytes_written) {
                return mi_FALSE;
        }

        ++p_j2k->m_current_tile_number;

        return mi_TRUE;
}

static mi_BOOL mi_j2k_setup_end_compress (mi_j2k_t *p_j2k, mi_event_mgr_t * p_manager)
{
        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);

        /* DEVELOPER CORNER, insert your custom procedures */
        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_write_eoc, p_manager)) {
                return mi_FALSE;
        }

        if (mi_IS_CINEMA(p_j2k->m_cp.rsiz)) {
                if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_write_updated_tlm, p_manager)) {
                        return mi_FALSE;
                }
        }

        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_write_epc, p_manager)) {
                return mi_FALSE;
        }
        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_end_encoding, p_manager)) {
                return mi_FALSE;
        }
        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_destroy_header_memory, p_manager)) {
                return mi_FALSE;
        }
        return mi_TRUE;
}
//装载编码验证函数（用户自定义）
static mi_BOOL mi_j2k_setup_encoding_validation (mi_j2k_t *p_j2k, mi_event_mgr_t * p_manager)
{
        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);

        if (! mi_procedure_list_add_procedure(p_j2k->m_validation_list, (mi_procedure)mi_j2k_build_encoder, p_manager)) {
                return mi_FALSE;
        }
        if (! mi_procedure_list_add_procedure(p_j2k->m_validation_list, (mi_procedure)mi_j2k_encoding_validation, p_manager)) {
                return mi_FALSE;
				}

        /* DEVELOPER CORNER, add your custom validation procedure */
        if (! mi_procedure_list_add_procedure(p_j2k->m_validation_list, (mi_procedure)mi_j2k_mct_validation, p_manager)) {
                return mi_FALSE;
        }
	
        return mi_TRUE;
}
//!!!!!!这个函数用于向JPEG2000输出格式文件写文件头与marker
static mi_BOOL mi_j2k_setup_header_writing (mi_j2k_t *p_j2k, mi_event_mgr_t * p_manager)
{
        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);

        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_init_info, p_manager)) {
                return mi_FALSE;
        }
        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_write_soc, p_manager)) {
                return mi_FALSE;
        }
        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_write_siz, p_manager)) {
                return mi_FALSE;
        }
        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_write_cod, p_manager)) {
                return mi_FALSE;
        }
        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_write_qcd, p_manager)) {
                return mi_FALSE;
        }
        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_write_all_coc, p_manager)) {
                return mi_FALSE;
        }
        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_write_all_qcc, p_manager)) {
                return mi_FALSE;
        }

        if (mi_IS_CINEMA(p_j2k->m_cp.rsiz)) {
                if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_write_tlm, p_manager)) {
                        return mi_FALSE;
                }

                if (p_j2k->m_cp.rsiz == mi_PROFILE_CINEMA_4K) {
                        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_write_poc, p_manager)) {
                                return mi_FALSE;
                        }
                }
        }

        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_write_regions, p_manager)) {
                return mi_FALSE;
        }

        if (p_j2k->m_cp.comment != 00)  {
                if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_write_com, p_manager)) {
                        return mi_FALSE;
                }
        }

        /* DEVELOPER CORNER, insert your custom procedures */
        if (p_j2k->m_cp.rsiz & mi_EXTENSION_MCT) {
                if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_write_mct_data_group, p_manager)) {
                        return mi_FALSE;
                }
        }
        /* End of Developer Corner */

        if (p_j2k->cstr_index) {
                if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_get_end_header, p_manager)) {
                        return mi_FALSE;
                }
        }

        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_create_tcd, p_manager)) {
                return mi_FALSE;
        }
        if (! mi_procedure_list_add_procedure(p_j2k->m_procedure_list,(mi_procedure)mi_j2k_update_rates, p_manager)) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_first_tile_part (mi_j2k_t *p_j2k,
                                                                        mi_BYTE * p_data,
                                                                        mi_UINT32 * p_data_written,
                                                                        mi_UINT32 p_total_data_size,
                                                                        mi_stream_private_t *p_stream,
                                                                        struct mi_event_mgr * p_manager )
{
        mi_UINT32 l_nb_bytes_written = 0;
        mi_UINT32 l_current_nb_bytes_written;
        mi_BYTE * l_begin_data = 00;

        mi_tcd_t * l_tcd = 00;
        mi_cp_t * l_cp = 00;

        l_tcd = p_j2k->m_tcd;
        l_cp = &(p_j2k->m_cp);

        l_tcd->cur_pino = 0;

        /*Get number of tile parts*/
        p_j2k->m_specific_param.m_encoder.m_current_poc_tile_part_number = 0;

        /* INDEX >> */
        /* << INDEX */

        l_current_nb_bytes_written = 0;
        l_begin_data = p_data;
        if (! mi_j2k_write_sot(p_j2k,p_data,&l_current_nb_bytes_written,p_stream,p_manager))
        {
                return mi_FALSE;
        }

        l_nb_bytes_written += l_current_nb_bytes_written;
        p_data += l_current_nb_bytes_written;
        p_total_data_size -= l_current_nb_bytes_written;

        if (!mi_IS_CINEMA(l_cp->rsiz)) {
                if (l_cp->tcps[p_j2k->m_current_tile_number].numpocs) {
                        l_current_nb_bytes_written = 0;
                        mi_j2k_write_poc_in_memory(p_j2k,p_data,&l_current_nb_bytes_written,p_manager);
                        l_nb_bytes_written += l_current_nb_bytes_written;
                        p_data += l_current_nb_bytes_written;
                        p_total_data_size -= l_current_nb_bytes_written;
                }
        }

        l_current_nb_bytes_written = 0;
        if (! mi_j2k_write_sod(p_j2k,l_tcd,p_data,&l_current_nb_bytes_written,p_total_data_size,p_stream,p_manager)) {
                return mi_FALSE;
        }

        l_nb_bytes_written += l_current_nb_bytes_written;
        * p_data_written = l_nb_bytes_written;

        /* Writing Psot in SOT marker */
        mi_write_bytes(l_begin_data + 6,l_nb_bytes_written,4);                                 /* PSOT */

        if (mi_IS_CINEMA(l_cp->rsiz)){
                mi_j2k_update_tlm(p_j2k,l_nb_bytes_written);
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_all_tile_parts(  mi_j2k_t *p_j2k,
                                                                        mi_BYTE * p_data,
                                                                        mi_UINT32 * p_data_written,
                                                                        mi_UINT32 p_total_data_size,
                                                                        mi_stream_private_t *p_stream,
                                                                        struct mi_event_mgr * p_manager
                                                                )
{
        mi_UINT32 tilepartno=0;
        mi_UINT32 l_nb_bytes_written = 0;
        mi_UINT32 l_current_nb_bytes_written;
        mi_UINT32 l_part_tile_size;
        mi_UINT32 tot_num_tp;
        mi_UINT32 pino;

        mi_BYTE * l_begin_data;
        mi_tcp_t *l_tcp = 00;
        mi_tcd_t * l_tcd = 00;
        mi_cp_t * l_cp = 00;

        l_tcd = p_j2k->m_tcd;
        l_cp = &(p_j2k->m_cp);
        l_tcp = l_cp->tcps + p_j2k->m_current_tile_number;

        /*Get number of tile parts*/
        tot_num_tp = mi_j2k_get_num_tp(l_cp,0,p_j2k->m_current_tile_number);

        /* start writing remaining tile parts */
        ++p_j2k->m_specific_param.m_encoder.m_current_tile_part_number;
        for (tilepartno = 1; tilepartno < tot_num_tp ; ++tilepartno) {
                p_j2k->m_specific_param.m_encoder.m_current_poc_tile_part_number = tilepartno;
                l_current_nb_bytes_written = 0;
                l_part_tile_size = 0;
                l_begin_data = p_data;

                if (! mi_j2k_write_sot(p_j2k,p_data,&l_current_nb_bytes_written,p_stream,p_manager)) {
                        return mi_FALSE;
                }

                l_nb_bytes_written += l_current_nb_bytes_written;
                p_data += l_current_nb_bytes_written;
                p_total_data_size -= l_current_nb_bytes_written;
                l_part_tile_size += l_current_nb_bytes_written;

                l_current_nb_bytes_written = 0;
                if (! mi_j2k_write_sod(p_j2k,l_tcd,p_data,&l_current_nb_bytes_written,p_total_data_size,p_stream,p_manager)) {
                        return mi_FALSE;
                }

                p_data += l_current_nb_bytes_written;
                l_nb_bytes_written += l_current_nb_bytes_written;
                p_total_data_size -= l_current_nb_bytes_written;
                l_part_tile_size += l_current_nb_bytes_written;

                /* Writing Psot in SOT marker */
                mi_write_bytes(l_begin_data + 6,l_part_tile_size,4);                                   /* PSOT */

                if (mi_IS_CINEMA(l_cp->rsiz)) {
                        mi_j2k_update_tlm(p_j2k,l_part_tile_size);
                }

                ++p_j2k->m_specific_param.m_encoder.m_current_tile_part_number;
        }

        for (pino = 1; pino <= l_tcp->numpocs; ++pino) {
                l_tcd->cur_pino = pino;

                /*Get number of tile parts*/
                tot_num_tp = mi_j2k_get_num_tp(l_cp,pino,p_j2k->m_current_tile_number);
                for (tilepartno = 0; tilepartno < tot_num_tp ; ++tilepartno) {
                        p_j2k->m_specific_param.m_encoder.m_current_poc_tile_part_number = tilepartno;
                        l_current_nb_bytes_written = 0;
                        l_part_tile_size = 0;
                        l_begin_data = p_data;

                        if (! mi_j2k_write_sot(p_j2k,p_data,&l_current_nb_bytes_written,p_stream,p_manager)) {
                                return mi_FALSE;
                        }

                        l_nb_bytes_written += l_current_nb_bytes_written;
                        p_data += l_current_nb_bytes_written;
                        p_total_data_size -= l_current_nb_bytes_written;
                        l_part_tile_size += l_current_nb_bytes_written;

                        l_current_nb_bytes_written = 0;

                        if (! mi_j2k_write_sod(p_j2k,l_tcd,p_data,&l_current_nb_bytes_written,p_total_data_size,p_stream,p_manager)) {
                                return mi_FALSE;
                        }

                        l_nb_bytes_written += l_current_nb_bytes_written;
                        p_data += l_current_nb_bytes_written;
                        p_total_data_size -= l_current_nb_bytes_written;
                        l_part_tile_size += l_current_nb_bytes_written;

                        /* Writing Psot in SOT marker */
                        mi_write_bytes(l_begin_data + 6,l_part_tile_size,4);                                   /* PSOT */

                        if (mi_IS_CINEMA(l_cp->rsiz)) {
                                mi_j2k_update_tlm(p_j2k,l_part_tile_size);
                        }

                        ++p_j2k->m_specific_param.m_encoder.m_current_tile_part_number;
                }
        }

        *p_data_written = l_nb_bytes_written;

        return mi_TRUE;
}

static mi_BOOL mi_j2k_write_updated_tlm( mi_j2k_t *p_j2k,
                                                                    struct mi_stream_private *p_stream,
                                                                    struct mi_event_mgr * p_manager )
{
        mi_UINT32 l_tlm_size;
        mi_OFF_T l_tlm_position, l_current_position;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        l_tlm_size = 5 * p_j2k->m_specific_param.m_encoder.m_total_tile_parts;
        l_tlm_position = 6 + p_j2k->m_specific_param.m_encoder.m_tlm_start;
        l_current_position = mi_stream_tell(p_stream);

        if (! mi_stream_seek(p_stream,l_tlm_position,p_manager)) {
                return mi_FALSE;
        }

        if (mi_stream_write_data(p_stream,p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer,l_tlm_size,p_manager) != l_tlm_size) {
                return mi_FALSE;
        }

        if (! mi_stream_seek(p_stream,l_current_position,p_manager)) {
                return mi_FALSE;
        }

        return mi_TRUE;
}

static mi_BOOL mi_j2k_end_encoding(  mi_j2k_t *p_j2k,
                                                        struct mi_stream_private *p_stream,
                                                        struct mi_event_mgr * p_manager )
{
        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        mi_tcd_destroy(p_j2k->m_tcd);
        p_j2k->m_tcd = 00;

        if (p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer) {
                mi_free(p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer);
                p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_buffer = 0;
                p_j2k->m_specific_param.m_encoder.m_tlm_sot_offsets_current = 0;
        }

        if (p_j2k->m_specific_param.m_encoder.m_encoded_tile_data) {
                mi_free(p_j2k->m_specific_param.m_encoder.m_encoded_tile_data);
                p_j2k->m_specific_param.m_encoder.m_encoded_tile_data = 0;
        }

        p_j2k->m_specific_param.m_encoder.m_encoded_tile_size = 0;

        return mi_TRUE;
}

/**
 * Destroys the memory associated with the decoding of headers.
 */
static mi_BOOL mi_j2k_destroy_header_memory ( mi_j2k_t * p_j2k,
                                                mi_stream_private_t *p_stream,
                                                mi_event_mgr_t * p_manager
                                                )
{
        /* preconditions */
        assert(p_j2k != 00);
        assert(p_stream != 00);
        assert(p_manager != 00);

        if (p_j2k->m_specific_param.m_encoder.m_header_tile_data) {
                mi_free(p_j2k->m_specific_param.m_encoder.m_header_tile_data);
                p_j2k->m_specific_param.m_encoder.m_header_tile_data = 0;
        }

        p_j2k->m_specific_param.m_encoder.m_header_tile_data_size = 0;

        return mi_TRUE;
}

static mi_BOOL mi_j2k_init_info(     mi_j2k_t *p_j2k,
                                                struct mi_stream_private *p_stream,
                                                struct mi_event_mgr * p_manager )
{
        mi_codestream_info_t * l_cstr_info = 00;

        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);
  (void)l_cstr_info;

        /* TODO mergeV2: check this part which use cstr_info */
        /*l_cstr_info = p_j2k->cstr_info;

        if (l_cstr_info)  {
                mi_UINT32 compno;
                l_cstr_info->tile = (mi_tile_info_t *) mi_malloc(p_j2k->m_cp.tw * p_j2k->m_cp.th * sizeof(mi_tile_info_t));

                l_cstr_info->image_w = p_j2k->m_image->x1 - p_j2k->m_image->x0;
                l_cstr_info->image_h = p_j2k->m_image->y1 - p_j2k->m_image->y0;

                l_cstr_info->prog = (&p_j2k->m_cp.tcps[0])->prg;

                l_cstr_info->tw = p_j2k->m_cp.tw;
                l_cstr_info->th = p_j2k->m_cp.th;

                l_cstr_info->tile_x = p_j2k->m_cp.tdx;*/        /* new version parser */
                /*l_cstr_info->tile_y = p_j2k->m_cp.tdy;*/      /* new version parser */
                /*l_cstr_info->tile_Ox = p_j2k->m_cp.tx0;*/     /* new version parser */
                /*l_cstr_info->tile_Oy = p_j2k->m_cp.ty0;*/     /* new version parser */

                /*l_cstr_info->numcomps = p_j2k->m_image->numcomps;

                l_cstr_info->numlayers = (&p_j2k->m_cp.tcps[0])->numlayers;

                l_cstr_info->numdecompos = (mi_INT32*) mi_malloc(p_j2k->m_image->numcomps * sizeof(mi_INT32));

                for (compno=0; compno < p_j2k->m_image->numcomps; compno++) {
                        l_cstr_info->numdecompos[compno] = (&p_j2k->m_cp.tcps[0])->tccps->numresolutions - 1;
                }

                l_cstr_info->D_max = 0.0;       */      /* ADD Marcela */

                /*l_cstr_info->main_head_start = mi_stream_tell(p_stream);*/ /* position of SOC */

                /*l_cstr_info->maxmarknum = 100;
                l_cstr_info->marker = (mi_marker_info_t *) mi_malloc(l_cstr_info->maxmarknum * sizeof(mi_marker_info_t));
                l_cstr_info->marknum = 0;
        }*/

        return mi_j2k_calculate_tp(p_j2k,&(p_j2k->m_cp),&p_j2k->m_specific_param.m_encoder.m_total_tile_parts,p_j2k->m_private_image,p_manager);
}

/**
 * Creates a tile-coder decoder.
 *
 * @param       p_stream                the stream to write data to.
 * @param       p_j2k                   J2K codec.
 * @param       p_manager               the user event manager.
*/
static mi_BOOL mi_j2k_create_tcd(     mi_j2k_t *p_j2k,
                                                                    mi_stream_private_t *p_stream,
                                                                    mi_event_mgr_t * p_manager
                                    )
{
        /* preconditions */
        assert(p_j2k != 00);
        assert(p_manager != 00);
        assert(p_stream != 00);

        p_j2k->m_tcd = mi_tcd_create(mi_FALSE);

        if (! p_j2k->m_tcd) {
                mi_event_msg(p_manager, EVT_ERROR, "Not enough memory to create Tile Coder\n");
                return mi_FALSE;
        }

        if (!mi_tcd_init(p_j2k->m_tcd,p_j2k->m_private_image,&p_j2k->m_cp)) {
                mi_tcd_destroy(p_j2k->m_tcd);
                p_j2k->m_tcd = 00;
                return mi_FALSE;
        }

        return mi_TRUE;
}

mi_BOOL mi_j2k_write_tile (mi_j2k_t * p_j2k,
                                                 mi_UINT32 p_tile_index,
                                                 mi_BYTE * p_data,
                                                 mi_UINT32 p_data_size,
                                                 mi_stream_private_t *p_stream,
                                                 mi_event_mgr_t * p_manager )
{
        if (! mi_j2k_pre_write_tile(p_j2k,p_tile_index,p_stream,p_manager)) {
                mi_event_msg(p_manager, EVT_ERROR, "Error while mi_j2k_pre_write_tile with tile index = %d\n", p_tile_index);
                return mi_FALSE;
        }
        else {
                mi_UINT32 j;
                /* Allocate data */
                for (j=0;j<p_j2k->m_tcd->image->numcomps;++j) {
                        mi_tcd_tilecomp_t* l_tilec = p_j2k->m_tcd->tcd_image->tiles->comps + j;

                        if(! mi_alloc_tile_component_data(l_tilec)) {
												        mi_event_msg(p_manager, EVT_ERROR, "Error allocating tile component data." );
                                return mi_FALSE;
                        }
                }

                /* now copy data into the tile component */
                if (! mi_tcd_copy_tile_data(p_j2k->m_tcd,p_data,p_data_size)) {
                        mi_event_msg(p_manager, EVT_ERROR, "Size mismatch between tile data and sent data." );
                        return mi_FALSE;
                }
                if (! mi_j2k_post_write_tile(p_j2k,p_stream,p_manager)) {
                        mi_event_msg(p_manager, EVT_ERROR, "Error while mi_j2k_post_write_tile with tile index = %d\n", p_tile_index);
                        return mi_FALSE;
                }
        }

        return mi_TRUE;
}
