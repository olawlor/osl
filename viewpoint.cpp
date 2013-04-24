/*
Describes a viewpoint in 3d space: a projection matrix.
  (Copied out of Orion's Standard Library)

Orion Sky Lawlor, olawlor@acm.org, 2003/3/28
*/
#include "viewpoint.h"
#include "math_header.h"

using namespace osl;

inline bool orthogonal(const Vector3d &a,const Vector3d &b) {
	return fabs(a.dot(b))<1.0e-4;
}

/// Fill our projection matrix m with values from E, R, X, Y, Z
void Viewpoint::buildM(void) {
	/**
	  Want project(R+x*X+y*Y+z*Z) = (x,y,z).
	    so, e.g.,
	    	(x*X+y*Y+z*Z) dot sX = x
	    so sX should be orthogonal to Y and Z, and
	    have magnitude such that X dot sX = 1.
	*/
	Vector3d sX=Y.cross(Z); sX*=1.0/X.dot(sX);
	Vector3d sY=X.cross(Z); sY*=1.0/Y.dot(sY);	
	
	if (isPerspective) 
	{
	/*
	  The projection matrix derivation begins by postulating
	  a universe point P, which we need to project into the view plane.
	  The view plane projection, S, satisfies S=E+t*(P-E) for 
	  some parameter value t, and also Z.dot(S-R)=0.
	  Solving this and taking screen_x=sX.dot(S-R), screen_y=sY.dot(S-R),
	  and screen_z=Z.dot(R-E)/Z.dot(P-E) leads to our matrix.
	 */		
		
		// Compute skew factors and skewed axes
		double skew_x=sX.dot(R-E), skew_y=sY.dot(R-E), skew_z=Z.dot(R-E);
		Vector3d gX=skew_x*Z-skew_z*sX;
		Vector3d gY=skew_y*Z-skew_z*sY;
		
		// Assign values to the matrix
		m(0,0)=gX.x; m(0,1)=gX.y; m(0,2)=gX.z; m(0,3)=-gX.dot(E); 
		m(1,0)=gY.x; m(1,1)=gY.y; m(1,2)=gY.z; m(1,3)=-gY.dot(E); 
		m(2,0)=0;    m(2,1)=0;    m(2,2)=0;    m(2,3)=-skew_z;
		m(3,0)=-Z.x; m(3,1)=-Z.y; m(3,2)=-Z.z; m(3,3)=Z.dot(E); 
	}
	else /* orthographic projection */
	{
		Vector3d sZ=X.cross(Y); sZ*=1.0/Z.dot(sZ);
		m(0,0)=sX.x; m(0,1)=sX.y; m(0,2)=sX.z; m(0,3)=-sX.dot(R); 
		m(1,0)=sY.x; m(1,1)=sY.y; m(1,2)=sY.z; m(1,3)=-sY.dot(R); 
		m(2,0)=sZ.x; m(2,1)=sZ.y; m(2,2)=sZ.z; m(2,3)=-sZ.dot(R);
		m(3,0)=0;    m(3,1)=0;    m(3,2)=0;    m(3,3)=1.0; 
	}
}

/// Build a camera at eye point E pointing toward R, with up vector Y.
///   The up vector is not allowed to be parallel to E-R.
Viewpoint::Viewpoint(const Vector3d &E_,const Vector3d &R_,Vector3d Y_)
	:E(E_), R(R_), Y(Y_), wid(-1), ht(-1)
{
	isPerspective=true;
	Z=(E-R).dir(); //Make view plane orthogonal to eye-origin line
	X=Y.cross(Z).dir();
	Y=Z.cross(X).dir();
	// assert: X, Y, and Z are orthogonal
	buildM();
}

/// Build a camera at eye point E for view plane with origin R
///  and X and Y as pixel sizes. 
Viewpoint::Viewpoint(const Vector3d &E_,const Vector3d &R_,
	const Vector3d &X_,const Vector3d &Y_,int w,int h)
	:E(E_), R(R_), X(X_), Y(Y_), wid(w), ht(h)
{
	isPerspective=true;
	Z=X.cross(Y).dir();
	buildM();
}

