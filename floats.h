/**
 SSE and AVX implementation of Dr. Lawlor's "floats" class.

 The basic idea is to build an operator-overloaded parallel-data
 class, so given "floats a,b;" you can write "floats c=2.7*a+b;".

 In SSE, a "floats" has 4 individual floats (see floats::n).
 In AVX, a "floats" has 8 individual floats.

 For a bigger but more full-featured implementation, see
 Intel's Array Building Blocks (ArBB) "f32" class.

 Dr. Orion Lawlor, lawlor@alaska.edu, 2012-10-22 (public domain)
*/
#ifndef __OSL_FLOATS_H__
#include <iostream> /* for std::ostream */

#if defined(__AVX__)
/*********************** 8-float AVX version *************************/
#include <immintrin.h> /* Intel's AVX intrinsics header */

class floats; // forward declaration

// One set of boolean values
class bools {
	__m256 v; /* 8 boolean values, represented as all-zeros or all-ones 32-bit masks */
public:
	enum {n=8}; /* number of bools inside us */
	bools(__m256 val) {v=val;}
	__m256 get(void) const {return v;}
	
	// Initialize all our fields to all-ones (true), or all-zeros (false).
	bools(bool value) {
		int iv=(value?~0:0);
		v=_mm256_broadcast_ss((float *)&iv);
	}
	
	/* Combines sets of logical operations */
	bools operator&&(const bools &rhs) const {return _mm256_and_ps(v,rhs.v);}
	bools operator||(const bools &rhs) const {return _mm256_or_ps(v,rhs.v);}
	bools operator!=(const bools &rhs) const {return _mm256_xor_ps(v,rhs.v);}
	void operator&=(const bools &rhs) {v=_mm256_and_ps(v,rhs.v);}
	void operator|=(const bools &rhs) {v=_mm256_or_ps(v,rhs.v);}
	
	/* Use masking to combine the then_part (for true bools) and else part (for false bools). */
	floats if_then_else(const floats &then,const floats &else_part) const; 
	/* Return trueval if we're true, or zero if we're false. */
	floats this_or_zero(const floats &trueval) const;
	floats zero_or_this(const floats &falseval) const;
	
	/** 
	  Return true if *all* our bools are equal to this single value.
	*/
	bool operator==(bool allvalue) const {
		int m=_mm256_movemask_ps(v); /* 8 bits == high bits of each of our bools */
		if (allvalue==false)    return m==0; /* all false */
		else /*allvalue==true*/ return m==255; /* all true (every bit set) */
	}
	
	/**
	  Return true if *any* of our bools are true.
	*/
	bool any(void) const {
		return _mm256_movemask_ps(v)!=0;
	}
	
	
	/* Extract one bool from our set.  index must be between 0 and 3 */
	bool operator[](int index) const { return !!(_mm256_movemask_ps(v)&(1<<index)); }

	friend std::ostream &operator<<(std::ostream &o,const bools &y) {
		for (int i=0;i<n;i++) o<<(y[i]?"true ":"false")<<" ";
		return o;
	}
};


/**
  Represents an entire set of float values.
  
CAUTION!  Storing this class in a std::vector may cause a segfault at runtime!
The problem is std::vector doesn't do 32-byte alignment correctly.
   use a std::vector<T,alignocator<T,32> > to prevent these crashes!
*/
class floats {
	__m256 v; /* 8 floating point values */
public:
	enum {n=8}; /* number of floats inside us */
	floats() {}
	floats(__m256 val) {v=val;}
	void operator=(__m256 val) {v=val;}
	__m256 get(void) const {return v;}
	floats(float x) {v=_mm256_broadcast_ss(&x);}
	void operator=(float x) {v=_mm256_broadcast_ss(&x);}
	
	/* Load from unaligned memory */
	floats(const float *src) {v=_mm256_loadu_ps(src);}
	void operator=(const float *src) {v=_mm256_loadu_ps(src);}

	/* Load from 32-byte aligned memory */
	void load_aligned(const float *src) {v=_mm256_load_ps(src);}
	
