/*
 *  ascp.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.18.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  Amateur Station Control Protocol - http://www.moetronix.com/ae4jy/ascp.htm
 *
 */

#ifndef __PROTOCOL_ASCP_H__
#define __PROTOCOL_ASCP_H__

#include "../protocol.h"
#include "../../misc/mem/cobject.h"
#include "../../misc/mem/opool.h"
#include "../../misc/net/connection.h"

#define ASCP_DATA_END 1
#define ASCP_DATA_BEG 2
#define ASCP_DATA_CON 0
#define ASCP_DATA_ONE 2

#define ASCP_NAME "ASCP"

#define ASCP_IQ_COUNT 4096





//
// ascp_host_type
//
typedef enum
{
	ASCP_HOST_CTRL_ITEM_SET = 0,			// set control item
	ASCP_HOST_CTRL_ITEM_GET = 1,			// request current control item
	ASCP_HOST_CTRL_RNGE_GET = 2,			// request control item range
	ASCP_HOST_DATA_ITEM_ACK = 3,			// data item ack from host to target
	ASCP_HOST_DATA_ITEM_0   = 4,			// host data item 0
	ASCP_HOST_DATA_ITEM_1   = 5,			// host data item 1
	ASCP_HOST_DATA_ITEM_2   = 6,			// host data item 2
	ASCP_HOST_DATA_ITEM_3   = 7				// host data item 3
} ascp_host_type;

//
// ascp_trgt_type
//
typedef enum
{
	ASCP_TRGT_CTRL_ITEM_RSP = 0,			// response to control item set/get
	ASCP_TRGT_CTRL_ITEM_UNS = 1,			// unsolicited conttrol item
	ASCP_TRGT_CTRL_RNGE_RSP = 2,			// response to control item range get
	ASCP_TRGT_DATA_ITEM_ACK = 3,			// data item ack from target to host
	ASCP_TRGT_DATA_ITEM_0   = 4,			// target data item 0
	ASCP_TRGT_DATA_ITEM_1   = 5,			// target data item 1
	ASCP_TRGT_DATA_ITEM_2   = 6,			// target data item 2
	ASCP_TRGT_DATA_ITEM_3   = 7				// target data item 3
} ascp_trgt_type;

//
// ascp_type
//
typedef enum
{
	/* nco center frequency */
	ASCP_TYPE_FREQ_SET_REQ = 0x0101,	// frequency set request
	ASCP_TYPE_FREQ_SET_RSP = 0x0102,	// frequency set response
	ASCP_TYPE_FREQ_GET_REQ = 0x0103,	// frequency get request
	ASCP_TYPE_FREQ_GET_RSP = 0x0104,	// frequency get response
	
	/* sample rate */
	ASCP_TYPE_RATE_SET_REQ = 0x0201,	// sample rate set request
	ASCP_TYPE_RATE_SET_RSP = 0x0202,	// sample rate set response
	
	/* rf gain */
	ASCP_TYPE_GAIN_SET_REQ = 0x0301,	// rf gain set request
	ASCP_TYPE_GAIN_SET_RSP = 0x0302,	// rf gain set response
	ASCP_TYPE_GAIN_GET_REQ = 0x0303,	// rf gain get request
	ASCP_TYPE_GAIN_GET_RSP = 0x0304,	// rf gain get response
	
	/* device name */
	ASCP_TYPE_NAME_GET_REQ = 0x0401,	// device name get request
	ASCP_TYPE_NAME_GET_RSP = 0x0402,	// device name get response
	
	/* serial number */
	ASCP_TYPE_NUMB_GET_REQ = 0x0501,	// serial number get request
	ASCP_TYPE_NUMB_GET_RSP = 0x0502,	// serial number get response
	
	/* interface version */
	ASCP_TYPE_IVER_GET_REQ = 0x0601,	// interface version get request
	ASCP_TYPE_IVER_GET_RSP = 0x0602,	// interface version get response
	
	/* hardware version */
	ASCP_TYPE_HVER_GET_REQ = 0x0701,	// hardware version get request
	ASCP_TYPE_HVER_GET_RSP = 0x0702,	// hardware version get response
	
	/* firmware version */
	ASCP_TYPE_FVER_GET_REQ = 0x0801,	// firmware version get request
	ASCP_TYPE_FVER_GET_RSP = 0x0802,	// firmware version get response
	
	/* error code */
	ASCP_TYPE_ERRC_GET_REQ = 0x0901,	// error code get request
	ASCP_TYPE_ERRC_GET_RSP = 0x0902,	// error code get response
	
	/* error string */
	ASCP_TYPE_ERRS_GET_REQ = 0x0A01,	// error string get request
	ASCP_TYPE_ERRS_GET_RSP = 0x0A02,	// error string get response
	
	/* data */
	ASCP_TYPE_DATA_BEG_REQ = 0x0B01,	// data begin request
	ASCP_TYPE_DATA_BEG_RSP = 0x0B02,	// data begin response
	ASCP_TYPE_DATA_END_REQ = 0x0B03,	// data end request
	ASCP_TYPE_DATA_END_RSP = 0x0B04,	// data end response
	ASCP_TYPE_DATA_BLOCK   = 0x0B05,	// data packet
	
	ASCP_TYPE_6620_SET_REQ = 0x0C01,	// ad6620 programming request
	ASCP_TYPE_6620_SET_RSP = 0x0C02		// ad6620 programming response
} ascp_type;

