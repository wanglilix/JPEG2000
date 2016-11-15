
#ifndef __IMAGE_H
#define __IMAGE_H
#include"openjpeg.h"
/**
@file image.h
@brief Implementation of operations on images (IMAGE)

The functions in IMAGE.C have for goal to realize operations on images.
*/

struct mi_image;
struct mi_cp;

/** @defgroup IMAGE IMAGE - Implementation of operations on images */
/*@{*/

/**
 * Create an empty image
 *
 * @return returns an empty image if successful, returns NULL otherwise
 */
mi_image_t* mi_image_create0(void);



/**
 * Updates the components characteristics of the image from the coding parameters.
 *
 * @param p_image_header		the image header to update.
 * @param p_cp					the coding parameters from which to update the image.
 */
void mi_image_comp_header_update(mi_image_t * p_image, const struct mi_cp* p_cp);

void mi_copy_image_header(const mi_image_t* p_image_src, mi_image_t* p_image_dest);

/*@}*/

#endif /* __IMAGE_H */

