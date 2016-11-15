
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

//#include "mi_apps_config.h"
#include "openjpeg.h"
#include "color.h"

#ifdef mi_HAVE_LIBLCMS2
#include <lcms2.h>
#endif
#ifdef mi_HAVE_LIBLCMS1
#include <lcms.h>
#endif

#ifdef mi_USE_LEGACY
#define mi_CLRSPC_GRAY CLRSPC_GRAY
#define mi_CLRSPC_SRGB CLRSPC_SRGB
#endif

/*--------------------------------------------------------
Matrix for sYCC, Amendment 1 to IEC 61966-2-1
（RGB向YCbCr转换的矩阵系数）
Y :   0.299   0.587    0.114   :R
Cb:  -0.1687 -0.3312   0.5     :G
Cr:   0.5    -0.4187  -0.0812  :B

Inverse:

R: 1        -3.68213e-05    1.40199      :Y
G: 1.00003  -0.344125      -0.714128     :Cb - 2^(prec - 1)
B: 0.999823  1.77204       -8.04142e-06  :Cr - 2^(prec - 1)

-----------------------------------------------------------*/
static void sycc_to_rgb(int offset, int upb, int y, int cb, int cr,
	int *out_r, int *out_g, int *out_b)
{
	int r, g, b;

	cb -= offset; cr -= offset;
	r = y + (int)(1.402 * (float)cr);
	if(r < 0) r = 0; else if(r > upb) r = upb; *out_r = r;

	g = y - (int)(0.344 * (float)cb + 0.714 * (float)cr);
	if(g < 0) g = 0; else if(g > upb) g = upb; *out_g = g;

	b = y + (int)(1.772 * (float)cb);
	if(b < 0) b = 0; else if(b > upb) b = upb; *out_b = b;
}

static void sycc444_to_rgb(mi_image_t *img)
{
	int *d0, *d1, *d2, *r, *g, *b;
	const int *y, *cb, *cr;
	size_t maxw, maxh, max, i;
	int offset, upb;

	upb = (int)img->comps[0].prec;
	offset = 1<<(upb - 1); upb = (1<<upb)-1;

	maxw = (size_t)img->comps[0].w; maxh = (size_t)img->comps[0].h;
	max = maxw * maxh;

	y = img->comps[0].data;
	cb = img->comps[1].data;
	cr = img->comps[2].data;

	d0 = r = (int*)malloc(sizeof(int) * max);
	d1 = g = (int*)malloc(sizeof(int) * max);
	d2 = b = (int*)malloc(sizeof(int) * max);

	if(r == NULL || g == NULL || b == NULL) goto fails;

	for(i = 0U; i < max; ++i)
	{
		sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);
		++y; ++cb; ++cr; ++r; ++g; ++b;
	}
	free(img->comps[0].data); img->comps[0].data = d0;
	free(img->comps[1].data); img->comps[1].data = d1;
	free(img->comps[2].data); img->comps[2].data = d2;
	img->color_space = mi_CLRSPC_SRGB;
	return;

fails:
	free(r);
	free(g);
	free(b);
}/* sycc444_to_rgb() */

