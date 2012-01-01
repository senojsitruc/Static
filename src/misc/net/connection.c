/*
 *  connection.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.22.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "connection.h"
#include "chunk.h"
#include "../../core/core.h"
#include "../dump.h"
#include "../logger.h"
#include "../swap.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

void* __connection_read_loop (connection_t*);
int __connection_refill (connection_t*, uint32_t*);





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
connection_init (connection_t *connection)
{
	int error = 0;
	
	if (unlikely(connection == NULL))
		LOG_ERROR_AND_RETURN(-1, "null connection_t");
	
	if (unlikely(0 != (error = mutex_init(&connection->rxlock, LOCK_TYPE_SPIN))))
		LOG_ERROR_AND_GOTO(-101, fail, "failed to mutex_init(rxlock), 0x%08X [%d]", error, error);
	
	if (unlikely(0 != (error = mutex_init(&connection->txlock, LOCK_TYPE_SPIN))))
		LOG_ERROR_AND_GOTO(-102, fail, "failed to mutex_init(txlock), 0x%08X [%d]", error, error);
	
	if (unlikely(0 != (error = mutex_init(&connection->xxlock, LOCK_TYPE_SPIN))))
		LOG_ERROR_AND_GOTO(-103, fail, "failed to mutex_init(xxlock), 0x%08X [%d]", error, error);
	
	if (unlikely(0 != (error = slist_init(&connection->recv_queue, &core_pools()->slist_item))))
		LOG_ERROR_AND_GOTO(-104, fail, "failed to slist_init(recv_queue), %d", error);
	
	if (unlikely(0 != (error = slist_init(&connection->send_queue, &core_pools()->slist_item))))
		LOG_ERROR_AND_GOTO(-105, fail, "failed to slist_init(send_queue), %d", error);
	
	if (unlikely(0 != (error = thread_init(&connection->rthread, "connection-read", (thread_start_func)__connection_read_loop, connection, NULL))))
		LOG_ERROR_AND_GOTO(-106, fail, "failed to thread_init(rthread), %d", error);
	
	return 0;
	
fail:
	connection_destroy(connection);
	return error;
}

/**
 *
 *
 */
int
connection_destroy (connection_t *connection)
{
	if (unlikely(connection == NULL))
		LOG_ERROR_AND_RETURN(-1, "null connection_t");
	
	LOG5("sent=%llu, rcvd=%llu", connection->data_sent, connection->data_rcvd);
	
	mutex_destroy(&connection->rxlock);
	mutex_destroy(&connection->txlock);
	
	slist_destroy(&connection->recv_queue);
	slist_destroy(&connection->send_queue);
	
	thread_destroy(&connection->rthread);
	
	return 0;
}





#pragma mark -
#pragma mark open / close

/**
 *
 *
 */
int
connection_open (connection_t *connection)
{
	int error;
	
	if (unlikely(connection == NULL))
		LOG_ERROR_AND_RETURN(-1, "null connection_t");
	
	connection->stop = 0;
	
	if (unlikely(0 != (error = thread_start(&connection->rthread))))
		LOG_ERROR_AND_RETURN(-101, "failed to thread_start(rthread), %d", error);
	
	return 0;
}

/**
 *
 *
 */
int
connection_close (connection_t *connection)
{
	if (unlikely(connection == NULL))
		LOG_ERROR_AND_RETURN(-1, "null connection_t");
	
	connection->stop = 1;
	
	return 0;
}





#pragma mark -
#pragma mark write

/**
 *
 *
 */
