
//#include "mi_apps_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "openjpeg.h"
#include "convert.h"
//BMP文件头
typedef struct {
	mi_UINT16 bfType;      /* 'BM' for Bitmap (19776) */
	mi_UINT32 bfSize;      /* Size of the file        */
	mi_UINT16 bfReserved1; /* Reserved : 0            */
	mi_UINT16 bfReserved2; /* Reserved : 0            */
	mi_UINT32 bfOffBits;   /* Offset                  */
} mi_BITMAPFILEHEADER;
//BMP文件信息头
typedef struct {
	mi_UINT32 biSize;             /* Size of the structure in bytes */
	mi_UINT32 biWidth;            /* Width of the image in pixels */
	mi_UINT32 biHeight;           /* Heigth of the image in pixels */
	mi_UINT16 biPlanes;           /* 1 */
	mi_UINT16 biBitCount;         /* Number of color bits by pixels */
	mi_UINT32 biCompression;      /* Type of encoding 0: none 1: RLE8 2: RLE4（RLE为windows下的运行长度编码技术） */
	mi_UINT32 biSizeImage;        /* Size of the image in bytes */
	mi_UINT32 biXpelsPerMeter;    /* Horizontal (X) resolution in pixels/meter */
	mi_UINT32 biYpelsPerMeter;    /* Vertical (Y) resolution in pixels/meter */
	mi_UINT32 biClrUsed;          /* Number of color used in the image (0: ALL) */
	mi_UINT32 biClrImportant;     /* Number of important color (0: ALL) */
	mi_UINT32 biRedMask;          /* Red channel bit mask */
	mi_UINT32 biGreenMask;        /* Green channel bit mask */
	mi_UINT32 biBlueMask;         /* Blue channel bit mask */
	mi_UINT32 biAlphaMask;        /* Alpha channel bit mask */
	mi_UINT32 biColorSpaceType;   /* Color space type */
	mi_UINT8  biColorSpaceEP[36]; /* Color space end points */
	mi_UINT32 biRedGamma;         /* Red channel gamma */
	mi_UINT32 biGreenGamma;       /* Green channel gamma */
	mi_UINT32 biBlueGamma;        /* Blue channel gamma */
	mi_UINT32 biIntent;           /* Intent */
	mi_UINT32 biIccProfileData;   /* ICC profile data */
	mi_UINT32 biIccProfileSize;   /* ICC profile size */
	mi_UINT32 biReserved;         /* Reserved */
} mi_BITMAPINFOHEADER;

static void mi_applyLUT8u_8u32s_C1R(
	mi_UINT8 const* pSrc, mi_INT32 srcStride,
	mi_INT32* pDst, mi_INT32 dstStride,
	mi_UINT8 const* pLUT,
	mi_UINT32 width, mi_UINT32 height)
{
	mi_UINT32 y;
	
	for (y = height; y != 0U; --y) {
		mi_UINT32 x;
		
		for(x = 0; x < width; x++)
		{
			pDst[x] = (mi_INT32)pLUT[pSrc[x]];
		}
		pSrc += srcStride;
		pDst += dstStride;
	}
}

static void mi_applyLUT8u_8u32s_C1P3R(
	mi_UINT8 const* pSrc, mi_INT32 srcStride,
	mi_INT32* const* pDst, mi_INT32 const* pDstStride,
	mi_UINT8 const* const* pLUT,
	mi_UINT32 width, mi_UINT32 height)
{
	mi_UINT32 y;
	mi_INT32* pR = pDst[0];
	mi_INT32* pG = pDst[1];
	mi_INT32* pB = pDst[2];
	mi_UINT8 const* pLUT_R = pLUT[0];
	mi_UINT8 const* pLUT_G = pLUT[1];
	mi_UINT8 const* pLUT_B = pLUT[2];
	
	for (y = height; y != 0U; --y) {
		mi_UINT32 x;
		
		for(x = 0; x < width; x++)
		{
			mi_UINT8 idx = pSrc[x];
			pR[x] = (mi_INT32)pLUT_R[idx];
			pG[x] = (mi_INT32)pLUT_G[idx];
			pB[x] = (mi_INT32)pLUT_B[idx];
		}
		pSrc += srcStride;
		pR += pDstStride[0];
		pG += pDstStride[1];
		pB += pDstStride[2];
	}
}
//图像数据铺到参考网格上
static void bmp24toimage(const mi_UINT8* pData, mi_UINT32 stride, mi_image_t* image)
{
	int index;
	mi_UINT32 width, height;
	mi_UINT32 x, y;
	const mi_UINT8 *pSrc = NULL;

	width  = image->comps[0].w;
	height = image->comps[0].h;
	
	index = 0;
	pSrc = pData + (height - 1U) * stride;
	for(y = 0; y < height; y++)
	{
		for(x = 0; x < width; x++)
		{
			image->comps[0].data[index] = (mi_INT32)pSrc[3*x+2];	/* R */
			image->comps[1].data[index] = (mi_INT32)pSrc[3*x+1];	/* G */
			image->comps[2].data[index] = (mi_INT32)pSrc[3*x+0];	/* B */
			index++;
		}
		pSrc -= stride;
	}
}

