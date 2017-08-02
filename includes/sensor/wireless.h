#pragma once

#include <stdint.h>
#include <stdbool.h>

/***************************
 **		868MHz PROTOCOL    **
 ***************************/

/* General macros */
#define WIRELESS_PAYLOAD_LENGTH 32
#define PACKET_SENSOR_HEADER_LENGTH (OPENING_FLAG_SENSOR_SIZE + MESSAGE_LENGTH_SENSOR_FIELD_SIZE)
#define PACKET_SENSOR_TRAILER_LENGTH (CRC_SENSOR_FIELD_SIZE + CLOSING_FLAG_SENSOR_SIZE)
#define SENSOR_FIELD_MASK  0xFF

/* Opening and closing flags macros */
#define OPENING_FLAG_SENSOR      0xF7
#define CLOSING_FLAG_SENSOR      0xF6
#define OPENING_FLAG_SENSOR_SIZE 1
#define CLOSING_FLAG_SENSOR_SIZE 1

/* Message length field macros */
#define MESSAGE_LENGTH_SENSOR_FIELD_SIZE     1
#define MESSAGE_LENGTH_SENSOR_FIELD_POS      1
#define MESSAGE_LENGTH_SENSOR_FIELD(X)   	(X & SENSOR_FIELD_MASK)

/* Message body field macros */
#define MAX_MESSAGE_FIELD_SENSOR_SIZE 28  /* 28 bytes of max size for the message body */
#define MESSAGE_SENSOR_FIELD_SIZE     1
#define MESSAGE_SENSOR_FIELD_POS      2
#define MESSAGE_SENSOR_FIELD(X)   	  (X & SENSOR_FIELD_MASK)

/* CRC8 field macros */
#define CRC_SENSOR_FIELD_SIZE     1
#define CRC_SENSOR_FIELD(X)   	  (X & SENSOR_FIELD_MASK)


/*
 * This struct is intended to build a packet to be sent to a sensor.
 * Included pragma pack to optimized memory.
 */
#pragma pack(1)
typedef struct{
	uint8_t op_flag;
	uint8_t message_size;
	uint8_t message_body[MAX_MESSAGE_FIELD_SENSOR_SIZE];
	uint8_t close_flag;

}T_Packet_Gateway;


/*
 * Enum with possible responses to be sent to the backend.
 */
typedef enum
{
	ACK_SENSOR = 0,
	NACK_INVALID_COMMAND_SENSOR,
	NACK_LENGTH_INVALID_SENSOR,
	NACK_CRC8_INVALID_SENSOR,
	NACK_PACKET_INVALID_SENSOR,
	STILL_ALIVE_SENSOR,

}T_Response_To_Gateway;



/**
 * Checks if there is an incoming packet from a gateway available, if there is
 * writes the data of the packet to
 * `*data` and returns true; if there wasn't then returns false and doesn't
 * touch `data`.
 */
bool wireless_dequeue_incoming(uint8_t data[static WIRELESS_PAYLOAD_LENGTH]);

/**
 * Enqueues a packet to be sent to a gateway.  The data is copied out during
 * this call so `data` can be reused as soon as this returns.
 */
void wireless_enqueue_outgoing(uint8_t const data[static WIRELESS_PAYLOAD_LENGTH]);
