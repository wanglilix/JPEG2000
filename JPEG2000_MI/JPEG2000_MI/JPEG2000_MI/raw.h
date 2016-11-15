
#ifndef __RAW_H
#define __RAW_H
/**
@file raw.h
@brief Implementation of operations for raw encoding (RAW)

The functions in RAW.C have for goal to realize the operation of raw encoding linked
with the corresponding mode switch.
*/

/** @defgroup RAW RAW - Implementation of operations for raw encoding */
/*@{*/

/**
RAW encoding operations
*/
typedef struct mi_raw {
	/** temporary buffer where bits are coded or decoded */
	mi_BYTE c;
	/** number of bits already read or free to write */
	mi_UINT32 ct;
	/** maximum length to decode */
	mi_UINT32 lenmax;
	/** length decoded */
	mi_UINT32 len;
	/** pointer to the current position in the buffer */
	mi_BYTE *bp;
	/** pointer to the start of the buffer */
	mi_BYTE *start;
	/** pointer to the end of the buffer */
	mi_BYTE *end;
} mi_raw_t;

/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */
/**
Create a new RAW handle 
@return Returns a new RAW handle if successful, returns NULL otherwise
*/
mi_raw_t* mi_raw_create(void);
/**
Destroy a previously created RAW handle
@param raw RAW handle to destroy
*/
void mi_raw_destroy(mi_raw_t *raw);
/**
Return the number of bytes written/read since initialisation
@param raw RAW handle to destroy
@return Returns the number of bytes already encoded
*/
mi_UINT32 mi_raw_numbytes(mi_raw_t *raw);
/**
Initialize the decoder
@param raw RAW handle
@param bp Pointer to the start of the buffer from which the bytes will be read
@param len Length of the input buffer
*/
void mi_raw_init_dec(mi_raw_t *raw, mi_BYTE *bp, mi_UINT32 len);
/**
Decode a symbol using raw-decoder. Cfr p.506 TAUBMAN
@param raw RAW handle
@return Returns the decoded symbol (0 or 1)
*/
mi_UINT32 mi_raw_decode(mi_raw_t *raw);
/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/

#endif /* __RAW_H */
