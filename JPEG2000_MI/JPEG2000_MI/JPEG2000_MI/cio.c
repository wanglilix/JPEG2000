
#include "mi_includes.h"

/* ----------------------------------------------------------------------- */


/* ----------------------------------------------------------------------- */

void mi_write_bytes_BE (mi_BYTE * p_buffer, mi_UINT32 p_value, mi_UINT32 p_nb_bytes)
{
	const mi_BYTE * l_data_ptr = ((const mi_BYTE *) &p_value)+sizeof(mi_UINT32)-p_nb_bytes;

	assert(p_nb_bytes > 0 && p_nb_bytes <=  sizeof(mi_UINT32));

	memcpy(p_buffer,l_data_ptr,p_nb_bytes);
}

void mi_write_bytes_LE (mi_BYTE * p_buffer, mi_UINT32 p_value, mi_UINT32 p_nb_bytes)
{
	const mi_BYTE * l_data_ptr = ((const mi_BYTE *) &p_value) + p_nb_bytes - 1;
	mi_UINT32 i;

	assert(p_nb_bytes > 0 && p_nb_bytes <= sizeof(mi_UINT32));

	for	(i=0;i<p_nb_bytes;++i) {
		*(p_buffer++) = *(l_data_ptr--);
	}
}

void mi_read_bytes_BE(const mi_BYTE * p_buffer, mi_UINT32 * p_value, mi_UINT32 p_nb_bytes)
{
	mi_BYTE * l_data_ptr = ((mi_BYTE *) p_value);

	assert(p_nb_bytes > 0 && p_nb_bytes <= sizeof(mi_UINT32));

	*p_value = 0;
	memcpy(l_data_ptr+sizeof(mi_UINT32)-p_nb_bytes,p_buffer,p_nb_bytes);
}

void mi_read_bytes_LE(const mi_BYTE * p_buffer, mi_UINT32 * p_value, mi_UINT32 p_nb_bytes)
{
	mi_BYTE * l_data_ptr = ((mi_BYTE *) p_value) + p_nb_bytes-1;
	mi_UINT32 i;

	assert(p_nb_bytes > 0 && p_nb_bytes <= sizeof(mi_UINT32));

	*p_value = 0;
	for (i=0;i<p_nb_bytes;++i) {
		*(l_data_ptr--) = *(p_buffer++);
	}
}

void mi_write_double_BE(mi_BYTE * p_buffer, mi_FLOAT64 p_value)
{
	const mi_BYTE * l_data_ptr = ((const mi_BYTE *) &p_value);
	memcpy(p_buffer,l_data_ptr,sizeof(mi_FLOAT64));
}

void mi_write_double_LE(mi_BYTE * p_buffer, mi_FLOAT64 p_value)
{
	const mi_BYTE * l_data_ptr = ((const mi_BYTE *) &p_value) + sizeof(mi_FLOAT64) - 1;
	mi_UINT32 i;
	for	(i=0;i<sizeof(mi_FLOAT64);++i) {
		*(p_buffer++) = *(l_data_ptr--);
	}
}

void mi_read_double_BE(const mi_BYTE * p_buffer, mi_FLOAT64 * p_value)
{
	mi_BYTE * l_data_ptr = ((mi_BYTE *) p_value);
	memcpy(l_data_ptr,p_buffer,sizeof(mi_FLOAT64));
}

void mi_read_double_LE(const mi_BYTE * p_buffer, mi_FLOAT64 * p_value)
{
	mi_BYTE * l_data_ptr = ((mi_BYTE *) p_value) + sizeof(mi_FLOAT64)-1;
	mi_UINT32 i;
	for (i=0;i<sizeof(mi_FLOAT64);++i) {
		*(l_data_ptr--) = *(p_buffer++);
	}
}

void mi_write_float_BE(mi_BYTE * p_buffer, mi_FLOAT32 p_value)
{
	const mi_BYTE * l_data_ptr = ((const mi_BYTE *) &p_value);
	memcpy(p_buffer,l_data_ptr,sizeof(mi_FLOAT32));
}

void mi_write_float_LE(mi_BYTE * p_buffer, mi_FLOAT32 p_value)
{
	const mi_BYTE * l_data_ptr = ((const mi_BYTE *) &p_value) + sizeof(mi_FLOAT32) - 1;
	mi_UINT32 i;
	for	(i=0;i<sizeof(mi_FLOAT32);++i) {
		*(p_buffer++) = *(l_data_ptr--);
	}
}

