
#ifndef __BIO_H
#define __BIO_H

#include <stddef.h> /* ptrdiff_t */
#include"openjpeg.h"
/** 
@file bio.h
@brief Implementation of an individual bit input-output (BIO)

The functions in BIO.C have for goal to realize an individual bit input - output.
*/

/** @defgroup BIO BIO - Individual bit input-output stream */
/*@{*/

/**
Individual bit input-output stream (BIO)
*/
typedef struct mi_bio {
	/** pointer to the start of the buffer */
	mi_BYTE *start;
	/** pointer to the end of the buffer */
	mi_BYTE *end;
	/** pointer to the present position in the buffer */
	mi_BYTE *bp;
	/** temporary place where each byte is read or written */
	mi_UINT32 buf;
	/** coder : number of bits free to write. decoder : number of bits read */
	mi_UINT32 ct;
} mi_bio_t;

/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */
/**
Create a new BIO handle 
@return Returns a new BIO handle if successful, returns NULL otherwise
*/
mi_bio_t* mi_bio_create(void);
/**
Destroy a previously created BIO handle
@param bio BIO handle to destroy
*/
void mi_bio_destroy(mi_bio_t *bio);
/**
Number of bytes written.
@param bio BIO handle
@return Returns the number of bytes written
*/
ptrdiff_t mi_bio_numbytes(mi_bio_t *bio);
/**
Init encoder
@param bio BIO handle
@param bp Output buffer
@param len Output buffer length 
*/
void mi_bio_init_enc(mi_bio_t *bio, mi_BYTE *bp, mi_UINT32 len);
/**
Init decoder
@param bio BIO handle
@param bp Input buffer
@param len Input buffer length 
*/
void mi_bio_init_dec(mi_bio_t *bio, mi_BYTE *bp, mi_UINT32 len);
/**
Write bits
@param bio BIO handle
@param v Value of bits
@param n Number of bits to write
*/
void mi_bio_write(mi_bio_t *bio, mi_UINT32 v, mi_UINT32 n);
/**
Read bits
@param bio BIO handle
@param n Number of bits to read 
@return Returns the corresponding read number
*/
mi_UINT32 mi_bio_read(mi_bio_t *bio, mi_UINT32 n);
/**
Flush bits
@param bio BIO handle
@return Returns mi_TRUE if successful, returns mi_FALSE otherwise
*/
mi_BOOL mi_bio_flush(mi_bio_t *bio);
/**
Passes the ending bits (coming from flushing)
@param bio BIO handle
@return Returns mi_TRUE if successful, returns mi_FALSE otherwise
*/
mi_BOOL mi_bio_inalign(mi_bio_t *bio);
/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/

#endif /* __BIO_H */