static void bmp_mask_get_shift_and_prec(mi_UINT32 mask, mi_UINT32* shift, mi_UINT32* prec)
{
	mi_UINT32 l_shift, l_prec;
	
	l_shift = l_prec = 0U;
	
	if (mask != 0U) {
		while ((mask & 1U) == 0U) {
			mask >>= 1;
			l_shift++;
		}
		while (mask & 1U) {
			mask >>= 1;
			l_prec++;
		}
	}
	*shift = l_shift; *prec = l_prec;
}

static void bmpmask32toimage(const mi_UINT8* pData, mi_UINT32 stride, mi_image_t* image, mi_UINT32 redMask, mi_UINT32 greenMask, mi_UINT32 blueMask, mi_UINT32 alphaMask)
{
	int index;
	mi_UINT32 width, height;
	mi_UINT32 x, y;
	const mi_UINT8 *pSrc = NULL;
	mi_BOOL hasAlpha;
	mi_UINT32 redShift,   redPrec;
	mi_UINT32 greenShift, greenPrec;
	mi_UINT32 blueShift,  bluePrec;
	mi_UINT32 alphaShift, alphaPrec;
	
	width  = image->comps[0].w;
	height = image->comps[0].h;
	
	hasAlpha = image->numcomps > 3U;
	
	bmp_mask_get_shift_and_prec(redMask,   &redShift,   &redPrec);
	bmp_mask_get_shift_and_prec(greenMask, &greenShift, &greenPrec);
	bmp_mask_get_shift_and_prec(blueMask,  &blueShift,  &bluePrec);
	bmp_mask_get_shift_and_prec(alphaMask, &alphaShift, &alphaPrec);
	
	image->comps[0].bpp = redPrec;
	image->comps[0].prec = redPrec;
	image->comps[1].bpp = greenPrec;
	image->comps[1].prec = greenPrec;
	image->comps[2].bpp = bluePrec;
	image->comps[2].prec = bluePrec;
	if (hasAlpha) {
		image->comps[3].bpp = alphaPrec;
		image->comps[3].prec = alphaPrec;
	}
	
	index = 0;
	pSrc = pData + (height - 1U) * stride;
	for(y = 0; y < height; y++)
	{
		for(x = 0; x < width; x++)
		{
			mi_UINT32 value = 0U;
			
			value |= ((mi_UINT32)pSrc[4*x+0]) <<  0;
			value |= ((mi_UINT32)pSrc[4*x+1]) <<  8;
			value |= ((mi_UINT32)pSrc[4*x+2]) << 16;
			value |= ((mi_UINT32)pSrc[4*x+3]) << 24;
			
			image->comps[0].data[index] = (mi_INT32)((value & redMask)   >> redShift);   /* R */
			image->comps[1].data[index] = (mi_INT32)((value & greenMask) >> greenShift); /* G */
			image->comps[2].data[index] = (mi_INT32)((value & blueMask)  >> blueShift);  /* B */
			if (hasAlpha) {
				image->comps[3].data[index] = (mi_INT32)((value & alphaMask)  >> alphaShift);  /* A */
			}
			index++;
		}
		pSrc -= stride;
	}
}

static void bmpmask16toimage(const mi_UINT8* pData, mi_UINT32 stride, mi_image_t* image, mi_UINT32 redMask, mi_UINT32 greenMask, mi_UINT32 blueMask, mi_UINT32 alphaMask)
{
	int index;
	mi_UINT32 width, height;
	mi_UINT32 x, y;
	const mi_UINT8 *pSrc = NULL;
	mi_BOOL hasAlpha;
	mi_UINT32 redShift,   redPrec;
	mi_UINT32 greenShift, greenPrec;
	mi_UINT32 blueShift,  bluePrec;
	mi_UINT32 alphaShift, alphaPrec;
	
	width  = image->comps[0].w;
	height = image->comps[0].h;
	
	hasAlpha = image->numcomps > 3U;
	
	bmp_mask_get_shift_and_prec(redMask,   &redShift,   &redPrec);
	bmp_mask_get_shift_and_prec(greenMask, &greenShift, &greenPrec);
	bmp_mask_get_shift_and_prec(blueMask,  &blueShift,  &bluePrec);
	bmp_mask_get_shift_and_prec(alphaMask, &alphaShift, &alphaPrec);
	
	image->comps[0].bpp = redPrec;
	image->comps[0].prec = redPrec;
	image->comps[1].bpp = greenPrec;
	image->comps[1].prec = greenPrec;
	image->comps[2].bpp = bluePrec;
	image->comps[2].prec = bluePrec;
	if (hasAlpha) {
		image->comps[3].bpp = alphaPrec;
		image->comps[3].prec = alphaPrec;
	}
	
	index = 0;
	pSrc = pData + (height - 1U) * stride;
	for(y = 0; y < height; y++)
	{
		for(x = 0; x < width; x++)
		{
			mi_UINT32 value = 0U;
			
			value |= ((mi_UINT32)pSrc[2*x+0]) <<  0;
			value |= ((mi_UINT32)pSrc[2*x+1]) <<  8;
			
			image->comps[0].data[index] = (mi_INT32)((value & redMask)   >> redShift);   /* R */
			image->comps[1].data[index] = (mi_INT32)((value & greenMask) >> greenShift); /* G */
			image->comps[2].data[index] = (mi_INT32)((value & blueMask)  >> blueShift);  /* B */
			if (hasAlpha) {
				image->comps[3].data[index] = (mi_INT32)((value & alphaMask)  >> alphaShift);  /* A */
			}
			index++;
		}
		pSrc -= stride;
	}
}