void mi_read_float_BE(const mi_BYTE * p_buffer, mi_FLOAT32 * p_value)
{
	mi_BYTE * l_data_ptr = ((mi_BYTE *) p_value);
	memcpy(l_data_ptr,p_buffer,sizeof(mi_FLOAT32));
}

void mi_read_float_LE(const mi_BYTE * p_buffer, mi_FLOAT32 * p_value)
{
	mi_BYTE * l_data_ptr = ((mi_BYTE *) p_value) + sizeof(mi_FLOAT32)-1;
	mi_UINT32 i;
	for	(i=0;i<sizeof(mi_FLOAT32);++i) {
		*(l_data_ptr--) = *(p_buffer++);
	}
}

mi_stream_t* mi_CALLCONV mi_stream_create(mi_SIZE_T p_buffer_size,mi_BOOL l_is_input)
{
	mi_stream_private_t * l_stream = 00;
	l_stream = (mi_stream_private_t*) mi_calloc(1,sizeof(mi_stream_private_t));
	if (! l_stream) {
		return 00;
	}

	l_stream->m_buffer_size = p_buffer_size;
	l_stream->m_stored_data = (mi_BYTE *) mi_malloc(p_buffer_size);
	if (! l_stream->m_stored_data) {
		mi_free(l_stream);
		return 00;
	}

	l_stream->m_current_data = l_stream->m_stored_data;

	if (l_is_input) {
		l_stream->m_status |= mi_STREAM_STATUS_INPUT;
		l_stream->m_mi_skip = mi_stream_read_skip;
		l_stream->m_mi_seek = mi_stream_read_seek;
	}
	else {
		l_stream->m_status |= mi_STREAM_STATUS_OUTPUT;
		l_stream->m_mi_skip = mi_stream_write_skip;
		l_stream->m_mi_seek = mi_stream_write_seek;
	}

	l_stream->m_read_fn = mi_stream_default_read;
	l_stream->m_write_fn = mi_stream_default_write;
	l_stream->m_skip_fn = mi_stream_default_skip;
	l_stream->m_seek_fn = mi_stream_default_seek;

	return (mi_stream_t *) l_stream;
}

mi_stream_t* mi_CALLCONV mi_stream_default_create(mi_BOOL l_is_input)
{
	return mi_stream_create(mi_J2K_STREAM_CHUNK_SIZE,l_is_input);
}

void mi_CALLCONV mi_stream_destroy(mi_stream_t* p_stream)
{
	mi_stream_private_t* l_stream = (mi_stream_private_t*) p_stream;
	
	if (l_stream) {
		if (l_stream->m_free_user_data_fn) {
			l_stream->m_free_user_data_fn(l_stream->m_user_data);
		}
		mi_free(l_stream->m_stored_data);
		l_stream->m_stored_data = 00;
		mi_free(l_stream);
	}
}

void mi_CALLCONV mi_stream_set_read_function(mi_stream_t* p_stream, mi_stream_read_fn p_function)
{
	mi_stream_private_t* l_stream = (mi_stream_private_t*) p_stream;

	if ((!l_stream) || (! (l_stream->m_status & mi_STREAM_STATUS_INPUT))) {
		return;
	}

	l_stream->m_read_fn = p_function;
}

void mi_CALLCONV mi_stream_set_seek_function(mi_stream_t* p_stream, mi_stream_seek_fn p_function)
{
	mi_stream_private_t* l_stream = (mi_stream_private_t*) p_stream;
	
	if (!l_stream) {
		return;
	}
	l_stream->m_seek_fn = p_function;
}

void mi_CALLCONV mi_stream_set_write_function(mi_stream_t* p_stream, mi_stream_write_fn p_function)
{
	mi_stream_private_t* l_stream = (mi_stream_private_t*) p_stream;
	
	if ((!l_stream )|| (! (l_stream->m_status & mi_STREAM_STATUS_OUTPUT))) {
		return;
	}

	l_stream->m_write_fn = p_function;
}

void mi_CALLCONV mi_stream_set_skip_function(mi_stream_t* p_stream, mi_stream_skip_fn p_function)
{
	mi_stream_private_t* l_stream = (mi_stream_private_t*) p_stream;
	
	if (! l_stream) {
		return;
	}

	l_stream->m_skip_fn = p_function;
}

