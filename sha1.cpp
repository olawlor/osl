/*************************************************************
Perform one round of the SHA-1 message hash.  Input is a set of
16 32-bit native words (512 bits); output is 5 32-bit native
words (160 bits). Because it uses native arithmetic, the 
implementation works equally well with 32 and 64-bit big-
and little-endian systems. However, when the input or output
is interpreted as bytes, they should be considered big-endian.
The speed is about 400,000 transformed blocks per second 
(25 MB/s) on a 1 GHz machine.

Implemented and placed in the public domain by Steve Reid
Collected by Wei Dai (http://www.eskimo.com/~weidai/cryptlib.html)
Adapted for Charm++ by Orion Sky Lawlor, olawlor@acm.org, 7/20/2001
Adapted for stream mode and Orion's Standard Library on 2004/1/2
*/
#include <stdlib.h> /* needed by Visual C++ for _lrotl */
#include "sha1.h"
using namespace osl;

/// Initialize SHA1_hash_words of state.
void osl::SHA1_init(SHA1_word32 *state)
{
        state[0] = 0x67452301u;
        state[1] = 0xEFCDAB89u;
        state[2] = 0x98BADCFEu;
        state[3] = 0x10325476u;
        state[4] = 0xC3D2E1F0u;
}

/// Circular left shift in 32 bits
inline SHA1_word32 rotlFixed(SHA1_word32 x, SHA1_word32 y)
{
#if defined(_MSC_VER) || defined(__BCPLUSPLUS__)
	return y ? _lrotl(x, y) : x;
#elif defined(__MWERKS__) && TARGET_CPU_PPC
	return y ? __rlwinm(x,y,0,31) : x;
#else /*Default C version*/
	return ((0xFFffFFffu)&(x<<y)) | (((0xFFffFFffu)&x)>>(32-y));
#endif
}

#define blk0(i) (W[i] = data[i])
#define blk1(i) (W[i&15] = rotlFixed(W[(i+13)&15]^W[(i+8)&15]^W[(i+2)&15]^W[i&15],1))

#define f1(x,y,z) (z^(x&(y^z)))
#define f2(x,y,z) (x^y^z)
#define f3(x,y,z) ((x&y)|(z&(x|y)))
#define f4(x,y,z) (x^y^z)

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=f1(w,x,y)+blk0(i)+0x5A827999u+rotlFixed(v,5);w=rotlFixed(w,30);
#define R1(v,w,x,y,z,i) z+=f1(w,x,y)+blk1(i)+0x5A827999u+rotlFixed(v,5);w=rotlFixed(w,30);
#define R2(v,w,x,y,z,i) z+=f2(w,x,y)+blk1(i)+0x6ED9EBA1u+rotlFixed(v,5);w=rotlFixed(w,30);
#define R3(v,w,x,y,z,i) z+=f3(w,x,y)+blk1(i)+0x8F1BBCDCu+rotlFixed(v,5);w=rotlFixed(w,30);
#define R4(v,w,x,y,z,i) z+=f4(w,x,y)+blk1(i)+0xCA62C1D6u+rotlFixed(v,5);w=rotlFixed(w,30);

void osl::SHA1_transform(SHA1_word32 *state, const SHA1_word32 *data)
{
	SHA1_word32 W[16];
	/* Copy state to working vars */
	SHA1_word32 a = state[0];
	SHA1_word32 b = state[1];
	SHA1_word32 c = state[2];
	SHA1_word32 d = state[3];
	SHA1_word32 e = state[4];
	/* 4 rounds of 20 operations each. Loop unrolled. */
	R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
	R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
	R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
	R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
	R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
	R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
	R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
	R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
	R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
	R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
	R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
	R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
	R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
	R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
	R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
	R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
	R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
	R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
	R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
	R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
	/* Add the working vars back into context.state[] */
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;
}


void osl::SHA1_hasher::init(void) {
	SHA1_init(state);
	bits=0;m=0;bacc=0;b=0;
}
void osl::SHA1_hasher::addBytes(const void *p,int l)
{
	const unsigned char *ptr=(const unsigned char *)p; 
	for (int i=0;i<l;i++) addByte(ptr[i]);
}
SHA1_hash_t osl::SHA1_hasher::end(void)
{
	SHA1_hash_t ret;
	SHA1_hash_t *out=&ret;
	
	/* Paste on the end-of-message and length fields */
	unsigned long int totBits=bits;
	addByte(0x80u);/*End-of-message: one followed by zeros*/
	while (b!=0 || m!=SHA1_data_words-2) addByte(0); /* zero padding */
	addWord((totBits>>16)>>16);/*High word of message length (normally zero)*/
	addWord(totBits);/*Low word: message length, in bits*/
	
	/* Make sure we really flushed everything */
	// if (b!=0 || m!=0) osl::bad("Logic error in SHA1_hasher!\n");
	
	/*Convert the result from words back to bytes*/
	for (int i=0;i<SHA1_hash_words;i++) {
		out->data[i*4+0]=0xffu & (state[i]>>24);
		out->data[i*4+1]=0xffu & (state[i]>>16);
		out->data[i*4+2]=0xffu & (state[i]>> 8);
		out->data[i*4+3]=0xffu & (state[i]>> 0);
	}

	/* Prepare for next run round */
	init();
	return ret;
}


#if SHA1_TEST_DRIVER
#include <stdio.h>
/* Tiny test driver routine-- should print out:
A9993E364706816ABA3E25717850C26C9CD0D89D
This is NIST example 1.
*/
int main(int argc,char *argv[])
{
	int i;
	SHA1_hash_t h;
	h=SHA1_hash("abc",3);
	for (i=0;i<sizeof(h);i++) printf("%02X",h.data[i]);
	printf("\n");
}
#endif
