
#include "mi_includes.h"

/** @defgroup BIO BIO - Individual bit input-output stream */
/*@{*/

/** @name Local static functions */
/*@{*/

/**
Write a bit
@param bio BIO handle
@param b Bit to write (0 or 1)
*/
static void mi_bio_putbit(mi_bio_t *bio, mi_UINT32 b);
/**
Read a bit
@param bio BIO handle
@return Returns the read bit
*/
static mi_UINT32 mi_bio_getbit(mi_bio_t *bio);
/**
Write a byte
@param bio BIO handle
@return Returns mi_TRUE if successful, returns mi_FALSE otherwise
*/
static mi_BOOL mi_bio_byteout(mi_bio_t *bio);
/**
Read a byte
@param bio BIO handle
@return Returns mi_TRUE if successful, returns mi_FALSE otherwise
*/
static mi_BOOL mi_bio_bytein(mi_bio_t *bio);

/*@}*/

/*@}*/

/* 
==========================================================
   local functions
==========================================================
*/

static mi_BOOL mi_bio_byteout(mi_bio_t *bio) {
	bio->buf = (bio->buf << 8) & 0xffff;
	bio->ct = bio->buf == 0xff00 ? 7 : 8;
	if ((mi_SIZE_T)bio->bp >= (mi_SIZE_T)bio->end) {
		return mi_FALSE;
	}
	*bio->bp++ = (mi_BYTE)(bio->buf >> 8);
	return mi_TRUE;
}

static mi_BOOL mi_bio_bytein(mi_bio_t *bio) {
	bio->buf = (bio->buf << 8) & 0xffff;
	bio->ct = bio->buf == 0xff00 ? 7 : 8;
	if ((mi_SIZE_T)bio->bp >= (mi_SIZE_T)bio->end) {
		return mi_FALSE;
	}
	bio->buf |= *bio->bp++;
	return mi_TRUE;
}

static void mi_bio_putbit(mi_bio_t *bio, mi_UINT32 b) {
	if (bio->ct == 0) {
		mi_bio_byteout(bio); /* MSD: why not check the return value of this function ? */
	}
	bio->ct--;
	bio->buf |= b << bio->ct;
}

static mi_UINT32 mi_bio_getbit(mi_bio_t *bio) {
	if (bio->ct == 0) {
		mi_bio_bytein(bio); /* MSD: why not check the return value of this function ? */
	}
	bio->ct--;
	return (bio->buf >> bio->ct) & 1;
}

/* 
==========================================================
   Bit Input/Output interface
==========================================================
*/

mi_bio_t* mi_bio_create(void) {
	mi_bio_t *bio = (mi_bio_t*)mi_malloc(sizeof(mi_bio_t));
	return bio;
}

void mi_bio_destroy(mi_bio_t *bio) {
	if(bio) {
		mi_free(bio);
	}
}

ptrdiff_t mi_bio_numbytes(mi_bio_t *bio) {
	return (bio->bp - bio->start);
}

void mi_bio_init_enc(mi_bio_t *bio, mi_BYTE *bp, mi_UINT32 len) {
	bio->start = bp;
	bio->end = bp + len;
	bio->bp = bp;
	bio->buf = 0;
	bio->ct = 8;
}

void mi_bio_init_dec(mi_bio_t *bio, mi_BYTE *bp, mi_UINT32 len) {
	bio->start = bp;
	bio->end = bp + len;
	bio->bp = bp;
	bio->buf = 0;
	bio->ct = 0;
}

mi_NOSANITIZE("unsigned-integer-overflow")
void mi_bio_write(mi_bio_t *bio, mi_UINT32 v, mi_UINT32 n) {
	mi_UINT32 i;
	
	assert((n > 0U) && (n <= 32U));
	for (i = n - 1; i < n; i--) { /* overflow used for end-loop condition */
		mi_bio_putbit(bio, (v >> i) & 1);
	}
}

mi_NOSANITIZE("unsigned-integer-overflow")
mi_UINT32 mi_bio_read(mi_bio_t *bio, mi_UINT32 n) {
	mi_UINT32 i;
	mi_UINT32 v;
	
	assert((n > 0U) /* && (n <= 32U)*/);
	v = 0U;
	for (i = n - 1; i < n; i--) { /* overflow used for end-loop condition */
		v |= mi_bio_getbit(bio) << i; /* can't overflow, mi_bio_getbit returns 0 or 1 */
	}
	return v;
}

mi_BOOL mi_bio_flush(mi_bio_t *bio) {
	if (! mi_bio_byteout(bio)) {
		return mi_FALSE;
	}
	if (bio->ct == 7) {
		if (! mi_bio_byteout(bio)) {
			return mi_FALSE;
		}
	}
	return mi_TRUE;
}

mi_BOOL mi_bio_inalign(mi_bio_t *bio) {
	if ((bio->buf & 0xff) == 0xff) {
		if (! mi_bio_bytein(bio)) {
			return mi_FALSE;
		}
	}
	bio->ct = 0;
	return mi_TRUE;
}
