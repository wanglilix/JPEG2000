
#include "mi_includes.h"

/* <summary> */
/* This table contains the norms of the basis function of the reversible MCT. */
/* </summary> */
static const mi_FLOAT64 mi_mct_norms[3] = { 1.732, .8292, .8292 };

/* <summary> */
/* This table contains the norms of the basis function of the irreversible MCT. */
/* </summary> */
static const mi_FLOAT64 mi_mct_norms_real[3] = { 1.732, 1.805, 1.573 };

const mi_FLOAT64 * mi_mct_get_mct_norms ()
{
	return mi_mct_norms;
}

const mi_FLOAT64 * mi_mct_get_mct_norms_real ()
{
	return mi_mct_norms_real;
}

/* <summary> */
/* Forward reversible MCT. */
/* </summary> */
//多分量变换函数（RCT）
void mi_mct_encode(
		mi_INT32* restrict c0,
		mi_INT32* restrict c1,
		mi_INT32* restrict c2,
		mi_UINT32 n)
{
	mi_SIZE_T i;
	const mi_SIZE_T len = n;
	
	for(i = 0; i < len; ++i) {
		mi_INT32 r = c0[i];
		mi_INT32 g = c1[i];
		mi_INT32 b = c2[i];
		mi_INT32 y = (r + (g * 2) + b) >> 2;
		mi_INT32 u = b - g;
		mi_INT32 v = r - g;
		c0[i] = y;
		c1[i] = u;
		c2[i] = v;
	}
}

/* <summary> */
/* Inverse reversible MCT. */
/* </summary> */
void mi_mct_decode(
		mi_INT32* restrict c0,
		mi_INT32* restrict c1, 
		mi_INT32* restrict c2, 
		mi_UINT32 n)
{
	mi_UINT32 i;
	for (i = 0; i < n; ++i) {
		mi_INT32 y = c0[i];
		mi_INT32 u = c1[i];
		mi_INT32 v = c2[i];
		mi_INT32 g = y - ((u + v) >> 2);
		mi_INT32 r = v + g;
		mi_INT32 b = u + g;
		c0[i] = r;
		c1[i] = g;
		c2[i] = b;
	}
}

/* <summary> */
/* Get norm of basis function of reversible MCT. */
/* </summary> */
mi_FLOAT64 mi_mct_getnorm(mi_UINT32 compno) {
	return mi_mct_norms[compno];
}

/* <summary> */
/* Forward irreversible MCT. */
/* </summary> */
//多分变量变换函数（ICT）
void mi_mct_encode_real(
		mi_INT32* restrict c0,
		mi_INT32* restrict c1,
		mi_INT32* restrict c2,
		mi_UINT32 n)
{
	mi_UINT32 i;
	for(i = 0; i < n; ++i) {
		mi_INT32 r = c0[i];
		mi_INT32 g = c1[i];
		mi_INT32 b = c2[i];
		mi_INT32 y =  mi_int_fix_mul(r, 2449) + mi_int_fix_mul(g, 4809) + mi_int_fix_mul(b, 934);
		mi_INT32 u = -mi_int_fix_mul(r, 1382) - mi_int_fix_mul(g, 2714) + mi_int_fix_mul(b, 4096);
		mi_INT32 v =  mi_int_fix_mul(r, 4096) - mi_int_fix_mul(g, 3430) - mi_int_fix_mul(b, 666);
		c0[i] = y;
		c1[i] = u;
		c2[i] = v;
	}
}

/* <summary> */
/* Inverse irreversible MCT. */
/* </summary> */
void mi_mct_decode_real(
		mi_FLOAT32* restrict c0,
		mi_FLOAT32* restrict c1,
		mi_FLOAT32* restrict c2,
		mi_UINT32 n)
{
	mi_UINT32 i;
	for(i = 0; i < n; ++i) {
		mi_FLOAT32 y = c0[i];
		mi_FLOAT32 u = c1[i];
		mi_FLOAT32 v = c2[i];
		mi_FLOAT32 r = y + (v * 1.402f);
		mi_FLOAT32 g = y - (u * 0.34413f) - (v * (0.71414f));
		mi_FLOAT32 b = y + (u * 1.772f);
		c0[i] = r;
		c1[i] = g;
		c2[i] = b;
	}
}

/* <summary> */
/* Get norm of basis function of irreversible MCT. */
/* </summary> */
mi_FLOAT64 mi_mct_getnorm_real(mi_UINT32 compno) {
	return mi_mct_norms_real[compno];
}

