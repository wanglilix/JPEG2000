
#ifndef __mi_CLOCK_H
#define __mi_CLOCK_H
#include"openjpeg.h"
/**
@file mi_clock.h
@brief Internal function for timing

The functions in mi_CLOCK.C are internal utilities mainly used for timing.
*/

/** @defgroup MISC MISC - Miscellaneous internal functions */
/*@{*/

/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */

/**
Difference in successive mi_clock() calls tells you the elapsed time
@return Returns time in seconds
*/
mi_FLOAT64 mi_clock(void);

/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/

#endif /* __mi_CLOCK_H */