int
connection_send (connection_t *connection, uint8_t *data, uint32_t data_i, uint32_t *data_o)
{
	int error = 0;
	size_t tlen=0, wlen;
	uint8_t *dataptr = data;
	chunk_t *chunk = NULL;
	
	if (unlikely(connection == NULL))
		LOG_ERROR_AND_RETURN(-1, "null connection_t");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-2, "null data");
	
	if (unlikely(data_i == 0))
		LOG_ERROR_AND_RETURN(-3, "invalid read length (0)");
	
	if (unlikely(data_o == NULL))
		LOG_ERROR_AND_RETURN(-4, "null data_o");
	
	mutex_lock(&connection->txlock);
	
	if (unlikely(0 != (error = slist_head(&connection->send_queue, (cobject_t**)&chunk))))
		LOG_ERROR_AND_GOTO(-101, done, "failed to slist_head, %d", error);
	
	// first look at the last chunk on the queue. if it has any space left on it, then append as much
	// of this data to that chunk as will fit.
	if (chunk != NULL && chunk->leng < sizeof(chunk->data)) {
		wlen = data_i;
		
		if (wlen > sizeof(chunk->data) - chunk->leng)
			wlen = sizeof(chunk->data) - chunk->leng;
		
		memcpy((void*)chunk->data+chunk->leng, dataptr, wlen);
		
		tlen += wlen;
		dataptr += wlen;
		chunk->leng += wlen;
		connection->sendbuf += wlen;
		
		chunk = NULL;
	}
	
	// grab new chunk_t's and append data until we've appended all of
	// the data. push the chunk_t's onto the send queue.
	while (tlen < data_i) {
		if (unlikely(0 != (error = core_chunk(&chunk)) || chunk == NULL))
			LOG_ERROR_AND_GOTO(-102, done, "failed to core_chunk, %d", error);
		
		wlen = sizeof(chunk->data) - chunk->leng;
		
		if (wlen > data_i - tlen)
			wlen = data_i - tlen;
		
		memcpy((void*)chunk->data+chunk->leng, dataptr, wlen);
		
		tlen += wlen;
		dataptr += wlen;
		chunk->leng += wlen;
		connection->sendbuf += wlen;
		
		if (unlikely(0 != (error = slist_push(&connection->send_queue, (cobject_t*)chunk)))) {
			chunk_release(chunk);
			LOG_ERROR_AND_BREAK("failed to slist_push(send_queue), %d", error);
		}
		
		chunk_release(chunk);
		chunk = NULL;
	}
	
done:
	mutex_unlock(&connection->txlock);
	*data_o = (uint32_t)tlen;
	return error;
}

/**
 *
 *
 */
inline int
connection_write2 (connection_t *connection, uint32_t *data_o, uint8_t *format, ...)
{
	int error=0, bytes;
	va_list listp;
	char buffer[1000];
	
	va_start(listp, format);
	bytes = vsnprintf(buffer, sizeof(buffer), (char*)format, listp);
	
	if (unlikely(0 != (error = connection_send(connection, (uint8_t*)buffer, (uint32_t)bytes, data_o))))
		LOG_ERROR_AND_GOTO(-101, done, "failed to connection_send, %d", error);
	
done:
	va_end(listp);
	return error;
}

/**
 *
 *
 */
inline int
connection_write_uint8 (connection_t *connection, uint8_t data)
{
	int error;
	uint32_t data_i=1, data_o=0;
	
	if (unlikely(0 != (error = connection_send(connection, &data, data_i, &data_o))))
		{ LOG_ERROR_AND_RETURN(-101, "failed to connection_send, %d", error); }
	else if (data_i != data_o)
		{ LOG_ERROR_AND_RETURN(-102, "failed; wrote %d of %d bytes", data_o, data_i); }
	
	return 0;
}

/**
 *
 *
 */
inline int
connection_write_uint16 (connection_t *connection, uint16_t data)
{
	int error;
	uint32_t data_i=2, data_o=0;
	
	if (connection->endian == CONNECTION_BIG_ENDIAN)
		data = swap16be(data);
	else if (connection->endian == CONNECTION_LITTLE_ENDIAN)
		data = swap16le(data);
	
	if (unlikely(0 != (error = connection_send(connection, (uint8_t*)&data, data_i, &data_o))))
		{ LOG_ERROR_AND_RETURN(-101, "failed to connection_send, %d", error); }
	else if (data_i != data_o)
		{ LOG_ERROR_AND_RETURN(-102, "failed; wrote %d of %d bytes", data_o, data_i); }
	
	return 0;
}

