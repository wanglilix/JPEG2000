
#ifndef __mi_MALLOC_H
#define __mi_MALLOC_H

#include <stddef.h>
/**
@file mi_malloc.h
@brief Internal functions

The functions in mi_malloc.h are internal utilities used for memory management.
*/

/** @defgroup MISC MISC - Miscellaneous internal functions */
/*@{*/

/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */

/**
Allocate an uninitialized memory block
@param size Bytes to allocate
@return Returns a void pointer to the allocated space, or NULL if there is insufficient memory available
*/
void * mi_malloc(size_t size);

/**
Allocate a memory block with elements initialized to 0
@param num Blocks to allocate
@param size Bytes per block to allocate
@return Returns a void pointer to the allocated space, or NULL if there is insufficient memory available
*/
void * mi_calloc(size_t numOfElements, size_t sizeOfElements);

/**
Allocate memory aligned to a 16 byte boundary
@param size Bytes to allocate
@return Returns a void pointer to the allocated space, or NULL if there is insufficient memory available
*/
void * mi_aligned_malloc(size_t size);
void * mi_aligned_realloc(void *ptr, size_t size);
void mi_aligned_free(void* ptr);

/**
Reallocate memory blocks.
@param m Pointer to previously allocated memory block
@param s New size in bytes
@return Returns a void pointer to the reallocated (and possibly moved) memory block
*/
void * mi_realloc(void * m, size_t s);

/**
Deallocates or frees a memory block.
@param m Previously allocated memory block to be freed
*/
void mi_free(void * m);

#if defined(__GNUC__) && !defined(mi_SKIP_POISON)
#pragma GCC poison malloc calloc realloc free
#endif

/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/

#endif /* __mi_MALLOC_H */

