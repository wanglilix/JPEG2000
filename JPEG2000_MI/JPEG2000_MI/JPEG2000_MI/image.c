
#include "mi_includes.h"
#include"mi_config_private.h"
mi_image_t* mi_image_create0(void) {
	mi_image_t *image = (mi_image_t*)mi_calloc(1, sizeof(mi_image_t));
	return image;
}
//图像分量初始化
mi_image_t* mi_CALLCONV mi_image_create(mi_UINT32 numcmpts, mi_image_cmptparm_t *cmptparms, mi_COLOR_SPACE clrspc) {
	mi_UINT32 compno;
	mi_image_t *image = NULL;

	image = (mi_image_t*) mi_calloc(1, sizeof(mi_image_t));
	if(image) {
		image->color_space = clrspc;
		image->numcomps = numcmpts;
		/* allocate memory for the per-component information */
		image->comps = (mi_image_comp_t*)mi_calloc(1,image->numcomps * sizeof(mi_image_comp_t));
		if(!image->comps) {
			/* TODO replace with event manager, breaks API */
			/* fprintf(stderr,"Unable to allocate memory for image.\n"); */
			mi_image_destroy(image);
			return NULL;
		}
		/* create the individual image components */
		for(compno = 0; compno < numcmpts; compno++) {
			mi_image_comp_t *comp = &image->comps[compno];
			comp->dx = cmptparms[compno].dx;
			comp->dy = cmptparms[compno].dy;
			comp->w = cmptparms[compno].w;
			comp->h = cmptparms[compno].h;
			comp->x0 = cmptparms[compno].x0;
			comp->y0 = cmptparms[compno].y0;
			comp->prec = cmptparms[compno].prec;
			comp->bpp = cmptparms[compno].bpp;
			comp->sgnd = cmptparms[compno].sgnd;
			comp->data = (mi_INT32*) mi_calloc(comp->w * comp->h, sizeof(mi_INT32));
			if(!comp->data) {
				/* TODO replace with event manager, breaks API */
				/* fprintf(stderr,"Unable to allocate memory for image.\n"); */
				mi_image_destroy(image);
				return NULL;
			}
		}
	}

	return image;
}

void mi_CALLCONV mi_image_destroy(mi_image_t *image) {
	if(image) {
		if(image->comps) {
			mi_UINT32 compno;

			/* image components */
			for(compno = 0; compno < image->numcomps; compno++) {
				mi_image_comp_t *image_comp = &(image->comps[compno]);
				if(image_comp->data) {
					mi_free(image_comp->data);
				}
			}
			mi_free(image->comps);
		}

		if(image->icc_profile_buf) {
			mi_free(image->icc_profile_buf);
		}

		mi_free(image);
	}
}

/**
 * Updates the components characteristics of the image from the coding parameters.
 *
 * @param p_image_header	the image header to update.
 * @param p_cp				the coding parameters from which to update the image.
 */
void mi_image_comp_header_update(mi_image_t * p_image_header, const struct mi_cp * p_cp)
{
	mi_UINT32 i, l_width, l_height;
	mi_UINT32 l_x0, l_y0, l_x1, l_y1;
	mi_UINT32 l_comp_x0, l_comp_y0, l_comp_x1, l_comp_y1;
	mi_image_comp_t* l_img_comp = NULL;

	l_x0 = mi_uint_max(p_cp->tx0 , p_image_header->x0);
	l_y0 = mi_uint_max(p_cp->ty0 , p_image_header->y0);
	l_x1 = p_cp->tx0 + (p_cp->tw - 1U) * p_cp->tdx; /* validity of p_cp members used here checked in mi_j2k_read_siz. Can't overflow. */
	l_y1 = p_cp->ty0 + (p_cp->th - 1U) * p_cp->tdy; /* can't overflow */
	l_x1 = mi_uint_min(mi_uint_adds(l_x1, p_cp->tdx), p_image_header->x1); /* use add saturated to prevent overflow */
	l_y1 = mi_uint_min(mi_uint_adds(l_y1, p_cp->tdy), p_image_header->y1); /* use add saturated to prevent overflow */

	l_img_comp = p_image_header->comps;
	for	(i = 0; i < p_image_header->numcomps; ++i) {
		l_comp_x0 = mi_uint_ceildiv(l_x0, l_img_comp->dx);
		l_comp_y0 = mi_uint_ceildiv(l_y0, l_img_comp->dy);
		l_comp_x1 = mi_uint_ceildiv(l_x1, l_img_comp->dx);
		l_comp_y1 = mi_uint_ceildiv(l_y1, l_img_comp->dy);
		l_width   = mi_uint_ceildivpow2(l_comp_x1 - l_comp_x0, l_img_comp->factor);
		l_height  = mi_uint_ceildivpow2(l_comp_y1 - l_comp_y0, l_img_comp->factor);
		l_img_comp->w = l_width;
		l_img_comp->h = l_height;
		l_img_comp->x0 = l_comp_x0;
		l_img_comp->y0 = l_comp_y0;
		++l_img_comp;
	}
}