void mi_CALLCONV mi_stream_set_user_data(mi_stream_t* p_stream, void * p_data, mi_stream_free_user_data_fn p_function)
{
	mi_stream_private_t* l_stream = (mi_stream_private_t*) p_stream;
	if (!l_stream)
		return;
	l_stream->m_user_data = p_data;
  l_stream->m_free_user_data_fn = p_function;
}

void mi_CALLCONV mi_stream_set_user_data_length(mi_stream_t* p_stream, mi_UINT64 data_length)
{
	mi_stream_private_t* l_stream = (mi_stream_private_t*) p_stream;
	if (!l_stream)
		return;
	l_stream->m_user_data_length = data_length;
}

mi_SIZE_T mi_stream_read_data (mi_stream_private_t * p_stream,mi_BYTE * p_buffer, mi_SIZE_T p_size, mi_event_mgr_t * p_event_mgr)
{
	mi_SIZE_T l_read_nb_bytes = 0;
	if (p_stream->m_bytes_in_buffer >= p_size) {
		memcpy(p_buffer,p_stream->m_current_data,p_size);
		p_stream->m_current_data += p_size;
		p_stream->m_bytes_in_buffer -= p_size;
		l_read_nb_bytes += p_size;
		p_stream->m_byte_offset += (mi_OFF_T)p_size;
		return l_read_nb_bytes;
	}

	/* we are now in the case when the remaining data if not sufficient */
	if (p_stream->m_status & mi_STREAM_STATUS_END) {
		l_read_nb_bytes += p_stream->m_bytes_in_buffer;
		memcpy(p_buffer,p_stream->m_current_data,p_stream->m_bytes_in_buffer);
		p_stream->m_current_data += p_stream->m_bytes_in_buffer;
		p_stream->m_byte_offset += (mi_OFF_T)p_stream->m_bytes_in_buffer;
		p_stream->m_bytes_in_buffer = 0;
		return l_read_nb_bytes ? l_read_nb_bytes : (mi_SIZE_T)-1;
	}

	/* the flag is not set, we copy data and then do an actual read on the stream */
	if (p_stream->m_bytes_in_buffer) {
		l_read_nb_bytes += p_stream->m_bytes_in_buffer;
		memcpy(p_buffer,p_stream->m_current_data,p_stream->m_bytes_in_buffer);
		p_stream->m_current_data = p_stream->m_stored_data;
		p_buffer += p_stream->m_bytes_in_buffer;
		p_size -= p_stream->m_bytes_in_buffer;
		p_stream->m_byte_offset += (mi_OFF_T)p_stream->m_bytes_in_buffer;
		p_stream->m_bytes_in_buffer = 0;
	}
	else {
    /* case where we are already at the end of the buffer
       so reset the m_current_data to point to the start of the
       stored buffer to get ready to read from disk*/
		p_stream->m_current_data = p_stream->m_stored_data;
	}

	for (;;) {
		/* we should read less than a chunk -> read a chunk */
		if (p_size < p_stream->m_buffer_size) {
			/* we should do an actual read on the media */
			p_stream->m_bytes_in_buffer = p_stream->m_read_fn(p_stream->m_stored_data,p_stream->m_buffer_size,p_stream->m_user_data);

			if (p_stream->m_bytes_in_buffer == (mi_SIZE_T)-1) {
				/* end of stream */
				mi_event_msg(p_event_mgr, EVT_INFO, "Stream reached its end !\n");

				p_stream->m_bytes_in_buffer = 0;
				p_stream->m_status |= mi_STREAM_STATUS_END;
				/* end of stream */
				return l_read_nb_bytes ? l_read_nb_bytes : (mi_SIZE_T)-1;
			}
			else if	(p_stream->m_bytes_in_buffer < p_size) {
				/* not enough data */
				l_read_nb_bytes += p_stream->m_bytes_in_buffer;
				memcpy(p_buffer,p_stream->m_current_data,p_stream->m_bytes_in_buffer);
				p_stream->m_current_data = p_stream->m_stored_data;
				p_buffer += p_stream->m_bytes_in_buffer;
				p_size -= p_stream->m_bytes_in_buffer;
				p_stream->m_byte_offset += (mi_OFF_T)p_stream->m_bytes_in_buffer;
				p_stream->m_bytes_in_buffer = 0;
			}
			else {
				l_read_nb_bytes += p_size;
				memcpy(p_buffer,p_stream->m_current_data,p_size);
				p_stream->m_current_data += p_size;
				p_stream->m_bytes_in_buffer -= p_size;
				p_stream->m_byte_offset += (mi_OFF_T)p_size;
				return l_read_nb_bytes;
			}
		}
		else {
			/* direct read on the dest buffer */
			p_stream->m_bytes_in_buffer = p_stream->m_read_fn(p_buffer,p_size,p_stream->m_user_data);

			if (p_stream->m_bytes_in_buffer == (mi_SIZE_T)-1) {
				/*  end of stream */
				mi_event_msg(p_event_mgr, EVT_INFO, "Stream reached its end !\n");

				p_stream->m_bytes_in_buffer = 0;
				p_stream->m_status |= mi_STREAM_STATUS_END;
				/* end of stream */
				return l_read_nb_bytes ? l_read_nb_bytes : (mi_SIZE_T)-1;
			}
			else if (p_stream->m_bytes_in_buffer < p_size) {
				/* not enough data */
				l_read_nb_bytes += p_stream->m_bytes_in_buffer;
				p_stream->m_current_data = p_stream->m_stored_data;
				p_buffer += p_stream->m_bytes_in_buffer;
				p_size -= p_stream->m_bytes_in_buffer;
				p_stream->m_byte_offset += (mi_OFF_T)p_stream->m_bytes_in_buffer;
				p_stream->m_bytes_in_buffer = 0;
			}
			else {
				/* we have read the exact size */
				l_read_nb_bytes += p_stream->m_bytes_in_buffer;
				p_stream->m_byte_offset += (mi_OFF_T)p_stream->m_bytes_in_buffer;
				p_stream->m_current_data = p_stream->m_stored_data;
				p_stream->m_bytes_in_buffer = 0;
				return l_read_nb_bytes;
			}
		}
	}
}

