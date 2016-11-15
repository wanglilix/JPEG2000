
#ifndef __CIO_H
#define __CIO_H
#include"openjpeg.h"
/**
@file cio.h
@brief Implementation of a byte input-output process (CIO)

The functions in CIO.C have for goal to realize a byte input / output process.
*/

/** @defgroup CIO CIO - byte input-output stream */
/*@{*/

//#include "mi_config_private.h"

/* ----------------------------------------------------------------------- */
	#define mi_write_bytes		mi_write_bytes_LE
	#define mi_read_bytes		mi_read_bytes_LE
	#define mi_write_double	mi_write_double_LE
	#define mi_read_double		mi_read_double_LE
	#define mi_write_float		mi_write_float_LE
	#define mi_read_float		mi_read_float_LE


#define mi_STREAM_STATUS_OUTPUT  0x1U
#define mi_STREAM_STATUS_INPUT   0x2U
#define mi_STREAM_STATUS_END     0x4U
#define mi_STREAM_STATUS_ERROR   0x8U

/**
Byte input-output stream.
*/
typedef struct mi_stream_private
{
	/**
	 * User data, be it files, ... The actual data depends on the type of the stream.
	 */
	void *					m_user_data;

	/**
	 * Pointer to function to free m_user_data (NULL at initialization)
	 * when destroying the stream. If pointer is NULL the function is not
	 * called and the m_user_data is not freed (even if non-NULL).
	 */
	mi_stream_free_user_data_fn		m_free_user_data_fn;

	/**
	 * User data length
	 */
	mi_UINT64 				m_user_data_length;

	/**
	 * Pointer to actual read function (NULL at the initialization of the cio.
	 */
	mi_stream_read_fn		m_read_fn;

	/**
	 * Pointer to actual write function (NULL at the initialization of the cio.
	 */
	mi_stream_write_fn		m_write_fn;

	/**
	 * Pointer to actual skip function (NULL at the initialization of the cio.
	 * There is no seek function to prevent from back and forth slow procedures.
	 */
	mi_stream_skip_fn		m_skip_fn;

	/**
	 * Pointer to actual seek function (if available).
	 */
	mi_stream_seek_fn		m_seek_fn;

	/**
	 * Actual data stored into the stream if readed from. Data is read by chunk of fixed size.
	 * you should never access this data directly.
	 */
	mi_BYTE *					m_stored_data;

	/**
	 * Pointer to the current read data.
	 */
	mi_BYTE *					m_current_data;

    /**
    * FIXME DOC.
    */
	mi_OFF_T (* m_mi_skip)(struct mi_stream_private * ,mi_OFF_T , struct mi_event_mgr *);

    /**
    * FIXME DOC.
    */
	mi_BOOL (* m_mi_seek) (struct mi_stream_private * , mi_OFF_T , struct mi_event_mgr *);

	/**
	 * number of bytes containing in the buffer.
	 */
	mi_SIZE_T			m_bytes_in_buffer;

	/**
	 * The number of bytes read/written from the beginning of the stream
	 */
	mi_OFF_T			m_byte_offset;

	/**
	 * The size of the buffer.
	 */
	mi_SIZE_T			m_buffer_size;

	/**
	 * Flags to tell the status of the stream.
	 * Used with mi_STREAM_STATUS_* defines.
	 */
	mi_UINT32 m_status;

}mi_stream_private_t;

/** @name Exported functions (see also openjpeg.h) */
/*@{*/
/* ----------------------------------------------------------------------- */
/**
 * Write some bytes to the given data buffer, this function is used in Big Endian cpus.
 * @param p_buffer		pointer the data buffer to write data to.
 * @param p_value		the value to write
 * @param p_nb_bytes	the number of bytes to write
*/
void mi_write_bytes_BE (mi_BYTE * p_buffer, mi_UINT32 p_value, mi_UINT32 p_nb_bytes);

/**
 * Reads some bytes from the given data buffer, this function is used in Big Endian cpus.
 * @param p_buffer		pointer the data buffer to read data from.
 * @param p_value		pointer to the value that will store the data.
 * @param p_nb_bytes	the nb bytes to read.
 * @return				the number of bytes read or -1 if an error occurred.
 */
void mi_read_bytes_BE(const mi_BYTE * p_buffer, mi_UINT32 * p_value, mi_UINT32 p_nb_bytes);

