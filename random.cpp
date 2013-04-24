/* Random Number Generation routines
collected from Paul Borke's homepage at
http://www.swin.edu.au/astronomy/pbourke/
and converted to C++ by 
Orion Sky Lawlor, olawlor@acm.org, 11/1/1999
*/
#include <math.h>
#include "random.h"
using namespace osl;

osl::Random::~Random() {}

//Return the next integer in the sequence between zero and n-1.
int osl::Random::nextInt(int n)
{
	if ((n & -n) == n)  // i.e., n is a power of 2
		return (int)((n * (int64)next(31)) >> 31);

	int bits, val;
	do {
		bits = next(31);
		val = bits % n;
	} while(bits - val + (n-1) < 0);
	return val;
}

//Return the next floating-Point number on [0,1)
const static float float_bitScale=1.0/ ((float)(1 << 24));
float osl::Random::nextFloat(void) {
	return next(24) *float_bitScale;
}

//Return the next double-precision floating-Point number on [0,1)
const static double 
	double_loBitScale=1.0 / ((double)(1<<26)*(double)(1<<27)),
	double_hiBitScale=1.0 /  (double)(1<<26);
double osl::Random::nextDouble(void) {
	return next(26)*double_hiBitScale + next(27)*double_loBitScale;
}

//Return the next unit-variance, zero-mean gaussian
//   This uses the Box-Muller Transform method.
double osl::Random::nextGaussian(void) {
	if (haveNextNextGaussian) { //Return old cached value
		haveNextNextGaussian=false;
		return nextNextGaussian;
	}
	double v1, v2, s;
	do { 
		v1 = 2 * nextFloat() - 1;   // between -1.0 and 1.0
		v2 = 2 * nextFloat() - 1;   // between -1.0 and 1.0
		s = v1 * v1 + v2 * v2;
	} while (s >= 1);
	double multiplier = sqrt(-2 * log(s)/s);
	nextNextGaussian = v2 * multiplier;
	haveNextNextGaussian = true;
	return v1 * multiplier;
}

/************** Simple 32-bit (crappy) Linear Congruential Generator *********/
void osl::Random32::setSeed(int nSeed) {
	seed=nSeed ^ 0x7fce66d0;
	//Mul and add from the VAX Random number generator.
	// Note that the low-order (even/odd) bit just 
	// repeats "0101010101..."
	mul=69069;
	add=1;
}
void osl::Random32::setParameters(int nSeed,int nMul,int nAdd) {
	seed=nSeed;mul=nMul;add=nAdd;
}
int osl::Random32::next(int bits) {
	seed = 0xffffFFFF&(seed * mul + add);
	unsigned int ret=(unsigned int)seed;
	ret=(ret<<16)|(ret>>16);
	return (int)(((unsigned int)ret) >> (32 - bits));
}

/************** Much better (Knuth) 48-bit LCG ****************/
int64 osl::Random48::multiplier=(((int64)0x5DEEC)<<16)|(0xE66D);
void osl::Random48::setSeed(int nSeed) {
	seed=nSeed ^ multiplier;
}
int osl::Random48::next(int bits) {
	static const int64 mask48=(((int64)1) << 48) - 1;
	seed = (seed * multiplier + 0xB) & mask48;
	return (int)(seed >> (48 - bits));
}

