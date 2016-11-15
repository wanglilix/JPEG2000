
#include "mi_includes.h"

/* 
==========================================================
   local functions
==========================================================
*/


/* 
==========================================================
   RAW encoding interface
==========================================================
*/

mi_raw_t* mi_raw_create(void) {
	mi_raw_t *raw = (mi_raw_t*)mi_malloc(sizeof(mi_raw_t));
	return raw;
}

void mi_raw_destroy(mi_raw_t *raw) {
	if(raw) {
		mi_free(raw);
	}
}

mi_UINT32 mi_raw_numbytes(mi_raw_t *raw) {
	const ptrdiff_t diff = raw->bp - raw->start;
  assert( diff <= (ptrdiff_t)0xffffffff && diff >= 0 ); /* UINT32_MAX */
	return (mi_UINT32)diff;
}

void mi_raw_init_dec(mi_raw_t *raw, mi_BYTE *bp, mi_UINT32 len) {
	raw->start = bp;
	raw->lenmax = len;
	raw->len = 0;
	raw->c = 0;
	raw->ct = 0;
}

mi_UINT32 mi_raw_decode(mi_raw_t *raw) {
	mi_UINT32 d;
	if (raw->ct == 0) {
		raw->ct = 8;
		if (raw->len == raw->lenmax) {
			raw->c = 0xff;
		} else {
			if (raw->c == 0xff) {
				raw->ct = 7;
			}
			raw->c = *(raw->start + raw->len);
			raw->len++;
		}
	}
	raw->ct--;
	d = ((mi_UINT32)raw->c >> raw->ct) & 0x01U;
	
	return d;
}

