
#ifndef __DWT_H
#define __DWT_H
#include"openjpeg.h"
#include"tcd.h"
#include"j2k.h"
#include"mi_includes.h"
/**
@file dwt.h
@brief Implementation of a discrete wavelet transform (DWT)

The functions in DWT.C have for goal to realize forward and inverse discret wavelet
transform with filter 5-3 (reversible) and filter 9-7 (irreversible). The functions in
DWT.C are used by some function in TCD.C.
*/

/** @defgroup DWT DWT - Implementation of a discrete wavelet transform */
/*@{*/


/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */
/**
Forward 5-3 wavelet transform in 2-D.
Apply a reversible DWT transform to a component of an image.
@param tilec Tile component information (current tile)
*/
mi_BOOL mi_dwt_encode(mi_tcd_tilecomp_t * tilec);

/**
Inverse 5-3 wavelet transform in 2-D.
Apply a reversible inverse DWT transform to a component of an image.
@param tilec Tile component information (current tile)
@param numres Number of resolution levels to decode
*/
mi_BOOL mi_dwt_decode(mi_tcd_tilecomp_t* tilec, mi_UINT32 numres);

/**
Get the gain of a subband for the reversible 5-3 DWT.
@param orient Number that identifies the subband (0->LL, 1->HL, 2->LH, 3->HH)
@return Returns 0 if orient = 0, returns 1 if orient = 1 or 2, returns 2 otherwise
*/
mi_UINT32 mi_dwt_getgain(mi_UINT32 orient) ;
/**
Get the norm of a wavelet function of a subband at a specified level for the reversible 5-3 DWT.
@param level Level of the wavelet function
@param orient Band of the wavelet function
@return Returns the norm of the wavelet function
*/
mi_FLOAT64 mi_dwt_getnorm(mi_UINT32 level, mi_UINT32 orient);
/**
Forward 9-7 wavelet transform in 2-D. 
Apply an irreversible DWT transform to a component of an image.
@param tilec Tile component information (current tile)
*/
mi_BOOL mi_dwt_encode_real(mi_tcd_tilecomp_t * tilec);
/**
Inverse 9-7 wavelet transform in 2-D. 
Apply an irreversible inverse DWT transform to a component of an image.
@param tilec Tile component information (current tile)
@param numres Number of resolution levels to decode
*/
mi_BOOL mi_dwt_decode_real(mi_tcd_tilecomp_t* restrict tilec, mi_UINT32 numres);

/**
Get the gain of a subband for the irreversible 9-7 DWT.
@param orient Number that identifies the subband (0->LL, 1->HL, 2->LH, 3->HH)
@return Returns the gain of the 9-7 wavelet transform
*/
mi_UINT32 mi_dwt_getgain_real(mi_UINT32 orient);
/**
Get the norm of a wavelet function of a subband at a specified level for the irreversible 9-7 DWT
@param level Level of the wavelet function
@param orient Band of the wavelet function
@return Returns the norm of the 9-7 wavelet
*/
mi_FLOAT64 mi_dwt_getnorm_real(mi_UINT32 level, mi_UINT32 orient);
/**
Explicit calculation of the Quantization Stepsizes 
@param tccp Tile-component coding parameters
@param prec Precint analyzed
*/
void mi_dwt_calc_explicit_stepsizes(mi_tccp_t * tccp, mi_UINT32 prec);
/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/

#endif /* __DWT_H */
