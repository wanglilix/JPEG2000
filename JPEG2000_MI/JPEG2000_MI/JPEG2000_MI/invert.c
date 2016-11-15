
#include "mi_includes.h"

/** 
 * LUP decomposition
 */
static mi_BOOL mi_lupDecompose(mi_FLOAT32 * matrix,
                                 mi_UINT32 * permutations, 
                                 mi_FLOAT32 * p_swap_area,
                                 mi_UINT32 nb_compo);
/** 
 * LUP solving
 */
static void mi_lupSolve(mi_FLOAT32 * pResult, 
                         mi_FLOAT32* pMatrix, 
                         mi_FLOAT32* pVector, 
                         mi_UINT32* pPermutations, 
                         mi_UINT32 nb_compo,
                         mi_FLOAT32 * p_intermediate_data);

/** 
 *LUP inversion (call with the result of lupDecompose)
 */
static void mi_lupInvert ( mi_FLOAT32 * pSrcMatrix,
                            mi_FLOAT32 * pDestMatrix,
                            mi_UINT32 nb_compo,
                            mi_UINT32 * pPermutations,
                            mi_FLOAT32 * p_src_temp,
                            mi_FLOAT32 * p_dest_temp,
                            mi_FLOAT32 * p_swap_area);

/*
==========================================================
   Matric inversion interface
==========================================================
*/
/**
 * Matrix inversion.
 */
mi_BOOL mi_matrix_inversion_f(mi_FLOAT32 * pSrcMatrix,
                                mi_FLOAT32 * pDestMatrix, 
                                mi_UINT32 nb_compo)
{
	mi_BYTE * l_data = 00;
	mi_UINT32 l_permutation_size = nb_compo * (mi_UINT32)sizeof(mi_UINT32);
	mi_UINT32 l_swap_size = nb_compo * (mi_UINT32)sizeof(mi_FLOAT32);
	mi_UINT32 l_total_size = l_permutation_size + 3 * l_swap_size;
	mi_UINT32 * lPermutations = 00;
	mi_FLOAT32 * l_double_data = 00;

	l_data = (mi_BYTE *) mi_malloc(l_total_size);
	if (l_data == 0) {
		return mi_FALSE;
	}
	lPermutations = (mi_UINT32 *) l_data;
	l_double_data = (mi_FLOAT32 *) (l_data + l_permutation_size);
	memset(lPermutations,0,l_permutation_size);

	if(! mi_lupDecompose(pSrcMatrix,lPermutations,l_double_data,nb_compo)) {
		mi_free(l_data);
		return mi_FALSE;
	}
	
    mi_lupInvert(pSrcMatrix,pDestMatrix,nb_compo,lPermutations,l_double_data,l_double_data + nb_compo,l_double_data + 2*nb_compo);
	mi_free(l_data);
	
    return mi_TRUE;
}


/*
==========================================================
   Local functions（矩阵求逆——LUP分解法）
==========================================================
*/
static mi_BOOL mi_lupDecompose(mi_FLOAT32 * matrix,mi_UINT32 * permutations,
                          mi_FLOAT32 * p_swap_area,
                          mi_UINT32 nb_compo) 
{
	mi_UINT32 * tmpPermutations = permutations;
	mi_UINT32 * dstPermutations;
	mi_UINT32 k2=0,t;
	mi_FLOAT32 temp;
	mi_UINT32 i,j,k;
	mi_FLOAT32 p;
	mi_UINT32 lLastColum = nb_compo - 1;
	mi_UINT32 lSwapSize = nb_compo * (mi_UINT32)sizeof(mi_FLOAT32);
	mi_FLOAT32 * lTmpMatrix = matrix;
	mi_FLOAT32 * lColumnMatrix,* lDestMatrix;
	mi_UINT32 offset = 1;
	mi_UINT32 lStride = nb_compo-1;

	/*initialize permutations */
	for (i = 0; i < nb_compo; ++i) 
	{
    	*tmpPermutations++ = i;
	}
	/* now make a pivot with column switch */
	tmpPermutations = permutations;
	for (k = 0; k < lLastColum; ++k) {
		p = 0.0;

		/* take the middle element */
		lColumnMatrix = lTmpMatrix + k;
		
		/* make permutation with the biggest value in the column */
        for (i = k; i < nb_compo; ++i) {
			temp = ((*lColumnMatrix > 0) ? *lColumnMatrix : -(*lColumnMatrix));
     		if (temp > p) {
     			p = temp;
     			k2 = i;
     		}
			/* next line */
			lColumnMatrix += nb_compo;
     	}

     	/* a whole rest of 0 -> non singular */
     	if (p == 0.0) {
    		return mi_FALSE;
		}

		/* should we permute ? */
		if (k2 != k) {
			/*exchange of line */
     		/* k2 > k */
			dstPermutations = tmpPermutations + k2 - k;
			/* swap indices */
			t = *tmpPermutations;
     		*tmpPermutations = *dstPermutations;
     		*dstPermutations = t;

			/* and swap entire line. */
			lColumnMatrix = lTmpMatrix + (k2 - k) * nb_compo;
			memcpy(p_swap_area,lColumnMatrix,lSwapSize);
			memcpy(lColumnMatrix,lTmpMatrix,lSwapSize);
			memcpy(lTmpMatrix,p_swap_area,lSwapSize);
		}

		/* now update data in the rest of the line and line after */
		lDestMatrix = lTmpMatrix + k;
		lColumnMatrix = lDestMatrix + nb_compo;
		/* take the middle element */
		temp = *(lDestMatrix++);

		/* now compute up data (i.e. coeff up of the diagonal). */
     	for (i = offset; i < nb_compo; ++i)  {
			/*lColumnMatrix; */
			/* divide the lower column elements by the diagonal value */

			/* matrix[i][k] /= matrix[k][k]; */
     		/* p = matrix[i][k] */
			p = *lColumnMatrix / temp;
			*(lColumnMatrix++) = p;
     		
            for (j = /* k + 1 */ offset; j < nb_compo; ++j) {
				/* matrix[i][j] -= matrix[i][k] * matrix[k][j]; */
     			*(lColumnMatrix++) -= p * (*(lDestMatrix++));
			}
			/* come back to the k+1th element */
			lDestMatrix -= lStride;
			/* go to kth element of the next line */
			lColumnMatrix += k;
     	}

		/* offset is now k+2 */
		++offset;
		/* 1 element less for stride */
		--lStride;
		/* next line */
		lTmpMatrix+=nb_compo;
		/* next permutation element */
		++tmpPermutations;
	}
    return mi_TRUE;
}
 //应该也是LUP分解法一部分  		
