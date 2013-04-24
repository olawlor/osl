/**
 A 4x4 column-oriented matrix type suspiciously similar 
 to GLSL's "mat4".
 
 Orion Sky Lawlor, olawlor@acm.org, 2006/09/12 (Public Domain)
*/
#ifndef __OSL_MAT4_H
#define __OSL_MAT4_H

#include "vec4.h"

namespace osl {

/** A mat4 is really just an array of 4 column vectors.
	mat4 m;
	m[0].x=7; // sets 0'th column, x row to 7 (input x, output x) 
	m[3].x=8; // sets 3rd column, x row to 8  (input w, output x) 
	m[2].y=9; // sets 2nd column, y row to 9  (input z, output y) 

The command to upload a matrix into a GLSL program is glUniformMatrix4fv.  
Call it with a "mat4" like this:
    glUniformMatrix4fvARB(glGetUniformLocationARB(prog,"mF"),1,0,&mF[0][0]);
*/
class mat4 {
	typedef float real;
public:
	// These are actually the 4 columns of the matrix.
	//   Hence m.y.x is the (input) y column's (output) x coordinate.
	//   The standard GLSL m[1] (input y column) syntax works too.
	vec4 x,y,z,w;
	
	/// Default consructor.  All elements initialized to zero.
	mat4(void) {x=y=z=w=(vec4)0;}
	
	/// Simple 1-value constructors.  The diagonal is set to the value you specify.
	explicit mat4(int init) {x=y=z=w=(vec4)0; x.x=y.y=z.z=w.w=init;}
	explicit mat4(float init) {x=y=z=w=(vec4)0; x.x=y.y=z.z=w.w=init;}
	explicit mat4(double init) {x=y=z=w=(vec4)0; x.x=y.y=z.z=w.w=init;}
	
	/* Column-wise construction by floats */
	explicit mat4(real c0x,real c0y,real c0z,real c0w,
		real c1x,real c1y,real c1z,real c1w,
		real c2x,real c2y,real c2z,real c2w,
		real c3x,real c3y,real c3z,real c3w
		):
		x(c0x,c0y,c0z,c0w),
		y(c1x,c1y,c1z,c1w),
		z(c2x,c2y,c2z,c2w),
		w(c3x,c3y,c3z,c3w) {}
	
	/// 4-column constructor
	mat4(const vec4 Nx,const vec4 Ny,const vec4 Nz,const vec4 Nw) {x=Nx;y=Ny;z=Nz;w=Nw;}
	/// vec4 array constructor
	mat4(const vec4 *arr) {x=arr[0];y=arr[1];z=arr[2];w=arr[3];}
	/// Column-wise array-of-doubles constructor (like glLoadMatrixd)
	mat4(const double *arr) :x(&arr[0]),y(&arr[4]), z(&arr[8]), w(&arr[12]) {}
	mat4(const float *arr) :x(&arr[0]),y(&arr[4]), z(&arr[8]), w(&arr[12]) {}

	/// Copy constructor & assignment operator by default
	
	/// This lets you typecast a vector to a vec4 array
	//operator vec4 *() {return (vec4 *)&x;}
	//operator const vec4 *() const {return (const vec4 *)&x;}
	/// Column-wise indexing
	vec4 &operator[] (int i) {return ((vec4 *)&x)[i];}
	const vec4 &operator[] (int i) const {return ((vec4 *)&x)[i];}

/// Basic mathematical operators	
	int operator==(const mat4 &b) const {return (x==b.x)&&(y==b.y)&&(z==b.z)&&(w==b.w);}
	int operator!=(const mat4 &b) const {return (x!=b.x)||(y!=b.y)||(z!=b.z)||(w!=b.w);}
	mat4 operator+(const mat4 &b) const {return mat4(x+b.x,y+b.y,z+b.z,w+b.w);}
	mat4 operator-(const mat4 &b) const {return mat4(x-b.x,y-b.y,z-b.z,w-b.w);}
	mat4 operator*(const real scale) const 
		{return mat4(x*scale,y*scale,z*scale,w*scale);}
	mat4 operator/(const real &div) const
		{real scale=real(1.0/div);return mat4(x*scale,y*scale,z*scale,w*scale);}
	mat4 operator-(void) const {return mat4(-x,-y,-z,-w);}
	void operator+=(const mat4 &b) {x+=b.x;y+=b.y;z+=b.z;w+=b.w;}
	void operator-=(const mat4 &b) {x-=b.x;y-=b.y;z-=b.z;w-=b.w;}
	void operator*=(const real scale) {x*=scale;y*=scale;z*=scale;w*=scale;}
	void operator/=(const real div) {real scale=1.0/div;x*=scale;y*=scale;z*=scale;w*=scale;}

	/// Return the square of the magnitude of this matrix
	real normSqr(void) const {return x.dot(x)+y.dot(y)+z.dot(z)+w.dot(w);}
	/// Return the magnitude (length) of this matrix
	real norm(void) const {return sqrt(normSqr());}
};


/* Matrix-vector multiply (column vectors; postmultiply) */
inline vec4 operator*(const mat4 &M,const vec4 &v) {
	return  M[0]*v.x+
		M[1]*v.y+
		M[2]*v.z+
		M[3]*v.w;
}
/* Vector-matrix multiply (row vectors; premultiply) */
inline vec4 operator*(const vec4 &v,const mat4 &M) {
	return vec4(
		M[0].dot(v),
		M[1].dot(v),
		M[2].dot(v),
		M[3].dot(v)
	);
}

/* Matrix-matrix multiply */
inline mat4 operator*(const mat4 &L,const mat4 &R) {
	return mat4(
		L*R[0],
		L*R[1],
		L*R[2],
		L*R[3]
	);
}
/* Matrix-float multiply */
inline mat4 operator*(const float &L,mat4 R) {
	R*=L;
	return R;
}
/**
  NONSTANDARD Orion function:
  Transform this location vector by this projection matrix,
  then divide by w to complete the perspective projection.
*/
inline vec3 project(const mat4 &m,const vec3 &v) {
	vec4 p=m*vec4(v.x,v.y,v.z,1.0);
	float wi=1.0f/p.w;
	return vec3(p*wi);
}

inline mat4 transpose(const mat4 &m) {
	return mat4(
		m[0][0],m[1][0],m[2][0],m[3][0],
		m[0][1],m[1][1],m[2][1],m[3][1],
		m[0][2],m[1][2],m[2][2],m[3][2],
		m[0][3],m[1][3],m[2][3],m[3][3]
	);
}

}; /* end namespace */
#endif