/// Build an orthogonal camera for a view plane with origin R
///  and X and Y as pixel sizes, and the given Z axis.
/// This is a difficult-to-use, but completely general routine.
Viewpoint::Viewpoint(const Vector3d &R_,
	const Vector3d &X_,const Vector3d &Y_,const Vector3d &Z_,
	int w,int h,bool yesThisIsPerspective)
	:E(R_), R(R_), X(X_), Y(Y_), Z(Z_), wid(w), ht(h)
{
	isPerspective=false;
	buildM();
}

/// Make this camera, fresh-built with the above constructor, have
///  this X and Y resolution and horizontal full field-of-view (degrees).
/// This routine rescales X and Y to have the appropriate length for
///  the field of view, and shifts the projection origin by (-wid/2,-ht/2). 
void Viewpoint::discretize(int w,int h,double hFOV) {
	wid=w; ht=h;
	double pixSize=E.dist(R)*tan(0.5*(M_PI/180.0)*hFOV)*2.0/w;
	X*=pixSize;
	Y*=pixSize;
	R-=X*(0.5*w)+Y*(0.5*h);
	buildM();
}

/// Like discretize, but flips the Y axis (for typical raster viewing)
void Viewpoint::discretizeFlip(int w,int h,double hFOV) {
	discretize(w,h,hFOV);
	flip();
}

/// Flip the image's Y axis (for typical raster viewing)
void Viewpoint::flip(void) {
	R+=Y*ht;
	Y*=-1;
	buildM();
}

/// Extract a window with this width and height, with origin at this pixel
void Viewpoint::window(int w,int h, int x,int y)
{
	R+=X*x+Y*y;
	wid=w; ht=h;
	buildM();
}

/// Make this perspective camera orthogonal-- turn off perspective.
void Viewpoint::disablePerspective(void)
{
	isPerspective=false;
	buildM();
}

/// Get our i'th clipping plane.
Halfspace3d Viewpoint::getClip(int i) const
{
	double target=0, dir=1;
	int r=0; // matrix row
	switch (i) {
	case 0: break;
	case 1: target=-wid; dir=-1; break;
	case 2: r=1; break;
	case 3: target=-ht; dir=-1; r=1; break;
	}
	// Require:  dir * proj(v) >= target
	//  where proj(x)=(v dot m(r) + m(r,3))/(v dot m(3) + m(3,3))
	//  so  (assuming w positive)
	//      dir * (v dot m(r) + m(r,3)) >= target * (v dot m(3) + m(3,3))
	//  which we cast as (v dot h.n) + h.d >= 0
	Halfspace3d h;
	h.n=  dir*Vector3d(m(r,0),m(r,1),m(r,2))
	  -target*Vector3d(m(3,0),m(3,1),m(3,2));
	h.d=  dir*m(r,3) - target*m(3,3);
	return h;
}

//Return an OpenGL-compatible projection matrix for this viewpoint:
void Viewpoint::makeOpenGL(double *dest,double z_near,double z_far) const {
	ViewMatrix3d g=m;
	/// Step 1: convert X and Y from outputting pixels to outputting [0,2]:
	g.scaleRow(0,2.0/wid);
	g.scaleRow(1,2.0/ht);
	/// Step 2: center X and Y on [-1,1], by shifting post-divide output:
	g.addRow(3,-1.0,0);
	g.addRow(3,-1.0,1);
	
	if (isPerspective) {
		/// Step 3: map output Z from [-z_far,-z_near] to [-1,1]
		///  Will compute as z_out = (a + b * w_out)/w_out = a/w_out+b;
		double a=-2.0*z_near*z_far/(z_far-z_near); // 1/w term
		double b=(z_near+z_far)/(z_far-z_near); // constant term
		g(2,2)=0; g(2,3)=a;
		g.addRow(3,b,2); /* add "b*w_out" to z output */
	}
	
	g.makeOpenGL(dest);
}


// Make this row have this vector value plus this offset.
//  The output for this row is then v dot x + off.
void ViewMatrix3d::setRow(int r,const Vector3d &v,double off) {
	for (int c=0;c<3;c++) 
		(*this)(r,c)=v[c];
	(*this)(r,3)=off;
}

// Copy this matrix out to an OpenGL-compatible column matrix.
void ViewMatrix3d::makeOpenGL(double *dest) const {
	for (int r=0;r<4;r++)
	for (int c=0;c<4;c++) 
		dest[c*4+r]=(*this)(r,c);
}
