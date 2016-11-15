
#ifndef _mi_COLOR_H_
#define _mi_COLOR_H_
#include"openjpeg.h"
extern void color_sycc_to_rgb(mi_image_t *img);
extern void color_apply_icc_profile(mi_image_t *image);
extern void color_cielab_to_rgb(mi_image_t *image);

extern void color_cmyk_to_rgb(mi_image_t *image);
extern void color_esycc_to_rgb(mi_image_t *image);
#endif /* _mi_COLOR_H_ */