static mi_image_t* bmp8toimage(const mi_UINT8* pData, mi_UINT32 stride, mi_image_t* image, mi_UINT8 const* const* pLUT)
{
	mi_UINT32 width, height;
	const mi_UINT8 *pSrc = NULL;
	
	width  = image->comps[0].w;
	height = image->comps[0].h;
	
	pSrc = pData + (height - 1U) * stride;
	if (image->numcomps == 1U) {
		mi_applyLUT8u_8u32s_C1R(pSrc, -(mi_INT32)stride, image->comps[0].data, (mi_INT32)width, pLUT[0], width, height);
	}
	else {
		mi_INT32* pDst[3];
		mi_INT32  pDstStride[3];
		
		pDst[0] = image->comps[0].data; pDst[1] = image->comps[1].data; pDst[2] = image->comps[2].data;
		pDstStride[0] = (mi_INT32)width; pDstStride[1] = (mi_INT32)width; pDstStride[2] = (mi_INT32)width;
		mi_applyLUT8u_8u32s_C1P3R(pSrc, -(mi_INT32)stride, pDst, pDstStride, pLUT, width, height);
	}
	return image;
}
//解析BMP文件头
static mi_BOOL bmp_read_file_header(FILE* IN, mi_BITMAPFILEHEADER* header)
{
	//字段若表示数字则在文件里按照高位在后低位在前的顺序排列
	header->bfType  = (mi_UINT16)getc(IN);
	header->bfType |= (mi_UINT16)((mi_UINT32)getc(IN) << 8);//这样一来便将ASCII码值'BM'变成了'MB',换成int就是19778
	
	if (header->bfType != 19778) {
		fprintf(stderr,"Error, not a BMP file!\n");
		return mi_FALSE;
	}
	
	/* FILE HEADER */
	/* ------------- */
	header->bfSize  = (mi_UINT32)getc(IN);
	header->bfSize |= (mi_UINT32)getc(IN) << 8;
	header->bfSize |= (mi_UINT32)getc(IN) << 16;
	header->bfSize |= (mi_UINT32)getc(IN) << 24;
	
	header->bfReserved1  = (mi_UINT16)getc(IN);
	header->bfReserved1 |= (mi_UINT16)((mi_UINT32)getc(IN) << 8);
	
	header->bfReserved2  = (mi_UINT16)getc(IN);
	header->bfReserved2 |= (mi_UINT16)((mi_UINT32)getc(IN) << 8);
	
	header->bfOffBits  = (mi_UINT32)getc(IN);
	header->bfOffBits |= (mi_UINT32)getc(IN) << 8;
	header->bfOffBits |= (mi_UINT32)getc(IN) << 16;
	header->bfOffBits |= (mi_UINT32)getc(IN) << 24;
	return mi_TRUE;
}
//解析BMP文件信息头
static mi_BOOL bmp_read_info_header(FILE* IN, mi_BITMAPINFOHEADER* header)
{
	memset(header, 0, sizeof(*header));
	/* INFO HEADER */
	/* ------------- */
	header->biSize  = (mi_UINT32)getc(IN);
	header->biSize |= (mi_UINT32)getc(IN) << 8;
	header->biSize |= (mi_UINT32)getc(IN) << 16;
	header->biSize |= (mi_UINT32)getc(IN) << 24;
	
	switch (header->biSize) {
		case 12U:  /* BITMAPCOREHEADER */
		case 40U:  /* BITMAPINFOHEADER */
		case 52U:  /* BITMAPV2INFOHEADER */
		case 56U:  /* BITMAPV3INFOHEADER */
		case 108U: /* BITMAPV4HEADER */
		case 124U: /* BITMAPV5HEADER */
			break;
  default:
			fprintf(stderr,"Error, unknown BMP header size %d\n", header->biSize);
			return mi_FALSE;
	}
	
	header->biWidth  = (mi_UINT32)getc(IN);
	header->biWidth |= (mi_UINT32)getc(IN) << 8;
	header->biWidth |= (mi_UINT32)getc(IN) << 16;
	header->biWidth |= (mi_UINT32)getc(IN) << 24;
	
	header->biHeight  = (mi_UINT32)getc(IN);
	header->biHeight |= (mi_UINT32)getc(IN) << 8;
	header->biHeight |= (mi_UINT32)getc(IN) << 16;
	header->biHeight |= (mi_UINT32)getc(IN) << 24;
	
	header->biPlanes  = (mi_UINT16)getc(IN);
	header->biPlanes |= (mi_UINT16)((mi_UINT32)getc(IN) << 8);
	
	header->biBitCount  = (mi_UINT16)getc(IN);
	header->biBitCount |= (mi_UINT16)((mi_UINT32)getc(IN) << 8);
	
	if(header->biSize >= 40U) {
		header->biCompression  = (mi_UINT32)getc(IN);
		header->biCompression |= (mi_UINT32)getc(IN) << 8;
		header->biCompression |= (mi_UINT32)getc(IN) << 16;
		header->biCompression |= (mi_UINT32)getc(IN) << 24;
		
		header->biSizeImage  = (mi_UINT32)getc(IN);
		header->biSizeImage |= (mi_UINT32)getc(IN) << 8;
		header->biSizeImage |= (mi_UINT32)getc(IN) << 16;
		header->biSizeImage |= (mi_UINT32)getc(IN) << 24;
		
		header->biXpelsPerMeter  = (mi_UINT32)getc(IN);
		header->biXpelsPerMeter |= (mi_UINT32)getc(IN) << 8;
		header->biXpelsPerMeter |= (mi_UINT32)getc(IN) << 16;
		header->biXpelsPerMeter |= (mi_UINT32)getc(IN) << 24;
		
		header->biYpelsPerMeter  = (mi_UINT32)getc(IN);
		header->biYpelsPerMeter |= (mi_UINT32)getc(IN) << 8;
		header->biYpelsPerMeter |= (mi_UINT32)getc(IN) << 16;
		header->biYpelsPerMeter |= (mi_UINT32)getc(IN) << 24;
		
		header->biClrUsed  = (mi_UINT32)getc(IN);
		header->biClrUsed |= (mi_UINT32)getc(IN) << 8;
		header->biClrUsed |= (mi_UINT32)getc(IN) << 16;
		header->biClrUsed |= (mi_UINT32)getc(IN) << 24;
		
		header->biClrImportant  = (mi_UINT32)getc(IN);
		header->biClrImportant |= (mi_UINT32)getc(IN) << 8;
		header->biClrImportant |= (mi_UINT32)getc(IN) << 16;
		header->biClrImportant |= (mi_UINT32)getc(IN) << 24;
	}
	
	if(header->biSize >= 56U) {
		header->biRedMask  = (mi_UINT32)getc(IN);
		header->biRedMask |= (mi_UINT32)getc(IN) << 8;
		header->biRedMask |= (mi_UINT32)getc(IN) << 16;
		header->biRedMask |= (mi_UINT32)getc(IN) << 24;
		
		header->biGreenMask  = (mi_UINT32)getc(IN);
		header->biGreenMask |= (mi_UINT32)getc(IN) << 8;
		header->biGreenMask |= (mi_UINT32)getc(IN) << 16;
		header->biGreenMask |= (mi_UINT32)getc(IN) << 24;
		
		header->biBlueMask  = (mi_UINT32)getc(IN);
		header->biBlueMask |= (mi_UINT32)getc(IN) << 8;
		header->biBlueMask |= (mi_UINT32)getc(IN) << 16;
		header->biBlueMask |= (mi_UINT32)getc(IN) << 24;
		
		header->biAlphaMask  = (mi_UINT32)getc(IN);
		header->biAlphaMask |= (mi_UINT32)getc(IN) << 8;
		header->biAlphaMask |= (mi_UINT32)getc(IN) << 16;
		header->biAlphaMask |= (mi_UINT32)getc(IN) << 24;
	}
	
	if(header->biSize >= 108U) {
		header->biColorSpaceType  = (mi_UINT32)getc(IN);
		header->biColorSpaceType |= (mi_UINT32)getc(IN) << 8;
		header->biColorSpaceType |= (mi_UINT32)getc(IN) << 16;
		header->biColorSpaceType |= (mi_UINT32)getc(IN) << 24;
		
		if (fread(&(header->biColorSpaceEP), 1U, sizeof(header->biColorSpaceEP), IN) != sizeof(header->biColorSpaceEP)) {
			fprintf(stderr,"Error, can't  read BMP header\n");
			return mi_FALSE;
		}
		
		header->biRedGamma  = (mi_UINT32)getc(IN);
		header->biRedGamma |= (mi_UINT32)getc(IN) << 8;
		header->biRedGamma |= (mi_UINT32)getc(IN) << 16;
		header->biRedGamma |= (mi_UINT32)getc(IN) << 24;
		
		header->biGreenGamma  = (mi_UINT32)getc(IN);
		header->biGreenGamma |= (mi_UINT32)getc(IN) << 8;
		header->biGreenGamma |= (mi_UINT32)getc(IN) << 16;
		header->biGreenGamma |= (mi_UINT32)getc(IN) << 24;
		
		header->biBlueGamma  = (mi_UINT32)getc(IN);
		header->biBlueGamma |= (mi_UINT32)getc(IN) << 8;
		header->biBlueGamma |= (mi_UINT32)getc(IN) << 16;
		header->biBlueGamma |= (mi_UINT32)getc(IN) << 24;
	}
	
	if(header->biSize >= 124U) {
		header->biIntent  = (mi_UINT32)getc(IN);
		header->biIntent |= (mi_UINT32)getc(IN) << 8;
		header->biIntent |= (mi_UINT32)getc(IN) << 16;
		header->biIntent |= (mi_UINT32)getc(IN) << 24;
		
		header->biIccProfileData  = (mi_UINT32)getc(IN);
		header->biIccProfileData |= (mi_UINT32)getc(IN) << 8;
		header->biIccProfileData |= (mi_UINT32)getc(IN) << 16;
		header->biIccProfileData |= (mi_UINT32)getc(IN) << 24;
		
		header->biIccProfileSize  = (mi_UINT32)getc(IN);
		header->biIccProfileSize |= (mi_UINT32)getc(IN) << 8;
		header->biIccProfileSize |= (mi_UINT32)getc(IN) << 16;
		header->biIccProfileSize |= (mi_UINT32)getc(IN) << 24;
		
		header->biReserved  = (mi_UINT32)getc(IN);
		header->biReserved |= (mi_UINT32)getc(IN) << 8;
		header->biReserved |= (mi_UINT32)getc(IN) << 16;
		header->biReserved |= (mi_UINT32)getc(IN) << 24;
	}
	return mi_TRUE;
}
//读取BMP图像数据
static mi_BOOL bmp_read_raw_data(FILE* IN, mi_UINT8* pData, mi_UINT32 stride, mi_UINT32 width, mi_UINT32 height)
{
	mi_ARG_NOT_USED(width);
	
	if ( fread(pData, sizeof(mi_UINT8), stride * height, IN) != (stride * height) )
	{
		fprintf(stderr, "\nError: fread return a number of element different from the expected.\n");
		return mi_FALSE;
	}
	return mi_TRUE;
}