/****************** Marsaglia/Zaman 10^30 Random generator *****************
   This Random Number Generator is based on the algorithm in a FORTRAN
   version published by George Marsaglia and Arif Zaman, Florida State
   University; ref.: see original comments below.
   At the fhw (Fachhochschule Wiesbaden, W.Germany), Dept. of Computer
   Science, we have written sources in further languages (C, Modula-2
   Turbo-Pascal(3.0, 5.0), Basic and Ada) to get exactly the same test
   results compared with the original FORTRAN version.
   April 1989
   Karl-L. Noell <NOELL@DWIFH1.BITNET>
      and  Helmut  Weber <WEBER@DWIFH1.BITNET>

   This Random number generator originally appeared in "Toward a Universal
   Random Number Generator" by George Marsaglia and Arif Zaman.
   Florida State University Report: FSU-SCRI-87-50 (1987)
   It was later modified by F. James and published in "A Review of Pseudo-
   Random Number Generators"
   THIS IS THE BEST KNOWN RANDOM NUMBER GENERATOR AVAILABLE.
   (However, a newly discovered technique can yield
   a period of 10^600. But that is still in the development stage.)
   It passes ALL of the tests for Random number generators and has a period
   of 2^144, is completely portable (gives bit identical results on all
   machines with at least 24-bit mantissas in the floating Point
   representation).
   The algorithm is a combination of a Fibonacci sequence (with lags of 97
   and 33, and operation "subtraction plus one, modulo one") and an
   "arithmetic sequence" (using subtraction).

   Use IJ = 1802 & KL = 9373 to test the Random number generator. The
   subroutine RANMAR should be used to generate 20000 Random numbers.
   Then display the next six Random numbers generated multiplied by 4096*4096
   If the Random number generator is working properly, the Random numbers
   should be:
           6533892.0  14220222.0  7275067.0
           6172232.0  8354498.0   10633180.0
*/


/*
   This is the initialization routine for the Random number generator.
   NOTE: The seed variables can have values between:    0 <= IJ <= 31328
                                                        0 <= KL <= 30081
   The Random number sequences created by these two seeds are of sufficient
   length to complete an entire calculation with. For example, if sveral
   different groups are working on different parts of the same calculation,
   each group could be assigned its own IJ seed. This would leave each group
   with 30000 choices for the second seed. That is to say, this Random
   number generator can create 900 million different subsequences -- with
   each subsequence having a length of approximately 10^30.
*/
void osl::RandomMz::setSeed(int seed)
{
	int ij=0xffff&(seed>>16),kl=0xffff&seed;/*OSL*/
   double s,t;
   int ii,i,j,k,l,jj,m;

   /*
      Handle the seed range errors
         First Random number seed must be between 0 and 31328
         Second seed must have a value between 0 and 30081
   */
   while (ij > 31328 )
   	ij-=31328;
   while (kl > 30081) 
   	kl-=30081;

   i = (ij / 177) % 177 + 2;
   j = (ij % 177)       + 2;
   k = (kl / 169) % 178 + 1;
   l = (kl % 169);

   for (ii=0; ii<97; ii++) {
      s = 0.0;
      t = 0.5;
      for (jj=0; jj<24; jj++) {
         m = (((i * j) % 179) * k) % 179;
         i = j;
         j = k;
         k = m;
         l = (53 * l + 1) % 169;
         if (((l * m % 64)) >= 32)
            s += t;
         t *= 0.5;
      }
      u[ii] = s;
   }

   c    = 362436.0 / 16777216.0;
   cd   = 7654321.0 / 16777216.0;
   cm   = 16777213.0 / 16777216.0;
   i97  = 97;
   j97  = 33;
}

/* 
   This is the Random number generator proposed by George Marsaglia in
   Florida State University Report: FSU-SCRI-87-50
*/
double osl::RandomMz::nextDouble(void)
{
   double uni;

   uni = u[i97-1] - u[j97-1];
   if (uni <= 0.0)
      uni+=1.0;
   u[i97-1] = uni;
   i97--;
   if (i97 == 0)
      i97 = 97;
   j97--;
   if (j97 == 0)
      j97 = 97;
   c -= cd;
   if (c < 0.0)
      c += cm;
   uni -= c;
   if (uni < 0.0)
      uni+=1.0;

   return(uni);
}
//Compute integer-type Random numbers from getDouble:

//Return the next integer in the sequence
// with the low "bits" Random.  Calls nextDouble twice if it needs
// the extra bits.
int osl::RandomMz::next(int bits)
{
	if (bits<=23)
		return (int)(nextDouble()*(1<<bits));
	else
		return (  (int)(nextDouble()*(1<<16))|
		         ((int)(nextDouble()*(1<<16))<<16))>>(32-bits);
}
//Return the next integer in the sequence between zero and n-1.
int osl::RandomMz::nextInt(int n)
{
	if (n<(1<<23))
	//This n is small enough that the 24-bit Randomness doesn't matter
		return (int)(nextDouble()*n);
	else
		return Random::nextInt(n);
}

//Return the next floating-Point number on [0,1)
float osl::RandomMz::nextFloat(void)
{
	return (float)nextDouble();
}
