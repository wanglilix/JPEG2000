
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#ifdef _WIN32
#include "windirent.h"
#else
#include <dirent.h>
#endif /* _WIN32 */

#ifdef _WIN32
#include <windows.h>
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#else
#include <strings.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/times.h>
#endif /* _WIN32 */

//#include "mi_apps_config.h"
#include "openjpeg.h"
#include "mi_getopt.h"
#include "convert.h"
#include "index.h"

#include "format_defs.h"
#include "mi_string.h"
//缓冲区相关信息
typedef struct dircnt{
    /** Buffer for holding images read from Directory*/
    char *filename_buf;
    /** Pointer to the buffer*/
    char **filename;
}dircnt_t;

typedef struct img_folder{
    /** The directory path of the folder containing input images*/
    char *imgdirpath;
    /** Output format*/
    char *out_format;
    /** Enable option*/
    char set_imgdir;
    /** Enable Cod Format for output*/
    char set_out_format;
}img_fol_t;

static mi_PROG_ORDER give_progression(const char progression[4]) {
    if(strncmp(progression, "LRCP", 4) == 0) {
        return mi_LRCP;
    }
    if(strncmp(progression, "RLCP", 4) == 0) {
        return mi_RLCP;
    }
    if(strncmp(progression, "RPCL", 4) == 0) {
        return mi_RPCL;
    }
    if(strncmp(progression, "PCRL", 4) == 0) {
        return mi_PCRL;
    }
    if(strncmp(progression, "CPRL", 4) == 0) {
        return mi_CPRL;
    }

    return mi_PROG_UNKNOWN;
}

static unsigned int get_num_images(char *imgdirpath){
    DIR *dir;
    struct dirent* content;
    unsigned int num_images = 0;

    /*Reading the input images from given input directory*/

    dir= opendir(imgdirpath);
    if(!dir){
        fprintf(stderr,"Could not open Folder %s\n",imgdirpath);
        return 0;
    }

    num_images=0;
    while((content=readdir(dir))!=NULL){
        if(strcmp(".",content->d_name)==0 || strcmp("..",content->d_name)==0 )
            continue;
        num_images++;
    }
    closedir(dir);
    return num_images;
}

static int load_images(dircnt_t *dirptr, char *imgdirpath){
    DIR *dir;
    struct dirent* content;
    int i = 0;

    /*Reading the input images from given input directory*/

    dir= opendir(imgdirpath);
    if(!dir){
        fprintf(stderr,"Could not open Folder %s\n",imgdirpath);
        return 1;
    }else	{
        fprintf(stderr,"Folder opened successfully\n");
    }

    while((content=readdir(dir))!=NULL){
        if(strcmp(".",content->d_name)==0 || strcmp("..",content->d_name)==0 )
            continue;

        strcpy(dirptr->filename[i],content->d_name);
        i++;
    }
	closedir(dir);
    return 0;
}

static int get_file_format(char *filename) {
    unsigned int i;
    static const char *extension[] = {
        "pgx", "pnm", "pgm", "ppm", "pbm", "pam", "bmp", "tif", "raw", "rawl", "tga", "png", "j2k", "jp2", "j2c", "jpc"
    };
    static const int format[] = {
        PGX_DFMT, PXM_DFMT, PXM_DFMT, PXM_DFMT, PXM_DFMT, PXM_DFMT, BMP_DFMT, TIF_DFMT, RAW_DFMT, RAWL_DFMT, TGA_DFMT, PNG_DFMT, J2K_CFMT, JP2_CFMT, J2K_CFMT, J2K_CFMT
    };
    char * ext = strrchr(filename, '.');
    if (ext == NULL)
        return -1;
    ext++;
    for(i = 0; i < sizeof(format)/sizeof(*format); i++) {
        if(strcasecmp(ext, extension[i]) == 0) {
            return format[i];
        }
    }
    return -1;
}

static char * get_file_name(char *name){
    char *fname = strtok(name,".");
    return fname;
}