static mi_BOOL bmp_read_rle8_data(FILE* IN, mi_UINT8* pData, mi_UINT32 stride, mi_UINT32 width, mi_UINT32 height)
{
	mi_UINT32 x, y;
	mi_UINT8 *pix;
	const mi_UINT8 *beyond;
	
	beyond = pData + stride * height;
	pix = pData;
	
	x = y = 0U;
	while (y < height)
	{
		int c = getc(IN);
		
		if (c) {
			int j;
			mi_UINT8 c1 = (mi_UINT8)getc(IN);
			
			for (j = 0; (j < c) && (x < width) && ((mi_SIZE_T)pix < (mi_SIZE_T)beyond); j++, x++, pix++) {
				*pix = c1;
			}
		}
		else {
			c = getc(IN);
			if (c == 0x00) { /* EOL */
				x = 0;
				++y;
				pix = pData + y * stride + x;
			}
			else if (c == 0x01) { /* EOP */
				break;
			}
			else if (c == 0x02) { /* MOVE by dxdy */
				c = getc(IN);
				x += (mi_UINT32)c;
				c = getc(IN);
				y += (mi_UINT32)c;
				pix = pData + y * stride + x;
			}
			else /* 03 .. 255 */
			{
				int j;
				for (j = 0; (j < c) && (x < width) && ((mi_SIZE_T)pix < (mi_SIZE_T)beyond); j++, x++, pix++)
				{
					mi_UINT8 c1 = (mi_UINT8)getc(IN);
					*pix = c1;
				}
				if ((mi_UINT32)c & 1U) { /* skip padding byte */
					getc(IN);
				}
			}
		}
	}/* while() */
	return mi_TRUE;
}