	/** Basic arithmetic, returning floats */
	friend floats operator+(const floats &lhs,const floats &rhs)
		{return _mm256_add_ps(lhs.v,rhs.v);}
	friend floats operator-(const floats &lhs,const floats &rhs)
		{return _mm256_sub_ps(lhs.v,rhs.v);}
	friend floats operator*(const floats &lhs,const floats &rhs)
		{return _mm256_mul_ps(lhs.v,rhs.v);}
	friend floats operator/(const floats &lhs,const floats &rhs)
		{return _mm256_div_ps(lhs.v,rhs.v);}
	floats operator+=(const floats &rhs) {v=_mm256_add_ps(v,rhs.v); return *this; }
	floats operator-=(const floats &rhs) {v=_mm256_sub_ps(v,rhs.v); return *this; }
	floats operator*=(const floats &rhs) {v=_mm256_mul_ps(v,rhs.v); return *this; }
	floats operator/=(const floats &rhs) {v=_mm256_div_ps(v,rhs.v); return *this; }
	
	/** Comparisons, returning "bools" */
	bools operator==(const floats &rhs) const {return _mm256_cmp_ps(v,rhs.v,_CMP_EQ_OQ);}
	bools operator!=(const floats &rhs) const {return _mm256_cmp_ps(v,rhs.v,_CMP_NEQ_OQ);}
	bools operator<(const floats &rhs) const {return _mm256_cmp_ps(v,rhs.v,_CMP_LT_OQ);}
	bools operator<=(const floats &rhs) const {return _mm256_cmp_ps(v,rhs.v,_CMP_LE_OQ);}
	bools operator>(const floats &rhs) const {return _mm256_cmp_ps(v,rhs.v,_CMP_GT_OQ);}
	bools operator>=(const floats &rhs) const {return _mm256_cmp_ps(v,rhs.v,_CMP_GE_OQ);}

	/* Store to unaligned memory */
	void store(float *ptr) const { _mm256_storeu_ps(ptr,v); }
	/* Store with mask--won't compile with new __m256i avxintrin.h
	void store_mask(float *ptr,const bools &mask) const 
		{ _mm256_maskstore_ps(ptr,(__mm256i)mask.get(),v); }
	*/
	/* Store to 256-bit aligned memory (if not aligned, will segfault!) */
	void store_aligned(float *ptr) const { _mm256_store_ps(ptr,v); }

	/* Round our value to an integer, towards zero (standard C++ conversion) */
	floats trunc(void) const {return _mm256_cvtepi32_ps(_mm256_cvttps_epi32(v));}

	/* Extract one float from our set.  index must be between 0 and 7 */
	float &operator[](int index) { return ((float *)&v)[index]; }
	float operator[](int index) const { return ((const float *)&v)[index]; }

	friend std::ostream &operator<<(std::ostream &o,const floats &y) {
		for (int i=0;i<n;i++) o<<y[i]<<" ";
		return o;
	}
};

inline floats bools::if_then_else(const floats &then,const floats &else_part) const {
	return _mm256_or_ps( _mm256_and_ps(v,then.get()),
		 _mm256_andnot_ps(v, else_part.get())
	);
}
inline floats bools::this_or_zero(const floats &trueval) const {
	return _mm256_and_ps(v,trueval.get());
}
inline floats bools::zero_or_this(const floats &falseval) const {
	return _mm256_andnot_ps(v, falseval.get());
}
inline floats max(const floats &a,const floats &b) {
	return _mm256_max_ps(a.get(),b.get());
}
inline floats min(const floats &a,const floats &b) {
	return _mm256_min_ps(a.get(),b.get());
}
inline floats sqrt(const floats &v) {return _mm256_sqrt_ps(v.get());}
inline floats rsqrt(const floats &v) {return _mm256_rsqrt_ps(v.get());}



/** One set of integer values.  These aren't much good for arithmetic, but
   they're useful for exponent manipulation stuff.  Because AVX doesn't have
   8-way integer operations yet, these devolve to SSE2 128-bit operations.
*/
class ints {
	__m128i L; /* low 4 32-bit integer values, packed into one register */
	__m128i H; /* high 4 32-bit integer values, packed into one register */
public:
	enum {n=8}; 
	ints() {} /* default uninitialized */
	ints(int value) {L=H=_mm_set1_epi32(value);} // splat constant across all eight ints
	void set(__m256i v) {
		L=_mm256_extractf128_si256(v,0); 
		H=_mm256_extractf128_si256(v,1);
	}
	__m256i get(void) const {
		return _mm256_insertf128_si256(_mm256_castsi128_si256(L),H,1);
	}
	
