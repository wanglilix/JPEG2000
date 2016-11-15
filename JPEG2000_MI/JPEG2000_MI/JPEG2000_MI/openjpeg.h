
#ifndef OPENJPEG_H
#define OPENJPEG_H


/* 
==========================================================
   Compiler directives（编译器指令）
==========================================================
*/

/*
The inline keyword is supported by C99 but not by C90. 
Most compilers implement their own version of this keyword ... 
*/
#ifndef INLINE
	#if defined(_MSC_VER)
		#define INLINE __forceinline
	#elif defined(__GNUC__)
		#define INLINE __inline__
	#elif defined(__MWERKS__)
		#define INLINE inline
	#else 
		/* add other compilers here ... */
		#define INLINE 
	#endif /* defined(<Compiler>) */
#endif /* INLINE */

/* deprecated attribute （过时的特性）*/
#ifdef __GNUC__
	#define mi_DEPRECATED(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
	#define mi_DEPRECATED(func) __declspec(deprecated) func
#else
	#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
	#define mi_DEPRECATED(func) func
#endif

#if defined(mi_STATIC) || !defined(_WIN32)
#	if __GNUC__ >= 4
#		if defined(mi_STATIC) /* static library uses "hidden" */
#			define mi_API    __attribute__ ((visibility ("hidden")))
#		else
#			define mi_API    __attribute__ ((visibility ("default")))
#		endif
#		define mi_LOCAL  __attribute__ ((visibility ("hidden")))
#	else
#		define mi_API
#		define mi_LOCAL
#	endif
#	define mi_CALLCONV
#else
#	define mi_CALLCONV __stdcall
/*
The following ifdef block is the standard way of creating macros which make exporting 
from a DLL simpler. All files within this DLL are compiled with the mi_EXPORTS
symbol defined on the command line. this symbol should not be defined on any project
that uses this DLL. This way any other project whose source files include this file see 
mi_API functions as being imported from a DLL, whereas this DLL sees symbols
defined with this macro as being exported.
*/
#	if defined(mi_EXPORTS) || defined(DLL_EXPORT)
#		define mi_API __declspec(dllexport)
#	else
#		define mi_API __declspec(dllimport)
#	endif /* mi_EXPORTS */
#endif /* !mi_STATIC || !_WIN32 */

typedef int mi_BOOL;
#define mi_TRUE 1
#define mi_FALSE 0

typedef char          mi_CHAR;
typedef float         mi_FLOAT32;
typedef double        mi_FLOAT64;
typedef unsigned char mi_BYTE;

#include "mi_stdint.h"

typedef int8_t   mi_INT8;
typedef uint8_t  mi_UINT8;
typedef int16_t  mi_INT16;
typedef uint16_t mi_UINT16;
typedef int32_t  mi_INT32;
typedef uint32_t mi_UINT32;
typedef int64_t  mi_INT64;
typedef uint64_t mi_UINT64;

typedef int64_t  mi_OFF_T; /* 64-bit file offset type */

#include <stdio.h>
typedef size_t   mi_SIZE_T;

/* Avoid compile-time warning because parameter is not used */
#define mi_ARG_NOT_USED(x) (void)(x)

/* 
==========================================================
   Useful constant definitions
==========================================================
*/

#define mi_PATH_LEN 4096 /**< Maximum allowed size for filenames */

#define mi_J2K_MAXRLVLS 33					/**< Number of maximum resolution level authorized */
#define mi_J2K_MAXBANDS (3*mi_J2K_MAXRLVLS-2)	/**< Number of maximum sub-band linked to number of resolution level */

#define mi_J2K_DEFAULT_NB_SEGS				10
#define mi_J2K_STREAM_CHUNK_SIZE			0x100000 /** 1 mega by default */
#define mi_J2K_DEFAULT_HEADER_SIZE			1000
#define mi_J2K_MCC_DEFAULT_NB_RECORDS		10
#define mi_J2K_MCT_DEFAULT_NB_RECORDS		10

/* UniPG>> */ /* NOT YET USED IN THE V2 VERSION OF OPENJPEG */
#define JPWL_MAX_NO_TILESPECS	16 /**< Maximum number of tile parts expected by JPWL: increase at your will */
#define JPWL_MAX_NO_PACKSPECS	16 /**< Maximum number of packet parts expected by JPWL: increase at your will */
#define JPWL_MAX_NO_MARKERS	512 /**< Maximum number of JPWL markers: increase at your will */
#define JPWL_PRIVATEINDEX_NAME "jpwl_index_privatefilename" /**< index file name used when JPWL is on */
#define JPWL_EXPECTED_COMPONENTS 3 /**< Expect this number of components, so you'll find better the first EPB */
#define JPWL_MAXIMUM_TILES 8192 /**< Expect this maximum number of tiles, to avoid some crashes */
#define JPWL_MAXIMUM_HAMMING 2 /**< Expect this maximum number of bit errors in marker id's */
#define JPWL_MAXIMUM_EPB_ROOM 65450 /**< Expect this maximum number of bytes for composition of EPBs */
/* <<UniPG */

/**
 * EXPERIMENTAL FOR THE MOMENT
 * Supported options about file information used only in j2k_dump
*/
#define mi_IMG_INFO		1	/**< Basic image information provided to the user */
#define mi_J2K_MH_INFO		2	/**< Codestream information based only on the main header */
#define mi_J2K_TH_INFO		4	/**< Tile information based on the current tile header */
#define mi_J2K_TCH_INFO	8	/**< Tile/Component information of all tiles */
#define mi_J2K_MH_IND		16	/**< Codestream index based only on the main header */
#define mi_J2K_TH_IND		32	/**< Tile index based on the current tile */
/*FIXME #define mi_J2K_CSTR_IND	48*/	/**<  */
#define mi_JP2_INFO		128	/**< JP2 file information */
#define mi_JP2_IND			256	/**< JP2 file index */

/**
 * JPEG 2000 Profiles, see Table A.10 from 15444-1 (updated in various AMD)
 * These values help chosing the RSIZ value for the J2K codestream.
 * The RSIZ value triggers various encoding options, as detailed in Table A.10.
 * If mi_PROFILE_PART2 is chosen, it has to be combined with one or more extensions
 * described hereunder.
 *   Example: rsiz = mi_PROFILE_PART2 | mi_EXTENSION_MCT;
 * For broadcast profiles, the mi_PROFILE value has to be combined with the targeted
 * mainlevel (3-0 LSB, value between 0 and 11):
 *   Example: rsiz = mi_PROFILE_BC_MULTI | 0x0005; (here mainlevel 5)
 * For IMF profiles, the mi_PROFILE value has to be combined with the targeted mainlevel
 * (3-0 LSB, value between 0 and 11) and sublevel (7-4 LSB, value between 0 and 9):
 *   Example: rsiz = mi_PROFILE_IMF_2K | 0x0040 | 0x0005; (here main 5 and sublevel 4)
 * */