/**
 * Write some bytes to the given data buffer, this function is used in Little Endian cpus.
 * @param p_buffer		pointer the data buffer to write data to.
 * @param p_value		the value to write
 * @param p_nb_bytes	the number of bytes to write
 * @return				the number of bytes written or -1 if an error occurred
*/
void mi_write_bytes_LE (mi_BYTE * p_buffer, mi_UINT32 p_value, mi_UINT32 p_nb_bytes);

/**
 * Reads some bytes from the given data buffer, this function is used in Little Endian cpus.
 * @param p_buffer		pointer the data buffer to read data from.
 * @param p_value		pointer to the value that will store the data.
 * @param p_nb_bytes	the nb bytes to read.
 * @return				the number of bytes read or -1 if an error occurred.
 */
void mi_read_bytes_LE(const mi_BYTE * p_buffer, mi_UINT32 * p_value, mi_UINT32 p_nb_bytes);


/**
 * Write some bytes to the given data buffer, this function is used in Little Endian cpus.
 * @param p_buffer		pointer the data buffer to write data to.
 * @param p_value		the value to write
 */
void mi_write_double_LE(mi_BYTE * p_buffer, mi_FLOAT64 p_value);

/***
 * Write some bytes to the given data buffer, this function is used in Big Endian cpus.
 * @param p_buffer		pointer the data buffer to write data to.
 * @param p_value		the value to write
 */
void mi_write_double_BE(mi_BYTE * p_buffer, mi_FLOAT64 p_value);

/**
 * Reads some bytes from the given data buffer, this function is used in Little Endian cpus.
 * @param p_buffer		pointer the data buffer to read data from.
 * @param p_value		pointer to the value that will store the data.
 */
void mi_read_double_LE(const mi_BYTE * p_buffer, mi_FLOAT64 * p_value);

/**
 * Reads some bytes from the given data buffer, this function is used in Big Endian cpus.
 * @param p_buffer		pointer the data buffer to read data from.
 * @param p_value		pointer to the value that will store the data.
 */
void mi_read_double_BE(const mi_BYTE * p_buffer, mi_FLOAT64 * p_value);

/**
 * Reads some bytes from the given data buffer, this function is used in Little Endian cpus.
 * @param p_buffer		pointer the data buffer to read data from.
 * @param p_value		pointer to the value that will store the data.
 */
void mi_read_float_LE(const mi_BYTE * p_buffer, mi_FLOAT32 * p_value);

/**
 * Reads some bytes from the given data buffer, this function is used in Big Endian cpus.
 * @param p_buffer		pointer the data buffer to read data from.
 * @param p_value		pointer to the value that will store the data.
 */
void mi_read_float_BE(const mi_BYTE * p_buffer, mi_FLOAT32 * p_value);

/**
 * Write some bytes to the given data buffer, this function is used in Little Endian cpus.
 * @param p_buffer		pointer the data buffer to write data to.
 * @param p_value		the value to write
 */
void mi_write_float_LE(mi_BYTE * p_buffer, mi_FLOAT32 p_value);

/***
 * Write some bytes to the given data buffer, this function is used in Big Endian cpus.
 * @param p_buffer		pointer the data buffer to write data to.
 * @param p_value		the value to write
 */
void mi_write_float_BE(mi_BYTE * p_buffer, mi_FLOAT32 p_value);

/**
 * Reads some bytes from the stream.
 * @param		p_stream	the stream to read data from.
 * @param		p_buffer	pointer to the data buffer that will receive the data.
 * @param		p_size		number of bytes to read.
 * @param		p_event_mgr	the user event manager to be notified of special events.
 * @return		the number of bytes read, or -1 if an error occurred or if the stream is at the end.
 */
mi_SIZE_T mi_stream_read_data (mi_stream_private_t * p_stream,mi_BYTE * p_buffer, mi_SIZE_T p_size, struct mi_event_mgr * p_event_mgr);

/**
 * Writes some bytes to the stream.
 * @param		p_stream	the stream to write data to.
 * @param		p_buffer	pointer to the data buffer holds the data to be writtent.
 * @param		p_size		number of bytes to write.
 * @param		p_event_mgr	the user event manager to be notified of special events.
 * @return		the number of bytes writtent, or -1 if an error occurred.
 */
mi_SIZE_T mi_stream_write_data (mi_stream_private_t * p_stream,const mi_BYTE * p_buffer, mi_SIZE_T p_size, struct mi_event_mgr * p_event_mgr);