	/* Set us from the values of these floats.  Uses current rounding mode (to nearest) */
	void from_values(floats floatv) { set(_mm256_cvtps_epi32(floatv.get())); }

	/* Set us from the values of these floats.  Always rounds toward zero (truncation) */
	void from_values_trunc(floats floatv) {set(_mm256_cvttps_epi32(floatv.get()));}
	
	/* Set us from the bits of these floats.  Useful for exponent manipulation */
	void from_bits(floats floatv) {set(_mm256_castps_si256(floatv.get()));}

	/* Set us to 00000 for false, 111111 for true.  Useful for masking. */
	void from_bools(bools b) {set(_mm256_castps_si256(b.get()));}
	
	/* Extract our bits as floats (for bitwise operations) */
	floats bits_to_floats(void) const {return _mm256_castsi256_ps(get());}

	/* Extract our value as floats (simple integer conversion) */
	floats value_to_floats(void) const {return _mm256_cvtepi32_ps(get());}

#if 0 /* Supposedly AVX2 (Haswell New Instructions) will support integer operations on mm256 */
	... _mm256_add_epi32 ...
#else /* Fake the above instructions by breaking AVX down to SSE halves */

#define TWO_PIECES(operation,left,right) \
	L=operation(left L,right L); \
	H=operation(left H,right H); \
	
	/* Arithmetic operators */
	friend ints operator+(ints lhs,const ints &rhs) {lhs+=rhs; return lhs;}
	friend ints operator-(ints lhs,const ints &rhs) {lhs-=rhs; return lhs;}
	friend ints operator*(ints lhs,const ints &rhs) {lhs*=rhs; return lhs;}
	friend ints operator&(ints lhs,const ints &rhs) {lhs&=rhs; return lhs;}
	friend ints operator|(ints lhs,const ints &rhs) {lhs|=rhs; return lhs;}
	friend ints operator^(ints lhs,const ints &rhs) {lhs^=rhs; return lhs;}
	friend ints operator<<(ints lhs,const int count) {lhs<<=count; return lhs;}
	friend ints operator>>(ints lhs,const int count) {lhs>>=count; return lhs;}
	ints operator+=(const ints &rhs) {TWO_PIECES(_mm_add_epi32,,rhs.); return *this; }
	ints operator-=(const ints &rhs) {TWO_PIECES(_mm_sub_epi32,,rhs.); return *this; }
	ints operator*=(const ints &rhs) {TWO_PIECES(_mm_mul_epu32,,rhs.); return *this; }
	ints operator&=(const ints &rhs) {TWO_PIECES(_mm_and_si128,,rhs.); return *this; }
	ints operator|=(const ints &rhs) {TWO_PIECES(_mm_or_si128,,rhs.); return *this; }
	ints operator^=(const ints &rhs) {TWO_PIECES(_mm_xor_si128,,rhs.); return *this; }
	
#define TWO_PIECES_Rscalar(operation,left,right) \
	L=operation(left L,right); \
	H=operation(left H,right); 
	
	void operator<<=(const int count) {TWO_PIECES_Rscalar(_mm_slli_epi32,,count);}
	void operator>>=(const int count) {TWO_PIECES_Rscalar(_mm_srli_epi32,,count);}

#undef TWO_PIECES
#undef TWO_PIECES_Rscalar

#endif
	
	/* Extract one int from our set.  index must be between 0 and 7 */
	int &operator[](int index) { return ((int *)&L)[index]; }
	int operator[](int index) const { return ((const int *)&L)[index]; }
	
	friend std::ostream &operator<<(std::ostream &o,const ints &y) {
		for (int i=0;i<n;i++) o<<y[i]<<" ";
		return o;
	}
};




#elif defined(__SSE__) /* SSE implementation of above */
/********************** 4-float SSE version ***********************/

#include <emmintrin.h> /* Intel's SSE2 intrinsics header */

class floats; // forward declaration