#define mi_PROFILE_NONE        0x0000 /** no profile, conform to 15444-1 */
#define mi_PROFILE_0           0x0001 /** Profile 0 as described in 15444-1,Table A.45 */
#define mi_PROFILE_1           0x0002 /** Profile 1 as described in 15444-1,Table A.45 */
#define mi_PROFILE_PART2       0x8000 /** At least 1 extension defined in 15444-2 (Part-2) */
#define mi_PROFILE_CINEMA_2K   0x0003 /** 2K cinema profile defined in 15444-1 AMD1 */
#define mi_PROFILE_CINEMA_4K   0x0004 /** 4K cinema profile defined in 15444-1 AMD1 */
#define mi_PROFILE_CINEMA_S2K  0x0005 /** Scalable 2K cinema profile defined in 15444-1 AMD2 */
#define mi_PROFILE_CINEMA_S4K  0x0006 /** Scalable 4K cinema profile defined in 15444-1 AMD2 */
#define mi_PROFILE_CINEMA_LTS  0x0007 /** Long term storage cinema profile defined in 15444-1 AMD2 */
#define mi_PROFILE_BC_SINGLE   0x0100 /** Single Tile Broadcast profile defined in 15444-1 AMD3 */
#define mi_PROFILE_BC_MULTI    0x0200 /** Multi Tile Broadcast profile defined in 15444-1 AMD3 */
#define mi_PROFILE_BC_MULTI_R  0x0300 /** Multi Tile Reversible Broadcast profile defined in 15444-1 AMD3 */
#define mi_PROFILE_IMF_2K      0x0400 /** 2K Single Tile Lossy IMF profile defined in 15444-1 AMD 8 */
#define mi_PROFILE_IMF_4K      0x0401 /** 4K Single Tile Lossy IMF profile defined in 15444-1 AMD 8 */
#define mi_PROFILE_IMF_8K      0x0402 /** 8K Single Tile Lossy IMF profile defined in 15444-1 AMD 8 */
#define mi_PROFILE_IMF_2K_R    0x0403 /** 2K Single/Multi Tile Reversible IMF profile defined in 15444-1 AMD 8 */
#define mi_PROFILE_IMF_4K_R    0x0800 /** 4K Single/Multi Tile Reversible IMF profile defined in 15444-1 AMD 8 */
#define mi_PROFILE_IMF_8K_R    0x0801  /** 8K Single/Multi Tile Reversible IMF profile defined in 15444-1 AMD 8 */

/**
 * JPEG 2000 Part-2 extensions
 * */
#define mi_EXTENSION_NONE      0x0000 /** No Part-2 extension */
#define mi_EXTENSION_MCT       0x0100  /** Custom MCT support */

/**
 * JPEG 2000 profile macros
 * */
#define mi_IS_CINEMA(v)     (((v) >= mi_PROFILE_CINEMA_2K)&&((v) <= mi_PROFILE_CINEMA_S4K))
#define mi_IS_STORAGE(v)    ((v) == mi_PROFILE_CINEMA_LTS)
#define mi_IS_BROADCAST(v)  (((v) >= mi_PROFILE_BC_SINGLE)&&((v) <= ((mi_PROFILE_BC_MULTI_R) | (0x000b))))
#define mi_IS_IMF(v)        (((v) >= mi_PROFILE_IMF_2K)&&((v) <= ((mi_PROFILE_IMF_8K_R) | (0x009b))))
#define mi_IS_PART2(v)      ((v) & mi_PROFILE_PART2)

/**
 * JPEG 2000 codestream and component size limits in cinema profiles
 * */
#define mi_CINEMA_24_CS     1302083   	/** Maximum codestream length for 24fps */
#define mi_CINEMA_48_CS     651041     /** Maximum codestream length for 48fps */
#define mi_CINEMA_24_COMP   1041666    /** Maximum size per color component for 2K & 4K @ 24fps */
#define mi_CINEMA_48_COMP   520833		/** Maximum size per color component for 2K @ 48fps */

/* 
==========================================================
   enum definitions
==========================================================
*/

/**
 * DEPRECATED: use RSIZ, mi_PROFILE_* and mi_EXTENSION_* instead
 * Rsiz Capabilities
 * */
typedef enum RSIZ_CAPABILITIES {
	mi_STD_RSIZ = 0,		/** Standard JPEG2000 profile*/
	mi_CINEMA2K = 3,		/** Profile name for a 2K image*/
	mi_CINEMA4K = 4,		/** Profile name for a 4K image*/
	mi_MCT = 0x8100
} mi_RSIZ_CAPABILITIES;

/**
 * DEPRECATED: use RSIZ, mi_PROFILE_* and mi_EXTENSION_* instead
 * Digital cinema operation mode
 * */
typedef enum CINEMA_MODE {
	mi_OFF = 0,			/** Not Digital Cinema*/
	mi_CINEMA2K_24 = 1,	/** 2K Digital Cinema at 24 fps*/
	mi_CINEMA2K_48 = 2,	/** 2K Digital Cinema at 48 fps*/
	mi_CINEMA4K_24 = 3		/** 4K Digital Cinema at 24 fps*/
}mi_CINEMA_MODE;

/** 
 * Progression order
 * */
typedef enum PROG_ORDER {
	mi_PROG_UNKNOWN = -1,	/**< place-holder */
	mi_LRCP = 0,			/**< layer-resolution-component-precinct order */
	mi_RLCP = 1,			/**< resolution-layer-component-precinct order */
	mi_RPCL = 2,			/**< resolution-precinct-component-layer order */
	mi_PCRL = 3,			/**< precinct-component-resolution-layer order */
	mi_CPRL = 4			/**< component-precinct-resolution-layer order */
} mi_PROG_ORDER;

/**
 * Supported image color spaces
*/
typedef enum COLOR_SPACE {
	mi_CLRSPC_UNKNOWN = -1,	/**< not supported by the library */
	mi_CLRSPC_UNSPECIFIED = 0,	/**< not specified in the codestream */
	mi_CLRSPC_SRGB = 1,		/**< sRGB */
	mi_CLRSPC_GRAY = 2,		/**< grayscale */
	mi_CLRSPC_SYCC = 3,		/**< YUV */
	mi_CLRSPC_EYCC = 4,        /**< e-YCC */
	mi_CLRSPC_CMYK = 5         /**< CMYK */
} mi_COLOR_SPACE;

/**
 * Supported codec
*/
typedef enum CODEC_FORMAT {
	mi_CODEC_UNKNOWN = -1,	/**< place-holder */
	mi_CODEC_J2K  = 0,		/**< JPEG-2000 codestream : read/write */
	mi_CODEC_JPT  = 1,		/**< JPT-stream (JPEG 2000, JPIP) : read only */
	mi_CODEC_JP2  = 2,		/**< JP2 file format : read/write */
	mi_CODEC_JPP  = 3,		/**< JPP-stream (JPEG 2000, JPIP) : to be coded */
	mi_CODEC_JPX  = 4		/**< JPX file format (JPEG 2000 Part-2) : to be coded */
} mi_CODEC_FORMAT;


/* 
==========================================================
   event manager typedef definitions
==========================================================
*/

/**
 * Callback function prototype for events
 * @param msg               Event message
 * @param client_data       Client object where will be return the event message 
 * */
typedef void (*mi_msg_callback) (const char *msg, void *client_data);

/* 
==========================================================
   codec typedef definitions
==========================================================
*/

/**
 * Progression order changes
 * 
 */