/**
 * Writes the content of the stream buffer to the stream.
 * @param		p_stream	the stream to write data to.
 * @param		p_event_mgr	the user event manager to be notified of special events.
 * @return		true if the data could be flushed, false else.
 */
mi_BOOL mi_stream_flush (mi_stream_private_t * p_stream, struct mi_event_mgr * p_event_mgr);

/**
 * Skips a number of bytes from the stream.
 * @param		p_stream	the stream to skip data from.
 * @param		p_size		the number of bytes to skip.
 * @param		p_event_mgr	the user event manager to be notified of special events.
 * @return		the number of bytes skipped, or -1 if an error occurred.
 */
mi_OFF_T mi_stream_skip (mi_stream_private_t * p_stream,mi_OFF_T p_size, struct mi_event_mgr * p_event_mgr);

/**
 * Tells the byte offset on the stream (similar to ftell).
 *
 * @param		p_stream	the stream to get the information from.
 *
 * @return		the current position o fthe stream.
 */
mi_OFF_T mi_stream_tell (const mi_stream_private_t * p_stream);


/**
 * Get the number of bytes left before the end of the stream (similar to cio_numbytesleft).
 *
 * @param		p_stream	the stream to get the information from.
 *
 * @return		Number of bytes left before the end of the stream.
 */
mi_OFF_T mi_stream_get_number_byte_left (const mi_stream_private_t * p_stream);

/**
 * Skips a number of bytes from the stream.
 * @param		p_stream	the stream to skip data from.
 * @param		p_size		the number of bytes to skip.
 * @param		p_event_mgr	the user event manager to be notified of special events.
 * @return		the number of bytes skipped, or -1 if an error occurred.
 */
mi_OFF_T mi_stream_write_skip (mi_stream_private_t * p_stream, mi_OFF_T p_size, struct mi_event_mgr * p_event_mgr);

/**
 * Skips a number of bytes from the stream.
 * @param		p_stream	the stream to skip data from.
 * @param		p_size		the number of bytes to skip.
 * @param		p_event_mgr	the user event manager to be notified of special events.
 * @return		the number of bytes skipped, or -1 if an error occurred.
 */
mi_OFF_T mi_stream_read_skip (mi_stream_private_t * p_stream, mi_OFF_T p_size, struct mi_event_mgr * p_event_mgr);

/**
 * Skips a number of bytes from the stream.
 * @param		p_stream	the stream to skip data from.
 * @param		p_size		the number of bytes to skip.
 * @param		p_event_mgr	the user event manager to be notified of special events.
 * @return		mi_TRUE if success, or mi_FALSE if an error occurred.
 */
mi_BOOL mi_stream_read_seek (mi_stream_private_t * p_stream, mi_OFF_T p_size, struct mi_event_mgr * p_event_mgr);

/**
 * Skips a number of bytes from the stream.
 * @param		p_stream	the stream to skip data from.
 * @param		p_size		the number of bytes to skip.
 * @param		p_event_mgr	the user event manager to be notified of special events.
 * @return		the number of bytes skipped, or -1 if an error occurred.
 */
mi_BOOL mi_stream_write_seek (mi_stream_private_t * p_stream, mi_OFF_T p_size, struct mi_event_mgr * p_event_mgr);

/**
 * Seeks a number of bytes from the stream.
 * @param		p_stream	the stream to skip data from.
 * @param		p_size		the number of bytes to skip.
 * @param		p_event_mgr	the user event manager to be notified of special events.
 * @return		true if the stream is seekable.
 */
mi_BOOL mi_stream_seek (mi_stream_private_t * p_stream, mi_OFF_T p_size, struct mi_event_mgr * p_event_mgr);

/**
 * Tells if the given stream is seekable.
 */
mi_BOOL mi_stream_has_seek (const mi_stream_private_t * p_stream);

/**
 * FIXME DOC.
 */
mi_SIZE_T mi_stream_default_read (void * p_buffer, mi_SIZE_T p_nb_bytes, void * p_user_data);

/**
 * FIXME DOC.
 */
mi_SIZE_T mi_stream_default_write (void * p_buffer, mi_SIZE_T p_nb_bytes, void * p_user_data);

/**
 * FIXME DOC.
 */
mi_OFF_T mi_stream_default_skip (mi_OFF_T p_nb_bytes, void * p_user_data);

/**
 * FIXME DOC.
 */
mi_BOOL mi_stream_default_seek (mi_OFF_T p_nb_bytes, void * p_user_data);

/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/


#endif /* __CIO_H */