/**
 *
 *
 */
inline int
connection_write_uint32 (connection_t *connection, uint32_t data)
{
	int error;
	uint32_t data_i=4, data_o=0;
	
	if (connection->endian == CONNECTION_BIG_ENDIAN)
		data = swap32be(data);
	else if (connection->endian == CONNECTION_LITTLE_ENDIAN)
		data = swap32le(data);
	
	if (unlikely(0 != (error = connection_send(connection, (uint8_t*)&data, data_i, &data_o))))
		{ LOG_ERROR_AND_RETURN(-101, "failed to connection_send, %d", error); }
	else if (data_i != data_o)
		{ LOG_ERROR_AND_RETURN(-102, "failed; wrote %d of %d bytes", data_o, data_i); }
	
	return 0;
}

/**
 *
 *
 */
inline int
connection_write_uint64 (connection_t *connection, uint64_t data)
{
	int error;
	uint32_t data_i=8, data_o=0;
	
	if (connection->endian == CONNECTION_BIG_ENDIAN)
		data = swap64be(data);
	else if (connection->endian == CONNECTION_LITTLE_ENDIAN)
		data = swap64le(data);
	
	if (unlikely(0 != (error = connection_send(connection, (uint8_t*)&data, data_i, &data_o))))
		{ LOG_ERROR_AND_RETURN(-101, "failed to connection_send, %d", error); }
	else if (data_i != data_o)
		{ LOG_ERROR_AND_RETURN(-102, "failed; wrote %d of %d bytes", data_o, data_i); }
	
	return 0;
}

/**
 *
 *
 */
int
connection_flush (connection_t *connection)
{
	int error=0;
	uint32_t tlen=0, wlen;
	chunk_t *chunk = NULL;
	
	if (unlikely(connection == NULL))
		LOG_ERROR_AND_RETURN(-1, "null connection_t");
	
	if (unlikely(connection->send == NULL))
		LOG_ERROR_AND_RETURN(-2, "null connection->send callback");
	
	mutex_lock(&connection->txlock);
	
	while (1) {
		if (unlikely(0 != (error = slist_peek(&connection->send_queue, (cobject_t**)&chunk, 0))))
			LOG_ERROR_AND_GOTO(-101, done, "failed to slist_peek, %d", error);
		
		if (chunk == NULL)
			break;
		
		mutex_lock(&connection->xxlock);
		
		if (0 != (error = (*connection->send)(connection, (void*)chunk->data+chunk->offs, chunk->leng - chunk->offs, &wlen))) {
			mutex_unlock(&connection->xxlock);
			LOG_ERROR_AND_GOTO(-102, done, "failed to send, %d", error);
		}
		
		mutex_unlock(&connection->xxlock);
		
		chunk->offs += wlen;
		tlen += wlen;
		connection->data_sent += wlen;
		connection->sendbuf -= wlen;
		
		if (chunk->offs == chunk->leng) {
			if (unlikely(0 != (error = slist_pop(&connection->send_queue, (cobject_t**)&chunk))))
				LOG_ERROR_AND_GOTO(-103, done, "failed to slist_pop, %d", error);
			
			chunk_release(chunk);
		}
	}
	
done:
	mutex_unlock(&connection->txlock);
	return error;
}





#pragma mark -
#pragma mark read

/**
 *
 *
 */
