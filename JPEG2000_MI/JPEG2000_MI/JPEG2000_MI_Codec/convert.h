
#ifndef __J2K_CONVERT_H
#define __J2K_CONVERT_H
#include"openjpeg.h"
/**@name RAW component encoding parameters */
/*@{*/
typedef struct raw_comp_cparameters {
    /** subsampling in X direction */
    int dx;
    /** subsampling in Y direction */
    int dy;
    /*@}*/
} raw_comp_cparameters_t;

/**@name RAW image encoding parameters */
/*@{*/
typedef struct raw_cparameters {
	/** width of the raw image */
	int rawWidth;
	/** height of the raw image */
	int rawHeight;
    /** number of components of the raw image */
	int rawComp;
    /** bit depth of the raw image */
    int rawBitDepth;
    /** signed/unsigned raw image */
    mi_BOOL rawSigned;
    /** raw components parameters */
    raw_comp_cparameters_t *rawComps;
	/*@}*/
} raw_cparameters_t;

/* Component precision clipping */
void clip_component(mi_image_comp_t* component, mi_UINT32 precision);
/* Component precision scaling */
void scale_component(mi_image_comp_t* component, mi_UINT32 precision);

/* planar / interleaved conversions */
typedef void (* convert_32s_CXPX)(const mi_INT32* pSrc, mi_INT32* const* pDst, mi_SIZE_T length);
extern const convert_32s_CXPX convert_32s_CXPX_LUT[5];
typedef void (* convert_32s_PXCX)(mi_INT32 const* const* pSrc, mi_INT32* pDst, mi_SIZE_T length, mi_INT32 adjust);
extern const convert_32s_PXCX convert_32s_PXCX_LUT[5];
/* bit depth conversions */
typedef void (* convert_XXx32s_C1R)(const mi_BYTE* pSrc, mi_INT32* pDst, mi_SIZE_T length);
extern const convert_XXx32s_C1R convert_XXu32s_C1R_LUT[9]; /* up to 8bpp */
typedef void (* convert_32sXXx_C1R)(const mi_INT32* pSrc, mi_BYTE* pDst, mi_SIZE_T length);
extern const convert_32sXXx_C1R convert_32sXXu_C1R_LUT[9]; /* up to 8bpp */


/* TGA conversion */
mi_image_t* tgatoimage(const char *filename, mi_cparameters_t *parameters);
int imagetotga(mi_image_t * image, const char *outfile);

/* BMP conversion */
mi_image_t* bmptoimage(const char *filename, mi_cparameters_t *parameters);
int imagetobmp(mi_image_t *image, const char *outfile);

/* TIFF conversion*/
mi_image_t* tiftoimage(const char *filename, mi_cparameters_t *parameters);
int imagetotif(mi_image_t *image, const char *outfile);
/**
Load a single image component encoded in PGX file format
@param filename Name of the PGX file to load
@param parameters *List ?*
@return Returns a greyscale image if successful, returns NULL otherwise
*/
mi_image_t* pgxtoimage(const char *filename, mi_cparameters_t *parameters);
int imagetopgx(mi_image_t *image, const char *outfile);

mi_image_t* pnmtoimage(const char *filename, mi_cparameters_t *parameters);
int imagetopnm(mi_image_t *image, const char *outfile, int force_split);

/* RAW conversion */
int imagetoraw(mi_image_t * image, const char *outfile);
int imagetorawl(mi_image_t * image, const char *outfile);
mi_image_t* rawtoimage(const char *filename, mi_cparameters_t *parameters, raw_cparameters_t *raw_cp);
mi_image_t* rawltoimage(const char *filename, mi_cparameters_t *parameters, raw_cparameters_t *raw_cp);

/* PNG conversion*/
extern int imagetopng(mi_image_t *image, const char *write_idf);
extern mi_image_t* pngtoimage(const char *filename, mi_cparameters_t *parameters);

#endif /* __J2K_CONVERT_H */