static void sycc422_to_rgb(mi_image_t *img)
{	
	int *d0, *d1, *d2, *r, *g, *b;
	const int *y, *cb, *cr;
	size_t maxw, maxh, max, offx, loopmaxw;
	int offset, upb;
	size_t i;

	upb = (int)img->comps[0].prec;
	offset = 1<<(upb - 1); upb = (1<<upb)-1;

	maxw = (size_t)img->comps[0].w; maxh = (size_t)img->comps[0].h;
	max = maxw * maxh;

	y = img->comps[0].data;
	cb = img->comps[1].data;
	cr = img->comps[2].data;

	d0 = r = (int*)malloc(sizeof(int) * max);
	d1 = g = (int*)malloc(sizeof(int) * max);
	d2 = b = (int*)malloc(sizeof(int) * max);

	if(r == NULL || g == NULL || b == NULL) goto fails;

	/* if img->x0 is odd, then first column shall use Cb/Cr = 0 */
	offx = img->x0 & 1U;
	loopmaxw = maxw - offx;
	
	for(i=0U; i < maxh; ++i)
	{
		size_t j;
		
		if (offx > 0U) {
			sycc_to_rgb(offset, upb, *y, 0, 0, r, g, b);
			++y; ++r; ++g; ++b;
		}
		
		for(j=0U; j < (loopmaxw & ~(size_t)1U); j += 2U)
		{
			sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);
			++y; ++r; ++g; ++b;
			sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);
			++y; ++r; ++g; ++b; ++cb; ++cr;
		}
		if (j < loopmaxw) {
			sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);
			++y; ++r; ++g; ++b; ++cb; ++cr;
		}
	}
	
	free(img->comps[0].data); img->comps[0].data = d0;
	free(img->comps[1].data); img->comps[1].data = d1;
	free(img->comps[2].data); img->comps[2].data = d2;

	img->comps[1].w = img->comps[2].w = img->comps[0].w;
	img->comps[1].h = img->comps[2].h = img->comps[0].h;
	img->comps[1].dx = img->comps[2].dx = img->comps[0].dx;
	img->comps[1].dy = img->comps[2].dy = img->comps[0].dy;
	img->color_space = mi_CLRSPC_SRGB;
	return;

fails:
	free(r);
	free(g);
	free(b);
}/* sycc422_to_rgb() */

static void sycc420_to_rgb(mi_image_t *img)
{
	int *d0, *d1, *d2, *r, *g, *b, *nr, *ng, *nb;
	const int *y, *cb, *cr, *ny;
	size_t maxw, maxh, max, offx, loopmaxw, offy, loopmaxh;
	int offset, upb;
	size_t i;

	upb = (int)img->comps[0].prec;
	offset = 1<<(upb - 1); upb = (1<<upb)-1;

	maxw = (size_t)img->comps[0].w; maxh = (size_t)img->comps[0].h;
	max = maxw * maxh;

	y = img->comps[0].data;
	cb = img->comps[1].data;
	cr = img->comps[2].data;

	d0 = r = (int*)malloc(sizeof(int) * max);
	d1 = g = (int*)malloc(sizeof(int) * max);
	d2 = b = (int*)malloc(sizeof(int) * max);
	
	if (r == NULL || g == NULL || b == NULL) goto fails;
	
	/* if img->x0 is odd, then first column shall use Cb/Cr = 0 */
	offx = img->x0 & 1U;
	loopmaxw = maxw - offx;
	/* if img->y0 is odd, then first line shall use Cb/Cr = 0 */
	offy = img->y0 & 1U;
	loopmaxh = maxh - offy;
	
	if (offy > 0U) {
		size_t j;
		
		for(j=0; j < maxw; ++j)
		{
			sycc_to_rgb(offset, upb, *y, 0, 0, r, g, b);
			++y; ++r; ++g; ++b;
		}
	}

	for(i=0U; i < (loopmaxh & ~(size_t)1U); i += 2U)
	{
		size_t j;
		
		ny = y + maxw;
		nr = r + maxw; ng = g + maxw; nb = b + maxw;
		
		if (offx > 0U) {
			sycc_to_rgb(offset, upb, *y, 0, 0, r, g, b);
			++y; ++r; ++g; ++b;
			sycc_to_rgb(offset, upb, *ny, *cb, *cr, nr, ng, nb);
			++ny; ++nr; ++ng; ++nb;
		}

		for(j=0; j < (loopmaxw & ~(size_t)1U); j += 2U)
		{
			sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);
			++y; ++r; ++g; ++b;
			sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);
			++y; ++r; ++g; ++b;

			sycc_to_rgb(offset, upb, *ny, *cb, *cr, nr, ng, nb);
			++ny; ++nr; ++ng; ++nb;
			sycc_to_rgb(offset, upb, *ny, *cb, *cr, nr, ng, nb);
			++ny; ++nr; ++ng; ++nb; ++cb; ++cr;
		}
		if(j < loopmaxw)
		{
			sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);
			++y; ++r; ++g; ++b;

			sycc_to_rgb(offset, upb, *ny, *cb, *cr, nr, ng, nb);
			++ny; ++nr; ++ng; ++nb; ++cb; ++cr;
		}
		y += maxw; r += maxw; g += maxw; b += maxw;
	}
	if(i < loopmaxh)
	{
		size_t j;
		
		for(j=0U; j < (maxw & ~(size_t)1U); j += 2U)
		{
			sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);

			++y; ++r; ++g; ++b;

			sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);

			++y; ++r; ++g; ++b; ++cb; ++cr;
		}
		if(j < maxw)
		{
			sycc_to_rgb(offset, upb, *y, *cb, *cr, r, g, b);
		}
	}

	free(img->comps[0].data); img->comps[0].data = d0;
	free(img->comps[1].data); img->comps[1].data = d1;
	free(img->comps[2].data); img->comps[2].data = d2;

	img->comps[1].w = img->comps[2].w = img->comps[0].w;
	img->comps[1].h = img->comps[2].h = img->comps[0].h;
	img->comps[1].dx = img->comps[2].dx = img->comps[0].dx;
	img->comps[1].dy = img->comps[2].dy = img->comps[0].dy;
	img->color_space = mi_CLRSPC_SRGB;
	return;

