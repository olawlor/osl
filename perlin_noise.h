/*
Smooth 3D pseudorandom noise generation.

Orion Sky Lawlor, olawlor@acm.org, 2005/12/14
*/
#ifndef __OSL_PERLIN_NOISE_H
#define __OSL_PERLIN_NOISE_H

namespace osl {

/**
 Generates noise on a regular grid, but uses a nice interpolation function to 
 make the result look smooth.  Output has a DC component of zero, and range
 on [-1,+1].  Zones of maximum contrast are around the half-integer values of
 the function, due to the gradient noise.

From the 
 JAVA REFERENCE IMPLEMENTATION OF IMPROVED NOISE - COPYRIGHT 2002 KEN PERLIN.
 http://mrl.nyu.edu/~perlin/noise/
  Translated to C++ and hacked by Orion Lawlor, 2005/12/13
*/
class PerlinNoise {
public:
   static double noise(double x, double y, double z);
   
private:
   static inline double fade(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }
   static inline double lerp(double t, double a, double b) { return a + t * (b - a); }
   static inline double grad(int hash, double x, double y, double z) {
      int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
      double u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
             v = h<4 ? y : h==12||h==14 ? x : z;
      return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
   }
};

};

#endif /* def(thisHeader) */
