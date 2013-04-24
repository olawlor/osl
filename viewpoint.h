/*
Describes a viewpoint in 3d space: a camera or projection matrix.

Orion Sky Lawlor, olawlor@acm.org, 2003/3/28  (Public Domain)
*/

#ifndef __OSL_VIEWPOINT_H
#define __OSL_VIEWPOINT_H

#include "vector3d.h" /* basic 3D vector class */
#include "vector2d.h" /* basic 2D vector class */

#ifdef __CHARMC__
#  include "pup.h" /* UIUC Charm++ header file, for migration */
#endif

namespace osl {

/**
 * A start point and a direction
 */
class Ray {
public:
	Vector3d pos,dir;

	Ray() {}
	Ray(const Vector3d &s,const Vector3d &d)
		:pos(s), dir(d) {}

	Vector3d at(double t) const {return pos+t*dir;}	

#ifdef ___PUP_H
	void pup(PUP::er &p) {p|pos; p|dir;}
#endif
};

/**
 * A 4x4 matrix.  This is only used for projection, below, so it's 
 * a trivial class.
 */
class ViewMatrix3d {
	double data[4][4];
public:
	inline double &operator() (int r,int c) {return data[r][c];}
	inline const double &operator() (int r,int c) const {return data[r][c];}
	
	// Scale this row by this scalar
	inline void scaleRow(int r,double s) {
		for (int c=0;c<4;c++) data[r][c]*=s;
	}
	// Add this scaling of this row to this destination row
	inline void addRow(int rSrc,double s,int rDest) {
		for (int c=0;c<4;c++) data[rDest][c]+=s*data[rSrc][c];
	}
	// Make this row have this vector value plus this offset.
	//  The output for this row is then v dot x + off.
	void setRow(int r,const Vector3d &v,double off);
	
	// Copy this matrix out to an OpenGL-compatible column matrix.
	void makeOpenGL(double *dest) const;
	
#ifdef __CK_PUP_H
	void pup(PUP::er &p) {
		p(&data[0][0],16);
	}
#endif
};

/**
 * Describes a method for converting universe locations to screen pixels
 * and ba again: a projection matrix.
 */
class Viewpoint {
	Vector3d E; //< Eye point (PHIGS Projection Reference Point)
	Vector3d R; //< Projection plane origin (PHIGS View Reference Point)
	Vector3d X,Y; //< Pixel-length axes of projection plane (Y is PHIGS View Up Vector)
	Vector3d Z; //< Arbitrary-length normal to projection plane (PHIGS View Plane Normal)
	ViewMatrix3d m; //< 4x4 projection matrix: universe to screen pixels
	int wid,ht; //< Width and height of view plane, in pixels
	
	bool isPerspective; //< If true, perspective projection is enabled.
	
	/// Fill our projection matrix m with values from E, R, X, Y, Z
	void buildM(void);
public:
/** Modification-- "set" routines. **/
	/// Build a camera at eye point E pointing toward R, with up vector Y.
	///   The up vector is not allowed to be parallel to E-R.
	/// This is an easy-to-use, but restricted (no off-axis) routine.
	/// It is normally followed by discretize or discretizeFlip.
	Viewpoint(const Vector3d &E_,const Vector3d &R_,Vector3d Y_=Vector3d(0,1,1.0e-8));
	
	/// Build a camera at eye point E for view plane with origin R
	///  and X and Y as pixel sizes.  
	Viewpoint(const Vector3d &E_,const Vector3d &R_,
		const Vector3d &X_,const Vector3d &Y_,int w,int h);
	
	/**
	  Build an orthogonal camera for a view plane with origin R
	   and X and Y as pixel sizes, and the given Z axis.
	  For a parallel-projection camera (yesThisIsPerspective==false) and
	    project(R_+x*X_+y*Y_+z*Z_) = (x,y,z)
	*/
	Viewpoint(const Vector3d &R_,
		const Vector3d &X_,const Vector3d &Y_,const Vector3d &Z_,
		int w,int h,bool yesThisIsPerspective);
	
	/// Make this perspective camera orthogonal-- turn off perspective.
	void disablePerspective(void);
	