fails:
	free(r);
	free(g);
	free(b);
}/* sycc420_to_rgb() */

void color_sycc_to_rgb(mi_image_t *img)
{
	if(img->numcomps < 3)
	{
		img->color_space = mi_CLRSPC_GRAY;
		return;
	}

	if((img->comps[0].dx == 1)
	&& (img->comps[1].dx == 2)
	&& (img->comps[2].dx == 2)
	&& (img->comps[0].dy == 1)
	&& (img->comps[1].dy == 2)
	&& (img->comps[2].dy == 2))/* horizontal and vertical sub-sample */
  {
		sycc420_to_rgb(img);
  }
	else
	if((img->comps[0].dx == 1)
	&& (img->comps[1].dx == 2)
	&& (img->comps[2].dx == 2)
	&& (img->comps[0].dy == 1)
	&& (img->comps[1].dy == 1)
	&& (img->comps[2].dy == 1))/* horizontal sub-sample only */
  {
		sycc422_to_rgb(img);
  }
	else
	if((img->comps[0].dx == 1)
	&& (img->comps[1].dx == 1)
	&& (img->comps[2].dx == 1)
	&& (img->comps[0].dy == 1)
	&& (img->comps[1].dy == 1)
	&& (img->comps[2].dy == 1))/* no sub-sample */
  {
		sycc444_to_rgb(img);
  }
	else
  {
		fprintf(stderr,"%s:%d:color_sycc_to_rgb\n\tCAN NOT CONVERT\n", __FILE__,__LINE__);
		return;
  }
}/* color_sycc_to_rgb() */