mi_SIZE_T mi_stream_write_data (mi_stream_private_t * p_stream,
								  const mi_BYTE * p_buffer,
								  mi_SIZE_T p_size, 
								  mi_event_mgr_t * p_event_mgr)
{
	mi_SIZE_T l_remaining_bytes = 0;
	mi_SIZE_T l_write_nb_bytes = 0;

	if (p_stream->m_status & mi_STREAM_STATUS_ERROR) {
		return (mi_SIZE_T)-1;
	}

	for (;;) {
		l_remaining_bytes = p_stream->m_buffer_size - p_stream->m_bytes_in_buffer;
		
		/* we have more memory than required */
		if (l_remaining_bytes >= p_size) {
			memcpy(p_stream->m_current_data, p_buffer, p_size);
			
			p_stream->m_current_data += p_size;
			p_stream->m_bytes_in_buffer += p_size;
			l_write_nb_bytes += p_size;
			p_stream->m_byte_offset += (mi_OFF_T)p_size;
			
			return l_write_nb_bytes;
		}

		/* we copy data and then do an actual read on the stream */
		if (l_remaining_bytes) {
			l_write_nb_bytes += l_remaining_bytes;
			
			memcpy(p_stream->m_current_data,p_buffer,l_remaining_bytes);
			
			p_stream->m_current_data = p_stream->m_stored_data;
			
			p_buffer += l_remaining_bytes;
			p_size -= l_remaining_bytes;
			p_stream->m_bytes_in_buffer += l_remaining_bytes;
			p_stream->m_byte_offset += (mi_OFF_T)l_remaining_bytes;
		}

		if (! mi_stream_flush(p_stream, p_event_mgr)) {
			return (mi_SIZE_T)-1;
		}
	}

}

mi_BOOL mi_stream_flush (mi_stream_private_t * p_stream, mi_event_mgr_t * p_event_mgr)
{
	/* the number of bytes written on the media. */
	mi_SIZE_T l_current_write_nb_bytes = 0;

	p_stream->m_current_data = p_stream->m_stored_data;

	while (p_stream->m_bytes_in_buffer) {
		/* we should do an actual write on the media */
		l_current_write_nb_bytes = p_stream->m_write_fn(p_stream->m_current_data,
														p_stream->m_bytes_in_buffer,
														p_stream->m_user_data);
		
		if (l_current_write_nb_bytes == (mi_SIZE_T)-1) {
			p_stream->m_status |= mi_STREAM_STATUS_ERROR;
			mi_event_msg(p_event_mgr, EVT_INFO, "Error on writing stream!\n");

			return mi_FALSE;
		}

		p_stream->m_current_data += l_current_write_nb_bytes;
		p_stream->m_bytes_in_buffer -= l_current_write_nb_bytes;
	}

	p_stream->m_current_data = p_stream->m_stored_data;
	
	return mi_TRUE;
}