int
connection_recv (connection_t *connection, uint8_t *data, uint32_t data_i, uint32_t *data_o)
{
	int error = 0;
	uint32_t tlen=0, rlen;
	uint8_t *dataptr = data;
	chunk_t *chunk = NULL;
	
	if (unlikely(connection == NULL))
		LOG_ERROR_AND_RETURN(-1, "null connection_t");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-2, "null data");
	
	// wait until there's enough data in the receive buffer to fulfill this request
	while (data_i > connection->recvbuf)
		usleep(10000);
	
	// we're going to stay stuck in this loop until we satisfy the request
	while (1) {
		mutex_lock(&connection->rxlock);
		
		// get the top-most chunk of the receive buffer, but don't actually remove it from the receive
		// buffer, in case we don't fully use it.
		if (unlikely(0 != (error = slist_peek(&connection->recv_queue, (cobject_t**)&chunk, 0)))) {
			mutex_unlock(&connection->rxlock);
			LOG_ERROR_AND_GOTO(-101, done, "failed to slist_head(recv_queue), %d", error);
		}
		
		mutex_unlock(&connection->rxlock);
		
		// this shouldn't ever happen because we verified that there was sufficient data in the receive
		// buffer before we entered this loop.
		if (chunk == NULL)
			LOG_ERROR_AND_GOTO(-102, done, "got back a null chunk from slist_peek");
		
		// first, let's find out how many bytes, total, we can read from this chunk
		rlen = chunk->leng - chunk->offs;
		
		// if we need fewer bytes than this chunk can provide, only read the amount requested
		if (rlen > data_i - tlen)
			rlen = data_i - tlen;
		
		// copy the data from the chunk, onto the return buffer thingy
		memcpy(dataptr, (void*)chunk->data+chunk->offs, rlen);
		
		// increase the chunk offset, increase the total number of bytes read, increase the return data
		// pointer, decrease the total number of bytes available
		chunk->offs += rlen;
		tlen += rlen;
		dataptr += rlen;
		connection->recvbuf -= rlen;
		
		// if we've completely emptied this chunk of data, pop it off the receive queue and release it
		if (chunk->leng == chunk->offs) {
			mutex_lock(&connection->rxlock);
			
			if (unlikely(0 != (error = slist_pop(&connection->recv_queue, (cobject_t**)&chunk)))) {
				mutex_unlock(&connection->rxlock);
				LOG_ERROR_AND_GOTO(-103, done, "failed to slist_pop(recv_queue), %d", error);
			}
			
			mutex_unlock(&connection->rxlock);
			
			chunk_release(chunk);
		}
		
		// we can break out of the loop if we've satisfied the request.
		if (tlen == data_i)
			break;
	}
	
done:
	*data_o = tlen;
	
	return error;
}

/**
 *
 *
 */
inline int
connection_read_uint8 (connection_t *connection, uint8_t *data)
{
	int error;
	uint32_t data_i=1, data_o=0;
	
	if (unlikely(0 != (error = connection_recv(connection, data, data_i, &data_o))))
		{ LOG_ERROR_AND_RETURN(-101, "failed to connection_recv(), %d", error); }
	else if (data_i != data_o)
		{ LOG_ERROR_AND_RETURN(-102, "failed: only read %d of %d bytes", data_o, data_i); }
	else
		return 0;
}

/**
 *
 *
 */
inline int
connection_read_uint16 (connection_t *connection, uint16_t *data)
{
	int error;
	uint32_t data_i=2, data_o=0;
	
	if (unlikely(0 != (error = connection_recv(connection, (uint8_t*)data, data_i, &data_o))))
		{ LOG_ERROR_AND_RETURN(-101, "failed to connection_recv, %d", error); }
	else if (data_i != data_o)
		{ LOG_ERROR_AND_RETURN(-102, "failed: only read %d of %d bytes", data_o, data_i); }
	
	if (connection->endian == CONNECTION_BIG_ENDIAN)
		*data = swap16be(*data);
	else if (connection->endian == CONNECTION_LITTLE_ENDIAN)
		*data = swap16le(*data);
	
	return 0;
}

/**
 *
 *
 */