typedef struct mi_poc {
	/** Resolution num start, Component num start, given by POC */
	mi_UINT32 resno0, compno0;
	/** Layer num end,Resolution num end, Component num end, given by POC */
	mi_UINT32 layno1, resno1, compno1;
	/** Layer num start,Precinct num start, Precinct num end */
	mi_UINT32 layno0, precno0, precno1;
	/** Progression order enum*/
	mi_PROG_ORDER prg1,prg;
	/** Progression order string*/
	mi_CHAR progorder[5];
	/** Tile number */
	mi_UINT32 tile;
	/** Start and end values for Tile width and height*/
	mi_INT32 tx0,tx1,ty0,ty1;
	/** Start value, initialised in pi_initialise_encode*/
	mi_UINT32 layS, resS, compS, prcS;
	/** End value, initialised in pi_initialise_encode */
	mi_UINT32 layE, resE, compE, prcE;
	/** Start and end values of Tile width and height, initialised in pi_initialise_encode*/
	mi_UINT32 txS,txE,tyS,tyE,dx,dy;
	/** Temporary values for Tile parts, initialised in pi_create_encode */
	mi_UINT32 lay_t, res_t, comp_t, prc_t,tx0_t,ty0_t;
} mi_poc_t;

/**
 * Compression parameters
 * */
typedef struct mi_cparameters {
	/** size of tile: tile_size_on = false (not in argument) or = true (in argument) */
	mi_BOOL tile_size_on;
	/** XTOsiz */
	int cp_tx0;
	/** YTOsiz */
	int cp_ty0;
	/** XTsiz */
	int cp_tdx;
	/** YTsiz */
	int cp_tdy;
	/** allocation by rate/distortion */
	int cp_disto_alloc;
	/** allocation by fixed layer */
	int cp_fixed_alloc;
	/** add fixed_quality */
	int cp_fixed_quality;
	/** fixed layer */
	int *cp_matrice;
	/** comment for coding */
	char *cp_comment;
	/** csty : coding style */
	int csty;
	/** progression order (default mi_LRCP) */
	mi_PROG_ORDER prog_order;
	/** progression order changes */
	mi_poc_t POC[32];
	/** number of progression order changes (POC), default to 0 */
	mi_UINT32 numpocs;
	/** number of layers */
	int tcp_numlayers;
	/** rates of layers - might be subsequently limited by the max_cs_size field */
	float tcp_rates[100];
	/** different psnr for successive layers */
	float tcp_distoratio[100];
	/** number of resolutions */
	int numresolution;
	/** initial code block width, default to 64 */
	int cblockw_init;
	/** initial code block height, default to 64 */
	int cblockh_init;
	/** mode switch (cblk_style) */
	int mode;
	/** 1 : use the irreversible DWT 9-7, 0 : use lossless compression (default) */
	int irreversible;
	/** region of interest: affected component in [0..3], -1 means no ROI */
	int roi_compno;
	/** region of interest: upshift value */
	int roi_shift;
	/* number of precinct size specifications 
	用户在命令行指定的区尺寸的数目
	（若数目比分辨率级数小则最后一个指定的区尺寸会用在之后的所有分辨率级数中，
	并且是减半后再用到之后所有的分辨率级数中，级数大一次减半一次）*/
	int res_spec;
	/** initial precinct width */
	int prcw_init[mi_J2K_MAXRLVLS];
	/** initial precinct height */
	int prch_init[mi_J2K_MAXRLVLS];

	/**@name command line encoder parameters (not used inside the library) */
	/*@{*/
	/** input file name */
	char infile[mi_PATH_LEN];
	/** output file name */
	char outfile[mi_PATH_LEN];
	/** DEPRECATED. Index generation is now handeld with the mi_encode_with_info() function. Set to NULL */
	int index_on;
	/** DEPRECATED. Index generation is now handeld with the mi_encode_with_info() function. Set to NULL */
	char index[mi_PATH_LEN];
	/** subimage encoding: origin image offset in x direction */
	int image_offset_x0;
	/** subimage encoding: origin image offset in y direction */
	int image_offset_y0;
	/** subsampling value for dx */
	int subsampling_dx;
	/** subsampling value for dy */
	int subsampling_dy;
	/** input file format 0: PGX, 1: PxM, 2: BMP 3:TIF*/
	int decod_format;
	/** output file format 0: J2K, 1: JP2, 2: JPT */
	int cod_format;
	/*@}*/

/* UniPG>> */ /* NOT YET USED IN THE V2 VERSION OF OPENJPEG */
	/**@name JPWL encoding parameters */
	/*@{*/
	/** enables writing of EPC in MH, thus activating JPWL */
	mi_BOOL jpwl_epc_on;
	/** error protection method for MH (0,1,16,32,37-128) */
	int jpwl_hprot_MH;
	/** tile number of header protection specification (>=0) */
	int jpwl_hprot_TPH_tileno[JPWL_MAX_NO_TILESPECS];
	/** error protection methods for TPHs (0,1,16,32,37-128) */
	int jpwl_hprot_TPH[JPWL_MAX_NO_TILESPECS];
	/** tile number of packet protection specification (>=0) */
	int jpwl_pprot_tileno[JPWL_MAX_NO_PACKSPECS];
	/** packet number of packet protection specification (>=0) */
	int jpwl_pprot_packno[JPWL_MAX_NO_PACKSPECS];
	/** error protection methods for packets (0,1,16,32,37-128) */
	int jpwl_pprot[JPWL_MAX_NO_PACKSPECS];
	/** enables writing of ESD, (0=no/1/2 bytes) */
	int jpwl_sens_size;
	/** sensitivity addressing size (0=auto/2/4 bytes) */
	int jpwl_sens_addr;
	/** sensitivity range (0-3) */
	int jpwl_sens_range;
	/** sensitivity method for MH (-1=no,0-7) */
	int jpwl_sens_MH;
	/** tile number of sensitivity specification (>=0) */
	int jpwl_sens_TPH_tileno[JPWL_MAX_NO_TILESPECS];
	/** sensitivity methods for TPHs (-1=no,0-7) */
	int jpwl_sens_TPH[JPWL_MAX_NO_TILESPECS];
	/*@}*/
/* <<UniPG */

	/**
	 * DEPRECATED: use RSIZ, mi_PROFILE_* and MAX_COMP_SIZE instead
	 * Digital Cinema compliance 0-not compliant, 1-compliant
	 * */
	mi_CINEMA_MODE cp_cinema;
	/**
	 * Maximum size (in bytes) for each component.
	 * If == 0, component size limitation is not considered
	 * */
	int max_comp_size;
	/**
	 * DEPRECATED: use RSIZ, mi_PROFILE_* and mi_EXTENSION_* instead
	 * Profile name
	 * */
	mi_RSIZ_CAPABILITIES cp_rsiz;
	/** Tile part generation*/
	char tp_on;
	/** Flag for Tile part generation*/
	char tp_flag;
	/** MCT (multiple component transform)（就是JPEG2000第4步：分量变换（多分量变换）） */
	char tcp_mct;
	/** Enable JPIP indexing*/
	mi_BOOL jpip_on;
	/** Naive implementation of MCT restricted to a single reversible array based 
		encoding without offset concerning all the components. */
	void * mct_data;
	/**
	 * Maximum size (in bytes) for the whole codestream.
	 * If == 0, codestream size limitation is not considered
	 * If it does not comply with tcp_rates, max_cs_size prevails
	 * and a warning is issued.
	 * */
	int max_cs_size;
	/** RSIZ value
		To be used to combine mi_PROFILE_*, mi_EXTENSION_* and (sub)levels values. */
	mi_UINT16 rsiz;
} mi_cparameters_t;  

