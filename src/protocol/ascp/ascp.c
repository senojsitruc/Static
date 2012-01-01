/*
 *  ascp.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.18.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "ascp.h"
#include "../../core/core.h"
#include "../../misc/dump.h"
#include "../../misc/logger.h"





#pragma mark -
#pragma mark ascp structors

/**
 *
 *
 */
int
ascp_init (ascp_t *ascp)
{
	int error;
	
	if (unlikely(ascp == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ascp_t");
	
	if (unlikely(0 != (error = protocol_init((protocol_t*)ascp, ASCP_NAME))))
		LOG_ERROR_AND_RETURN(-101, "failed to protocol_init, %d", error);
	
	return 0;
}

/**
 *
 *
 */
int
ascp_destroy (ascp_t *ascp)
{
	int error;
	
	if (unlikely(ascp == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ascp_t");
	
	if (unlikely(0 != (error = protocol_destroy((protocol_t*)ascp))))
		LOG_ERROR_AND_RETURN(-101, "failed to protocol_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark message structors

/**
 *
 *
 */
int
ascp_message_init (ascp_message_t *message, char *name, opool_t *pool)
{
	int error;
	
	if (unlikely(message == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ascp_message_t");
	
	if (unlikely(0 != (error = cobject_init((cobject_t*)message, name, (cobject_destroy_func)ascp_message_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_init, %d", error);
	
	return 0;
}

/**
 *
 *
 */
int
ascp_message_destroy (ascp_message_t *message)
{
	int error;
	
	if (unlikely(message == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ascp_message_t");
	
	if (unlikely(0 != (error = cobject_destroy((cobject_t*)message))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
int
ascp_data_get (ascp_message_data_t *message, uint8_t command, uint8_t mode, uint8_t count)
{
	if (unlikely(message == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ascp_message_data_t");
	
	message->header.code = ASCP_CODE_DATA;
	message->header.type = ASCP_TYPE_DATA_BEG_REQ;
	message->header.length = 8;
	message->channel = 0x81;
	message->command = command;
	message->mode = mode;
	message->count = count;
	
	return 0;
}

/**
 *
 *
 */
int
ascp_frequency_set (ascp_message_freq_t *message, uint32_t frequency)
{
	if (unlikely(message == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ascp_message_freq_t");
	
	message->header.code = ASCP_CODE_FREQ;
	message->header.type = ASCP_TYPE_FREQ_SET_REQ;
	message->header.length = (ASCP_HOST_CTRL_ITEM_SET << 13) | 0x0A;
	message->channel = 0;
	message->frequency = frequency;
	message->multiplier = 1;
	
	return 0;
}

/**
 *
 *
 */
int
ascp_frequency_get (ascp_message_freq_t *message)
{
	if (unlikely(message == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ascp_message_freq_t");
	
	message->header.code = ASCP_CODE_FREQ;
	message->header.type = ASCP_TYPE_FREQ_GET_REQ;
	message->header.length = (ASCP_HOST_CTRL_ITEM_GET << 13) | 0x05;
	message->channel = 0;
	message->frequency = 0;
	message->multiplier = 1;
	
	return 0;
}

/**
 *
 *
 */
int
ascp_gain_set (ascp_message_gain_t *message, uint8_t mode, int8_t gain)
{
	if (unlikely(message == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ascp_message_gain_t");
	
	message->header.code = ASCP_CODE_GAIN;
	message->header.type = ASCP_TYPE_GAIN_SET_REQ;
	message->header.length = (ASCP_HOST_CTRL_ITEM_SET << 13) | 0x06;
//message->mode = mode;
//message->gain = (uint8_t)gain;
	message->mode = 1;
	message->gain = 0xF6;
	
	return 0;
}

/**
 *
 *
 */
int
ascp_ad6620_set (ascp_message_6620_t *message, uint16_t address, int32_t data)
{
	if (unlikely(message == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ascp_message_6620_t");
	
	message->header.code = ASCP_CODE_6620;
	message->header.type = ASCP_TYPE_6620_SET_REQ;
	message->header.length = 0;
	message->address = address;
	message->data0 = (data      ) & 0xFF;
	message->data1 = (data >>  8) & 0xFF;
	message->data2 = (data >> 16) & 0xFF;
	message->data3 = (data >> 24) & 0xFF;
	message->data4 = 0;
	
	return 0;
}





#pragma mark -
#pragma mark read / write

/**
 *
 *
 */
int
ascp_message_read (ascp_t *ascp, ascp_message_t **message, connection_t *connection)
{
	int error;
	uint16_t tlength, rlength, header, mode, code;
	
	if (unlikely(ascp == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ascp_t");
	
	if (unlikely(message == NULL))
		LOG_ERROR_AND_RETURN(-2, "null ascp_message_t");
	
	if (unlikely(connection == NULL))
		LOG_ERROR_AND_RETURN(-3, "null connection_t");
	
	*message = NULL;
	
	// get the two bytes that represent the message length
	if (unlikely(0 != (error = connection_read_uint16(connection, &header))))
		LOG_ERROR_AND_RETURN(-101, "failed to connection_read_uint16(length), %d", error);
	
	if (ASCP_CODE_DACK == header) {
		rlength = 1;
		code = ASCP_CODE_DACK;
	}
	
	else {
		// the high three bits on the mode
		mode = (header & 0xE000) >> 13;
		tlength = rlength = header & 0x1FFF;
		
		// data messages use a special length of zero, which is understood to mean 8194 (less the two byte
		// message length value. and data messages do not have a two byte message type thingy.
		if (tlength == 0) {
			code = ASCP_CODE_DATA;
			rlength = 8192;
		}
		
		// this _should_ never happen, but it will happen. this indicates that we're out-of-sync because
		// the idiotic device doesn't understand the notion of a full buffer. we need to clear out our
		// receive buffer, refill it from the device, and scan forward until we find the start of a
		// data message (if we're in data streaming mode).
		//
		// after we think we may have achieved "sync" again, we'll return a null message and the caller
		// can call us yet again and we'll see if it works out any better.
		//
		else if (tlength > 50 && ascp->data) {
			LOG3("* * * * * sync! header=0x%04X", header);
			
			uint8_t mark[2] = { 0x00, 0x80 };
			
			connection_clear_rx(connection);
			
			while (connection->recvbuf < 8194)
				usleep(10000);
			
			connection_scan(connection, mark, 2);
			
			return 0;
		}
		
		// process a normal, non-data message
		else {
			// get the two bytes that represent the message type
			if (unlikely(0 != (error = connection_read_uint16(connection, &code))))
				LOG_ERROR_AND_RETURN(-102, "failed to connection_read_uint16(code), %d", error);
			
			rlength -= 4;
		}
	}
	
	switch (code) {
			/*
		case ASCP_CODE_NAME:
			LOG_ERROR_AND_RETURN(-201, "ASCP_CODE_NAME not yet supported");
			break;
		case ASCP_CODE_NUMB:
			LOG_ERROR_AND_RETURN(-301, "ASCP_CODE_NUMB not yet supported");
			break;
		case ASCP_CODE_IVER:
			LOG_ERROR_AND_RETURN(-401, "ASCP_CODE_IVER not yet supported");
			break;
		case ASCP_CODE_HVER:
			LOG_ERROR_AND_RETURN(-501, "ASCP_CODE_HVER not yet supported");
			break;
		case ASCP_CODE_ERRC:
			LOG_ERROR_AND_RETURN(-601, "ASCP_CODE_ERRC not yet supported");
			break;
		case ASCP_CODE_ERRS:
			LOG_ERROR_AND_RETURN(-701, "ASCP_CODE_ERRS not yet supported");
			break;
			*/
		case ASCP_CODE_DATA:
			{
				uint32_t bytes;
				ascp_message_data_t *m = NULL;
				
				if (unlikely(0 != (error = core_ascp_message_data(&m))))
					LOG_ERROR_AND_RETURN(-801, "[DATA] failed to core_ascp_message_data, %d", error);
				
				*message = (ascp_message_t*)m;
				m->header.length = header;
				m->header.code = code;
				
				// this is a data block
				if (rlength == 8192) {
					m->header.type = ASCP_TYPE_DATA_BLOCK;
					
					if (unlikely(0 != (error = connection_recv(connection, (uint8_t*)m->data, 8192, &bytes))))
						LOG_ERROR_AND_GOTO(-806, fail, "[DATA] failed to connection_recv(data), %d", error);
					
					//hexdump((uint8_t*)m->data, 100);
				}
				// this is a data message (sans the actual data)
				else {
					if (unlikely(0 != (error = connection_read_uint8(connection, &m->channel))))
						LOG_ERROR_AND_GOTO(-802, fail, "[DATA] failed to connection_read_uint8(channel), %d", error);
					
					if (unlikely(0 != (error = connection_read_uint8(connection, &m->command))))
						LOG_ERROR_AND_GOTO(-803, fail, "[DATA] failed to connection_read_uint8(command), %d", error);
					
					// determining the message type (not the code) based on the message requires that we be able
					// to inspect the command in the message ... so we have to parse the channel, mode and 
					// command before we can set the correct message type.
					if (ASCP_TRGT_DATA_ITEM_0 == mode ||
							ASCP_TRGT_DATA_ITEM_1 == mode ||
							ASCP_TRGT_DATA_ITEM_2 == mode ||
							ASCP_TRGT_DATA_ITEM_3 == mode)
						m->header.type = ASCP_TYPE_DATA_BLOCK;
					else if (ASCP_TRGT_CTRL_ITEM_RSP == mode) {
						if (m->command == ASCP_DATA_BEG)
							m->header.type = ASCP_TYPE_DATA_BEG_RSP;
						else
							m->header.type = ASCP_TYPE_DATA_END_RSP;
					}
					else if (ASCP_TRGT_CTRL_ITEM_UNS == mode)
						m->header.type = ASCP_TYPE_DATA_END_RSP;
					
					if (unlikely(0 != (error = connection_read_uint8(connection, &m->mode))))
						LOG_ERROR_AND_GOTO(-804, fail, "[DATA] failed to connection_read_uint8(mode), %d", error);
					
					if (unlikely(0 != (error = connection_read_uint8(connection, &m->count))))
						LOG_ERROR_AND_GOTO(-805, fail, "[DATA] failed to connection_read_uint8(count), %d", error);
				}
				
				break;
			}
		
		case ASCP_CODE_FREQ:
			{
				ascp_message_freq_t *m = NULL;
				
				if (unlikely(0 != (error = core_ascp_message_freq(&m))))
					LOG_ERROR_AND_RETURN(-901, "[FREQ] failed to core_ascp_message_freq, %d", error);
				
				*message = (ascp_message_t*)m;
				m->header.length = header;
				m->header.code = code;
				m->header.type = ASCP_TYPE_FREQ_GET_RSP;
				
				if (unlikely(0 != (error = connection_read_uint8(connection, &m->channel))))
					LOG_ERROR_AND_GOTO(-902, fail, "[FREQ] failed to connection_read_uint8(channel), %d", error);
				
				if (unlikely(0 != (error = connection_read_uint32(connection, &m->frequency))))
					LOG_ERROR_AND_GOTO(-903, fail, "[FREQ] failed to connection_read_uint32(frequency), %d", error);
				
				if (unlikely(0 != (error = connection_read_uint8(connection, &m->multiplier))))
					LOG_ERROR_AND_GOTO(-904, fail, "[FREQ] failed to connection_read_uint8(multiplier), %d", error);
				
				LOG3("[FREQ] message received; frequency = %u", m->frequency);
				
				break;
			}
		
		case ASCP_CODE_GAIN:
			{
				ascp_message_gain_t *m = NULL;
				
				if (unlikely(0 != (error = core_ascp_message_gain(&m))))
					LOG_ERROR_AND_RETURN(-1001, "[GAIN] failed to core_ascp_message_gain(), %d", error);
				
				*message = (ascp_message_t*)m;
				m->header.length = header;
				m->header.code = code;
				m->header.type = ASCP_TYPE_GAIN_GET_RSP;
				
				if (unlikely(0 != (error = connection_read_uint8(connection, &m->mode))))
					LOG_ERROR_AND_RETURN(-1002, "[GAIN] failed to connection_read_uint8(mode), %d", error);
				
				if (unlikely(0 != (error = connection_read_uint8(connection, &m->gain))))
					LOG_ERROR_AND_RETURN(-1003, "[GAIN] failed to connection_read_uint8(gain), %d", error);
				
				LOG3("[GAIN] message received; gain = 0x%02X", (int)m->gain);
				
				break;
			}
		
			/*
		case ASCP_CODE_RATE:
			LOG_ERROR_AND_RETURN(-1101, "ASCP_CODE_RATE not yet supported");
			break;
			*/
			
		case ASCP_CODE_DACK:
			{
				ascp_message_dack_t *m = NULL;
				
				if (unlikely(0 != (error = core_ascp_message_dack(&m))))
					LOG_ERROR_AND_RETURN(-1201, "[DACK] failed to core_ascp_message_dack, %d", error);
				
				*message = (ascp_message_t*)m;
				m->header.length = 0;
				m->header.code = code;
				m->header.type = ASCP_TYPE_6620_SET_RSP;
				
				if (unlikely(0 != (error = connection_read_uint8(connection, &m->data))))
					LOG_ERROR_AND_GOTO(-1202, fail, "[DACK] failed to connection_read_uint8(data), %d", error);
				
				LOG3("[DACK] message received; data=0x%02X", m->data);
				
				if (m->data != 0x01)
					LOG3("[DACK] message received; data=0x%02X [ERROR!]", m->data);
				
				break;
			}
			
		default:
			{
				uint8_t data[100];
				uint32_t rbytes, tbytes = rlength;
				
				LOG3("[0x%04X] unsupported message type; reading another %u bytes", code, tbytes);
				
				while (tbytes > 0) {
					rbytes = tbytes > sizeof(data) ? sizeof(data) : tbytes;
					
					if (unlikely(0 != (error = connection_recv(connection, data, rbytes, &rbytes))))
						LOG_ERROR_AND_RETURN(-1301, "[0x%04X] failed to connection_recv for unsupported message, %d", code, error);
					
					tbytes -= rbytes;
					
					//LOG3("[0x%04X] read %lu of %lu bytes for this unsupport message", code, (rlength-tbytes), rlength);
				}
				
				break;
			}
	};
	
	return 0;
	
fail:
	if (*message != NULL)
		ascp_message_release(*message);
	
	return error;
}

/**
 *
 *
 */
int
ascp_message_send (ascp_t *ascp, ascp_message_t *message, connection_t *connection)
{
	int error;
	
	if (unlikely(ascp == NULL))
		LOG_ERROR_AND_RETURN(-1, "null ascp_t");
	
	if (unlikely(message == NULL))
		LOG_ERROR_AND_RETURN(-2, "null ascp_message_t");
	
	if (unlikely(connection == NULL))
		LOG_ERROR_AND_RETURN(-3, "null connection_t");
	
	switch (message->code) {
			/*
		case ASCP_CODE_NAME:
			LOG_ERROR_AND_RETURN(-201, "ASCP_CODE_NAME not yet supported");
			break;
		case ASCP_CODE_NUMB:
			LOG_ERROR_AND_RETURN(-301, "ASCP_CODE_NUMB not yet supported");
			break;
		case ASCP_CODE_IVER:
			LOG_ERROR_AND_RETURN(-401, "ASCP_CODE_IVER not yet supported");
			break;
		case ASCP_CODE_HVER:
			LOG_ERROR_AND_RETURN(-501, "ASCP_CODE_HVER not yet supported");
			break;
		case ASCP_CODE_ERRC:
			LOG_ERROR_AND_RETURN(-601, "ASCP_CODE_ERRC not yet supported");
			break;
		case ASCP_CODE_ERRS:
			LOG_ERROR_AND_RETURN(-701, "ASCP_CODE_ERRS not yet supported");
			break;
			*/
			
		case ASCP_CODE_DATA:
			{
				ascp_message_data_t *m = (ascp_message_data_t*)message;
				
				if (unlikely(0 != (error = connection_write_uint16(connection, m->header.length))))
					LOG_ERROR_AND_RETURN(-801, "[DATA] failed to connection_write_uint16(length), %d", error);
				
				if (unlikely(0 != (error = connection_write_uint16(connection, m->header.code))))
					LOG_ERROR_AND_RETURN(-802, "[DATA] failed to connection_write_uint16(code), %d", error);
				
				if (unlikely(0 != (error = connection_write_uint8(connection, m->channel))))
					LOG_ERROR_AND_RETURN(-803, "[DATA] failed to connection_write_uint8(channel), %d", error);
				
				if (unlikely(0 != (error = connection_write_uint8(connection, m->command))))
					LOG_ERROR_AND_RETURN(-804, "[DATA] failed to connection_write_uint8(command), %d", error);
				
				if (unlikely(0 != (error = connection_write_uint8(connection, m->mode))))
					LOG_ERROR_AND_RETURN(-805, "[DATA] failed to connection_write_uint8(mode), %d", error);
				
				if (unlikely(0 != (error = connection_write_uint8(connection, m->count))))
					LOG_ERROR_AND_RETURN(-806, "[DATA] failed to connection_write_uint8(count), %d", error);
				
				LOG3("[DATA] message sent; command=0x%02X, mode=0x%02X, count=0x%02X", m->command, m->mode, m->count);
				
				ascp->data = 1;
				
				break;
			}
			
		case ASCP_CODE_FREQ:
			{
				ascp_message_freq_t *m = (ascp_message_freq_t*)message;
				
				if (unlikely(0 != (error = connection_write_uint16(connection, m->header.length))))
					LOG_ERROR_AND_RETURN(-901, "[FREQ] failed to connection_write_uint16(length), %d", error);
				
				if (unlikely(0 != (error = connection_write_uint16(connection, m->header.code))))
					LOG_ERROR_AND_RETURN(-902, "[FREQ] failed to connection_write_uint16(code), %d", error);
				
				if (unlikely(0 != (error = connection_write_uint8(connection, m->channel))))
					LOG_ERROR_AND_RETURN(-903, "[FREQ] failed to connection_write_uint8(channel), %d", error);
				
				// the frequency and multiplier are only sent for set requests
				if (ASCP_TYPE_FREQ_SET_REQ == message->type) {
					if (unlikely(0 != (error = connection_write_uint32(connection, m->frequency))))
						LOG_ERROR_AND_RETURN(-904, "[FREQ] failed to connection_write_uint32(frequency), %d", error);
					
					if (unlikely(0 != (error = connection_write_uint8(connection, m->multiplier))))
						LOG_ERROR_AND_RETURN(-905, "[FREQ] failed to connection_write_uint8(multiplier), %d", error);
				}
				
				LOG3("[FREQ] message sent; frequency = %u", m->frequency);
				
				break;
			}
			
		case ASCP_CODE_GAIN:
			{
				ascp_message_gain_t *m = (ascp_message_gain_t*)message;
				
				if (unlikely(0 != (error = connection_write_uint16(connection, m->header.length))))
					LOG_ERROR_AND_RETURN(-1001, "[GAIN] failed to connection_write_uint16(length), %d", error);
				
				if (unlikely(0 != (error = connection_write_uint16(connection, m->header.code))))
					LOG_ERROR_AND_RETURN(-1002, "[GAIN] failed to connection_write_uint16(code), %d", error);
				
				if (unlikely(0 != (error = connection_write_uint8(connection, m->mode))))
					LOG_ERROR_AND_RETURN(-1003, "[GAIN] failed to connection_write_uint8(mode), %d", error);
				
				if (unlikely(0 != (error = connection_write_uint8(connection, m->gain))))
					LOG_ERROR_AND_RETURN(-1004, "[GAIN] failed to connection_write_uint8(gain), %d", error);
				
				LOG3("[GAIN] message sent; mode = %d, gain = %d", m->mode, (int)m->gain);
				
				break;
			}
			
			/*
		case ASCP_CODE_RATE:
			LOG_ERROR_AND_RETURN(-1101, "ASCP_CODE_RATE not yet supported");
			break;
			*/
			
		case ASCP_CODE_6620:
			{
				ascp_message_6620_t *m = (ascp_message_6620_t*)message;
				
				if (unlikely(0 != (error = connection_write_uint16(connection, m->header.code))))
					LOG_ERROR_AND_RETURN(-1201, "[FREQ] failed to connection_write_uint16(code), %d", error);
				
				if (unlikely(0 != (error = connection_write_uint16(connection, m->address))))
					LOG_ERROR_AND_RETURN(-1202, "[FREQ] failed to connection_write_uint16(address), %d", error);
				
				if (unlikely(0 != (error = connection_write_uint8(connection, m->data0))))
					LOG_ERROR_AND_RETURN(-1203, "[FREQ] failed to connection_write_uint8(data0), %d", error);
				
				if (unlikely(0 != (error = connection_write_uint8(connection, m->data1))))
					LOG_ERROR_AND_RETURN(-1204, "[FREQ] failed to connection_write_uint8(data1), %d", error);
				
				if (unlikely(0 != (error = connection_write_uint8(connection, m->data2))))
					LOG_ERROR_AND_RETURN(-1205, "[FREQ] failed to connection_write_uint8(data2), %d", error);
				
				if (unlikely(0 != (error = connection_write_uint8(connection, m->data3))))
					LOG_ERROR_AND_RETURN(-1206, "[FREQ] failed to connection_write_uint8(data3), %d", error);
				
				if (unlikely(0 != (error = connection_write_uint8(connection, m->data4))))
					LOG_ERROR_AND_RETURN(-1207, "[FREQ] failed to connection_write_uint8(data4), %d", error);
				
				LOG3("[6620] message sent; address=0x%04X", m->address);
				
				break;
			}
		default:
			LOG_ERROR_AND_RETURN(-1201, "unsupported message type, 0x%04X", message->code);
			break;
	};
	
	// flush the connection write buffer
	if (unlikely(0 != (error = connection_flush(connection))))
		LOG_ERROR_AND_RETURN(-1301, "failed to connection_flush, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline ascp_message_t*
ascp_message_retain (ascp_message_t *message)
{
	if (unlikely(message == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null ascp_message_t");
	
	return (ascp_message_t*)cobject_retain((cobject_t*)message);
}

/**
 *
 *
 */
inline void
ascp_message_release (ascp_message_t *message)
{
	if (unlikely(message == NULL))
		LOG_ERROR_AND_RETURN(, "null ascp_message_t");
	
	cobject_release((cobject_t*)message);
}