static char get_next_file(int imageno,dircnt_t *dirptr,img_fol_t *img_fol, mi_cparameters_t *parameters){
    char image_filename[mi_PATH_LEN], infilename[mi_PATH_LEN],outfilename[mi_PATH_LEN],temp_ofname[mi_PATH_LEN];
    char *temp_p, temp1[mi_PATH_LEN]="";

    strcpy(image_filename,dirptr->filename[imageno]);
    fprintf(stderr,"File Number %d \"%s\"\n",imageno,image_filename);
    parameters->decod_format = get_file_format(image_filename);
    if (parameters->decod_format == -1)
        return 1;
    sprintf(infilename,"%s/%s",img_fol->imgdirpath,image_filename);
    if (mi_strcpy_s(parameters->infile, sizeof(parameters->infile), infilename) != 0) {
        return 1;
    }
	
    /*Set output file*/
    strcpy(temp_ofname,get_file_name(image_filename));
    while((temp_p = strtok(NULL,".")) != NULL){
        strcat(temp_ofname,temp1);
        sprintf(temp1,".%s",temp_p);
    }
    if(img_fol->set_out_format==1){
        sprintf(outfilename,"%s/%s.%s",img_fol->imgdirpath,temp_ofname,img_fol->out_format);
        if (mi_strcpy_s(parameters->outfile, sizeof(parameters->outfile), outfilename) != 0) {
            return 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------------------------ */
//分析命令行字符串
static int parse_cmdline_encoder(
	int argc, 
	char **argv, 
	mi_cparameters_t *parameters,
    img_fol_t *img_fol, 
	raw_cparameters_t *raw_cp, 
	char *indexfilename,
	size_t indexfilename_size) {

    mi_UINT32 i, j;
    int totlen, c;
    mi_option_t long_option[]={
        {"cinema2K",REQ_ARG, NULL ,'w'},
        {"cinema4K",NO_ARG, NULL ,'y'},
        {"ImgDir",REQ_ARG, NULL ,'z'},
        {"TP",REQ_ARG, NULL ,'u'},
        {"SOP",NO_ARG, NULL ,'S'},
        {"EPH",NO_ARG, NULL ,'E'},
        {"OutFor",REQ_ARG, NULL ,'O'},
        {"POC",REQ_ARG, NULL ,'P'},
        {"ROI",REQ_ARG, NULL ,'R'},
        {"jpip",NO_ARG, NULL, 'J'},
        {"mct",REQ_ARG, NULL, 'Y'}
    };

    /* parse the command line */
    const char optlist[] = "i:o:r:q:n:b:c:t:p:s:SEM:x:R:d:T:If:P:C:F:u:JY:"
        #ifdef USE_JPWL
            "W:"
        #endif /* USE_JPWL */
            "h";

    totlen=sizeof(long_option);
    img_fol->set_out_format=0;
    raw_cp->rawWidth = 0;

    do{
        c = mi_getopt_long(argc, argv, optlist,long_option,totlen);
        if (c == -1)
            break;
        switch (c) {
        case 'i':			/* input file */
        {
            char *infile = mi_optarg;
            parameters->decod_format = get_file_format(infile);
            switch(parameters->decod_format) {
            case PGX_DFMT:
            case PXM_DFMT:
            case BMP_DFMT:
            case TIF_DFMT:
            case RAW_DFMT:
            case RAWL_DFMT:
            case TGA_DFMT:
            case PNG_DFMT:
                break;
            default:
                fprintf(stderr,
                        "[ERROR] Unknown input file format: %s \n"
                        "        Known file formats are *.pnm, *.pgm, *.ppm, *.pgx, *png, *.bmp, *.tif, *.raw or *.tga\n",
                        infile);
                return 1;
            }
            if (mi_strcpy_s(parameters->infile, sizeof(parameters->infile), infile) != 0) {
                return 1;
            }
        }
            break;

            /* ----------------------------------------------------- */

        case 'o':			/* output file */
        {
            char *outfile = mi_optarg;
            parameters->cod_format = get_file_format(outfile);
            switch(parameters->cod_format) {
            case J2K_CFMT:
            case JP2_CFMT:
                break;
            default:
                fprintf(stderr, "Unknown output format image %s [only *.j2k, *.j2c or *.jp2]!! \n", outfile);
                return 1;
            }
            if (mi_strcpy_s(parameters->outfile, sizeof(parameters->outfile), outfile) != 0) {
                return 1;
            }
        }
            break;

            /* ----------------------------------------------------- */
        case 'O':			/* output format */
        {
            char outformat[50];
            char *of = mi_optarg;
            sprintf(outformat,".%s",of);
            img_fol->set_out_format = 1;
            parameters->cod_format = get_file_format(outformat);
            switch(parameters->cod_format) {
            case J2K_CFMT:
            case JP2_CFMT:
                img_fol->out_format = mi_optarg;
                break;
            default:
                fprintf(stderr, "Unknown output format image [only j2k, j2c, jp2]!! \n");
                return 1;
            }
        }
            break;


            /* ----------------------------------------------------- */


        case 'r':			/* rates rates/distorsion */
        {
            char *s = mi_optarg;
            parameters->tcp_numlayers = 0;
            while (sscanf(s, "%f", &parameters->tcp_rates[parameters->tcp_numlayers]) == 1) {
                parameters->tcp_numlayers++;
                while (*s && *s != ',') {
                    s++;
                }
                if (!*s)
                    break;
                s++;
            }
            parameters->cp_disto_alloc = 1;
        }
            break;

            /* ----------------------------------------------------- */


        case 'F':			/* Raw image format parameters */
        {
            mi_BOOL wrong = mi_FALSE;
            char *substr1;
            char *substr2;
            char *sep;
            char signo;
            int width,height,bitdepth,ncomp;
            mi_UINT32 len;
            mi_BOOL raw_signed = mi_FALSE;
            substr2 = strchr(mi_optarg,'@');
            if (substr2 == NULL) {
                len = (mi_UINT32) strlen(mi_optarg);
            } else {
                len = (mi_UINT32) (substr2 - mi_optarg);
                substr2++; /* skip '@' character */
            }
            substr1 = (char*) malloc((len+1)*sizeof(char));
            if (substr1 == NULL) {
                return 1;
            }
            memcpy(substr1,mi_optarg,len);
            substr1[len] = '\0';
            if (sscanf(substr1, "%d,%d,%d,%d,%c", &width, &height, &ncomp, &bitdepth, &signo) == 5) {
                if (signo == 's') {
                    raw_signed = mi_TRUE;
                } else if (signo == 'u') {
                    raw_signed = mi_FALSE;
                } else {
                    wrong = mi_TRUE;
                }
            } else {
                wrong = mi_TRUE;
            }
            if (!wrong) {
                int compno;
                int lastdx = 1;
                int lastdy = 1;
                raw_cp->rawWidth = width;
                raw_cp->rawHeight = height;
                raw_cp->rawComp = ncomp;
                raw_cp->rawBitDepth = bitdepth;
                raw_cp->rawSigned  = raw_signed;
                raw_cp->rawComps = (raw_comp_cparameters_t*) malloc(((mi_UINT32)(ncomp))*sizeof(raw_comp_cparameters_t));
				if(raw_cp->rawComps == NULL){
					free(substr1);
					return 1;
				}
                for (compno = 0; compno < ncomp && !wrong; compno++) {
                    if (substr2 == NULL) {
                        raw_cp->rawComps[compno].dx = lastdx;
                        raw_cp->rawComps[compno].dy = lastdy;
                    } else {
                        int dx,dy;
                        sep = strchr(substr2,':');
                        if (sep == NULL) {
                            if (sscanf(substr2, "%dx%d", &dx, &dy) == 2) {
                                lastdx = dx;
                                lastdy = dy;
                                raw_cp->rawComps[compno].dx = dx;
                                raw_cp->rawComps[compno].dy = dy;
                                substr2 = NULL;
                            } else {
                                wrong = mi_TRUE;
                            }
                        } else {
                            if (sscanf(substr2, "%dx%d:%s", &dx, &dy, substr2) == 3) {
                                raw_cp->rawComps[compno].dx = dx;
                                raw_cp->rawComps[compno].dy = dy;
                            } else {
                                wrong = mi_TRUE;
                            }
                        }
                    }
                }
            }
            free(substr1);
            if (wrong) {
                fprintf(stderr,"\nError: invalid raw image parameters\n");
                fprintf(stderr,"Please use the Format option -F:\n");
                fprintf(stderr,"-F <width>,<height>,<ncomp>,<bitdepth>,{s,u}@<dx1>x<dy1>:...:<dxn>x<dyn>\n");
                fprintf(stderr,"If subsampling is omitted, 1x1 is assumed for all components\n");
                fprintf(stderr,"Example: -i image.raw -o image.j2k -F 512,512,3,8,u@1x1:2x2:2x2\n");
                fprintf(stderr,"         for raw 512x512 image with 4:2:0 subsampling\n");
                fprintf(stderr,"Aborting.\n");
                return 1;
            }
        }
            break;

            /* ----------------------------------------------------- */

        case 'q':			/* add fixed_quality */
        {
            char *s = mi_optarg;
            while (sscanf(s, "%f", &parameters->tcp_distoratio[parameters->tcp_numlayers]) == 1) {
                parameters->tcp_numlayers++;
                while (*s && *s != ',') {
                    s++;
                }
                if (!*s)
                    break;
                s++;
            }
            parameters->cp_fixed_quality = 1;
        }
            break;

            /* dda */
            /* ----------------------------------------------------- */

        case 'f':			/* mod fixed_quality (before : -q) */
        {
            int *row = NULL, *col = NULL;
            mi_UINT32 numlayers = 0, numresolution = 0, matrix_width = 0;

            char *s = mi_optarg;
            sscanf(s, "%u", &numlayers);
            s++;
            if (numlayers > 9)
                s++;

            parameters->tcp_numlayers = (int)numlayers;
            numresolution = (mi_UINT32)parameters->numresolution;
            matrix_width = numresolution * 3;
            parameters->cp_matrice = (int *) malloc(numlayers * matrix_width * sizeof(int));
			if(parameters->cp_matrice == NULL){
				return 1;
			}
            s = s + 2;

            for (i = 0; i < numlayers; i++) {
                row = &parameters->cp_matrice[i * matrix_width];
                col = row;
                parameters->tcp_rates[i] = 1;
                sscanf(s, "%d,", &col[0]);
                s += 2;
                if (col[0] > 9)
                    s++;
                col[1] = 0;
                col[2] = 0;
                for (j = 1; j < numresolution; j++) {
                    col += 3;
                    sscanf(s, "%d,%d,%d", &col[0], &col[1], &col[2]);
                    s += 6;
                    if (col[0] > 9)
                        s++;
                    if (col[1] > 9)
                        s++;
                    if (col[2] > 9)
                        s++;
                }
                if (i < numlayers - 1)
                    s++;
            }
            parameters->cp_fixed_alloc = 1;
        }
            break;

            /* ----------------------------------------------------- */

        case 't':			/* tiles */
        {
            sscanf(mi_optarg, "%d,%d", &parameters->cp_tdx, &parameters->cp_tdy);
            parameters->tile_size_on = mi_TRUE;
        }
            break;

            /* ----------------------------------------------------- */

        case 'n':			/* resolution */
        {
            sscanf(mi_optarg, "%d", &parameters->numresolution);
        }
            break;

            /* ----------------------------------------------------- */
        case 'c':			/* precinct dimension */
        {
            char sep;
            int res_spec = 0;

            char *s = mi_optarg;
            int ret;
            do {
                sep = 0;
                ret = sscanf(s, "[%d,%d]%c", &parameters->prcw_init[res_spec],
                       &parameters->prch_init[res_spec], &sep);
                if( !(ret == 2 && sep == 0) && !(ret == 3 && sep == ',') )
                  {
                  fprintf(stderr,"\nError: could not parse precinct dimension: '%s' %x\n", s, sep);
                  fprintf(stderr,"Example: -i lena.raw -o lena.j2k -c [128,128],[128,128]\n");
                  return 1;
                  }
                parameters->csty |= 0x01;
                res_spec++;
                s = strpbrk(s, "]") + 2;
            } while (sep == ',');
            parameters->res_spec = res_spec;
        }
            break;

            /* ----------------------------------------------------- */

        case 'b':			/* code-block dimension */
        {
            int cblockw_init = 0, cblockh_init = 0;
            sscanf(mi_optarg, "%d,%d", &cblockw_init, &cblockh_init);
            if (cblockw_init * cblockh_init > 4096 || cblockw_init > 1024
                    || cblockw_init < 4 || cblockh_init > 1024 || cblockh_init < 4) {
                fprintf(stderr,
                        "!! Size of code_block error (option -b) !!\n\nRestriction :\n"
                        "    * width*height<=4096\n    * 4<=width,height<= 1024\n\n");
                return 1;
            }
            parameters->cblockw_init = cblockw_init;
            parameters->cblockh_init = cblockh_init;
        }
            break;

            /* ----------------------------------------------------- */

        case 'x':			/* creation of index file */
        {
            if (mi_strcpy_s(indexfilename, indexfilename_size, mi_optarg) != 0) {
                return 1;
            }
            /* FIXME ADE INDEX >> */
            fprintf(stderr,
                    "[WARNING] Index file generation is currently broken.\n"
                    "          '-x' option ignored.\n");
            /* << FIXME ADE INDEX */
        }
            break;

            /* ----------------------------------------------------- */

        case 'p':			/* progression order */
        {
            char progression[4];

            strncpy(progression, mi_optarg, 4);
            parameters->prog_order = give_progression(progression);
            if (parameters->prog_order == -1) {
                fprintf(stderr, "Unrecognized progression order "
                        "[LRCP, RLCP, RPCL, PCRL, CPRL] !!\n");
                return 1;
            }
        }
            break;

            /* ----------------------------------------------------- */

        case 's':			/* subsampling factor */
        {
            if (sscanf(mi_optarg, "%d,%d", &parameters->subsampling_dx,
                       &parameters->subsampling_dy) != 2) {
                fprintf(stderr,	"'-s' sub-sampling argument error !  [-s dx,dy]\n");
                return 1;
            }
        }
            break;

            /* ----------------------------------------------------- */

        case 'd':			/* coordonnate of the reference grid */
        {
            if (sscanf(mi_optarg, "%d,%d", &parameters->image_offset_x0,
                       &parameters->image_offset_y0) != 2) {
                fprintf(stderr,	"-d 'coordonnate of the reference grid' argument "
                        "error !! [-d x0,y0]\n");
                return 1;
            }
        }
            break;

            /* ----------------------------------------------------- */

        case 'h':			/* display an help description */
            //encode_help_display();
            return 1;

            /* ----------------------------------------------------- */

        case 'P':			/* POC */
        {
            int numpocs = 0;		/* number of progression order change (POC) default 0 */
            mi_poc_t *POC = NULL;	/* POC : used in case of Progression order change */

            char *s = mi_optarg;
            POC = parameters->POC;

            while (sscanf(s, "T%u=%u,%u,%u,%u,%u,%4s", &POC[numpocs].tile,
                          &POC[numpocs].resno0, &POC[numpocs].compno0,
                          &POC[numpocs].layno1, &POC[numpocs].resno1,
                          &POC[numpocs].compno1, POC[numpocs].progorder) == 7) {
                POC[numpocs].prg1 = give_progression(POC[numpocs].progorder);
                numpocs++;
                while (*s && *s != '/') {
                    s++;
                }
                if (!*s) {
                    break;
                }
                s++;
            }
            parameters->numpocs = (mi_UINT32)numpocs;
        }
            break;

            /* ------------------------------------------------------ */

        case 'S':			/* SOP marker */
        {
            parameters->csty |= 0x02;
        }
            break;

            /* ------------------------------------------------------ */

        case 'E':			/* EPH marker */
        {
            parameters->csty |= 0x04;
        }
            break;

            /* ------------------------------------------------------ */

        case 'M':			/* Mode switch pas tous au point !! */
        {
            int value = 0;
            if (sscanf(mi_optarg, "%d", &value) == 1) {
                for (i = 0; i <= 5; i++) {
                    int cache = value & (1 << i);
                    if (cache)
                        parameters->mode |= (1 << i);
                }
            }
        }
            break;

            /* ------------------------------------------------------ */

        case 'R':			/* ROI */
        {
            if (sscanf(mi_optarg, "c=%d,U=%d", &parameters->roi_compno,
                       &parameters->roi_shift) != 2) {
                fprintf(stderr, "ROI error !! [-ROI c='compno',U='shift']\n");
                return 1;
            }
        }
            break;

            /* ------------------------------------------------------ */

        case 'T':			/* Tile offset */
        {
            if (sscanf(mi_optarg, "%d,%d", &parameters->cp_tx0, &parameters->cp_ty0) != 2) {
                fprintf(stderr, "-T 'tile offset' argument error !! [-T X0,Y0]");
                return 1;
            }
        }
            break;

            /* ------------------------------------------------------ */

        case 'C':			/* add a comment */
        {
            parameters->cp_comment = (char*)malloc(strlen(mi_optarg) + 1);
            if(parameters->cp_comment) {
                strcpy(parameters->cp_comment, mi_optarg);
            }
        }
            break;


            /* ------------------------------------------------------ */

        case 'I':			/* reversible or not */
        {
            parameters->irreversible = 1;
        }
            break;

            /* ------------------------------------------------------ */

        case 'u':			/* Tile part generation*/
        {
            parameters->tp_flag = mi_optarg[0];
            parameters->tp_on = 1;
        }
            break;

            /* ------------------------------------------------------ */

        case 'z':			/* Image Directory path */
        {
            img_fol->imgdirpath = (char*)malloc(strlen(mi_optarg) + 1);
			if(img_fol->imgdirpath == NULL){
				return 1;
			}
            strcpy(img_fol->imgdirpath,mi_optarg);
            img_fol->set_imgdir=1;
        }
            break;

            /* ------------------------------------------------------ */

        case 'w':			/* Digital Cinema 2K profile compliance*/
        {
            int fps=0;
            sscanf(mi_optarg,"%d",&fps);
            if(fps == 24){
                parameters->rsiz = mi_PROFILE_CINEMA_2K;
                parameters->max_comp_size = mi_CINEMA_24_COMP;
                parameters->max_cs_size = mi_CINEMA_24_CS;
            }else if(fps == 48 ){
                parameters->rsiz = mi_PROFILE_CINEMA_2K;
                parameters->max_comp_size = mi_CINEMA_48_COMP;
                parameters->max_cs_size = mi_CINEMA_48_CS;
            }else {
                fprintf(stderr,"Incorrect value!! must be 24 or 48\n");
                return 1;
            }
            fprintf(stdout,"CINEMA 2K profile activated\n"
                    "Other options specified could be overriden\n");

        }
            break;

            /* ------------------------------------------------------ */

        case 'y':			/* Digital Cinema 4K profile compliance*/
        {
            parameters->rsiz = mi_PROFILE_CINEMA_4K;
            fprintf(stdout,"CINEMA 4K profile activated\n"
                    "Other options specified could be overriden\n");
        }
            break;

            /* ------------------------------------------------------ */

        case 'Y':			/* Shall we do an MCT ? 0:no_mct;1:rgb->ycc;2:custom mct (-m option required)*/
        {
            int mct_mode=0;
            sscanf(mi_optarg,"%d",&mct_mode);
            if(mct_mode < 0 || mct_mode > 2){
                fprintf(stderr,"MCT incorrect value!! Current accepted values are 0, 1 or 2.\n");
                return 1;
            }
            parameters->tcp_mct = (char) mct_mode;
        }
            break;

            /* ------------------------------------------------------ */


        case 'm':			/* mct input file */
        {
            char *lFilename = mi_optarg;
            char *lMatrix;
            char *lCurrentPtr ;
            float *lCurrentDoublePtr;
            float *lSpace;
            int *l_int_ptr;
            int lNbComp = 0, lTotalComp, lMctComp, i2;
            size_t lStrLen, lStrFread;

            /* Open file */
            FILE * lFile = fopen(lFilename,"r");
            if (lFile == NULL) {
                return 1;
            }

            /* Set size of file and read its content*/
            fseek(lFile,0,SEEK_END);
            lStrLen = (size_t)ftell(lFile);
            fseek(lFile,0,SEEK_SET);
            lMatrix = (char *) malloc(lStrLen + 1);
            if (lMatrix == NULL) {
                fclose(lFile);
                return 1;
            }
            lStrFread = fread(lMatrix, 1, lStrLen, lFile);
            fclose(lFile);
            if( lStrLen != lStrFread ) {
                free(lMatrix);
                return 1;
            }

            lMatrix[lStrLen] = 0;
            lCurrentPtr = lMatrix;

            /* replace ',' by 0 */
            while (*lCurrentPtr != 0 ) {
                if (*lCurrentPtr == ' ') {
                    *lCurrentPtr = 0;
                    ++lNbComp;
                }
                ++lCurrentPtr;
            }
            ++lNbComp;
            lCurrentPtr = lMatrix;

            lNbComp = (int) (sqrt(4*lNbComp + 1)/2. - 0.5);
            lMctComp = lNbComp * lNbComp;
            lTotalComp = lMctComp + lNbComp;
            lSpace = (float *) malloc((size_t)lTotalComp * sizeof(float));
            if(lSpace == NULL) {
                free(lMatrix);
                return 1;
            }
            lCurrentDoublePtr = lSpace;
            for (i2=0;i2<lMctComp;++i2) {
                lStrLen = strlen(lCurrentPtr) + 1;
                *lCurrentDoublePtr++ = (float) atof(lCurrentPtr);
                lCurrentPtr += lStrLen;
            }

            l_int_ptr = (int*) lCurrentDoublePtr;
            for (i2=0;i2<lNbComp;++i2) {
                lStrLen = strlen(lCurrentPtr) + 1;
                *l_int_ptr++ = atoi(lCurrentPtr);
                lCurrentPtr += lStrLen;
            }

            /* TODO should not be here ! */
            mi_set_MCT(parameters, lSpace, (int *)(lSpace + lMctComp), (mi_UINT32)lNbComp);

            /* Free memory*/
            free(lSpace);
            free(lMatrix);
        }
            break;


            /* ------------------------------------------------------ */
            /* ------------------------------------------------------ */

        case 'J':			/* jpip on */
        {
            parameters->jpip_on = mi_TRUE;
        }
            break;
            /* ------------------------------------------------------ */


        default:
            fprintf(stderr, "[WARNING] An invalid option has been ignored\n");
            break;
        }
    }while(c != -1);

    if(img_fol->set_imgdir == 1){
        if(!(parameters->infile[0] == 0)){
            fprintf(stderr, "[ERROR] options -ImgDir and -i cannot be used together !!\n");
            return 1;
        }
        if(img_fol->set_out_format == 0){
            fprintf(stderr, "[ERROR] When -ImgDir is used, -OutFor <FORMAT> must be used !!\n");
            fprintf(stderr, "Only one format allowed! Valid formats are j2k and jp2!!\n");
            return 1;
        }
        if(!((parameters->outfile[0] == 0))){
            fprintf(stderr, "[ERROR] options -ImgDir and -o cannot be used together !!\n");
            fprintf(stderr, "Specify OutputFormat using -OutFor<FORMAT> !!\n");
            return 1;
        }
    }else{
        if((parameters->infile[0] == 0) || (parameters->outfile[0] == 0)) {
            fprintf(stderr, "[ERROR] Required parameters are missing\n"
                            "Example: %s -i image.pgm -o image.j2k\n",argv[0]);
            fprintf(stderr, "   Help: %s -h\n",argv[0]);
            return 1;
        }
    }

    if ( (parameters->decod_format == RAW_DFMT && raw_cp->rawWidth == 0)
         || (parameters->decod_format == RAWL_DFMT && raw_cp->rawWidth == 0)) {
        fprintf(stderr,"[ERROR] invalid raw image parameters\n");
        fprintf(stderr,"Please use the Format option -F:\n");
        fprintf(stderr,"-F rawWidth,rawHeight,rawComp,rawBitDepth,s/u (Signed/Unsigned)\n");
        fprintf(stderr,"Example: -i lena.raw -o lena.j2k -F 512,512,3,8,u\n");
        fprintf(stderr,"Aborting\n");
        return 1;
    }

    if ((parameters->cp_disto_alloc || parameters->cp_fixed_alloc || parameters->cp_fixed_quality)
            && (!(parameters->cp_disto_alloc ^ parameters->cp_fixed_alloc ^ parameters->cp_fixed_quality))) {
        fprintf(stderr, "[ERROR] options -r -q and -f cannot be used together !!\n");
        return 1;
    }				/* mod fixed_quality */

    /* if no rate entered, lossless by default */
    if (parameters->tcp_numlayers == 0) {
        parameters->tcp_rates[0] = 0;	/* MOD antonin : losslessbug */
        parameters->tcp_numlayers++;
        parameters->cp_disto_alloc = 1;
    }

    if((parameters->cp_tx0 > parameters->image_offset_x0) || (parameters->cp_ty0 > parameters->image_offset_y0)) {
        fprintf(stderr,
                "[ERROR] Tile offset dimension is unnappropriate --> TX0(%d)<=IMG_X0(%d) TYO(%d)<=IMG_Y0(%d) \n",
                parameters->cp_tx0, parameters->image_offset_x0, parameters->cp_ty0, parameters->image_offset_y0);
        return 1;
    }

    for (i = 0; i < parameters->numpocs; i++) {
        if (parameters->POC[i].prg == -1) {
            fprintf(stderr,
                    "Unrecognized progression order in option -P (POC n %d) [LRCP, RLCP, RPCL, PCRL, CPRL] !!\n",
                    i + 1);
        }
    }

    /* If subsampled image is provided, automatically disable MCT */
    if ( ((parameters->decod_format == RAW_DFMT) || (parameters->decod_format == RAWL_DFMT))
         && (   ((raw_cp->rawComp > 1 ) && ((raw_cp->rawComps[1].dx > 1) || (raw_cp->rawComps[1].dy > 1)))
             || ((raw_cp->rawComp > 2 ) && ((raw_cp->rawComps[2].dx > 1) || (raw_cp->rawComps[2].dy > 1)))
						)) {
        parameters->tcp_mct = 0;
    }

    return 0;

}

/* -------------------------------------------------------------------------- */

/**
sample error debug callback expecting no client object
*/
static void error_callback(const char *msg, void *client_data) {
    (void)client_data;
    fprintf(stdout, "[ERROR] %s", msg);
}
/**
sample warning debug callback expecting no client object
*/
static void warning_callback(const char *msg, void *client_data) {
    (void)client_data;
    fprintf(stdout, "[WARNING] %s", msg);
}
/**
sample debug callback expecting no client object
*/
static void info_callback(const char *msg, void *client_data) {
    (void)client_data;
    fprintf(stdout, "[INFO] %s", msg);
}

mi_FLOAT64 mi_clock(void) {
#ifdef _WIN32
	/* _WIN32: use QueryPerformance (very accurate) */
    LARGE_INTEGER freq , t ;
    /* freq is the clock speed of the CPU */
    QueryPerformanceFrequency(&freq) ;
	/* cout << "freq = " << ((double) freq.QuadPart) << endl; */
    /* t is the high resolution performance counter (see MSDN) */
    QueryPerformanceCounter ( & t ) ;
    return freq.QuadPart ? ( t.QuadPart /(mi_FLOAT64) freq.QuadPart ) : 0 ;
#endif
}


/* -------------------------------------------------------------------------- */
/**
 * mi_COMPRESS MAIN
 */
/* -------------------------------------------------------------------------- */
int main(int argc, char **argv) {

    mi_cparameters_t parameters;	/* compression parameters */

    mi_stream_t *l_stream = 00;
    mi_codec_t* l_codec = 00;
    mi_image_t *image = NULL;
    raw_cparameters_t raw_cp;
    mi_SIZE_T num_compressed_files = 0;

    char indexfilename[mi_PATH_LEN];	/* index file name */

    unsigned int i, num_images, imageno;
    img_fol_t img_fol;
    dircnt_t *dirptr = NULL;

    mi_BOOL bSuccess;
    mi_BOOL bUseTiles = mi_FALSE; /* mi_TRUE */
    mi_UINT32 l_nb_tiles = 4;
    mi_FLOAT64 t = mi_clock();

    /* set encoding parameters to default values */
    mi_set_default_encoder_parameters(&parameters);

    /* Initialize indexfilename and img_fol */
    *indexfilename = 0;
    memset(&img_fol,0,sizeof(img_fol_t));

    /* raw_cp initialization */
    raw_cp.rawBitDepth = 0;
    raw_cp.rawComp = 0;
    raw_cp.rawComps = 0;
    raw_cp.rawHeight = 0;
    raw_cp.rawSigned = 0;
    raw_cp.rawWidth = 0;

    /* parse input and get user encoding parameters */
    parameters.tcp_mct = (char) 255; /* This will be set later according to the input image or the provided option */
    if(parse_cmdline_encoder(argc, argv, &parameters,&img_fol, &raw_cp, indexfilename, sizeof(indexfilename)) == 1) {
        goto fails;
    }

    /* Read directory if necessary */
    if(img_fol.set_imgdir==1){
        num_images=get_num_images(img_fol.imgdirpath);
        dirptr=(dircnt_t*)malloc(sizeof(dircnt_t));
        if(dirptr){
            dirptr->filename_buf = (char*)malloc(num_images*mi_PATH_LEN*sizeof(char));	/* Stores at max 10 image file names*/
            dirptr->filename = (char**) malloc(num_images*sizeof(char*));
            if(!dirptr->filename_buf){
                return 0;
            }
            for(i=0;i<num_images;i++){
                dirptr->filename[i] = dirptr->filename_buf + i*mi_PATH_LEN;
            }
        }
        if(load_images(dirptr,img_fol.imgdirpath)==1){
            return 0;
        }
        if (num_images==0){
            fprintf(stdout,"Folder is empty\n");
            return 0;
        }
    }else{
        num_images=1;
    }
    /*Encoding image one by one*/
    for(imageno=0;imageno<num_images;imageno++)	{
        image = NULL;
        fprintf(stderr,"\n");

        if(img_fol.set_imgdir==1){
            if (get_next_file((int)imageno, dirptr,&img_fol, &parameters)) {
                fprintf(stderr,"skipping file...\n");
                continue;
            }
        }

        switch(parameters.decod_format) {
        case PGX_DFMT:
            break;
        case PXM_DFMT:
            break;
        case BMP_DFMT:
            break;
        case TIF_DFMT:
            break;
        case RAW_DFMT:
        case RAWL_DFMT:
            break;
        case TGA_DFMT:
            break;
        case PNG_DFMT:
            break;
        default:
            fprintf(stderr,"skipping file...\n");
            continue;
        }

        /* decode the source image */
        /* ----------------------- */

        switch (parameters.decod_format) {
        case PGX_DFMT:
            image = pgxtoimage(parameters.infile, &parameters);
            if (!image) {
                fprintf(stderr, "Unable to load pgx file\n");
                return 1;
            }
            break;

        case PXM_DFMT:
            image = pnmtoimage(parameters.infile, &parameters);
            if (!image) {
                fprintf(stderr, "Unable to load pnm file\n");
                return 1;
            }
            break;

        case BMP_DFMT:
            image = bmptoimage(parameters.infile, &parameters);
            if (!image) {
                fprintf(stderr, "Unable to load bmp file\n");
                return 1;
            }
            break;

        case RAW_DFMT:
            image = rawtoimage(parameters.infile, &parameters, &raw_cp);
            if (!image) {
                fprintf(stderr, "Unable to load raw file\n");
                return 1;
            }
            break;

        case RAWL_DFMT:
            image = rawltoimage(parameters.infile, &parameters, &raw_cp);
            if (!image) {
                fprintf(stderr, "Unable to load raw file\n");
                return 1;
            }
            break;

        case TGA_DFMT:
            image = tgatoimage(parameters.infile, &parameters);
            if (!image) {
                fprintf(stderr, "Unable to load tga file\n");
                return 1;
            }
            break;
        }

        /* Can happen if input file is TIFF or PNG
 * and mi_HAVE_LIBTIF or mi_HAVE_LIBPNG is undefined
*/
        if( !image) {
            fprintf(stderr, "Unable to load file: got no image\n");
            return 1;
        }

        /* Decide if MCT should be used */
        if (parameters.tcp_mct == (char) 255) { /* mct mode has not been set in commandline */
            parameters.tcp_mct = (image->numcomps >= 3) ? 1 : 0;
        } else {            /* mct mode has been set in commandline */
            if ((parameters.tcp_mct == 1) && (image->numcomps < 3)){
                fprintf(stderr, "RGB->YCC conversion cannot be used:\n");
                fprintf(stderr, "Input image has less than 3 components\n");
                return 1;
            }
            if ((parameters.tcp_mct == 2) && (!parameters.mct_data)){
                fprintf(stderr, "Custom MCT has been set but no array-based MCT\n");
                fprintf(stderr, "has been provided. Aborting.\n");
                return 1;
            }
        }

        /* encode the destination image */
        /* ---------------------------- */

        switch(parameters.cod_format) {
        case J2K_CFMT:	/* JPEG-2000 codestream */
        {
            /* Get a decoder handle */
            l_codec = mi_create_compress(mi_CODEC_J2K);
            break;
        }
        case JP2_CFMT:	/* JPEG 2000 compressed image data */
        {
            /* Get a decoder handle */
            l_codec = mi_create_compress(mi_CODEC_JP2);
            break;
        }
        default:
            fprintf(stderr, "skipping file..\n");
            mi_stream_destroy(l_stream);
            continue;
        }

        /* catch events using our callbacks and give a local context */
        mi_set_info_handler(l_codec, info_callback,00);
        mi_set_warning_handler(l_codec, warning_callback,00);
        mi_set_error_handler(l_codec, error_callback,00);

        if( bUseTiles ) {
            parameters.cp_tx0 = 0;
            parameters.cp_ty0 = 0;
            parameters.tile_size_on = mi_TRUE;
            parameters.cp_tdx = 512;
            parameters.cp_tdy = 512;
        }
        if (! mi_setup_encoder(l_codec, &parameters, image)) {
            fprintf(stderr, "failed to encode image: mi_setup_encoder\n");
            mi_destroy_codec(l_codec);
            mi_image_destroy(image);
            return 1;
        }

        /* open a byte stream for writing and allocate memory for all tiles */
        l_stream = mi_stream_create_default_file_stream(parameters.outfile,mi_FALSE);
        if (! l_stream){
            return 1;
        }

        /* encode the image */
        bSuccess = mi_start_compress(l_codec,image,l_stream);
        if (!bSuccess)  {
            fprintf(stderr, "failed to encode image: mi_start_compress\n");
        }
        if( bSuccess && bUseTiles ) {
            mi_BYTE *l_data;
            mi_UINT32 l_data_size = 512*512*3;
            l_data = (mi_BYTE*) calloc( 1,l_data_size);
            if(l_data == NULL){
				goto fails;
			}
            for (i=0;i<l_nb_tiles;++i) {
                if (! mi_write_tile(l_codec,i,l_data,l_data_size,l_stream)) {
                    fprintf(stderr, "ERROR -> test_tile_encoder: failed to write the tile %d!\n",i);
                    mi_stream_destroy(l_stream);
                    mi_destroy_codec(l_codec);
                    mi_image_destroy(image);
                    return 1;
                }
            }
            free(l_data);
        }
        else {
            bSuccess = bSuccess && mi_encode(l_codec, l_stream);
            if (!bSuccess)  {
                fprintf(stderr, "failed to encode image: mi_encode\n");
            }
        }
        bSuccess = bSuccess && mi_end_compress(l_codec, l_stream);
        if (!bSuccess)  {
            fprintf(stderr, "failed to encode image: mi_end_compress\n");
        }

        if (!bSuccess)  {
            mi_stream_destroy(l_stream);
            mi_destroy_codec(l_codec);
            mi_image_destroy(image);
            fprintf(stderr, "failed to encode image\n");
			remove(parameters.outfile);
            return 1;
        }

		num_compressed_files++;
        fprintf(stdout,"[INFO] Generated outfile %s\n",parameters.outfile);
        /* close and free the byte stream */
        mi_stream_destroy(l_stream);

        /* free remaining compression structures */
        mi_destroy_codec(l_codec);

        /* free image data */
        mi_image_destroy(image);

    }

    /* free user parameters structure */
    if(parameters.cp_comment)   free(parameters.cp_comment);
    if(parameters.cp_matrice)   free(parameters.cp_matrice);
    if(raw_cp.rawComps) free(raw_cp.rawComps);
	
    t = mi_clock() - t;
    if (num_compressed_files) {
		    fprintf(stdout, "encode time: %d ms \n", (int)((t * 1000.0)/(mi_FLOAT64)num_compressed_files));
    }

    return 0;

fails:
	if(parameters.cp_comment)   free(parameters.cp_comment);
	if(parameters.cp_matrice)   free(parameters.cp_matrice);
	if(raw_cp.rawComps) free(raw_cp.rawComps);
	if(img_fol.imgdirpath) free(img_fol.imgdirpath);
	if(dirptr){
	    if(dirptr->filename_buf) free(dirptr->filename_buf);
	    if(dirptr->filename) free(dirptr->filename);
	    free(dirptr);
	}
	return 1;
}
