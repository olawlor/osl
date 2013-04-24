/* A simple TCP authenticated messaging protocol.

Orion Sky Lawlor, olawlor@acm.org, 2005/09/22 (Public Domain)
*/
#ifndef __OSL_AUTH_PIPE_H
#define __OSL_AUTH_PIPE_H

#include "socket.h"
#include "sha1.h"
#include <vector>

/* 
  Return a good random integer.
*/
int good_rand(void);

/**
 Authenticated message protocol.
 
 Both sides start by picking 4 random ints;
 this is 128 bits of randomness per side, used
 as nonces.  Both sides exchange nonces.
 Both sides should know who's on which side: A or B.
 
 On the wire, a message consists of:
	<32-bit big endian message size s>
	<s bytes of arbitrary message data>
	<20-byte SHA-1 hash code of:
		4 big endian 32-bit nonces picked by side A.
		4 nonces picked by side B.
		shared secret (see "reset" routine)
		s-byte data
	>
*/
class auth_pipe {
public:
	typedef enum {
		dir_A=0, dir_B=1
	} dir_t;
	/*
	  Prepare to send and receive messages on this socket.
	  Either side can send or receive messages repeatedly,
	  but the start/done executions have to match.
	  Will eventually close the socket.
	*/
	auth_pipe(const char *sharedSecret_,dir_t d_,SOCKET fd_);
	~auth_pipe();

/* Sending protocol: */
	/* Prepare to send */
	void send_start(void);
	
	/* Add this data to the message.  Data doesn't actually
	  leave until you hit "send_done" */
	void send(const void *b,int len);
	
	/* Send off the whole message. 
	   Called by default when switching to a receive.
	*/
	void send_done(void);

/* Receiving protocol: */
	/* Return how many bytes have arrived in this message. 
	   If "expect" is specified, and the message isn't that length,
	   abort.  Follow by calls to recv. 
	*/
	int recv_start(int expect=-1);
	
	/* Return the *next* len bytes of the message. 
	   Throws exception if there aren't len bytes remaining.
	*/
	const byte *recv(int len);
	/* As above, but copy the bytes here. */
	void recv(void *dest,int len);

	/* Done receiving */
	void recv_done(void);
	
	/* Return bytes left to receive */
	int recv_left(void);
	
	/* Finish any ongoing communication */
	void flush(void);

private:
	/* Message data, as going to or coming from the network */
	std::vector<byte> msg;
	int msgidx; /* current index into message */
	SOCKET fd;
	/* Hash of message data received so far */
	osl::SHA1_hasher h;
	/* Shared secret ASCII string */
	const char *sharedSecret;
	/* Nonces (random numbers) shared by sender and receiver */
	enum {n_nonce=4}; /*Nonces per send side */
	Big32 nonce[2*n_nonce];
	/* Message count, as Big32 integer */
	Big32 count;
	/* Reset hasher & prepare for a new hash */
	void reset(void);
	typedef enum {
		state_idle=0, state_send, state_recv
	} state_t;
	/* state */
	state_t state;
	
};

#endif