static mi_BOOL bmp_read_rle4_data(FILE* IN, mi_UINT8* pData, mi_UINT32 stride, mi_UINT32 width, mi_UINT32 height)
{
	mi_UINT32 x, y;
	mi_UINT8 *pix;
	const mi_UINT8 *beyond;
	
	beyond = pData + stride * height;
	pix = pData;
	x = y = 0U;
	while(y < height)
	{
		int c = getc(IN);
		if(c == EOF) break;
		
		if(c) {/* encoded mode */
			int j;
			mi_UINT8 c1 = (mi_UINT8)getc(IN);
		
			for (j = 0; (j < c) && (x < width) && ((mi_SIZE_T)pix < (mi_SIZE_T)beyond); j++, x++, pix++) {
				*pix = (mi_UINT8)((j&1) ? (c1 & 0x0fU) : ((c1>>4)&0x0fU));
			}
		}
		else { /* absolute mode */
			c = getc(IN);
			if(c == EOF) break;
		
			if(c == 0x00) { /* EOL */
				x = 0;  y++;  pix = pData + y * stride;
			}
			else if(c == 0x01) { /* EOP */
				break;
			}
			else if(c == 0x02) { /* MOVE by dxdy */
				c = getc(IN);  x += (mi_UINT32)c;
				c = getc(IN);  y += (mi_UINT32)c;
				pix = pData + y * stride + x;
			}
			else { /* 03 .. 255 : absolute mode */
				int j;
				mi_UINT8 c1 = 0U;
				
				for (j = 0; (j < c) && (x < width) && ((mi_SIZE_T)pix < (mi_SIZE_T)beyond); j++, x++, pix++) {
					if((j&1) == 0) {
							c1 = (mi_UINT8)getc(IN);
					}
					*pix =  (mi_UINT8)((j&1) ? (c1 & 0x0fU) : ((c1>>4)&0x0fU));
				}
				if(((c&3) == 1) || ((c&3) == 2)) { /* skip padding byte */
						getc(IN);
				}
			}
		}
	}  /* while(y < height) */
	return mi_TRUE;
}
//BMP图片解码函数
mi_image_t* bmptoimage(const char *filename, mi_cparameters_t *parameters)
{
	mi_image_cmptparm_t cmptparm[4];	/* maximum of 4 components */
	mi_UINT8 lut_R[256], lut_G[256], lut_B[256];//调色板
	mi_UINT8 const* pLUT[3];
	mi_image_t * image = NULL;
	FILE *IN;
	mi_BITMAPFILEHEADER File_h;
	mi_BITMAPINFOHEADER Info_h;
	mi_UINT32 i, palette_len, numcmpts = 1U;
	mi_BOOL l_result = mi_FALSE;
	mi_UINT8* pData = NULL;//图像数据
	mi_UINT32 stride;
	
	pLUT[0] = lut_R; pLUT[1] = lut_G; pLUT[2] = lut_B;
	
	IN = fopen(filename, "rb");
	if (!IN)
	{
		fprintf(stderr, "Failed to open %s for reading !!\n", filename);
		return NULL;
	}

	if (!bmp_read_file_header(IN, &File_h)) {
		fclose(IN);
		return NULL;
	}
	if (!bmp_read_info_header(IN, &Info_h)) {
		fclose(IN);
		return NULL;
	}
	
	/* Load palette */
	if (Info_h.biBitCount <= 8U)
	{
		memset(&lut_R[0], 0, sizeof(lut_R));
		memset(&lut_G[0], 0, sizeof(lut_G));
		memset(&lut_B[0], 0, sizeof(lut_B));
		
		palette_len = Info_h.biClrUsed;
		if((palette_len == 0U) && (Info_h.biBitCount <= 8U)) {
			palette_len = (1U << Info_h.biBitCount);
		}
		if (palette_len > 256U) {
			palette_len = 256U;
		}
		if (palette_len > 0U) {
			mi_UINT8 has_color = 0U;
			for (i = 0U; i < palette_len; i++) {
				lut_B[i] = (mi_UINT8)getc(IN);
				lut_G[i] = (mi_UINT8)getc(IN);
				lut_R[i] = (mi_UINT8)getc(IN);
				(void)getc(IN); /* padding */
				has_color |= (lut_B[i] ^ lut_G[i]) | (lut_G[i] ^ lut_R[i]);
			}
			if(has_color) {
				numcmpts = 3U;
			}
		}
	} else {
		numcmpts = 3U;
		if ((Info_h.biCompression == 3) && (Info_h.biAlphaMask != 0U)) {
			numcmpts++;
		}
	}
	
	stride = ((Info_h.biWidth * Info_h.biBitCount + 31U) / 32U) * 4U; /* rows are aligned on 32bits */
	if (Info_h.biBitCount == 4 && Info_h.biCompression == 2) { /* RLE 4 gets decoded as 8 bits data for now... */
		stride = ((Info_h.biWidth * 8U + 31U) / 32U) * 4U;
	}
	pData = (mi_UINT8 *) calloc(1, stride * Info_h.biHeight * sizeof(mi_UINT8));
	if (pData == NULL) {
		fclose(IN);
		return NULL;
	}
	/* Place the cursor at the beginning of the image information */
	fseek(IN, 0, SEEK_SET);
	fseek(IN, (long)File_h.bfOffBits, SEEK_SET);
	
	switch (Info_h.biCompression) {
		case 0:
		case 3:
			/* read raw data */
			l_result = bmp_read_raw_data(IN, pData, stride, Info_h.biWidth, Info_h.biHeight);
			break;
		case 1:
			/* read rle8 data */
			l_result = bmp_read_rle8_data(IN, pData, stride, Info_h.biWidth, Info_h.biHeight);
			break;
		case 2:
			/* read rle4 data */
			l_result = bmp_read_rle4_data(IN, pData, stride, Info_h.biWidth, Info_h.biHeight);
			break;
  default:
			fprintf(stderr, "Unsupported BMP compression\n");
			l_result = mi_FALSE;
			break;
	}
	if (!l_result) {
		free(pData);
		fclose(IN);
		return NULL;
	}
	
	/* create the image */
	memset(&cmptparm[0], 0, sizeof(cmptparm));
	for(i = 0; i < 4U; i++)
	{
		cmptparm[i].prec = 8;
		cmptparm[i].bpp  = 8;
		cmptparm[i].sgnd = 0;
		cmptparm[i].dx   = (mi_UINT32)parameters->subsampling_dx;
		cmptparm[i].dy   = (mi_UINT32)parameters->subsampling_dy;
		cmptparm[i].w    = Info_h.biWidth;
		cmptparm[i].h    = Info_h.biHeight;
	}

	image = mi_image_create(numcmpts, &cmptparm[0], (numcmpts == 1U) ? mi_CLRSPC_GRAY : mi_CLRSPC_SRGB);
	if(!image) {
		fclose(IN);
		free(pData);
		return NULL;
	}
	if (numcmpts == 4U) {//如果图像分量有四个，则说明还有个Alpha通道
		image->comps[3].alpha = 1;
	}
	
	/* set image offset and reference grid */
	//JPEG2000第一步：设置参考网格
	image->x0 = (mi_UINT32)parameters->image_offset_x0;
	image->y0 = (mi_UINT32)parameters->image_offset_y0;
	image->x1 =	image->x0 + (Info_h.biWidth  - 1U) * (mi_UINT32)parameters->subsampling_dx + 1U;
	image->y1 = image->y0 + (Info_h.biHeight - 1U) * (mi_UINT32)parameters->subsampling_dy + 1U;
	
	/* Read the data */
	if (Info_h.biBitCount == 24 && Info_h.biCompression == 0) { /*RGB */
		bmp24toimage(pData, stride, image);//原图位深度24
	}
	else if (Info_h.biBitCount == 8 && Info_h.biCompression == 0) { /* RGB 8bpp Indexed */
		bmp8toimage(pData, stride, image, pLUT);//原图位深度8
	}
	else if (Info_h.biBitCount == 8 && Info_h.biCompression == 1) { /*RLE8*/
		bmp8toimage(pData, stride, image, pLUT);
	}
	else if (Info_h.biBitCount == 4 && Info_h.biCompression == 2) { /*RLE4*/
		bmp8toimage(pData, stride, image, pLUT); /* RLE 4 gets decoded as 8 bits data for now */
	}
	else if (Info_h.biBitCount == 32 && Info_h.biCompression == 0) { /* RGBX */
		bmpmask32toimage(pData, stride, image, 0x00FF0000U, 0x0000FF00U, 0x000000FFU, 0x00000000U);
	}
	else if (Info_h.biBitCount == 32 && Info_h.biCompression == 3) { /* bitmask */
		bmpmask32toimage(pData, stride, image, Info_h.biRedMask, Info_h.biGreenMask, Info_h.biBlueMask, Info_h.biAlphaMask);
	}
	else if (Info_h.biBitCount == 16 && Info_h.biCompression == 0) { /* RGBX */
		bmpmask16toimage(pData, stride, image, 0x7C00U, 0x03E0U, 0x001FU, 0x0000U);
	}
	else if (Info_h.biBitCount == 16 && Info_h.biCompression == 3) { /* bitmask */
		if ((Info_h.biRedMask == 0U) && (Info_h.biGreenMask == 0U) && (Info_h.biBlueMask == 0U)) {
			Info_h.biRedMask   = 0xF800U;
			Info_h.biGreenMask = 0x07E0U;
			Info_h.biBlueMask  = 0x001FU;
		}
		bmpmask16toimage(pData, stride, image, Info_h.biRedMask, Info_h.biGreenMask, Info_h.biBlueMask, Info_h.biAlphaMask);
	}
	else {
		mi_image_destroy(image);
		image = NULL;
		fprintf(stderr, "Other system than 24 bits/pixels or 8 bits (no RLE coding) is not yet implemented [%d]\n", Info_h.biBitCount);
	}
	free(pData);
	fclose(IN);
	return image;
}

