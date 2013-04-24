/*
SHA-1 (Secure Hash Algorithm-1), as described in 
the NIST publication FIPS PUB 180-1.

Orion Sky Lawlor, olawlor@acm.org, 2004/1/2 (Public Domain)
*/
#ifndef __OSL_SHA1_H
#define __OSL_SHA1_H

#include <string.h> /* for memcmp */

namespace osl {

/********** Hash Basics (per chunk) ***********/
/** The output of the SHA-1 hash algorithm: 20 bytes. */
typedef struct {
	unsigned char data[20];
} SHA1_hash_t;

enum {SHA1_data_words=16}; /*Length of each chunk of input data (words)*/
enum {SHA1_hash_words=5}; /*Length of output hash code (words)*/

/*Contains at least the low 32 bits of a big-endian integer.*/
typedef unsigned int SHA1_word32;

/// Initialize SHA1_hash_words of state.
void SHA1_init(SHA1_word32 *state);

/// Add this SHA1_data_words chunk of data to this SHA1_hash_words of state.
void SHA1_transform(SHA1_word32 *state, const SHA1_word32 *data);


/************ Hash Stream Interface Code **********/
/**
  Computes a SHA-1 hash code on streamed input of
  arbitrary length.
*/
class SHA1_hasher {
	SHA1_word32 state[SHA1_hash_words];
	unsigned long int bits; /**< message length accumulator */
	
	SHA1_word32 message[SHA1_data_words]; /* word accumulator */
	int m;/* Words of message that have not been transformed */
	
	/// Add current (full) message buffer to output.
	void flushWords(void) {
		m=0;
		SHA1_transform(state,message);
	}
	/// Add this native 32-bit word to the message
	void addWord(int v) {
		message[m++]=v;
		if (m>=SHA1_data_words) flushWords();
	}
	
	SHA1_word32 bacc; /* byte accumulator */
	int b;/* bytes of message not yet transformed */
	
	/// Add current (full) message word to output.
	void flushBytes(void) {
		b=0; 
		addWord(bacc);
		bacc=0;
	}
	void init(void);
public:
	SHA1_hasher() {init();}
	
	/// Add this native 8-bit byte to the message
	void addByte(int v) {
		bits+=8;
		b++;
		bacc=(bacc<<8)+v;
		if (b>=4) flushBytes();
	}
	
	/// Add these bytes to this message.
	///  Can be called repeatedly to assemble a long message.
	void addBytes(const void *p,int l);
	
	/// End the message and extract the hash code.
	/// Can only be called once.
	SHA1_hash_t end(void);
};

inline SHA1_hash_t SHA1_hash(const void *p,int l) {
	SHA1_hasher h;
	h.addBytes(p,l);
	return h.end();
}

/*Compare two hashed values-- return 1 if they differ; 0 else
 */
inline int SHA1_differ(const SHA1_hash_t *a,const SHA1_hash_t *b)
{
  return 0!=memcmp(a->data,b->data,sizeof(SHA1_hash_t));
}


};

#endif
