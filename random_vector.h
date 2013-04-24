/*
Orion Sky Lawlor, olawlor@acm.org, 2004/9/29

Utilities for generating random vectors of various
types.  I keep having to rewrite these from scratch,
which is stupid.
*/
#ifndef __OSL_RANDOM_VECTOR_H
#define __OSL_RANDOM_VECTOR_H

#ifndef __OSL_RANDOM_H
#  include "random.h"
#endif

#ifndef __OSL_VECTOR2D_H
#  include "vector2d.h"
#endif

#ifndef __OSL_VECTOR3D_H
#  include "vector3d.h"
#endif

namespace osl {

/// Return a uniform random vector on [0,1)^3
template <class Random>
inline Vector3d nextCubeVector3d(Random &r) 
        { return Vector3d(r.nextFloat(),r.nextFloat(),r.nextFloat());}

/// Return a uniform random vector on [-0.5,0.5)^3
template <class Random>
inline Vector3d nextCenteredVector3d(Random &r) 
        { return Vector3d(r.nextFloat()-0.5,r.nextFloat()-0.5,r.nextFloat()-0.5);}

/// Return a uniform random vector on [-0.5,0.5)^2
template <class Random>
inline Vector2d nextCenteredVector2d(Random &r) 
        { return Vector2d(r.nextFloat()-0.5,r.nextFloat()-0.5);}

/// Return a random vector in the unit circle.
template <class Random>
inline Vector2d nextCircle(Random &r) {
        Vector2d v;
	double ms;
        do {
                v=nextCenteredVector2d(r);
		ms=v.magSqr();
        } while (ms>0.5*0.5); /* throw out vectors that are beyond the circle */
        return 2*v;
}

/// Return a random vector in the unit sphere.
template <class Random>
inline Vector3d nextSphere(Random &r) {
        Vector3d v;
	double ms;
        do {
                v=nextCenteredVector3d(r);
		ms=v.magSqr();
        } while (ms>0.5*0.5); /* throw out vectors that are beyond the sphere */
        return 2*v;
}

/// Return a random unit direction vector.
template <class Random>
inline Vector3d nextDirection(Random &r) {
        Vector3d v;
	double ms;
	/**
	  Subtle: this isn't just nextSphere(r).dir(), because doing so 
	  results in skewed statistics.  The problem is vectors very close
	  to the origin have high quantization error, and hence tend to point
	  along the coordinate axes.  The fix is to throw out too-short vectors.
	*/
        do {
                v=nextCenteredVector3d(r);
		ms=v.magSqr();
        } while (ms>0.5*0.5 || ms<0.2*0.2); /* throw out vectors that are too big or small */
        return v.dir();
}

};

#endif