//
// ascp_code
//
typedef enum
{
	ASCP_CODE_NAME = 0x0001,					// device name
	ASCP_CODE_NUMB = 0x0002,					// version number
	ASCP_CODE_IVER = 0x0003,					// interface version
	ASCP_CODE_HVER = 0x0004,					// hardware / firmware version
	ASCP_CODE_ERRC = 0x0005,					// error code
	ASCP_CODE_ERRS = 0x0006,					// error string
	ASCP_CODE_DATA = 0x0018,					// data
	ASCP_CODE_FREQ = 0x0020,					// frequency
	ASCP_CODE_GAIN = 0x0038,					// rf gain
	ASCP_CODE_RATE = 0x00B0,					// sample rate
	ASCP_CODE_6620 = 0xA009,					// program the AD6620
	ASCP_CODE_DACK = 0x6003						// AD6620 programming ack
} ascp_code;





//
// ascp_message
//
struct ascp_message
{
	cobject_t cobject;								// parent class
	ascp_type type;										// message type
	uint16_t length;									// message length
	uint16_t code;										// message type
};
typedef struct ascp_message ascp_message_t;

//
// ascp_message_data
//
struct ascp_message_data
{
	ascp_message_t header;						// message header
	uint8_t channel;									// channel
	uint8_t command;									// run or stop
	uint8_t mode;											// contiguous or one-shot
	uint8_t count;										// number of blocks
	int16_t data[4096];								// I and Q parameters
};
typedef struct ascp_message_data ascp_message_data_t;

//
// ascp_message_freq
//
struct ascp_message_freq
{
	ascp_message_t header;						// message header
	uint8_t channel;									// channel
	uint32_t frequency;								// frequency in hertz
	uint8_t multiplier;								// multiplier
};
typedef struct ascp_message_freq ascp_message_freq_t;

//
// ascp_message_gain
//
struct ascp_message_gain
{
	ascp_message_t header;						// message header
	uint8_t mode;											// 0 = fixed, 1 = manual
	uint8_t gain;											// gain (0, -10, -20, -30)
};
typedef struct ascp_message_gain ascp_message_gain_t;

//
// ascp_message_6620
//
struct ascp_message_6620
{
	ascp_message_t header;						// message header
	uint16_t address;									// address
	uint8_t data0;										// data byte 0
	uint8_t data1;										// data byte 1
	uint8_t data2;										// data byte 2
	uint8_t data3;										// data byte 3
	uint8_t data4;										// data byte 4
};
typedef struct ascp_message_6620 ascp_message_6620_t;

//
// ascp_message_dack
//
struct ascp_message_dack
{
	ascp_message_t header;						// message header
	uint8_t data;											// always "1"
};
typedef struct ascp_message_dack ascp_message_dack_t;

//
// ascp
//
struct ascp
{
	protocol_t protocol;							// parent class
	
	uint8_t resync;										// true if in need of resync
	uint8_t data;											// true if in data streaming mode
	
	uint64_t messages_sent;						// number of messages sent
	uint64_t messages_rcvd;						// number of messages received
};
typedef struct ascp ascp_t;





/**
 *
 */
int ascp_init (ascp_t*);

/**
 *
 */
int ascp_destroy (ascp_t*);





/**
 * message, name, pool
 */
int ascp_message_init (ascp_message_t*, char*, opool_t*);

/**
 *
 */
int ascp_message_destroy (ascp_message_t*);





/**
 * message, start/stop, contiguous/one-shot, block count
 */
int ascp_data_get (ascp_message_data_t*, uint8_t, uint8_t, uint8_t);

/**
 *
 */
int ascp_frequency_set (ascp_message_freq_t*, uint32_t);

/**
 *
 */
int ascp_frequency_get (ascp_message_freq_t*);

/**
 *
 */
int ascp_gain_set (ascp_message_gain_t*, uint8_t, int8_t);

/**
 * message, address, data (low five bytes)
 */
int ascp_ad6620_set (ascp_message_6620_t*, uint16_t, int32_t);





/**
 *
 */
int ascp_message_read (ascp_t*, ascp_message_t**, connection_t*);

/**
 *
 */
int ascp_message_send (ascp_t*, ascp_message_t*, connection_t*);





/**
 *
 */
ascp_message_t* ascp_message_retain (ascp_message_t*);

/**
 *
 */
void ascp_message_release (ascp_message_t*);

#endif /* __PROTOCOL_ASCP_H__ */