#define mi_DPARAMETERS_IGNORE_PCLR_CMAP_CDEF_FLAG	0x0001

/**
 * Decompression parameters
 * */
typedef struct mi_dparameters {
	/** 
	Set the number of highest resolution levels to be discarded. 
	The image resolution is effectively divided by 2 to the power of the number of discarded levels. 
	The reduce factor is limited by the smallest total number of decomposition levels among tiles.
	if != 0, then original dimension divided by 2^(reduce); 
	if == 0 or not used, image is decoded to the full resolution 
	*/
	mi_UINT32 cp_reduce;
	/** 
	Set the maximum number of quality layers to decode. 
	If there are less quality layers than the specified number, all the quality layers are decoded.
	if != 0, then only the first "layer" layers are decoded; 
	if == 0 or not used, all the quality layers are decoded 
	*/
	mi_UINT32 cp_layer;

	/**@name command line decoder parameters (not used inside the library) */
	/*@{*/
	/** input file name */
	char infile[mi_PATH_LEN];
	/** output file name */
	char outfile[mi_PATH_LEN];
	/** input file format 0: J2K, 1: JP2, 2: JPT */
	int decod_format;
	/** output file format 0: PGX, 1: PxM, 2: BMP */
	int cod_format;

	/** Decoding area left boundary */
	mi_UINT32 DA_x0;
	/** Decoding area right boundary */
	mi_UINT32 DA_x1;
	/** Decoding area up boundary */
	mi_UINT32 DA_y0;
	/** Decoding area bottom boundary */
	mi_UINT32 DA_y1;
	/** Verbose mode */
	mi_BOOL m_verbose;

	/** tile number ot the decoded tile*/
	mi_UINT32 tile_index;
	/** Nb of tile to decode */
	mi_UINT32 nb_tile_to_decode;

	/*@}*/

/* UniPG>> */ /* NOT YET USED IN THE V2 VERSION OF OPENJPEG */
	/**@name JPWL decoding parameters */
	/*@{*/
	/** activates the JPWL correction capabilities */
	mi_BOOL jpwl_correct;
	/** expected number of components */
	int jpwl_exp_comps;
	/** maximum number of tiles */
	int jpwl_max_tiles;
	/*@}*/
/* <<UniPG */

	unsigned int flags;

} mi_dparameters_t;


/**
 * JPEG2000 codec V2.
 * */
typedef void * mi_codec_t;

/* 
==========================================================
   I/O stream typedef definitions
==========================================================
*/

/**
 * Stream open flags.
 * */
/** The stream was opened for reading. */
#define mi_STREAM_READ	mi_TRUE
/** The stream was opened for writing. */
#define mi_STREAM_WRITE mi_FALSE

/*
 * Callback function prototype for read function
 */
typedef mi_SIZE_T (* mi_stream_read_fn) (void * p_buffer, mi_SIZE_T p_nb_bytes, void * p_user_data) ;

/*
 * Callback function prototype for write function
 */
typedef mi_SIZE_T (* mi_stream_write_fn) (void * p_buffer, mi_SIZE_T p_nb_bytes, void * p_user_data) ;

/*
 * Callback function prototype for skip function
 */
typedef mi_OFF_T (* mi_stream_skip_fn) (mi_OFF_T p_nb_bytes, void * p_user_data) ;

/*
 * Callback function prototype for seek function
 */
typedef mi_BOOL (* mi_stream_seek_fn) (mi_OFF_T p_nb_bytes, void * p_user_data) ;

/*
 * Callback function prototype for free user data function
 */
typedef void (* mi_stream_free_user_data_fn) (void * p_user_data) ;

/*
 * JPEG2000 Stream.
 */
typedef void * mi_stream_t;

/* 
==========================================================
   image typedef definitions
==========================================================
*/

/**
 * Defines a single image component
 * */
typedef struct mi_image_comp {
	/** XRsiz: horizontal separation of a sample of ith component with respect to the reference grid */
	mi_UINT32 dx;
	/** YRsiz: vertical separation of a sample of ith component with respect to the reference grid */
	mi_UINT32 dy;
	/** data width */
	mi_UINT32 w;
	/** data height */
	mi_UINT32 h;
	/** x component offset compared to the whole image */
	mi_UINT32 x0;
	/** y component offset compared to the whole image */
	mi_UINT32 y0;
	/** precision */
	mi_UINT32 prec;
	/** image depth in bits */
	mi_UINT32 bpp;
	/** signed (1) / unsigned (0) */
	mi_UINT32 sgnd;
	/** number of decoded resolution */
	mi_UINT32 resno_decoded;
	/** number of division by 2 of the out image compared to the original size of image */
	mi_UINT32 factor;
	/** image component data */
	mi_INT32 *data;
  /** alpha channel */
  mi_UINT16 alpha;
} mi_image_comp_t;

/** 
 * Defines image data and characteristics
 * */
typedef struct mi_image {
	/** XOsiz: horizontal offset from the origin of the reference grid to the left side of the image area */
	mi_UINT32 x0;
	/** YOsiz: vertical offset from the origin of the reference grid to the top side of the image area */
	mi_UINT32 y0;
	/** Xsiz: width of the reference grid */
	mi_UINT32 x1;
	/** Ysiz: height of the reference grid */
	mi_UINT32 y1;
	/** number of components in the image */
	mi_UINT32 numcomps;
	/** color space: sRGB, Greyscale or YUV */
	mi_COLOR_SPACE color_space;
	/** image components */
	mi_image_comp_t *comps;
	/** 'restricted' ICC profile */
	mi_BYTE *icc_profile_buf;
	/** size of ICC profile */
	mi_UINT32 icc_profile_len;
} mi_image_t;


/**
 * Component parameters structure used by the mi_image_create function
 * */
typedef struct mi_image_comptparm {
	/** XRsiz: horizontal separation of a sample of ith component with respect to the reference grid */
	mi_UINT32 dx;
	/** YRsiz: vertical separation of a sample of ith component with respect to the reference grid */
	mi_UINT32 dy;
	/** data width */
	mi_UINT32 w;
	/** data height */
	mi_UINT32 h;
	/** x component offset compared to the whole image */
	mi_UINT32 x0;
	/** y component offset compared to the whole image */
	mi_UINT32 y0;
	/** precision */
	mi_UINT32 prec;
	/** image depth in bits */
	mi_UINT32 bpp;
	/** signed (1) / unsigned (0) */
	mi_UINT32 sgnd;
} mi_image_cmptparm_t;


/* 
==========================================================
   Information on the JPEG 2000 codestream
==========================================================
*/
/* QUITE EXPERIMENTAL FOR THE MOMENT */

/**
 * Index structure : Information concerning a packet inside tile
 * */