	/// Make this camera, fresh-built with the above constructor, have
	///  this X and Y resolution and horizontal full field-of-view (degrees).
	/// This routine rescales X and Y to have the appropriate length for
	///  the field of view, and shifts the projection origin by (-w/2,-h/2). 
	void discretize(int w,int h,double hFOV);
	
	/// Like discretize, but flips the Y axis (for typical raster viewing)
	void discretizeFlip(int w,int h,double hFOV);
	
	/// Flip the image's Y axis (for typical raster viewing)
	void flip(void);
	
	/// Extract a window with this width and height, with origin at this pixel
	void window(int w,int h, int x,int y);
	
	/// For use by pup:
	Viewpoint() { wid=ht=-1; }

/** Extraction-- "get" routines. **/
	/// Return the center of projection (eye point)
	inline const Vector3d &getEye(void) const {return E;}
	/// Return true if this is an orthographic (perspective-free) camera.
	inline bool isOrthographic(void) const {return !isPerspective;}
	
	/// Return the projection plane origin (View Reference Point)
	inline const Vector3d &getOrigin(void) const {return R;}
	/// Return the projection plane pixel-length X axis
	inline const Vector3d &getX(void) const {return X;}
	/// Return the projection plane pixel-length Y axis (View Up Vector)
	inline const Vector3d &getY(void) const {return Y;}
	/// Return the z-unit-length Z axis (from reference towards camera)
	inline const Vector3d &getZ(void) const {return Z;}
	
	/// Return the number of pixels in the X direction
	inline int getXsize(void) const { return wid; }
	inline int getWidth(void) const { return wid; }
	/// Return the number of pixels in the Y direction
	inline int getYsize(void) const { return ht; }
	inline int getHeight(void) const { return ht; }
	
	/// Return the 4x4 projection matrix.  This is a column-wise
	/// matrix, meaning the translation portion is the rightmost column.
	inline const ViewMatrix3d &getMatrix(void) const {return m;}
	
	/// Return a 16-entry OpenGL-compatible projection matrix for this viewpoint.
	///   z_near and z_far are the depth buffer minimum and maximum Z's.
	void makeOpenGL(double *dest,double z_near,double z_far) const;
	
/**** Projection **/
	/// Project this point into the camera volume.
	///  The projected screen point is (return.x,return.y);
	///
	///  return.z is 1.0/depth: +inf at the eye to 1 at the projection plane.
	///  This is the "perspective scale value"; the screen size 
	///  multiplier needed because of perspective.
	inline Vector3d project(const Vector3d &in) const {
		float w=1.0f/(float)(
		  m(3,0)*in.x+m(3,1)*in.y+m(3,2)*in.z+m(3,3)
		);
		return Vector3d(
		  w*(m(0,0)*in.x+m(0,1)*in.y+m(0,2)*in.z+m(0,3)),
		  w*(m(1,0)*in.x+m(1,1)*in.y+m(1,2)*in.z+m(1,3)),
		  w*(m(2,0)*in.x+m(2,1)*in.y+m(2,2)*in.z+m(2,3))
		);
	}
	
	/// Project this point onto the screen, returning zero for the z axis.
	/// Slightly more efficient than normal "project".
	Vector2d project_noz(const Vector3d &in) const {
		float w=1.0f/(float)(
		  m(3,0)*in.x+m(3,1)*in.y+m(3,2)*in.z+m(3,3)
		);
		return Vector2d(
		  w*(m(0,0)*in.x+m(0,1)*in.y+m(0,2)*in.z+m(0,3)),
		  w*(m(1,0)*in.x+m(1,1)*in.y+m(1,2)*in.z+m(1,3))
		);
	}
	
/**** Clipping ****/
	enum {nClip=4};
	
	/// Get our i'th clipping plane.
	///  0 and 1 are the left and right horizontal clip planes.
	///  2 and 3 are the top and bottom vertical clip planes.
	Halfspace3d getClip(int i) const; 
	