/**
 * Copy only header of image and its component header (no data are copied)
 * if dest image have data, they will be freed
 *
 * @param	p_image_src		the src image
 * @param	p_image_dest	the dest image
 *
 */
void mi_copy_image_header(const mi_image_t* p_image_src, mi_image_t* p_image_dest)
{
	mi_UINT32 compno;

	/* preconditions */
	assert(p_image_src != 00);
	assert(p_image_dest != 00);

	p_image_dest->x0 = p_image_src->x0;
	p_image_dest->y0 = p_image_src->y0;
	p_image_dest->x1 = p_image_src->x1;
	p_image_dest->y1 = p_image_src->y1;

	if (p_image_dest->comps){
		for(compno = 0; compno < p_image_dest->numcomps; compno++) {
			mi_image_comp_t *image_comp = &(p_image_dest->comps[compno]);
			if(image_comp->data) {
				mi_free(image_comp->data);
			}
		}
		mi_free(p_image_dest->comps);
		p_image_dest->comps = NULL;
	}

	p_image_dest->numcomps = p_image_src->numcomps;

	p_image_dest->comps = (mi_image_comp_t*) mi_malloc(p_image_dest->numcomps * sizeof(mi_image_comp_t));
	if (!p_image_dest->comps){
		p_image_dest->comps = NULL;
		p_image_dest->numcomps = 0;
		return;
	}

	for (compno=0; compno < p_image_dest->numcomps; compno++){
		memcpy( &(p_image_dest->comps[compno]),
				&(p_image_src->comps[compno]),
				sizeof(mi_image_comp_t));
		p_image_dest->comps[compno].data = NULL;
	}

	p_image_dest->color_space = p_image_src->color_space;
	p_image_dest->icc_profile_len = p_image_src->icc_profile_len;

	if (p_image_dest->icc_profile_len) {
		p_image_dest->icc_profile_buf = (mi_BYTE*)mi_malloc(p_image_dest->icc_profile_len);
		if (!p_image_dest->icc_profile_buf){
			p_image_dest->icc_profile_buf = NULL;
			p_image_dest->icc_profile_len = 0;
			return;
		}
		memcpy( p_image_dest->icc_profile_buf,
				p_image_src->icc_profile_buf,
				p_image_src->icc_profile_len);
		}
		else
			p_image_dest->icc_profile_buf = NULL;

	return;
}

mi_image_t* mi_CALLCONV mi_image_tile_create(mi_UINT32 numcmpts, mi_image_cmptparm_t *cmptparms, mi_COLOR_SPACE clrspc) {
	mi_UINT32 compno;
	mi_image_t *image = 00;

	image = (mi_image_t*) mi_calloc(1,sizeof(mi_image_t));
	if (image)
	{
		
		image->color_space = clrspc;
		image->numcomps = numcmpts;
		
		/* allocate memory for the per-component information */
		image->comps = (mi_image_comp_t*)mi_calloc(image->numcomps, sizeof(mi_image_comp_t));
		if (!image->comps) {
			mi_image_destroy(image);
			return 00;
		}
		
		/* create the individual image components */
		for(compno = 0; compno < numcmpts; compno++) {
			mi_image_comp_t *comp = &image->comps[compno];
			comp->dx = cmptparms[compno].dx;
			comp->dy = cmptparms[compno].dy;
			comp->w = cmptparms[compno].w;
			comp->h = cmptparms[compno].h;
			comp->x0 = cmptparms[compno].x0;
			comp->y0 = cmptparms[compno].y0;
			comp->prec = cmptparms[compno].prec;
			comp->sgnd = cmptparms[compno].sgnd;
			comp->data = 0;
		}
	}

	return image;
}
