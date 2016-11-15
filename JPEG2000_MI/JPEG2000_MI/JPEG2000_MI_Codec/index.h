
#ifndef __J2K_INDEX_H
#define __J2K_INDEX_H
#include"cio.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
Write a structured index to a file
@param cstr_info Codestream information 
@param index Index filename
@return Returns 0 if successful, returns 1 otherwise
*/
int write_index_file(mi_codestream_info_t *cstr_info, char *index);

#ifdef __cplusplus
}
#endif

#endif /* __J2K_INDEX_H */

