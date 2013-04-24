OSL: Small C++ Utility Routines
  by Orion Sky Lawlor, olawlor@acm.org
  released 2013-04-24 (Public Domain)

Here's a bunch of small, self-contained utility routines
to evaluate various mathematical functions, make portable
OS functions.  Just read the header files to see how 
they work--everything should be copiously commented.

You should be able to pick out one or two header files,
and use them without using anything else here.  You may 
have to follow the #includes to find the few other OSL 
headers these routines rely on.  Linking should also be 
easy--if you use the .h file, build the .cpp file too.

Everything relies on the "osl/" directory name, so just
build your code one level up from this directory and say
#include "osl/vector3d.h"

All these files have built recently on:
	- Windows machines with Visual C++
	- Mac OS X machines with xcode
	- Linux machines with gcc 4
But of course some minor tweaking may be needed.
No major "porting" changes should be necessary.

Of particular scientific computing and mathematical interest:
  vector1d-4d.h: spatial vector class, with +, -, dot, cross, etc.
  random.h/.cpp: pseudorandom number generators for int, float, etc.
      "Random48" or better is recommended for Monte Carlo.
  random_vector.h: 2D and 3D random vectors over cubes and spheres.
  least_squares.h/.cpp: linear least-squares function fitting.
  statistics.h: means, variances, standard deviations in an easy interface.
  matrix.h/.cpp: a dynamically allocated arbitrary-size matrix.
  fn1d.h: 1D function root-finding, with bisection and secant methods.

Of particular computer graphics interest:
  vector3d.h: spatial vector class, with +, -, dot, cross, etc.
  vector_util.cpp: non-inlined routines for vector1d-4d.
  viewpoint.h/.cpp: 3D projective camera class.
  perlin_noise.h/.cpp: smooth 3D noise function (by Ken Perlin).

Of particular portability and networking interest:
  dir.h/.cpp: portable directory traversal (list files in a directory)
  dll.h/.cpp: portable dynamically linked library interface.
  socket.h/.cpp: portable easy-to-use TCP socket wrapper.
  sha1.h/.cpp: Secure Hash Algorithm-1.  FIPS standard crypto hash.
  authpipe.h/.cpp: network protocol for secret-key 
      authentiated messaging.

In alphabetical order:
  authpipe.h/.cpp: network protocol for secret-key 
      authentiated messaging.
  bbox2d.h/bbox3d.h: 2D/3D bounding box routines, with
      box intersection, union, and creation from points.
  dir.h/.cpp: portable directory traversal (list files in a directory)
  dll.h/.cpp: portable dynamically linked library interface.
  fast_math.h: float-to-integer conversion, but faster than (int)f.
  fn1d.h: 1D function root-finding, with bisection and secant methods.
  integrate.h: decompose a 2D polygon into trapezoids for easy integration.
     Includes routines for center of mass and moment of inertia.
  least_squares.h/.cpp: linear least-squares function fitting.
  matrix2d.h: a 3x3 matrix, for 2D rotation/translation/scale/skew
  matrix3d.h: a 4x4 matrix, for 3D rotation/translation/scale/skew/projection
  matrix_util.cpp: implementation of 2D and 3D routines not in headers.
  matrix.h/.cpp: a dynamically allocated arbitrary-size matrix.
  matrixpower.h/.cpp: raise a Matrix2d to a fractional power.
  matrixT.h: a fast templated fixed-size matrix.  Used by 2D and 3D matrices.
  ndarray.h: an N-dimensional allocatable array.
  perlin_noise.h/.cpp: smooth 3D noise function (by Ken Perlin).
  pointinpolygon.h: determine if a point is inside a 2D polygon.
  polygon.h/.cpp: determine 2D polygon's area, center of mass, inertia, ...
  random.h/.cpp: pseudorandom number generators for int, float, etc.
      "Random48" or better is recommended for Monte Carlo.
  random_vector.h: 2D and 3D random vectors over cubes and spheres.
  sha1.h/.cpp: Secure Hash Algorithm-1.  FIPS standard crypto hash.
  socket.h/.cpp: portable easy-to-use TCP sockets.
  spheretrig.h/.cpp: areas and angles measured along a sphere.
  statistics.h: means, variances, standard deviations in an easy interface.
  vector1d-4d.h: spatial vector class, with +, -, dot, cross, etc.
  vector_util.cpp: non-inlined routines for vector1d-4d.
  viewpoint.h/.cpp: 3D projective camera class.