void color_cmyk_to_rgb(mi_image_t *image)
{
	float C, M, Y, K;
	float sC, sM, sY, sK;
	unsigned int w, h, max, i;

	w = image->comps[0].w;
	h = image->comps[0].h;

	if (
			(image->numcomps < 4)
		 || (image->comps[0].dx != image->comps[1].dx) || (image->comps[0].dx != image->comps[2].dx) || (image->comps[0].dx != image->comps[3].dx)
		 || (image->comps[0].dy != image->comps[1].dy) || (image->comps[0].dy != image->comps[2].dy) || (image->comps[0].dy != image->comps[3].dy)
			) {
		fprintf(stderr,"%s:%d:color_cmyk_to_rgb\n\tCAN NOT CONVERT\n", __FILE__,__LINE__);
		return;
	}

	max = w * h;
	
	sC = 1.0F / (float)((1 << image->comps[0].prec) - 1);
	sM = 1.0F / (float)((1 << image->comps[1].prec) - 1);
	sY = 1.0F / (float)((1 << image->comps[2].prec) - 1);
	sK = 1.0F / (float)((1 << image->comps[3].prec) - 1);

	for(i = 0; i < max; ++i)
	{
		/* CMYK values from 0 to 1 */
		C = (float)(image->comps[0].data[i]) * sC;
		M = (float)(image->comps[1].data[i]) * sM;
		Y = (float)(image->comps[2].data[i]) * sY;
		K = (float)(image->comps[3].data[i]) * sK;
		
		/* Invert all CMYK values */
		C = 1.0F - C;
		M = 1.0F - M;
		Y = 1.0F - Y;
		K = 1.0F - K;

		/* CMYK -> RGB : RGB results from 0 to 255 */
		image->comps[0].data[i] = (int)(255.0F * C * K); /* R */
		image->comps[1].data[i] = (int)(255.0F * M * K); /* G */
		image->comps[2].data[i] = (int)(255.0F * Y * K); /* B */
	}

	free(image->comps[3].data); image->comps[3].data = NULL;
	image->comps[0].prec = 8;
	image->comps[1].prec = 8;
	image->comps[2].prec = 8;
	image->numcomps -= 1;
	image->color_space = mi_CLRSPC_SRGB;
	
	for (i = 3; i < image->numcomps; ++i) {
		memcpy(&(image->comps[i]), &(image->comps[i+1]), sizeof(image->comps[i]));
	}

}/* color_cmyk_to_rgb() */

/*
 * This code has been adopted from sjpx_openjpeg.c of ghostscript
 */
void color_esycc_to_rgb(mi_image_t *image)
{
	int y, cb, cr, sign1, sign2, val;
	unsigned int w, h, max, i;
	int flip_value = (1 << (image->comps[0].prec-1));
	int max_value = (1 << image->comps[0].prec) - 1;
	
	if (
		    (image->numcomps < 3)
		 || (image->comps[0].dx != image->comps[1].dx) || (image->comps[0].dx != image->comps[2].dx)
		 || (image->comps[0].dy != image->comps[1].dy) || (image->comps[0].dy != image->comps[2].dy)
	   ) {
		fprintf(stderr,"%s:%d:color_esycc_to_rgb\n\tCAN NOT CONVERT\n", __FILE__,__LINE__);
		return;
	}
	
	w = image->comps[0].w;
	h = image->comps[0].h;
	
	sign1 = (int)image->comps[1].sgnd;
	sign2 = (int)image->comps[2].sgnd;
	
	max = w * h;
	
	for(i = 0; i < max; ++i)
	{
		
		y = image->comps[0].data[i]; cb = image->comps[1].data[i]; cr = image->comps[2].data[i];
		
		if( !sign1) cb -= flip_value;
		if( !sign2) cr -= flip_value;
		
		val = (int)
		((float)y - (float)0.0000368 * (float)cb
		 + (float)1.40199 * (float)cr + (float)0.5);
		
		if(val > max_value) val = max_value; else if(val < 0) val = 0;
		image->comps[0].data[i] = val;
		
		val = (int)
		((float)1.0003 * (float)y - (float)0.344125 * (float)cb
		 - (float)0.7141128 * (float)cr + (float)0.5);
		
		if(val > max_value) val = max_value; else if(val < 0) val = 0;
		image->comps[1].data[i] = val;
		
		val = (int)
		((float)0.999823 * (float)y + (float)1.77204 * (float)cb
		 - (float)0.000008 *(float)cr + (float)0.5);
		
		if(val > max_value) val = max_value; else if(val < 0) val = 0;
		image->comps[2].data[i] = val;
	}
	image->color_space = mi_CLRSPC_SRGB;

}/* color_esycc_to_rgb() */