	/// Return true if any convex combination of these points is still offscreen.
	bool allOffscreen(int n,const Vector3d *p) const {
		for (int c=0;c<nClip;c++) {
			Halfspace3d h=getClip(c);
			int i;
			for (i=0;i<n;i++)
				if (h.side(p[i])>=0)
					break; /* this point in-bounds */
			if (i==n) /* every point was outside halfspace h */
				return true;
		}
		return false; /* not all points outside the same halfspace */
	}
	
	/// Return true if this 2d screen location is (entirely) in-bounds:
	bool isInbounds(const Vector2d &screen) const {
		return (screen.x>=0) && (screen.y>=0) &&
			(screen.x<wid) && (screen.y<ht);
	}
	/// Clip this 2d vector to lie totally onscreen
	void clip(Vector2d &screen) const {
		if (screen.x<0) screen.x=0;
		if (screen.y<0) screen.y=0;
		if (screen.x>wid) screen.x=wid;
		if (screen.y>ht) screen.y=ht;
	}

/*** Viewport and Viewplane manipulation ***/
	/// Backproject this view plane point into world coordinates
	inline Vector3d viewplane(const Vector2d &v) const {
		return R+v.x*X+v.y*Y;
	}
	/// Project to the viewplane, the back-project this universe point.
	///   Maps everything onto the viewplane (without clipping).
	inline Vector3d projectViewplane(const Vector3d &u) const {
		return viewplane(project_noz(u));
	}
	
	//Get the universe-coords view ray passing through this universe point
	Ray getRay(const Vector3d &univ) const {
		return Ray(getEye(),univ-getEye());
	}
	//Get the universe-coords view ray passing through this screen point
	Ray getPixelRay(const Vector2d &screen) const {
		if (isPerspective)
			return Ray(getEye(),viewplane(screen)-getEye());
		else /* orthogonal camera */
			return Ray(viewplane(screen)-1000.0*Z, Z);
	}
	/// Get a universe-coords vector pointing to the camera from this point
	Vector3d toCamera(const Vector3d &pt) const {
		if (isPerspective)
			return getEye()-pt;
		else /* parallel projection */
			return Z;
	}
	
#ifdef __CK_PUP_H
	void pup(PUP::er &p);
#endif
}; /* end Viewpoint3d */

/// X, Y, and Z axes: a right-handed frame, used for navigation
class Axes3d {
	Vector3d axes[3]; //X, Y, and Z axes (orthonormal set)
	
	Vector3d &x(void) {return axes[0];}
	Vector3d &y(void) {return axes[1];}
	Vector3d &z(void) {return axes[2];}

	//Make our axes orthogonal again by forcing z from x and y
	void ortho(void) {
		z()=x().cross(y());
		y()=z().cross(x());
	}
	//Make our axes all unit length
	void normalize(void) {
		for (int i=0;i<3;i++)
			axes[i]=axes[i].dir();
	}
public:	
	Axes3d() {
		axes[0]=Vector3d(1,0,0);
		axes[1]=Vector3d(0,1,0);
		axes[2]=Vector3d(0,0,1);
	}
	
	//Get our x, y, and z axes (unit vectors):
	const Vector3d &getX(void) const {return axes[0];}
	const Vector3d &getY(void) const {return axes[1];}
	const Vector3d &getZ(void) const {return axes[2];}
	
	//Push our x and y axes in the +z direction by these amounts.
	//  If dx and dy are small, they correspond to a right-handed
	//  rotation about the -Y and -X directions.
	void nudge(double dx,double dy) {
		x()+=dx*z();
		y()+=dy*z();
		ortho(); normalize();
	}
	//Rotate around our +Z axis by this differential amount.
	void rotate(double dz) {
		x()+=dz*y();
		y()-=dz*x();
		ortho(); normalize();
	}
}; /* end Axes3d */


}; /* end namespace */

#ifdef __CK_PUP_H
inline void osl::Viewpoint::pup(PUP::er &p) {
	p.comment("Viewpoint {");
	p.comment("axes");
	p|X; p|Y; p|Z;
	p.comment("origin");
	p|R;
	p.comment("eye");
	p|E;
	p.comment("width and height");
	p|wid; p|ht;
	p.comment("perspective");
	p|isPerspective;
	p.comment("} Viewpoint");
	if (p.isUnpacking()) buildM();
}
#endif

#endif /*def(thisHeader)*/
