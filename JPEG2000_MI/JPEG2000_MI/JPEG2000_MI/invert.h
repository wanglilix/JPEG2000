
#ifndef __INVERT_H
#define __INVERT_H
#include"openjpeg.h"
/**
@file invert.h
@brief Implementation of the matrix inversion

The function in INVERT.H compute a matrix inversion with a LUP method
*/

/** @defgroup INVERT INVERT - Implementation of a matrix inversion */
/*@{*/
/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */

/**
 * Calculates a n x n double matrix inversion with a LUP method. Data is aligned, rows after rows (or columns after columns).
 * The function does not take ownership of any memory block, data must be fred by the user.
 *
 * @param pSrcMatrix	the matrix to invert.
 * @param pDestMatrix	data to store the inverted matrix. 
 * @param n size of the matrix
 * @return mi_TRUE if the inversion is successful, mi_FALSE if the matrix is singular.
 */
mi_BOOL mi_matrix_inversion_f(mi_FLOAT32 * pSrcMatrix,
                                mi_FLOAT32 * pDestMatrix, 
                                mi_UINT32 nb_compo);
/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/

#endif /* __INVERT_H */ 