inline int
connection_read_uint32 (connection_t *connection, uint32_t *data)
{
	int error;
	uint32_t data_i=4, data_o=0;
	
	if (unlikely(0 != (error = connection_recv(connection, (uint8_t*)data, data_i, &data_o))))
		{ LOG_ERROR_AND_RETURN(-101, "failed to connection_recv, %d", error); }
	else if (data_i != data_o)
		{ LOG_ERROR_AND_RETURN(-102, "failed: only read %d of %d bytes", data_o, data_i); }
	
	if (connection->endian == CONNECTION_BIG_ENDIAN)
		*data = swap32be(*data);
	else if (connection->endian == CONNECTION_LITTLE_ENDIAN)
		*data = swap32le(*data);
	
	return 0;
}

/**
 *
 *
 */
inline int
connection_read_uint64 (connection_t *connection, uint64_t *data)
{
	int error;
	uint32_t data_i=8, data_o=0;
	
	if (unlikely(0 != (error = connection_recv(connection, (uint8_t*)data, data_i, &data_o))))
		{ LOG_ERROR_AND_RETURN(-101, "failed to connection_recv, %d", error); }
	else if (data_i != data_o)
		{ LOG_ERROR_AND_RETURN(-102, "failed: only read %d of %d bytes", data_o, data_i); }
	
	if (connection->endian == CONNECTION_BIG_ENDIAN)
		*data = swap64be(*data);
	else if (connection->endian == CONNECTION_LITTLE_ENDIAN)
		*data = swap64le(*data);
	
	return 0;
}

/**
 *
 *
 */
int
connection_peek (connection_t *connection, uint8_t *data, uint32_t data_i, uint32_t *data_o)
{
	int error = 0;
	uint32_t tlen=0, rlen, index=0;
	uint8_t *dataptr = data;
	chunk_t *chunk = NULL;
	
	if (unlikely(connection == NULL))
		LOG_ERROR_AND_RETURN(-1, "null connection_t");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-2, "null data");
	
	if (unlikely(data_i == 0))
		LOG_ERROR_AND_RETURN(-3, "invalid data_i (0)");
	
	if (unlikely(data_o == NULL))
		LOG_ERROR_AND_RETURN(-4, "null data_o");
	
	mutex_lock(&connection->rxlock);
	
	while (1) {
		if (unlikely(0 != (error = slist_peek(&connection->recv_queue, (cobject_t**)&chunk, index++))))
			LOG_ERROR_AND_GOTO(-101, done, "failed to slist_peek, %d", error);
		
		if (chunk == NULL)
			break;
		
		rlen = chunk->leng - chunk->offs;
		
		if (rlen > data_i - tlen)
			rlen = data_i - tlen;
		
		memcpy(dataptr, (void*)chunk->data+chunk->offs, rlen);
		
		tlen += rlen;
		dataptr += rlen;
		
		if (tlen == data_i)
			break;
	}
	
done:
	mutex_unlock(&connection->rxlock);
	*data_o = tlen;
	return error;
}

/**
 *
 *
 */
int
connection_scan (connection_t *connection, uint8_t *data, uint32_t size)
{
	int error=0, tlen=0;
	uint32_t i, j, chunks=0;
	chunk_t *chunk = NULL;
	
	if (unlikely(connection == NULL))
		LOG_ERROR_AND_RETURN(-1, "null connection_t");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-2, "null data");
	
	if (unlikely(size == 0))
		LOG_ERROR_AND_RETURN(-3, "size is zero");
	
	mutex_lock(&connection->rxlock);
	
	while (1) {
		if (unlikely(0 != (error = slist_peek(&connection->recv_queue, (cobject_t**)&chunk, chunks))))
			LOG_ERROR_AND_GOTO(-101, done, "failed to slist_peek, %d", error);
		
		if (chunk == NULL)
			break;
		
		for (i = chunk->offs; i < chunk->leng; ++i, ++tlen) {
			for (j = 0; j < size; ++j) {
				// if, while comparing from a particular point in this receive buffer chunk to the specified
				// data, we reached the end of the chunk, we need to grab the next chunk (if there is one).
				if (i == chunk->leng) {
					if (unlikely(0 != (error = slist_peek(&connection->recv_queue, (cobject_t**)&chunk, ++chunks))))
						LOG_ERROR_AND_GOTO(-102, done, "failed to slist_peek, %d", error);
					
					if (chunk == NULL)
						goto done;
					
					i = 0;
				}
				
				// if a particular byte does not match, we don't need to continue checking from this
				// location. move on to the next byte of the receive buffer chunk.
				if (chunk->data[i+j] != data[j])
					break;
			}
			
			// if we scanned all the way to the end of the specified data, we have found a complete match.
			if (j == size)
				goto done;
		}
		
		chunks++;
	}
	