// One set of boolean values
class bools {
	__m128 v; /* 4 boolean values, represented as all-zeros or all-ones 32-bit masks */
public:
	enum {n=4}; /* number of bools inside us */
	bools(__m128 val) {v=val;}
	__m128 get(void) const {return v;}
	// Initialize all our fields to all-ones (true), or all-zeros (false).
	bools(bool value) {v=(__m128)(_mm_set1_epi32(value?~0:0));}
	
	/* Combines sets of logical operations */
	bools operator&&(const bools &rhs) const {return _mm_and_ps(v,rhs.v);}
	bools operator||(const bools &rhs) const {return _mm_or_ps(v,rhs.v);}
	bools operator!=(const bools &rhs) const {return _mm_xor_ps(v,rhs.v);}
	void operator&=(const bools &rhs) {v=_mm_and_ps(v,rhs.v);}
	void operator|=(const bools &rhs) {v=_mm_or_ps(v,rhs.v);}
	
	/* Use masking to combine the then_part (for true bools) and else part (for false bools). */
	floats if_then_else(const floats &then,const floats &else_part) const; 

	/* Return trueval if we're true, or zero if we're false. */
	floats this_or_zero(const floats &trueval) const;
	floats zero_or_this(const floats &falseval) const;
	
	/** 
	  Return true if *all* our bools are equal to this single value.
	*/
	bool operator==(bool allvalue) const {
		int m=_mm_movemask_ps(v); /* 4 bits == high bits of each of our bools */
		if (allvalue==false)    return m==0; /* all false */
		else /*allvalue==true*/ return m==15; /* all true (every bit set) */
	}
	
	/**
	  Return true if *any* of our bools are true.
	*/
	bool any(void) const {
		return _mm_movemask_ps(v)!=0;
	}
	
	
	/* Extract one bool from our set.  index must be between 0 and 3 */
	bool operator[](int index) const { return !!(_mm_movemask_ps(v)&(1<<index)); }

	friend std::ostream &operator<<(std::ostream &o,const bools &y) {
		for (int i=0;i<n;i++) o<<(y[i]?"true ":"false")<<" ";
		return o;
	}
};

/**
  Represents an entire set of float values.
*/
class floats {
	__m128 v; /* 4 floating point values */
public:
	enum {n=4}; /* number of floats inside us */
	floats() {}
	floats(__m128 val) {v=val;}
	void operator=(__m128 val) {v=val;}
	__m128 get(void) const {return v;}
	floats(float x) {v=_mm_load1_ps(&x);}
	void operator=(float x) {v=_mm_load1_ps(&x);}
	
	floats(float a,float b,float c,float d) 
		{v=_mm_setr_ps(a,b,c,d);}
	
	// By default, assume data is not aligned
	floats(const float *src) {v=_mm_loadu_ps(src);}
	void operator=(const float *src) {v=_mm_loadu_ps(src);}
	
	// Use this if your data is 16-byte aligned.
	void load_aligned(const float *src) {v=_mm_load_ps(src);}
	
	/** Basic arithmetic, returning floats */
	friend floats operator+(const floats &lhs,const floats &rhs)
		{return _mm_add_ps(lhs.v,rhs.v);}
	friend floats operator-(const floats &lhs,const floats &rhs)
		{return _mm_sub_ps(lhs.v,rhs.v);}
	friend floats operator*(const floats &lhs,const floats &rhs)
		{return _mm_mul_ps(lhs.v,rhs.v);}
	friend floats operator/(const floats &lhs,const floats &rhs)
		{return _mm_div_ps(lhs.v,rhs.v);}
	floats operator+=(const floats &rhs) {v=_mm_add_ps(v,rhs.v); return *this; }
	floats operator-=(const floats &rhs) {v=_mm_sub_ps(v,rhs.v); return *this; }
	floats operator*=(const floats &rhs) {v=_mm_mul_ps(v,rhs.v); return *this; }
	floats operator/=(const floats &rhs) {v=_mm_div_ps(v,rhs.v); return *this; }
	
	/** Comparisons, returning "bools" */
	bools operator==(const floats &rhs) const {return _mm_cmpeq_ps (v,rhs.v);}
	bools operator!=(const floats &rhs) const {return _mm_cmpneq_ps(v,rhs.v);}
	bools operator<(const floats &rhs) const  {return _mm_cmplt_ps (v,rhs.v);}
	bools operator<=(const floats &rhs) const {return _mm_cmple_ps (v,rhs.v);}
	bools operator>(const floats &rhs) const  {return _mm_cmpgt_ps (v,rhs.v);}
	bools operator>=(const floats &rhs) const {return _mm_cmpge_ps (v,rhs.v);}