mi_OFF_T mi_stream_read_skip (mi_stream_private_t * p_stream, mi_OFF_T p_size, mi_event_mgr_t * p_event_mgr)
{
	mi_OFF_T l_skip_nb_bytes = 0;
	mi_OFF_T l_current_skip_nb_bytes = 0;
	
	assert( p_size >= 0 );
	
	if (p_stream->m_bytes_in_buffer >= (mi_SIZE_T)p_size) {
		p_stream->m_current_data += p_size;
		/* it is safe to cast p_size to mi_SIZE_T since it is <= m_bytes_in_buffer
		which is of type mi_SIZE_T */
		p_stream->m_bytes_in_buffer -= (mi_SIZE_T)p_size;
		l_skip_nb_bytes += p_size;
		p_stream->m_byte_offset += l_skip_nb_bytes;
		return l_skip_nb_bytes;
	}

	/* we are now in the case when the remaining data if not sufficient */
	if (p_stream->m_status & mi_STREAM_STATUS_END) {
		l_skip_nb_bytes += (mi_OFF_T)p_stream->m_bytes_in_buffer;
		p_stream->m_current_data += p_stream->m_bytes_in_buffer;
		p_stream->m_bytes_in_buffer = 0;
		p_stream->m_byte_offset += l_skip_nb_bytes;
		return l_skip_nb_bytes ? l_skip_nb_bytes : (mi_OFF_T) -1;
	}

	/* the flag is not set, we copy data and then do an actual skip on the stream */
	if (p_stream->m_bytes_in_buffer) {
		l_skip_nb_bytes += (mi_OFF_T)p_stream->m_bytes_in_buffer;
		p_stream->m_current_data = p_stream->m_stored_data;
		p_size -= (mi_OFF_T)p_stream->m_bytes_in_buffer;
		p_stream->m_bytes_in_buffer = 0;
	}

	while (p_size > 0) {
		/* we should do an actual skip on the media */
		l_current_skip_nb_bytes = p_stream->m_skip_fn(p_size, p_stream->m_user_data);
		if (l_current_skip_nb_bytes == (mi_OFF_T) -1) {
			mi_event_msg(p_event_mgr, EVT_INFO, "Stream reached its end !\n");

			p_stream->m_status |= mi_STREAM_STATUS_END;
			p_stream->m_byte_offset += l_skip_nb_bytes;
			/* end if stream */
			return l_skip_nb_bytes ? l_skip_nb_bytes : (mi_OFF_T) -1;
		}
		p_size -= l_current_skip_nb_bytes;
		l_skip_nb_bytes += l_current_skip_nb_bytes;
	}

	p_stream->m_byte_offset += l_skip_nb_bytes;
	
	return l_skip_nb_bytes;
}

mi_OFF_T mi_stream_write_skip (mi_stream_private_t * p_stream, mi_OFF_T p_size, mi_event_mgr_t * p_event_mgr)
{
	mi_BOOL l_is_written = 0;
	mi_OFF_T l_current_skip_nb_bytes = 0;
	mi_OFF_T l_skip_nb_bytes = 0;

	if (p_stream->m_status & mi_STREAM_STATUS_ERROR) {
		return (mi_OFF_T) -1;
	}

	/* we should flush data */
	l_is_written = mi_stream_flush (p_stream, p_event_mgr);
	if (! l_is_written) {
		p_stream->m_status |= mi_STREAM_STATUS_ERROR;
		p_stream->m_bytes_in_buffer = 0;
		return (mi_OFF_T) -1;
	}
	/* then skip */

	while (p_size > 0) {
		/* we should do an actual skip on the media */
		l_current_skip_nb_bytes = p_stream->m_skip_fn(p_size, p_stream->m_user_data);
		
		if (l_current_skip_nb_bytes == (mi_OFF_T)-1) {
			mi_event_msg(p_event_mgr, EVT_INFO, "Stream error!\n");

			p_stream->m_status |= mi_STREAM_STATUS_ERROR;
			p_stream->m_byte_offset += l_skip_nb_bytes;
			/* end if stream */
			return l_skip_nb_bytes ? l_skip_nb_bytes : (mi_OFF_T)-1;
		}
		p_size -= l_current_skip_nb_bytes;
		l_skip_nb_bytes += l_current_skip_nb_bytes;
	}

	p_stream->m_byte_offset += l_skip_nb_bytes;
	
	return l_skip_nb_bytes;
}