done:
	// if we scanned all the way to the end of the specified data, we have found a complete match.
	if (j == size && chunk != NULL) {
		LOG3("* * * * * found mark at offset %d of chunk %d {offs=%d, leng=%d} = 0x%02X%02X", i, chunks, chunk->offs, chunk->leng, *(uint8_t*)(chunk->data+i+0), *(uint8_t*)(chunk->data+i+1));
		
		// if the entire match was done within this last chunk, move the chunk offset to the start of
		// the match; decrease the connection's read buffer size. there are now zero bytes of the match 
		// string remaining to account for.
		if (chunk->leng - chunk->offs >= size) {
			uint32_t delta = i - chunk->offs;
			connection->recvbuf -= delta;
			chunk->offs += delta;
			size = 0;
			
			if (chunk->leng == chunk->offs)
				LOG3("* * * * * [1] we just emptied a chunk; we need to pop() it");
		}
		// if only the end of the match was made in this last chunk, decrease the "size" by the number
		// of bytes we matched in this chunk; and we'll find the remaining bytes in the previous chunks.
		else
			size -= (chunk->leng - chunk->offs);
		
		// if we scanned beyond simply the first chunk in the connection read buffer, then we need to
		// remove all of the chunks prior to the match, and update the chunk that includes the start of
		// the match.
		if (chunks > 0) {
			// starting with the chunk immediately prior to the chunk at which the end of the match was 
			// found, scan backwards from the chunk immediately prior to the end of the match until we 
			// find the chunk which contains the start of the match.
			while (size != 0 && --chunks != 0) {
				if (unlikely(0 != (error = slist_peek(&connection->recv_queue, (cobject_t**)&chunk, chunks))))
					LOG_ERROR_AND_GOTO(-104, fail, "* * * * * failed to slist_peek, %d", error);
				
				if (chunk == NULL)
					break;
				
				// if this chunk contains the start of the match, then increase the offset of the chunk to the
				// start of the match and decrease the connection read buffer size. break. all previous chunks
				// can be removed entirely.
				if (chunk->leng - chunk->offs > size) {
					uint32_t delta = (chunk->leng - chunk->offs) - size;
					connection->recvbuf -= delta;
					chunk->offs += delta;
//				size = 0;
					break;
				}
				else {
					size -= (chunk->leng - chunk->offs);
					
					if (chunk->leng == chunk->offs)
						LOG3("* * * * * [1] we just emptied a chunk; we need to pop() it");
				}
			}
		}
	}
	else
		LOG3("failed to find mark in %d chunks after scanning %d bytes", chunks, tlen);
	
	// remove all (if any) chunks that precede the chunk at which the match began. decrease the
	// connection read buffer size by the size of each chunk.
	for (uint32_t k = 0; k < chunks; ++k) {
		LOG3("* * * * * removed a chunk; %d / %d", chunks, connection->recv_queue.count);
		
		if (unlikely(0 != (error = slist_pop(&connection->recv_queue, (cobject_t**)&chunk))))
			LOG_ERROR_AND_GOTO(-105, fail, "* * * * * failed to slist_pop, %d", error);
		
		connection->recvbuf -= (chunk->leng - chunk->offs);
		
		chunk_release(chunk);
	}
	