static void mi_lupSolve (mi_FLOAT32 * pResult,
                   mi_FLOAT32 * pMatrix, 
                   mi_FLOAT32 * pVector, 
                   mi_UINT32* pPermutations, 
                   mi_UINT32 nb_compo,mi_FLOAT32 * p_intermediate_data) 
{
	mi_INT32 k;
    mi_UINT32 i,j;
	mi_FLOAT32 sum;
	mi_FLOAT32 u;
    mi_UINT32 lStride = nb_compo+1;
	mi_FLOAT32 * lCurrentPtr;
	mi_FLOAT32 * lIntermediatePtr;
	mi_FLOAT32 * lDestPtr;
	mi_FLOAT32 * lTmpMatrix;
	mi_FLOAT32 * lLineMatrix = pMatrix;
	mi_FLOAT32 * lBeginPtr = pResult + nb_compo - 1;
	mi_FLOAT32 * lGeneratedData;
	mi_UINT32 * lCurrentPermutationPtr = pPermutations;

	
	lIntermediatePtr = p_intermediate_data;
	lGeneratedData = p_intermediate_data + nb_compo - 1;
	
    for (i = 0; i < nb_compo; ++i) {
       	sum = 0.0;
		lCurrentPtr = p_intermediate_data;
		lTmpMatrix = lLineMatrix;
        for (j = 1; j <= i; ++j) 
		{
			/* sum += matrix[i][j-1] * y[j-1]; */
        	sum += (*(lTmpMatrix++)) * (*(lCurrentPtr++));
        }
		/*y[i] = pVector[pPermutations[i]] - sum; */
        *(lIntermediatePtr++) = pVector[*(lCurrentPermutationPtr++)] - sum;
		lLineMatrix += nb_compo;
	}

	/* we take the last point of the matrix */
	lLineMatrix = pMatrix + nb_compo*nb_compo - 1;

	/* and we take after the last point of the destination vector */
	lDestPtr = pResult + nb_compo;


    assert(nb_compo != 0);
	for (k = (mi_INT32)nb_compo - 1; k != -1 ; --k) {
		sum = 0.0;
		lTmpMatrix = lLineMatrix;
        u = *(lTmpMatrix++);
		lCurrentPtr = lDestPtr--;
        for (j = (mi_UINT32)(k + 1); j < nb_compo; ++j) {
			/* sum += matrix[k][j] * x[j] */
        	sum += (*(lTmpMatrix++)) * (*(lCurrentPtr++));
		}
		/*x[k] = (y[k] - sum) / u; */
        *(lBeginPtr--) = (*(lGeneratedData--) - sum) / u;
		lLineMatrix -= lStride;
	}
}
    
//应该也是LUP分解法一部分  		
static void mi_lupInvert (mi_FLOAT32 * pSrcMatrix,
                    mi_FLOAT32 * pDestMatrix,
                    mi_UINT32 nb_compo,
                    mi_UINT32 * pPermutations,
                    mi_FLOAT32 * p_src_temp,
                    mi_FLOAT32 * p_dest_temp,
                    mi_FLOAT32 * p_swap_area )
{
	mi_UINT32 j,i;
	mi_FLOAT32 * lCurrentPtr;
	mi_FLOAT32 * lLineMatrix = pDestMatrix;
	mi_UINT32 lSwapSize = nb_compo * (mi_UINT32)sizeof(mi_FLOAT32);

	for (j = 0; j < nb_compo; ++j) {
		lCurrentPtr = lLineMatrix++;
        memset(p_src_temp,0,lSwapSize);
    	p_src_temp[j] = 1.0;
		mi_lupSolve(p_dest_temp,pSrcMatrix,p_src_temp, pPermutations, nb_compo , p_swap_area);

		for (i = 0; i < nb_compo; ++i) {
    		*(lCurrentPtr) = p_dest_temp[i];
			lCurrentPtr+=nb_compo;
    	}
    }
}