typedef struct mi_packet_info {
	/** packet start position (including SOP marker if it exists) */
	mi_OFF_T start_pos;
	/** end of packet header position (including EPH marker if it exists)*/
	mi_OFF_T end_ph_pos;
	/** packet end position */
	mi_OFF_T end_pos;
	/** packet distorsion */
	double disto;
} mi_packet_info_t;


/* UniPG>> */
/**
 * Marker structure
 * */
typedef struct mi_marker_info {
	/** marker type */
	unsigned short int type;
	/** position in codestream */
	mi_OFF_T pos;
	/** length, marker val included */
	int len;
} mi_marker_info_t;
/* <<UniPG */

/**
 * Index structure : Information concerning tile-parts
*/
typedef struct mi_tp_info {
	/** start position of tile part */
	int tp_start_pos;
	/** end position of tile part header */
	int tp_end_header;
	/** end position of tile part */
	int tp_end_pos;
	/** start packet of tile part */
	int tp_start_pack;
	/** number of packets of tile part */
	int tp_numpacks;
} mi_tp_info_t;

/**
 * Index structure : information regarding tiles
*/
typedef struct mi_tile_info {
	/** value of thresh for each layer by tile cfr. Marcela   */
	double *thresh;
	/** number of tile */
	int tileno;
	/** start position */
	int start_pos;
	/** end position of the header */
	int end_header;
	/** end position */
	int end_pos;
	/** precinct number for each resolution level (width) */
	int pw[33];
	/** precinct number for each resolution level (height) */
	int ph[33];
	/** precinct size (in power of 2), in X for each resolution level */
	int pdx[33];
	/** precinct size (in power of 2), in Y for each resolution level */
	int pdy[33];
	/** information concerning packets inside tile */
	mi_packet_info_t *packet;
	/** add fixed_quality */
	int numpix;
	/** add fixed_quality */
	double distotile;
	/** number of markers */
	int marknum;
	/** list of markers */
	mi_marker_info_t *marker;
	/** actual size of markers array */
	int maxmarknum;
	/** number of tile parts */
	int num_tps;
	/** information concerning tile parts */
	mi_tp_info_t *tp;
} mi_tile_info_t;

/**
 * Index structure of the codestream
*/
typedef struct mi_codestream_info {
	/** maximum distortion reduction on the whole image (add for Marcela) */
	double D_max;
	/** packet number */
	int packno;
	/** writing the packet in the index with t2_encode_packets */
	int index_write;
	/** image width */
	int image_w;
	/** image height */
	int image_h;
	/** progression order */
	mi_PROG_ORDER prog;
	/** tile size in x */
	int tile_x;
	/** tile size in y */
	int tile_y;
	/** */
	int tile_Ox;
	/** */
	int tile_Oy;
	/** number of tiles in X */
	int tw;
	/** number of tiles in Y */
	int th;
	/** component numbers */
	int numcomps;
	/** number of layer */
	int numlayers;
	/** number of decomposition for each component */
	int *numdecompos;
/* UniPG>> */
	/** number of markers */
	int marknum;
	/** list of markers */
	mi_marker_info_t *marker;
	/** actual size of markers array */
	int maxmarknum;
/* <<UniPG */
	/** main header position */
	int main_head_start;
	/** main header position */
	int main_head_end;
	/** codestream's size */
	int codestream_size;
	/** information regarding tiles inside image */
	mi_tile_info_t *tile;
} mi_codestream_info_t;

/* <----------------------------------------------------------- */
/* new output management of the codestream information and index */

/**
 * Tile-component coding parameters information
 */
typedef struct mi_tccp_info
{
	/** component index */
	mi_UINT32 compno;
	/** coding style */
	mi_UINT32 csty;
	/** number of resolutions */
	mi_UINT32 numresolutions;
	/** code-blocks width */
	mi_UINT32 cblkw;
	/** code-blocks height */
	mi_UINT32 cblkh;
	/** code-block coding style */
	mi_UINT32 cblksty;
	/** discrete wavelet transform identifier */
	mi_UINT32 qmfbid;
	/** quantisation style */
	mi_UINT32 qntsty;
	/** stepsizes used for quantization */
	mi_UINT32 stepsizes_mant[mi_J2K_MAXBANDS];
	/** stepsizes used for quantization */
	mi_UINT32 stepsizes_expn[mi_J2K_MAXBANDS];
	/** number of guard bits */
	mi_UINT32 numgbits;
	/** Region Of Interest shift */
	mi_INT32 roishift;
	/** precinct width */
	mi_UINT32 prcw[mi_J2K_MAXRLVLS];
	/** precinct height */
	mi_UINT32 prch[mi_J2K_MAXRLVLS];
}
mi_tccp_info_t;

/**
 * Tile coding parameters information
 */
typedef struct mi_tile_v2_info {

	/** number (index) of tile */
	int tileno;
	/** coding style */
	mi_UINT32 csty;
	/** progression order */
	mi_PROG_ORDER prg;
	/** number of layers */
	mi_UINT32 numlayers;
	/** multi-component transform identifier */
	mi_UINT32 mct;

	/** information concerning tile component parameters*/
	mi_tccp_info_t *tccp_info;

} mi_tile_info_v2_t;

/**
 * Information structure about the codestream (FIXME should be expand and enhance)
 */
typedef struct mi_codestream_info_v2 {
	/* Tile info */
	/** tile origin in x = XTOsiz */
	mi_UINT32 tx0;
	/** tile origin in y = YTOsiz */
	mi_UINT32 ty0;
	/** tile size in x = XTsiz */
	mi_UINT32 tdx;
	/** tile size in y = YTsiz */
	mi_UINT32 tdy;
	/** number of tiles in X */
	mi_UINT32 tw;
	/** number of tiles in Y */
	mi_UINT32 th;

	/** number of components*/
	mi_UINT32 nbcomps;

	/** Default information regarding tiles inside image */
	mi_tile_info_v2_t m_default_tile_info;

	/** information regarding tiles inside image */
	mi_tile_info_v2_t *tile_info; /* FIXME not used for the moment */

} mi_codestream_info_v2_t;


/**
 * Index structure about a tile part
 */
typedef struct mi_tp_index {
	/** start position */
	mi_OFF_T start_pos;
	/** end position of the header */
	mi_OFF_T end_header;
	/** end position */
	mi_OFF_T end_pos;

} mi_tp_index_t;

/**
 * Index structure about a tile
 */
typedef struct mi_tile_index {
	/** tile index */
	mi_UINT32 tileno;

	/** number of tile parts */
	mi_UINT32 nb_tps;
	/** current nb of tile part (allocated)*/
	mi_UINT32 current_nb_tps;
	/** current tile-part index */
	mi_UINT32 current_tpsno;
	/** information concerning tile parts */
	mi_tp_index_t *tp_index;

	/* UniPG>> */ /* NOT USED FOR THE MOMENT IN THE V2 VERSION */
		/** number of markers */
		mi_UINT32 marknum;
		/** list of markers */
		mi_marker_info_t *marker;
		/** actual size of markers array */
		mi_UINT32 maxmarknum;
	/* <<UniPG */

	/** packet number */
	mi_UINT32 nb_packet;
	/** information concerning packets inside tile */
	mi_packet_info_t *packet_index;

} mi_tile_index_t;