fail:
	mutex_unlock(&connection->rxlock);
	return error;
}

/**
 *
 *
 */
int
connection_clear_rx (connection_t *connection)
{
	if (unlikely(connection == NULL))
		LOG_ERROR_AND_RETURN(-1, "null connection_t");
	
	// don't modify any of the buffers until we get a lock
	mutex_lock(&connection->rxlock);
	
	// clear the receive buffer of all data
	slist_clear(&connection->recv_queue);
	
	// reset the receive buffer size indicator
	connection->recvbuf = 0;
	
	// release the lock
	mutex_unlock(&connection->rxlock);
	
	return 0;
}

/**
 * Returns the number of bytes of data available to be read.
 *
 */
int
connection_available (connection_t *connection)
{
	if (unlikely(connection == NULL))
		LOG_ERROR_AND_RETURN(-1, "null connection_t");
	
	return (int)connection->recvbuf;
}

/**
 *
 *
 */
int
connection_dump (connection_t *connection)
{
	int error;
	uint32_t bytes;
	uint8_t data[200] = { 0 };
	
	if (unlikely(connection == NULL))
		LOG_ERROR_AND_RETURN(-1, "null connection_t");
	
	if (unlikely(0 != (error = connection_peek(connection, data, 200, &bytes))))
		LOG_ERROR_AND_RETURN(-101, "failed to connection_peak, %d", error);
	
	hexdump(data, (int)bytes);
	
	return 0;
}





#pragma mark -
#pragma mark private

/**
 *
 *
 */
int
__connection_refill (connection_t *connection, uint32_t *bytes)
{
	int error = 0;
	uint32_t rbytes = 0;
	chunk_t *chunk = NULL;
	
	if (unlikely(connection == NULL))
		LOG_ERROR_AND_RETURN(-1, "null connection_t");
	
	// ask the core to allocate a new chunk from the chunk memory pool
	if (unlikely(0 != (error = core_chunk(&chunk)) || chunk == NULL))
		LOG_ERROR_AND_GOTO(-101, done, "failed to core_chunk(), %d", error);
	
	mutex_lock(&connection->xxlock);
	
	// read the bytes into the chunk, starting at the appropriate offset
	if (unlikely(0 != (error = (*connection->recv)(connection, (void*)chunk->data, sizeof(chunk->data), &rbytes)))) {
		mutex_unlock(&connection->xxlock);
		LOG_ERROR_AND_GOTO(-102, done, "failed to connection->recv(), %d", error);
	}
	
	mutex_unlock(&connection->xxlock);
	
	// if we received some data, push the chunk onto the receive queue and update the various sizes
	if (rbytes > 0) {
		mutex_lock(&connection->rxlock);
		
		chunk->leng = rbytes;
		
		if (unlikely(0 != (error = slist_push(&connection->recv_queue, (cobject_t*)chunk)))) {
			mutex_unlock(&connection->rxlock);
			LOG_ERROR_AND_GOTO(-103, done, "failed to slist_push(), %d", error);
		}
		
		connection->data_rcvd += rbytes;
		connection->recvbuf += rbytes;
		
		mutex_unlock(&connection->rxlock);
	}
	
done:
	if (chunk != NULL)
		chunk_release(chunk);
	
	if (bytes != NULL)
		*bytes = rbytes;
	
	return error;
}

/**
 *
 *
 */
void*
__connection_read_loop (connection_t *connection)
{
	int error;
	uint32_t bytes;
	
	if (connection == NULL)
		LOG_ERROR_AND_RETURN(NULL, "null connection_t");
	
	while (!connection->stop) {
		if (unlikely(0 != (error = __connection_refill(connection, &bytes))))
			LOG_ERROR_AND_BREAK("failed to connection_refill(), %d", error);
		
		if (bytes == 0)
			usleep(10000);
	}
	
	LOG3("  done");
	
	return NULL;
}
