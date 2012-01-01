/*
 *  connection.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.22.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------
 *
 */

#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <netinet/in.h>
#include <sys/time.h>
#include "chunk.h"
#include "../mem/memlock.h"
#include "../mem/opool.h"
#include "../struct/slist.h"
#include "../thread/mutex.h"
#include "../thread/thread.h"

//
// connection_endian
//
typedef enum
{
	CONNECTION_BIG_ENDIAN    = (1<<1),   // big endian
	CONNECTION_LITTLE_ENDIAN = (1<<2)    // little endian
} connection_endian;

//
// connection
//
struct connection
{
	void *context1;								// implementation specific context object
	void *context2;								// implementation specific context object
	
	connection_endian endian;			// endianness of the underlying connection
	
	uint64_t volatile data_sent;	// data sent (int bytes)
	uint64_t volatile data_rcvd;	// data received (in bytes)
	struct timeval tv;						// time connection was established
	
	slist_t recv_queue;						// 
	slist_t send_queue;						// 
	uint32_t volatile recvbuf;		// bytes waiting in the receive buffer
	uint32_t volatile sendbuf;		// bytes waiting in the send buffer
	
	mutex_t rxlock;								// mutex for receive buffer
	mutex_t txlock;								// mutex for send buffer
	mutex_t xxlock;								// mutex for sending/receiver
	
	thread1_t rthread;             // thread that reads from source into buffer
	
	int stop;                     // if true, stop; otherwise continue
	
	int (*send)(struct connection*, uint8_t*, uint32_t, uint32_t*);
	int (*recv)(struct connection*, uint8_t*, uint32_t, uint32_t*);
	int (*available)(struct connection*, uint32_t*);
};
typedef struct connection connection_t;





//
// function typedefs
//
typedef int (*send_func)(connection_t*, uint8_t*, uint32_t, uint32_t*);
typedef int (*recv_func)(connection_t*, uint8_t*, uint32_t, uint32_t*);
typedef int (*available_func)(connection_t*, uint32_t*);





/**
 * The various data members that are pointers (pool, recv_queue,
 * etc.) need to be set by the caller before the connection_t can be
 * used.
 */
int connection_init (connection_t*);

/**
 *
 */
int connection_destroy (connection_t*);





/**
 *
 */
int connection_open (connection_t*);

/**
 *
 */
int connection_close (connection_t*);





/**
 *
 */
int connection_send (connection_t*, uint8_t*, uint32_t, uint32_t*);

/**
 *
 */
int connection_write2 (connection_t*, uint32_t*, uint8_t*, ...);

/**
 *
 */
int connection_write_uint8 (connection_t*, uint8_t);

/**
 *
 */
int connection_write_uint16 (connection_t*, uint16_t);

/**
 *
 */
int connection_write_uint32 (connection_t*, uint32_t);

/**
 *
 */
int connection_write_uint64 (connection_t*, uint64_t);

/**
 *
 */
int connection_flush (connection_t*);





/**
 *
 */
int connection_recv (connection_t*, uint8_t*, uint32_t, uint32_t*);

/**
 * Read an unsigned 8-bit integer.
 */
int connection_read_uint8 (connection_t*, uint8_t*);

/**
 * Read and byte-swap an unsigned 16-bit integer.
 */
int connection_read_uint16 (connection_t*, uint16_t*);

/**
 * Read and byte-swap an unsigned 32-bit integer.
 */
int connection_read_uint32 (connection_t*, uint32_t*);

/**
 * Read and byte-swap an unisgned 64-bit integer.
 */
int connection_read_uint64 (connection_t*, uint64_t*);

/**
 * same as recv(), except that it doesn't move up the offset.
 */
int connection_peek (connection_t*, uint8_t*, uint32_t, uint32_t*);

/**
 * Scan forward through the receive buffer until we find data that matches the given data. Stop at
 * that point and discard everything that precedes it.
 */
int connection_scan (connection_t*, uint8_t*, uint32_t);

/**
 *
 */
int connection_available (connection_t*);

/**
 *
 */
int connection_dump (connection_t*);

/**
 * Completely empties the receive buffer.
 */
int connection_clear_rx (connection_t*);

/**
 *
 */
void connection_lock_rx (connection_t*);

/**
 *
 */
void connection_unlock_tx (connection_t*);

/**
 *
 */
void connection_reset (connection_t*);

#endif /* __CONNECTION_H__ */
