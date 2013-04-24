/*
Clean up math.h for usability outside.
*/
#ifndef __OSL_MATH_HEADER_H
#define __OSL_MATH_HEADER_H

#include <stdlib.h> /* for NULL */
#include <math.h>
#ifndef M_PI  /* some machines don't have this */
#  define M_PI 3.14159265358979323
#endif

#endif