mi_OFF_T mi_stream_tell (const mi_stream_private_t * p_stream)
{
	return p_stream->m_byte_offset;
}

mi_OFF_T mi_stream_get_number_byte_left (const mi_stream_private_t * p_stream)
{
  assert( p_stream->m_byte_offset >= 0 );
  assert( p_stream->m_user_data_length >= (mi_UINT64)p_stream->m_byte_offset);
  return p_stream->m_user_data_length ?
				(mi_OFF_T)(p_stream->m_user_data_length) - p_stream->m_byte_offset :
				0;
}

mi_OFF_T mi_stream_skip (mi_stream_private_t * p_stream, mi_OFF_T p_size, mi_event_mgr_t * p_event_mgr)
{
	assert(p_size >= 0);
	return p_stream->m_mi_skip(p_stream,p_size,p_event_mgr);
}

mi_BOOL mi_stream_read_seek (mi_stream_private_t * p_stream, mi_OFF_T p_size, mi_event_mgr_t * p_event_mgr)
{
	mi_ARG_NOT_USED(p_event_mgr);
	p_stream->m_current_data = p_stream->m_stored_data;
	p_stream->m_bytes_in_buffer = 0;

	if( !(p_stream->m_seek_fn(p_size,p_stream->m_user_data)) ) {
		p_stream->m_status |= mi_STREAM_STATUS_END;
		return mi_FALSE;
	}
	else {
		/* reset stream status */
		p_stream->m_status &= (~mi_STREAM_STATUS_END);
		p_stream->m_byte_offset = p_size;

	}

	return mi_TRUE;
}

mi_BOOL mi_stream_write_seek (mi_stream_private_t * p_stream, mi_OFF_T p_size, mi_event_mgr_t * p_event_mgr)
{
	if (! mi_stream_flush(p_stream,p_event_mgr)) {
		p_stream->m_status |= mi_STREAM_STATUS_ERROR;
		return mi_FALSE;
	}

	p_stream->m_current_data = p_stream->m_stored_data;
	p_stream->m_bytes_in_buffer = 0;

	if (! p_stream->m_seek_fn(p_size,p_stream->m_user_data)) {
		p_stream->m_status |= mi_STREAM_STATUS_ERROR;
		return mi_FALSE;
	}
	else {
		p_stream->m_byte_offset = p_size;
	}

	return mi_TRUE;
}

mi_BOOL mi_stream_seek (mi_stream_private_t * p_stream, mi_OFF_T p_size, struct mi_event_mgr * p_event_mgr)
{
	assert(p_size >= 0);
	return p_stream->m_mi_seek(p_stream,p_size,p_event_mgr);
}

mi_BOOL mi_stream_has_seek (const mi_stream_private_t * p_stream)
{
	return p_stream->m_seek_fn != mi_stream_default_seek;
}

mi_SIZE_T mi_stream_default_read (void * p_buffer, mi_SIZE_T p_nb_bytes, void * p_user_data)
{
	mi_ARG_NOT_USED(p_buffer);
	mi_ARG_NOT_USED(p_nb_bytes);
	mi_ARG_NOT_USED(p_user_data);
	return (mi_SIZE_T) -1;
}

mi_SIZE_T mi_stream_default_write (void * p_buffer, mi_SIZE_T p_nb_bytes, void * p_user_data)
{
	mi_ARG_NOT_USED(p_buffer);
	mi_ARG_NOT_USED(p_nb_bytes);
	mi_ARG_NOT_USED(p_user_data);
	return (mi_SIZE_T) -1;
}

mi_OFF_T mi_stream_default_skip (mi_OFF_T p_nb_bytes, void * p_user_data)
{
	mi_ARG_NOT_USED(p_nb_bytes);
	mi_ARG_NOT_USED(p_user_data);
	return (mi_OFF_T) -1;
}

mi_BOOL mi_stream_default_seek (mi_OFF_T p_nb_bytes, void * p_user_data)
{
	mi_ARG_NOT_USED(p_nb_bytes);
	mi_ARG_NOT_USED(p_user_data);
	return mi_FALSE;
}