//多分量变换函数（我猜是作者留给用户的定制函数，暂时用不到）
mi_BOOL mi_mct_encode_custom(
					   mi_BYTE * pCodingdata,
					   mi_UINT32 n,
					   mi_BYTE ** pData,
					   mi_UINT32 pNbComp,
					   mi_UINT32 isSigned)
{
	mi_FLOAT32 * lMct = (mi_FLOAT32 *) pCodingdata;
	mi_UINT32 i;
	mi_UINT32 j;
	mi_UINT32 k;
	mi_UINT32 lNbMatCoeff = pNbComp * pNbComp;
	mi_INT32 * lCurrentData = 00;
	mi_INT32 * lCurrentMatrix = 00;
	mi_INT32 ** lData = (mi_INT32 **) pData;
	mi_UINT32 lMultiplicator = 1 << 13;
	mi_INT32 * lMctPtr;

    mi_ARG_NOT_USED(isSigned);

	lCurrentData = (mi_INT32 *) mi_malloc((pNbComp + lNbMatCoeff) * sizeof(mi_INT32));
	if (! lCurrentData) {
		return mi_FALSE;
	}

	lCurrentMatrix = lCurrentData + pNbComp;

	for (i =0;i<lNbMatCoeff;++i) {
		lCurrentMatrix[i] = (mi_INT32) (*(lMct++) * (mi_FLOAT32)lMultiplicator);
	}

	for (i = 0; i < n; ++i)  {
		lMctPtr = lCurrentMatrix;
		for (j=0;j<pNbComp;++j) {
			lCurrentData[j] = (*(lData[j]));
		}

		for (j=0;j<pNbComp;++j) {
			*(lData[j]) = 0;
			for (k=0;k<pNbComp;++k) {
				*(lData[j]) += mi_int_fix_mul(*lMctPtr, lCurrentData[k]);
				++lMctPtr;
			}

			++lData[j];
		}
	}

	mi_free(lCurrentData);

	return mi_TRUE;
}

mi_BOOL mi_mct_decode_custom(
					   mi_BYTE * pDecodingData,
					   mi_UINT32 n,
					   mi_BYTE ** pData,
					   mi_UINT32 pNbComp,
					   mi_UINT32 isSigned)
{
	mi_FLOAT32 * lMct;
	mi_UINT32 i;
	mi_UINT32 j;
	mi_UINT32 k;

	mi_FLOAT32 * lCurrentData = 00;
	mi_FLOAT32 * lCurrentResult = 00;
	mi_FLOAT32 ** lData = (mi_FLOAT32 **) pData;

    mi_ARG_NOT_USED(isSigned);

	lCurrentData = (mi_FLOAT32 *) mi_malloc (2 * pNbComp * sizeof(mi_FLOAT32));
	if (! lCurrentData) {
		return mi_FALSE;
	}
	lCurrentResult = lCurrentData + pNbComp;

	for (i = 0; i < n; ++i) {
		lMct = (mi_FLOAT32 *) pDecodingData;
		for (j=0;j<pNbComp;++j) {
			lCurrentData[j] = (mi_FLOAT32) (*(lData[j]));
		}
		for (j=0;j<pNbComp;++j) {
			lCurrentResult[j] = 0;
			for	(k=0;k<pNbComp;++k)	{
				lCurrentResult[j] += *(lMct++) * lCurrentData[k];
			}
			*(lData[j]++) = (mi_FLOAT32) (lCurrentResult[j]);
		}
	}
	mi_free(lCurrentData);
	return mi_TRUE;
}

void mi_calculate_norms(	mi_FLOAT64 * pNorms,
							mi_UINT32 pNbComps,
							mi_FLOAT32 * pMatrix)
{
	mi_UINT32 i,j,lIndex;
	mi_FLOAT32 lCurrentValue;
	mi_FLOAT64 * lNorms = (mi_FLOAT64 *) pNorms;
	mi_FLOAT32 * lMatrix = (mi_FLOAT32 *) pMatrix;

	for	(i=0;i<pNbComps;++i) {
		lNorms[i] = 0;
		lIndex = i;

		for	(j=0;j<pNbComps;++j) {
			lCurrentValue = lMatrix[lIndex];
			lIndex += pNbComps;
			lNorms[i] += lCurrentValue * lCurrentValue;
		}
		lNorms[i] = sqrt(lNorms[i]);
	}
}
