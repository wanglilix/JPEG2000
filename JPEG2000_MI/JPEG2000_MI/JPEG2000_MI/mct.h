
#ifndef __MCT_H
#define __MCT_H
#include"openjpeg.h"
/**
@file mct.h
@brief Implementation of a multi-component transforms (MCT)

The functions in MCT.C have for goal to realize reversible and irreversible multicomponent
transform. The functions in MCT.C are used by some function in TCD.C.
*/

/** @defgroup MCT MCT - Implementation of a multi-component transform */
/*@{*/

/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */
/**
Apply a reversible multi-component transform to an image
@param c0 Samples for red component
@param c1 Samples for green component
@param c2 Samples blue component
@param n Number of samples for each component
*/
void mi_mct_encode(mi_INT32 *c0, mi_INT32 *c1, mi_INT32 *c2, mi_UINT32 n);
/**
Apply a reversible multi-component inverse transform to an image
@param c0 Samples for luminance component
@param c1 Samples for red chrominance component
@param c2 Samples for blue chrominance component
@param n Number of samples for each component
*/
void mi_mct_decode(mi_INT32 *c0, mi_INT32 *c1, mi_INT32 *c2, mi_UINT32 n);
/**
Get norm of the basis function used for the reversible multi-component transform
@param compno Number of the component (0->Y, 1->U, 2->V)
@return 
*/
mi_FLOAT64 mi_mct_getnorm(mi_UINT32 compno);

/**
Apply an irreversible multi-component transform to an image
@param c0 Samples for red component
@param c1 Samples for green component
@param c2 Samples blue component
@param n Number of samples for each component
*/
void mi_mct_encode_real(mi_INT32 *c0, mi_INT32 *c1, mi_INT32 *c2, mi_UINT32 n);
/**
Apply an irreversible multi-component inverse transform to an image
@param c0 Samples for luminance component
@param c1 Samples for red chrominance component
@param c2 Samples for blue chrominance component
@param n Number of samples for each component
*/
void mi_mct_decode_real(mi_FLOAT32* c0, mi_FLOAT32* c1, mi_FLOAT32* c2, mi_UINT32 n);
/**
Get norm of the basis function used for the irreversible multi-component transform
@param compno Number of the component (0->Y, 1->U, 2->V)
@return 
*/
mi_FLOAT64 mi_mct_getnorm_real(mi_UINT32 compno);

/**
FIXME DOC
@param p_coding_data    MCT data
@param n                size of components
@param p_data           components
@param p_nb_comp        nb of components (i.e. size of p_data)
@param is_signed        tells if the data is signed
@return mi_FALSE if function encounter a problem, mi_TRUE otherwise
*/
mi_BOOL mi_mct_encode_custom(
					   mi_BYTE * p_coding_data,
					   mi_UINT32 n,
					   mi_BYTE ** p_data,
					   mi_UINT32 p_nb_comp,
					   mi_UINT32 is_signed);
/**
FIXME DOC
@param pDecodingData    MCT data
@param n                size of components
@param pData            components
@param pNbComp          nb of components (i.e. size of p_data)
@param isSigned         tells if the data is signed
@return mi_FALSE if function encounter a problem, mi_TRUE otherwise
*/
mi_BOOL mi_mct_decode_custom(
					   mi_BYTE * pDecodingData,
					   mi_UINT32 n,
					   mi_BYTE ** pData,
					   mi_UINT32 pNbComp,
					   mi_UINT32 isSigned);
/**
FIXME DOC
@param pNorms           MCT data
@param p_nb_comps       size of components
@param pMatrix          components
@return 
*/
void mi_calculate_norms(   mi_FLOAT64 * pNorms,
                            mi_UINT32 p_nb_comps,
                            mi_FLOAT32 * pMatrix);
/**
FIXME DOC 
*/
const mi_FLOAT64 * mi_mct_get_mct_norms (void);
/**
FIXME DOC 
*/
const mi_FLOAT64 * mi_mct_get_mct_norms_real (void);
/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/

#endif /* __MCT_H */
