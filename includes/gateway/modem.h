#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/***************************
 **		MODEM PROTOCOL    **
 ***************************/

/* General macros */
#define MODEM_MAX_PAYLOAD_LENGTH 128
#define PACKET_MODEM_HEADER_LENGTH (OPENING_FLAG_MODEM_SIZE + DEVICE_FIELD_SIZE + MESSAGE_LENGTH_FIELD_MODEM_SIZE)
#define PACKET_MODEM_TRAILER_LENGTH (CRC_FIELD_MODEM_SIZE + CLOSING_FLAG_MODEM_SIZE)
#define MODEM_FIELD_MASK  0xFF

/* Opening and closing flags macros */
#define OPENING_FLAG_MODEM      0xF9
#define CLOSING_FLAG_MODEM      0xF8
#define OPENING_FLAG_MODEM_SIZE 1
#define CLOSING_FLAG_MODEM_SIZE 1

/* Device field macros */
#define DEVICE_FIELD_SIZE     1
#define DEVICE_FIELD_POS      1
#define DEVICE_IS_GATEWAY(X)  (X & MODEM_FIELD_MASK)

/* Message length field macros */
#define MESSAGE_LENGTH_FIELD_MODEM_SIZE   1
#define MESSAGE_LENGTH_FIELD_MODEM_POS    2
#define MESSAGE_LENGTH_FIELD_MODEM(X)   	(X & MODEM_FIELD_MASK)

/* Message field macros */
#define MAX_MESSAGE_FIELD_MODEM_SIZE  123  /* 123 bytes of max size for the message body */
#define MESSAGE_FIELD_MODEM_POS       3

/* CRC8 field macros */
#define CRC_FIELD_MODEM_SIZE     1
#define CRC_FIELD_MODEM(X)   	   (X & MODEM_FIELD_MASK)


typedef enum
{
	SENSOR = 0,
	GATEWAY,

}T_Device_Type;


/*
 * Enum with possible responses to be sent to the backend.
 */
typedef enum
{
	ACK = 0,
	NACK_INVALID_COMMAND,
	NACK_LENGTH_INVALID,
	NACK_CRC8_INVALID,
	NACK_PACKET_INVALID,
	STILL_ALIVE,

}T_Response_To_Backend;


/*
 * This struct is intended to build a packet to be sent to the backend.
 * Included pragma pack to optimized memory.
 */
#pragma pack(1)
typedef struct{
	uint8_t opening_flag;
	uint8_t device;
	uint8_t length;
	uint8_t message[MAX_MESSAGE_FIELD_MODEM_SIZE];
	uint8_t closing_flag;

}T_Packet_Modem;



/**
 * Checks if there is an incoming packet from the backend available, if there is
 * writes a pointer to the packet to `*data`, the length of the packet to
 * `*length` and returns true; if there wasn't then returns false and doesn't
 * touch `data` or `length`. The maximum length of the buffer returned will be
 * MODEM_MAX_PAYLOAD_LENGTH.
 *
 * The buffer returned in `*data` is valid until the next time that
 * `modem_dequeue_incoming` is called.
 */
bool modem_dequeue_incoming(uint8_t const **data, size_t *length);

/**
 * Enqueues a packet to be sent to the backend, reads `length` bytes from
 * `data`. The data is copied out during this call so `data` can be reused as
 * soon as this returns. `length` must be less than MODEM_MAX_PAYLOAD_LENGTH.
 */
void modem_enqueue_outgoing(uint8_t const *data, size_t length);