	/* Store to unaligned memory */
	void store(float *ptr) const { _mm_storeu_ps(ptr,v); }
	/* Store to 128-bit aligned memory (if not aligned, will segfault!) */
	void store_aligned(float *ptr) const { _mm_store_ps(ptr,v); }

	/* Round our value to an integer, towards zero (standard C++ conversion) */
	floats trunc(void) const {return _mm_cvtepi32_ps(_mm_cvttps_epi32(v));}

	/* Extract one float from our set.  index must be between 0 and 3 */
	float &operator[](int index) { return ((float *)&v)[index]; }
	float operator[](int index) const { return ((const float *)&v)[index]; }

	friend std::ostream &operator<<(std::ostream &o,const floats &y) {
		for (int i=0;i<n;i++) o<<y[i]<<" ";
		return o;
	}
};

inline floats bools::if_then_else(const floats &trueval,const floats &falseval) const {
	return _mm_or_ps( _mm_and_ps(v,trueval.get()),
		 _mm_andnot_ps(v, falseval.get())
	);
}
inline floats bools::this_or_zero(const floats &trueval) const {
	return _mm_and_ps(v,trueval.get());
}
inline floats bools::zero_or_this(const floats &falseval) const {
	return _mm_andnot_ps(v, falseval.get());
}

inline floats max(const floats &a,const floats &b) {
	return _mm_max_ps(a.get(),b.get());
}
inline floats min(const floats &a,const floats &b) {
	return _mm_min_ps(a.get(),b.get());
}
inline floats sqrt(const floats &v) {return _mm_sqrt_ps(v.get());}
inline floats rsqrt(const floats &v) {return _mm_rsqrt_ps(v.get());}

#if defined(__SSE2__) 
/** One set of integer values.  These aren't much good for arithmetic, but
   they're useful for exponent manipulation stuff. */
class ints {
	__m128i v; /* 4 32-bit integer values, packed into one register */
public:
	enum {n=4}; 
	ints() {} /* default uninitialized */
	ints(__m128i val) {v=val;}
	__m128i get(void) const {return v;}
	ints(int value) {v=_mm_set1_epi32(value);} // splat constant across all four ints
	
	/* Set us from the values of these floats.  Uses current rounding mode (to nearest) */
	void from_values(floats floatv) {v=_mm_cvtps_epi32(floatv.get());}

	/* Set us from the values of these floats.  Always rounds toward zero (truncation) */
	void from_values_trunc(floats floatv) {v=_mm_cvttps_epi32(floatv.get());}
	
	/* Set us from the bits of these floats.  Useful for exponent manipulation */
	void from_bits(floats floatv) {v=(__m128i)(floatv.get());}

	/* Set us to 00000 for false, 111111 for true.  Useful for masking. */
	void from_bools(bools b) {v=(__m128i)(b.get());}
	
	/* Extract our bits as floats (for bitwise operations) */
	floats bits_to_floats(void) const {return (__m128)(v);}

	/* Extract our value as floats (simple integer conversion) */
	floats value_to_floats(void) const {return _mm_cvtepi32_ps(v);}

	/* Arithmetic operators */
	friend ints operator+(const ints &lhs,const ints &rhs)
		{return _mm_add_epi32(lhs.v,rhs.v);}
	friend ints operator-(const ints &lhs,const ints &rhs)
		{return _mm_sub_epi32(lhs.v,rhs.v);}
	friend ints operator*(const ints &lhs,const ints &rhs)
		{return _mm_mul_epu32(lhs.v,rhs.v);}
	ints operator+=(const ints &rhs) {v=_mm_add_epi32(v,rhs.v); return *this; }
	ints operator-=(const ints &rhs) {v=_mm_sub_epi32(v,rhs.v); return *this; }
	ints operator*=(const ints &rhs) {v=_mm_mul_epu32(v,rhs.v); return *this; }
	
