OSL: Small C++ Utility Routines
  by Orion Sky Lawlor, lawlor@alaska.edu
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

To get the files:
   git clone git://github.com/olawlor/osl.git


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
  statistics.h: means, variances, standard deviations in an easy interface.

Of particular computer graphics interest:
  vec4.h: GLSL-style "vec3" and "vec4" classes, with overloads.
  vector3d.h: spatial vector class, with +, -, dot, cross, etc.
  vector_util.cpp: non-inlined routines for vector1d-4d.
  viewpoint.h/.cpp: 3D projective camera class.
  perlin_noise.h/.cpp: smooth 3D noise function (by Ken Perlin).

Of particular networking interest:
  socket.h/.cpp: portable easy-to-use TCP socket wrapper.
  sha1.h/.cpp: Secure Hash Algorithm-1.  FIPS standard crypto hash.
  sha2.h/.cpp: SHA-256 hash.
  authpipe.h/.cpp: network protocol for secret-key 
      authentiated messaging.
  webserver.h/.cpp: simple HTTP server
  webservice.h/.cpp: simple HTTP client
  webconfig.h/.cpp: modify application variables via HTTP 

Portability functions:
  dir.h/.cpp: portable directory traversal (list files in a directory)
  dll.h/.cpp: portable dynamically linked library interface.
  porthread.h/.cpp: portable threading library

Let me know if you find any problems!  lawlor@alaska.edu