/**
 * Index structure of the codestream (FIXME should be expand and enhance)
 */
typedef struct mi_codestream_index {
	/** main header start position (SOC position) */
	mi_OFF_T main_head_start;
	/** main header end position (first SOT position) */
	mi_OFF_T main_head_end;

	/** codestream's size */
	mi_UINT64 codestream_size;

/* UniPG>> */ /* NOT USED FOR THE MOMENT IN THE V2 VERSION */
	/** number of markers */
	mi_UINT32 marknum;
	/** list of markers */
	mi_marker_info_t *marker;
	/** actual size of markers array */
	mi_UINT32 maxmarknum;
/* <<UniPG */

	/** */
	mi_UINT32 nb_of_tiles;
	/** */
	mi_tile_index_t *tile_index; /* FIXME not used for the moment */

}mi_codestream_index_t;
/* -----------------------------------------------------------> */

/*
==========================================================
   Metadata from the JP2file
==========================================================
*/

/**
 * Info structure of the JP2 file
 * EXPERIMENTAL FOR THE MOMENT
 */
typedef struct mi_jp2_metadata {
	/** */
	mi_INT32	not_used;

} mi_jp2_metadata_t;

/**
 * Index structure of the JP2 file
 * EXPERIMENTAL FOR THE MOMENT
 */
typedef struct mi_jp2_index {
	/** */
	mi_INT32	not_used;

} mi_jp2_index_t;