	/* Bitwise operators */
	ints operator&(const ints &rhs) const {return _mm_and_si128(v,rhs.v);}
	ints operator|(const ints &rhs) const {return _mm_or_si128(v,rhs.v);}
	ints operator^(const ints &rhs) const {return _mm_xor_si128(v,rhs.v);}
	ints operator<<(const int count) const {return _mm_slli_epi32(v,count);}
	ints operator>>(const int count) const {return _mm_srli_epi32(v,count);}
	ints operator<<(ints counts) const {return _mm_sll_epi32(v,counts.v);}
	ints operator>>(ints counts) const {return _mm_srl_epi32(v,counts.v);}
	
	/* Extract one int from our set.  index must be between 0 and 3 */
	int &operator[](int index) { return ((int *)&v)[index]; }
	int operator[](int index) const { return ((const int *)&v)[index]; }

	friend std::ostream &operator<<(std::ostream &o,const ints &y) {
		for (int i=0;i<n;i++) o<<y[i]<<" ";
		return o;
	}
};
#endif /* SSE2 */

#else /* FIXME: altivec, etc */
#error "You need some sort of SSE or AVX intrinsics for osl/floats.h to work."
#endif


/* exp and log closely follow "sse_mathfun.h" here:
    http://gruntthepeon.free.fr/ssemath/
 In turn, he's loosely following http://www.netlib.org/cephes/
 He's got a "sin" and "cos" there as well, which I haven't imported yet.
*/
/* Compute natural logarithm of all floats, in parallel.   
   Returns NaN for non-positive inputs. */
inline floats log(floats x) {
	bools invalid=(x<=0.0);
	
	x=max(x,1.1755e-38f); // round up out of denormal range
	ints xbits; xbits.from_bits(x);
	ints exponents=(xbits>>23)-0x7f; // extract IEEE exponent field
	xbits=xbits&~0x7f800000; // extract fraction field
	x=(xbits|0x3f000000).bits_to_floats(); // tack on fixed exponent for 0.5-0.999
	floats e=exponents.value_to_floats(); // exponent
	
	/*
	if( x < SQRTHF ) { x = x + x - 1.0;
	} else { e+=1; x = x - 1.0; }
	*/
	bools small=(x<0.707106781186547524f);
	e=e + small.zero_or_this(1.0f);
	x=x + small.this_or_zero(x) - 1.0f;

	// Nested polynomial evaluation
	floats z=x*x;
	floats y=0.070376836292; // approximation for further terms
	y=y*x - 0.11514610310f; // -1/10+roundoff
	y=y*x + 0.11676998740f; // +1/9
	y=y*x - 0.12420140846f; // -1/8
	y=y*x + 0.14249322787f; // +1/7
	y=y*x - 0.16668057665f; // -1/6
	y=y*x + 0.20000714765f; // +1/5
	y=y*x - 0.24999993993f; // -1/4
	y=y*x + 0.33333331174f; // +1/3
	
	y=y*x*z + e*-2.12194440e-4f - z*0.5f; 
	x=x + y + e*0.693359375f; // subtle: e term in two parts, to avoid roundoff?

	return x+invalid.get(); // subtle: tack on NaN if invalid input
}


/* Raise e to the power x. */
inline floats exp(floats x) {
	x=min(x,88.3762626647949f); // clamp to valid range
	x=max(x,-88.3762626647949f);

	/* split up exp(x) into exp(g + n*log(2)) */
	floats fx=x*1.44269504088896341f-0.5;
	floats tmp=fx.trunc(); // this is for computing "floor"
	fx=tmp-((tmp>fx).this_or_zero(1.0)); // fix negative number round direction
	
	tmp=fx*0.693359375f;
	floats z=fx*-2.12194440e-4f;
	x=x-tmp-z;
	z=x*x;
	floats y=1.9875691500E-4f;
	y=y*x+1.3981999507E-3f;
	y=y*x+8.3334519073E-3f;
	y=y*x+4.1665795894E-2f;
	y=y*x+1.6666665459E-1f;
	y=y*x+5.0000001201E-1f;

	y=y*z+x+1.0f;
	ints fxi; fxi.from_values_trunc(fx); // integer power of two
	fxi=(fxi+0x7f)<<23; // stick in IEEE exponent field
	floats pow2n=fxi.bits_to_floats();
	return y*pow2n;
}

#endif /* defined(this header) */
