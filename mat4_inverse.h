/*
Inverts a mat4 matrix using gaussian elimination.
This drags in a lot of template code, so it's not part of
the normal mat4 interface.

Orion Sky Lawlor, olawlor@acm.org, 2009-01-03 (Public Domain)
*/
#ifndef __OSL_MAT4_INVERSE_H
#define __OSL_MAT4_INVERSE_H

#include "mat4.h"
#include "matrixT.h"

inline osl::mat4 inverse(const osl::mat4 &src) {
	enum {nr=4}; enum {nc=4};
	int r,c;
	osl::MatrixT<float,nr,nc> S,D;
	for (r=0;r<nc;r++) for (c=0;c<nc;c++)
		S[r][c]=src[c][r];
	
	S.invert(D);
	
	osl::mat4 ret;
	for (r=0;r<nc;r++) for (c=0;c<nc;c++)
		ret[r][c]=D[c][r];
	return ret;
}



#endif // def(thisheader)
