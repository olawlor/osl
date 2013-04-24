/**
 A 4x4 column-oriented matrix type suspiciously similar 
 to GLSL's "mat2".
 
 Orion Sky Lawlor, olawlor@acm.org, 2006/09/12 (Public Domain)
*/
#ifndef __OSL_mat2_H
#define __OSL_mat2_H

#include "vec2.h"

namespace osl {

/** A mat2 is really just an array of 4 column vectors.
	mat2 m;
	m[0].x=7; // sets 0'th column, x row to 7 (input x, output x) 
	m[1].y=8; // sets 1'th column, y row to 8  (input y, output y) 
	m[1].x=9; // sets 1'th column, x row to 9  (input y, output x) 

The command to upload a matrix into a GLSL program is glUniformMatrix2fv.  
Call it with a "mat2" like this:
    glUniformMatrix2fvARB(glGetUniformLocationARB(prog,"mF"),1,0,&mF[0][0]);
*/
class mat2 {
	typedef float real;
public:
	// These are actually the 2 columns of the matrix.
	//   Hence m.y.x is the (input) y column's (output) x coordinate.
	//   The standard GLSL m[1] (input y column) syntax works too.
	vec2 x,y;
	
	/// Default consructor.  All elements initialized to zero.
	mat2(void) {x=y=vec2(0,0);}
	
	/// Simple 1-value constructors.  The diagonal is set to the value you specify.
	explicit mat2(int init) {x=y=vec2(0,0); x.x=y.y=init;}
	explicit mat2(float init) {x=y=vec2(0,0); x.x=y.y=init;}
	explicit mat2(double init) {x=y=vec2(0,0); x.x=y.y=init;}
	
	/* Column-wise construction by floats */
	explicit mat2(real c0x,real c0y,
		real c1x,real c1y
		):
		x(c0x,c0y),
		y(c1x,c1y) {}
	
	/// 2-column constructor
	mat2(const vec2 Nx,const vec2 Ny) {x=Nx;y=Ny;}
	/// vec2 array constructor
	mat2(const vec2 *arr) {x=arr[0];y=arr[1];}
	/// Column-wise array-of-doubles constructor (like glLoadMatrixd)
	mat2(const double *arr) :x(&arr[0]),y(&arr[2]) {}
	mat2(const float *arr) :x(&arr[0]),y(&arr[2]) {}

	/// Copy constructor & assignment operator by default
	
	/// This lets you typecast a vector to a vec2 array
	//operator vec2 *() {return (vec2 *)&x;}
	//operator const vec2 *() const {return (const vec2 *)&x;}
	/// Column-wise indexing
	vec2 &operator[] (int i) {return ((vec2 *)&x)[i];}
	const vec2 &operator[] (int i) const {return ((vec2 *)&x)[i];}

/// Basic mathematical operators	
	int operator==(const mat2 &b) const {return (x==b.x)&&(y==b.y);}
	int operator!=(const mat2 &b) const {return (x!=b.x)||(y!=b.y);}
	mat2 operator+(const mat2 &b) const {return mat2(x+b.x,y+b.y);}
	mat2 operator-(const mat2 &b) const {return mat2(x-b.x,y-b.y);}
	mat2 operator*(const real scale) const 
		{return mat2(x*scale,y*scale);}
	mat2 operator/(const real &div) const
		{real scale=real(1.0/div);return mat2(x*scale,y*scale);}
	mat2 operator-(void) const {return mat2(-x,-y);}
	void operator+=(const mat2 &b) {x+=b.x;y+=b.y;}
	void operator-=(const mat2 &b) {x-=b.x;y-=b.y;}
	void operator*=(const real scale) {x*=scale;y*=scale;}
	void operator/=(const real div) {real scale=1.0/div;x*=scale;y*=scale;}

	/// Return the square of the magnitude of this matrix
	real normSqr(void) const {return x.dot(x)+y.dot(y);}
	/// Return the magnitude (length) of this matrix
	real norm(void) const {return sqrt(normSqr());}
};


/* Matrix-vector multiply (column vectors; postmultiply) */
inline vec2 operator*(const mat2 &M,const vec2 &v) {
	return  M[0]*v.x+
		M[1]*v.y;
}
/* Vector-matrix multiply (row vectors; premultiply) */
inline vec2 operator*(const vec2 &v,const mat2 &M) {
	return vec2(
		M[0].dot(v),
		M[1].dot(v)
	);
}

/* Matrix-matrix multiply */
inline mat2 operator*(const mat2 &L,const mat2 &R) {
	return mat2(
		L*R[0],
		L*R[1]
	);
}
/* Matrix-float multiply */
inline mat2 operator*(const float &L,mat2 R) {
	R*=L;
	return R;
}

/* Matrix inversion (NON-GLSL).  See algorithm from:
	http://www.mathcentre.ac.uk/resources/leaflets/firstaidkits/5_4.pdf
*/
mat2 inverse(const mat2 &m) {
	float di=1.0/(m.x.x*m.y.y - m.y.x*m.x.y);
	return di*mat2(
		 m.y.y, -m.x.y,    -m.y.x,  m.x.x
	);
}

}; /* end namespace */

#ifdef STANDALONE
#include <iostream>
std::ostream &operator<<(std::ostream &o,const osl::mat2 &m) {
	o<<m.x.x<<" "<<m.y.x<<"\n";
	o<<m.x.y<<" "<<m.y.y<<"\n";
	return o;
}
int main() {
	osl::mat2 A(3,4, 1,2);
	std::cout<<"A="<<A;
	std::cout<<"inverse(A)="<<inverse(A);
	return 0;
}
#endif // standalone
#endif // header