#ifdef __cplusplus
extern "C" {
#endif


/* 
==========================================================
   openjpeg version
==========================================================
*/

/* Get the version of the openjpeg library*/
/*mi_API*/ const char * mi_CALLCONV mi_version(void);

/* 
==========================================================
   image functions definitions
==========================================================
*/

/**
 * Create an image
 *
 * @param numcmpts      number of components
 * @param cmptparms     components parameters
 * @param clrspc        image color space
 * @return returns      a new image structure if successful, returns NULL otherwise
 * */
/*mi_API*/ mi_image_t* mi_CALLCONV mi_image_create(mi_UINT32 numcmpts, mi_image_cmptparm_t *cmptparms, mi_COLOR_SPACE clrspc);

/**
 * Deallocate any resources associated with an image
 *
 * @param image         image to be destroyed
 */
/*mi_API*/ void mi_CALLCONV mi_image_destroy(mi_image_t *image);

/**
 * Creates an image without allocating memory for the image (used in the new version of the library).
 *
 * @param	numcmpts    the number of components
 * @param	cmptparms   the components parameters
 * @param	clrspc      the image color space
 *
 * @return	a new image structure if successful, NULL otherwise.
*/
/*mi_API*/ mi_image_t* mi_CALLCONV mi_image_tile_create(mi_UINT32 numcmpts, mi_image_cmptparm_t *cmptparms, mi_COLOR_SPACE clrspc);

/* 
==========================================================
   stream functions definitions
==========================================================
*/

/**
 * Creates an abstract stream. This function does nothing except allocating memory and initializing the abstract stream.
 *
 * @param	p_is_input		if set to true then the stream will be an input stream, an output stream else.
 *
 * @return	a stream object.
*/
/*mi_API*/ mi_stream_t* mi_CALLCONV mi_stream_default_create(mi_BOOL p_is_input);

/**
 * Creates an abstract stream. This function does nothing except allocating memory and initializing the abstract stream.
 *
 * @param	p_buffer_size  FIXME DOC
 * @param	p_is_input		if set to true then the stream will be an input stream, an output stream else.
 *
 * @return	a stream object.
*/
/*mi_API*/ mi_stream_t* mi_CALLCONV mi_stream_create(mi_SIZE_T p_buffer_size, mi_BOOL p_is_input);

/**
 * Destroys a stream created by mi_create_stream. This function does NOT close the abstract stream. If needed the user must
 * close its own implementation of the stream.
 *
 * @param	p_stream	the stream to destroy.
 */
/*mi_API*/ void mi_CALLCONV mi_stream_destroy(mi_stream_t* p_stream);
 
/**
 * Sets the given function to be used as a read function.
 * @param		p_stream	the stream to modify
 * @param		p_function	the function to use a read function.
*/
/*mi_API*/ void mi_CALLCONV mi_stream_set_read_function(mi_stream_t* p_stream, mi_stream_read_fn p_function);

/**
 * Sets the given function to be used as a write function.
 * @param		p_stream	the stream to modify
 * @param		p_function	the function to use a write function.
*/
/*mi_API*/ void mi_CALLCONV mi_stream_set_write_function(mi_stream_t* p_stream, mi_stream_write_fn p_function);

/**
 * Sets the given function to be used as a skip function.
 * @param		p_stream	the stream to modify
 * @param		p_function	the function to use a skip function.
*/
/*mi_API*/ void mi_CALLCONV mi_stream_set_skip_function(mi_stream_t* p_stream, mi_stream_skip_fn p_function);

/**
 * Sets the given function to be used as a seek function, the stream is then seekable.
 * @param		p_stream	the stream to modify
 * @param		p_function	the function to use a skip function.
*/
/*mi_API*/ void mi_CALLCONV mi_stream_set_seek_function(mi_stream_t* p_stream, mi_stream_seek_fn p_function);

/**
 * Sets the given data to be used as a user data for the stream.
 * @param		p_stream	the stream to modify
 * @param		p_data		the data to set.
 * @param		p_function	the function to free p_data when mi_stream_destroy() is called.
*/
/*mi_API*/ void mi_CALLCONV mi_stream_set_user_data (mi_stream_t* p_stream, void * p_data, mi_stream_free_user_data_fn p_function);

/**
 * Sets the length of the user data for the stream.
 *
 * @param p_stream    the stream to modify
 * @param data_length length of the user_data.
*/
/*mi_API*/ void mi_CALLCONV mi_stream_set_user_data_length(mi_stream_t* p_stream, mi_UINT64 data_length);

/**
 * Create a stream from a file identified with its filename with default parameters (helper function)
 * @param fname             the filename of the file to stream
 * @param p_is_read_stream  whether the stream is a read stream (true) or not (false)
*/
/*mi_API*/ mi_stream_t* mi_CALLCONV mi_stream_create_default_file_stream (const char *fname, mi_BOOL p_is_read_stream);
 
/** Create a stream from a file identified with its filename with a specific buffer size
 * @param fname             the filename of the file to stream
 * @param p_buffer_size     size of the chunk used to stream
 * @param p_is_read_stream  whether the stream is a read stream (true) or not (false)
*/
/*mi_API*/ mi_stream_t* mi_CALLCONV mi_stream_create_file_stream (const char *fname,
																	 mi_SIZE_T p_buffer_size,
																	 mi_BOOL p_is_read_stream);
 
/* 
==========================================================
   event manager functions definitions
==========================================================
*/
/**
 * Set the info handler use by openjpeg.
 * @param p_codec       the codec previously initialise
 * @param p_callback    the callback function which will be used
 * @param p_user_data   client object where will be returned the message
*/
/*mi_API*/ mi_BOOL mi_CALLCONV mi_set_info_handler(mi_codec_t * p_codec, 
												   mi_msg_callback p_callback,
												   void * p_user_data);
/**
 * Set the warning handler use by openjpeg.
 * @param p_codec       the codec previously initialise
 * @param p_callback    the callback function which will be used
 * @param p_user_data   client object where will be returned the message
*/
/*mi_API*/ mi_BOOL mi_CALLCONV mi_set_warning_handler(mi_codec_t * p_codec,
													  mi_msg_callback p_callback,
													  void * p_user_data);
/**
 * Set the error handler use by openjpeg.
 * @param p_codec       the codec previously initialise
 * @param p_callback    the callback function which will be used
 * @param p_user_data   client object where will be returned the message
*/
/*mi_API*/ mi_BOOL mi_CALLCONV mi_set_error_handler(mi_codec_t * p_codec, 
													mi_msg_callback p_callback,
													void * p_user_data);

/* 
==========================================================
   codec functions definitions
==========================================================
*/

/**
 * Creates a J2K/JP2 decompression structure
 * @param format 		Decoder to select
 *
 * @return Returns a handle to a decompressor if successful, returns NULL otherwise
 * */
/*mi_API*/ mi_codec_t* mi_CALLCONV mi_create_decompress(mi_CODEC_FORMAT format);

/**
 * Destroy a decompressor handle
 *
 * @param	p_codec			decompressor handle to destroy
 */
/*mi_API*/ void mi_CALLCONV mi_destroy_codec(mi_codec_t * p_codec);

/**
 * Read after the codestream if necessary
 * @param	p_codec			the JPEG2000 codec to read.
 * @param	p_stream		the JPEG2000 stream.
 */
/*mi_API*/ mi_BOOL mi_CALLCONV mi_end_decompress (	mi_codec_t *p_codec,
													mi_stream_t *p_stream);


/**
 * Set decoding parameters to default values
 * @param parameters Decompression parameters
 */
/*mi_API*/ void mi_CALLCONV mi_set_default_decoder_parameters(mi_dparameters_t *parameters);

/**
 * Setup the decoder with decompression parameters provided by the user and with the message handler
 * provided by the user.
 *
 * @param p_codec 		decompressor handler
 * @param parameters 	decompression parameters
 *
 * @return true			if the decoder is correctly set
 */
/*mi_API*/ mi_BOOL mi_CALLCONV mi_setup_decoder(mi_codec_t *p_codec,
												mi_dparameters_t *parameters );

/**
 * Decodes an image header.
 *
 * @param	p_stream		the jpeg2000 stream.
 * @param	p_codec			the jpeg2000 codec to read.
 * @param	p_image			the image structure initialized with the characteristics of encoded image.
 *
 * @return true				if the main header of the codestream and the JP2 header is correctly read.
 */
/*mi_API*/ mi_BOOL mi_CALLCONV mi_read_header (	mi_stream_t *p_stream,
												mi_codec_t *p_codec,
												mi_image_t **p_image);

/**
 * Sets the given area to be decoded. This function should be called right after mi_read_header and before any tile header reading.
 *
 * @param	p_codec			the jpeg2000 codec.
 * @param	p_image         the decoded image previously setted by mi_read_header
 * @param	p_start_x		the left position of the rectangle to decode (in image coordinates).
 * @param	p_end_x			the right position of the rectangle to decode (in image coordinates).
 * @param	p_start_y		the up position of the rectangle to decode (in image coordinates).
 * @param	p_end_y			the bottom position of the rectangle to decode (in image coordinates).
 *
 * @return	true			if the area could be set.
 */
/*mi_API*/ mi_BOOL mi_CALLCONV mi_set_decode_area(	mi_codec_t *p_codec,
													mi_image_t* p_image,
													mi_INT32 p_start_x, mi_INT32 p_start_y,
													mi_INT32 p_end_x, mi_INT32 p_end_y );

/**
 * Decode an image from a JPEG-2000 codestream
 *
 * @param p_decompressor 	decompressor handle
 * @param p_stream			Input buffer stream
 * @param p_image 			the decoded image
 * @return 					true if success, otherwise false
 * */
/*mi_API*/ mi_BOOL mi_CALLCONV mi_decode(   mi_codec_t *p_decompressor,
											mi_stream_t *p_stream,
											mi_image_t *p_image);

/**
 * Get the decoded tile from the codec
 *
 * @param	p_codec			the jpeg2000 codec.
 * @param	p_stream		input streamm
 * @param	p_image			output image
 * @param	tile_index		index of the tile which will be decode
 *
 * @return					true if success, otherwise false
 */
/*mi_API*/ mi_BOOL mi_CALLCONV mi_get_decoded_tile(	mi_codec_t *p_codec,
													mi_stream_t *p_stream,
													mi_image_t *p_image,
													mi_UINT32 tile_index);

/**
 * Set the resolution factor of the decoded image
 * @param	p_codec			the jpeg2000 codec.
 * @param	res_factor		resolution factor to set
 *
 * @return					true if success, otherwise false
 */
/*mi_API*/ mi_BOOL mi_CALLCONV mi_set_decoded_resolution_factor(mi_codec_t *p_codec, mi_UINT32 res_factor);

/**
 * Writes a tile with the given data.
 *
 * @param	p_codec		        the jpeg2000 codec.
 * @param	p_tile_index		the index of the tile to write. At the moment, the tiles must be written from 0 to n-1 in sequence.
 * @param	p_data				pointer to the data to write. Data is arranged in sequence, data_comp0, then data_comp1, then ... NO INTERLEAVING should be set.
 * @param	p_data_size			this value os used to make sure the data being written is correct. The size must be equal to the sum for each component of 
 *                              tile_width * tile_height * component_size. component_size can be 1,2 or 4 bytes, depending on the precision of the given component.
 * @param	p_stream			the stream to write data to.
 *
 * @return	true if the data could be written.
 */
/*mi_API*/ mi_BOOL mi_CALLCONV mi_write_tile (	mi_codec_t *p_codec,
												mi_UINT32 p_tile_index,
												mi_BYTE * p_data,
												mi_UINT32 p_data_size,
												mi_stream_t *p_stream );

/**
 * Reads a tile header. This function is compulsory and allows one to know the size of the tile that will be decoded.
 * The user may need to refer to the image got by mi_read_header to understand the size being taken by the tile.
 *
 * @param	p_codec			the jpeg2000 codec.
 * @param	p_tile_index	pointer to a value that will hold the index of the tile being decoded, in case of success.
 * @param	p_data_size		pointer to a value that will hold the maximum size of the decoded data, in case of success. In case
 *							of truncated codestreams, the actual number of bytes decoded may be lower. The computation of the size is the same
 *							as depicted in mi_write_tile.
 * @param	p_tile_x0		pointer to a value that will hold the x0 pos of the tile (in the image).
 * @param	p_tile_y0		pointer to a value that will hold the y0 pos of the tile (in the image).
 * @param	p_tile_x1		pointer to a value that will hold the x1 pos of the tile (in the image).
 * @param	p_tile_y1		pointer to a value that will hold the y1 pos of the tile (in the image).
 * @param	p_nb_comps		pointer to a value that will hold the number of components in the tile.
 * @param	p_should_go_on	pointer to a boolean that will hold the fact that the decoding should go on. In case the
 *							codestream is over at the time of the call, the value will be set to false. The user should then stop
 *							the decoding.
 * @param	p_stream		the stream to decode.
 * @return	true			if the tile header could be decoded. In case the decoding should end, the returned value is still true.
 *							returning false may be the result of a shortage of memory or an internal error.
 */
/*mi_API*/ mi_BOOL mi_CALLCONV mi_read_tile_header(	mi_codec_t *p_codec,
												mi_stream_t * p_stream,
												mi_UINT32 * p_tile_index,
												mi_UINT32 * p_data_size,
												mi_INT32 * p_tile_x0, mi_INT32 * p_tile_y0,
												mi_INT32 * p_tile_x1, mi_INT32 * p_tile_y1,
												mi_UINT32 * p_nb_comps,
												mi_BOOL * p_should_go_on );

/**
 * Reads a tile data. This function is compulsory and allows one to decode tile data. mi_read_tile_header should be called before.
 * The user may need to refer to the image got by mi_read_header to understand the size being taken by the tile.
 *
 * @param	p_codec			the jpeg2000 codec.
 * @param	p_tile_index	the index of the tile being decoded, this should be the value set by mi_read_tile_header.
 * @param	p_data			pointer to a memory block that will hold the decoded data.
 * @param	p_data_size		size of p_data. p_data_size should be bigger or equal to the value set by mi_read_tile_header.
 * @param	p_stream		the stream to decode.
 *
 * @return	true			if the data could be decoded.
 */
/*mi_API*/ mi_BOOL mi_CALLCONV mi_decode_tile_data(	mi_codec_t *p_codec,
													mi_UINT32 p_tile_index,
													mi_BYTE * p_data,
													mi_UINT32 p_data_size,
													mi_stream_t *p_stream );

/* COMPRESSION FUNCTIONS*/

/**
 * Creates a J2K/JP2 compression structure
 * @param 	format 		Coder to select
 * @return 				Returns a handle to a compressor if successful, returns NULL otherwise
 */
/*mi_API*/ mi_codec_t* mi_CALLCONV mi_create_compress(mi_CODEC_FORMAT format);

/**
Set encoding parameters to default values, that means : 
<ul>
<li>Lossless
<li>1 tile
<li>Size of precinct : 2^15 x 2^15 (means 1 precinct)
<li>Size of code-block : 64 x 64
<li>Number of resolutions: 6
<li>No SOP marker in the codestream
<li>No EPH marker in the codestream
<li>No sub-sampling in x or y direction
<li>No mode switch activated
<li>Progression order: LRCP
<li>No index file
<li>No ROI upshifted
<li>No offset of the origin of the image
<li>No offset of the origin of the tiles
<li>Reversible DWT 5-3
</ul>
@param parameters Compression parameters
*/
/*mi_API*/ void mi_CALLCONV mi_set_default_encoder_parameters(mi_cparameters_t *parameters);

/**
 * Setup the encoder parameters using the current image and using user parameters.
 * @param p_codec 		Compressor handle
 * @param parameters 	Compression parameters
 * @param image 		Input filled image
 */
/*mi_API*/ mi_BOOL mi_CALLCONV mi_setup_encoder(mi_codec_t *p_codec, 
												mi_cparameters_t *parameters, 
												mi_image_t *image);

/**
 * Start to compress the current image.
 * @param p_codec 		Compressor handle
 * @param image 	    Input filled image
 * @param p_stream 		Input stgream
 */
/*mi_API*/ mi_BOOL mi_CALLCONV mi_start_compress (	mi_codec_t *p_codec,
													mi_image_t * p_image,
													mi_stream_t *p_stream);

/**
 * End to compress the current image.
 * @param p_codec 		Compressor handle
 * @param p_stream 		Input stgream
 */
/*mi_API*/ mi_BOOL mi_CALLCONV mi_end_compress (mi_codec_t *p_codec,
												mi_stream_t *p_stream);

/**
 * Encode an image into a JPEG-2000 codestream
 * @param p_codec 		compressor handle
 * @param p_stream 		Output buffer stream
 *
 * @return 				Returns true if successful, returns false otherwise
 */
/*mi_API*/ mi_BOOL mi_CALLCONV mi_encode(mi_codec_t *p_codec,
										 mi_stream_t *p_stream);
/*
==========================================================
   codec output functions definitions
==========================================================
*/
/* EXPERIMENTAL FUNCTIONS FOR NOW, USED ONLY IN J2K_DUMP*/

/**
Destroy Codestream information after compression or decompression
@param cstr_info Codestream information structure
*/
/*mi_API*/ void mi_CALLCONV mi_destroy_cstr_info(mi_codestream_info_v2_t **cstr_info);


/**
 * Dump the codec information into the output stream
 *
 * @param	p_codec			the jpeg2000 codec.
 * @param	info_flag		type of information dump.
 * @param	output_stream	output stream where dump the informations get from the codec.
 *
 */
/*mi_API*/ void mi_CALLCONV mi_dump_codec(	mi_codec_t *p_codec,
											mi_INT32 info_flag,
											FILE* output_stream);

/**
 * Get the codestream information from the codec
 *
 * @param	p_codec			the jpeg2000 codec.
 *
 * @return					a pointer to a codestream information structure.
 *
 */
/*mi_API*/ mi_codestream_info_v2_t* mi_CALLCONV mi_get_cstr_info(mi_codec_t *p_codec);

/**
 * Get the codestream index from the codec
 *
 * @param	p_codec			the jpeg2000 codec.
 *
 * @return					a pointer to a codestream index structure.
 *
 */
/*mi_API*/ mi_codestream_index_t * mi_CALLCONV mi_get_cstr_index(mi_codec_t *p_codec);

/*mi_API*/ void mi_CALLCONV mi_destroy_cstr_index(mi_codestream_index_t **p_cstr_index);


/**
 * Get the JP2 file information from the codec FIXME
 *
 * @param	p_codec			the jpeg2000 codec.
 *
 * @return					a pointer to a JP2 metadata structure.
 *
 */
/*mi_API*/ mi_jp2_metadata_t* mi_CALLCONV mi_get_jp2_metadata(mi_codec_t *p_codec);

/**
 * Get the JP2 file index from the codec FIXME
 *
 * @param	p_codec			the jpeg2000 codec.
 *
 * @return					a pointer to a JP2 index structure.
 *
 */
/*mi_API*/ mi_jp2_index_t* mi_CALLCONV mi_get_jp2_index(mi_codec_t *p_codec);


/*
==========================================================
   MCT functions
==========================================================
*/

/**
 * Sets the MCT matrix to use.
 *
 * @param	parameters		the parameters to change.
 * @param	pEncodingMatrix	the encoding matrix.
 * @param	p_dc_shift		the dc shift coefficients to use.
 * @param	pNbComp			the number of components of the image.
 *
 * @return	true if the parameters could be set.
 */
/*mi_API*/ mi_BOOL mi_CALLCONV mi_set_MCT( mi_cparameters_t *parameters,
										   mi_FLOAT32 * pEncodingMatrix,
										   mi_INT32 * p_dc_shift,
										   mi_UINT32 pNbComp);



#ifdef __cplusplus
}
#endif

#endif /* OPENJPEG_H */
