/* Random Number Generation routines
collected from Paul Borke's homepage at
http://www.swin.edu.au/astronomy/pbourke/
and converted to C++ (modelled after java.util.Random) by 
Orion Sky Lawlor, olawlor@acm.org, 11/1/1999
*/
#ifndef __OSL_RANDOM_H
#define __OSL_RANDOM_H

namespace osl {

/* Find a 64-bit type */
#ifdef _WIN32
typedef __int64 int64;
#else
typedef long long int64;
#endif

/*Abstract superclass*/
class Random {
	bool haveNextNextGaussian;
	double nextNextGaussian;
public:
	Random() {haveNextNextGaussian=false;}
	virtual ~Random();
	
	virtual void setSeed(int seed)=0;

	//Return the next integer in the sequence
	// with the low "bits" Random.
	virtual int next(int bits)=0;
	//Return the next integer in the sequence between zero and n-1.
	virtual int nextInt(int n);
	
	//Return the next floating-Point number on [0,1)
	virtual float nextFloat(void);
	
	//Return the next double-precision floating-Point number on [0,1)
	virtual double nextDouble(void);
	
	//Return the next unit-variance, zero-mean gaussian
	virtual double nextGaussian(void);
};

/*Implementation class: uses a simple 32-bit linear
congruential Random number generator.  "next" implements
the classic:
	seed=(seed*mul+add) mod 2^32;
*/
class Random32:public Random {
protected:
	int seed,mul,add;
public:
        Random32(int s=1) {setSeed(s);}
	virtual void setSeed(int nSeed);
	virtual void setParameters(int nSeed,int nMul,int nAdd);
	virtual int next(int bits);
	
	inline int nextInline(int bits) {
		seed = 0xffffFFFF&(seed * mul + add);
		unsigned int ret=(unsigned int)seed;
		// ret=(ret<<16)|(ret>>16);
		return (int)(((unsigned int)ret) >> (32 - bits));
	}
	//Return the next floating-Point number on [0,1)
	inline float nextFloatInline(void) {
		const float float_bitScale=1.0/ ((float)(1 << 24));
		return nextInline(24)*float_bitScale;
	}
};

/*Implementation class: uses Donald Knuth's excellent
48-bit linear congruential Random number generator*/
class Random48:public Random {
protected:
	int64 seed;
	static int64 multiplier;
public:
        Random48(int s=1) {setSeed(s);}
	virtual void setSeed(int nSeed);
	virtual int next(int bits);
};

/*Implementation class: uses the Marsaglia-Zaman 
delayed Fibbonaci Random number generator. Only 
24 bits of nextDouble() are actually Random.
*/
class RandomMz:public Random {
protected:
/*State variables:*/
	double u[97],c,cd,cm;
	int i97,j97;
public:
        RandomMz(int s=1) {setSeed(s);}	
	virtual void setSeed(int seed);

	//Return the next integer in the sequence
	// with the low "bits" Random.
	virtual int next(int bits);
	//Return the next integer in the sequence between zero and n-1.
	virtual int nextInt(int n);
	
	//Return the next floating-Point number on [0,1)
	virtual float nextFloat(void);
	//Return the next double-precision floating-Point number on [0,1)
	// **ONLY 24 BITS are Random!**
	virtual double nextDouble(void);
};

}; //end namespace osl
#endif /* def(thisHeader) */