int imagetobmp(mi_image_t * image, const char *outfile) {
	int w, h;
	int i, pad;
	FILE *fdest = NULL;
	int adjustR, adjustG, adjustB;

	if (image->comps[0].prec < 8) {
		fprintf(stderr, "Unsupported number of components: %d\n", image->comps[0].prec);
		return 1;
	}
	if (image->numcomps >= 3 && image->comps[0].dx == image->comps[1].dx
			&& image->comps[1].dx == image->comps[2].dx
			&& image->comps[0].dy == image->comps[1].dy
			&& image->comps[1].dy == image->comps[2].dy
			&& image->comps[0].prec == image->comps[1].prec
			&& image->comps[1].prec == image->comps[2].prec) {

		/* -->> -->> -->> -->>
		24 bits color
		<<-- <<-- <<-- <<-- */

		fdest = fopen(outfile, "wb");
		if (!fdest) {
			fprintf(stderr, "ERROR -> failed to open %s for writing\n", outfile);
			return 1;
		}

		w = (int)image->comps[0].w;
		h = (int)image->comps[0].h;

		fprintf(fdest, "BM");

		/* FILE HEADER */
		/* ------------- */
		fprintf(fdest, "%c%c%c%c",
				(mi_UINT8) (h * w * 3 + 3 * h * (w % 2) + 54) & 0xff,
				(mi_UINT8) ((h * w * 3 + 3 * h * (w % 2) + 54)	>> 8) & 0xff,
				(mi_UINT8) ((h * w * 3 + 3 * h * (w % 2) + 54)	>> 16) & 0xff,
				(mi_UINT8) ((h * w * 3 + 3 * h * (w % 2) + 54)	>> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff, ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (54) & 0xff, ((54) >> 8) & 0xff,((54) >> 16) & 0xff, ((54) >> 24) & 0xff);

		/* INFO HEADER   */
		/* ------------- */
		fprintf(fdest, "%c%c%c%c", (40) & 0xff, ((40) >> 8) & 0xff,	((40) >> 16) & 0xff, ((40) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (mi_UINT8) ((w) & 0xff),
				(mi_UINT8) ((w) >> 8) & 0xff,
				(mi_UINT8) ((w) >> 16) & 0xff,
				(mi_UINT8) ((w) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (mi_UINT8) ((h) & 0xff),
				(mi_UINT8) ((h) >> 8) & 0xff,
				(mi_UINT8) ((h) >> 16) & 0xff,
				(mi_UINT8) ((h) >> 24) & 0xff);
		fprintf(fdest, "%c%c", (1) & 0xff, ((1) >> 8) & 0xff);
		fprintf(fdest, "%c%c", (24) & 0xff, ((24) >> 8) & 0xff);
		fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff, ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (mi_UINT8) (3 * h * w + 3 * h * (w % 2)) & 0xff,
				(mi_UINT8) ((h * w * 3 + 3 * h * (w % 2)) >> 8) & 0xff,
				(mi_UINT8) ((h * w * 3 + 3 * h * (w % 2)) >> 16) & 0xff,
				(mi_UINT8) ((h * w * 3 + 3 * h * (w % 2)) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff, ((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,	((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff, ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff, ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);

		if (image->comps[0].prec > 8) {
			adjustR = (int)image->comps[0].prec - 8;
			printf("BMP CONVERSION: Truncating component 0 from %d bits to 8 bits\n", image->comps[0].prec);
		}
		else
			adjustR = 0;
		if (image->comps[1].prec > 8) {
			adjustG = (int)image->comps[1].prec - 8;
			printf("BMP CONVERSION: Truncating component 1 from %d bits to 8 bits\n", image->comps[1].prec);
		}
		else
			adjustG = 0;
		if (image->comps[2].prec > 8) {
			adjustB = (int)image->comps[2].prec - 8;
			printf("BMP CONVERSION: Truncating component 2 from %d bits to 8 bits\n", image->comps[2].prec);
		}
		else
			adjustB = 0;

		for (i = 0; i < w * h; i++) {
			mi_UINT8 rc, gc, bc;
			int r, g, b;

			r = image->comps[0].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
			r += (image->comps[0].sgnd ? 1 << (image->comps[0].prec - 1) : 0);
			r = ((r >> adjustR)+((r >> (adjustR-1))%2));
			if(r > 255) r = 255; else if(r < 0) r = 0;
			rc = (mi_UINT8)r;

			g = image->comps[1].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
			g += (image->comps[1].sgnd ? 1 << (image->comps[1].prec - 1) : 0);
			g = ((g >> adjustG)+((g >> (adjustG-1))%2));
			if(g > 255) g = 255; else if(g < 0) g = 0;
			gc = (mi_UINT8)g;

			b = image->comps[2].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
			b += (image->comps[2].sgnd ? 1 << (image->comps[2].prec - 1) : 0);
			b = ((b >> adjustB)+((b >> (adjustB-1))%2));
			if(b > 255) b = 255; else if(b < 0) b = 0;
			bc = (mi_UINT8)b;

			fprintf(fdest, "%c%c%c", bc, gc, rc);

			if ((i + 1) % w == 0) {
				for (pad = ((3 * w) % 4) ? (4 - (3 * w) % 4) : 0; pad > 0; pad--)	/* ADD */
					fprintf(fdest, "%c", 0);
			}
		}
		fclose(fdest);
	} else {			/* Gray-scale */

		/* -->> -->> -->> -->>
		8 bits non code (Gray scale)
		<<-- <<-- <<-- <<-- */

		fdest = fopen(outfile, "wb");
		if (!fdest) {
			fprintf(stderr, "ERROR -> failed to open %s for writing\n", outfile);
			return 1;
		}
		w = (int)image->comps[0].w;
		h = (int)image->comps[0].h;

		fprintf(fdest, "BM");

		/* FILE HEADER */
		/* ------------- */
		fprintf(fdest, "%c%c%c%c", (mi_UINT8) (h * w + 54 + 1024 + h * (w % 2)) & 0xff,
				(mi_UINT8) ((h * w + 54 + 1024 + h * (w % 2)) >> 8) & 0xff,
				(mi_UINT8) ((h * w + 54 + 1024 + h * (w % 2)) >> 16) & 0xff,
				(mi_UINT8) ((h * w + 54 + 1024 + w * (w % 2)) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff, ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (54 + 1024) & 0xff, ((54 + 1024) >> 8) & 0xff,
				((54 + 1024) >> 16) & 0xff,
				((54 + 1024) >> 24) & 0xff);

		/* INFO HEADER */
		/* ------------- */
		fprintf(fdest, "%c%c%c%c", (40) & 0xff, ((40) >> 8) & 0xff,	((40) >> 16) & 0xff, ((40) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (mi_UINT8) ((w) & 0xff),
				(mi_UINT8) ((w) >> 8) & 0xff,
				(mi_UINT8) ((w) >> 16) & 0xff,
				(mi_UINT8) ((w) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (mi_UINT8) ((h) & 0xff),
				(mi_UINT8) ((h) >> 8) & 0xff,
				(mi_UINT8) ((h) >> 16) & 0xff,
				(mi_UINT8) ((h) >> 24) & 0xff);
		fprintf(fdest, "%c%c", (1) & 0xff, ((1) >> 8) & 0xff);
		fprintf(fdest, "%c%c", (8) & 0xff, ((8) >> 8) & 0xff);
		fprintf(fdest, "%c%c%c%c", (0) & 0xff, ((0) >> 8) & 0xff, ((0) >> 16) & 0xff, ((0) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (mi_UINT8) (h * w + h * (w % 2)) & 0xff,
				(mi_UINT8) ((h * w + h * (w % 2)) >> 8) &	0xff,
				(mi_UINT8) ((h * w + h * (w % 2)) >> 16) &	0xff,
				(mi_UINT8) ((h * w + h * (w % 2)) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,	((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (7834) & 0xff, ((7834) >> 8) & 0xff,	((7834) >> 16) & 0xff, ((7834) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (256) & 0xff, ((256) >> 8) & 0xff, ((256) >> 16) & 0xff, ((256) >> 24) & 0xff);
		fprintf(fdest, "%c%c%c%c", (256) & 0xff, ((256) >> 8) & 0xff, ((256) >> 16) & 0xff, ((256) >> 24) & 0xff);

		if (image->comps[0].prec > 8) {
			adjustR = (int)image->comps[0].prec - 8;
			printf("BMP CONVERSION: Truncating component 0 from %d bits to 8 bits\n", image->comps[0].prec);
		}else
			adjustR = 0;

		for (i = 0; i < 256; i++) {
			fprintf(fdest, "%c%c%c%c", i, i, i, 0);
		}

		for (i = 0; i < w * h; i++) {
			int r;

			r = image->comps[0].data[w * h - ((i) / (w) + 1) * w + (i) % (w)];
			r += (image->comps[0].sgnd ? 1 << (image->comps[0].prec - 1) : 0);
			r = ((r >> adjustR)+((r >> (adjustR-1))%2));
			if(r > 255) r = 255; else if(r < 0) r = 0;

			fprintf(fdest, "%c", (mi_UINT8)r);

			if ((i + 1) % w == 0) {
				for (pad = (w % 4) ? (4 - w % 4) : 0; pad > 0; pad--)	/* ADD */
					fprintf(fdest, "%c", 0);
			}
		}
		fclose(fdest);
	}

	return 0;
}
