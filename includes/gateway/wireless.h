#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "common/device.h"


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

/* Message length field macros */
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
	uint8_t opening_flag;
	uint8_t length;
	uint8_t message[MAX_MESSAGE_FIELD_SENSOR_SIZE];
	uint8_t closing_flag;

}T_Packet_Sensor;



/**
 * Checks if there is an incoming packet from a sensor available, if there is
 * writes the id of the sensor to `*device_id`, the data of the packet to
 * `*data` and returns true; if there wasn't then returns false and doesn't
 * touch `device_id` or `data`.
 */
bool wireless_dequeue_incoming(
    device_id_t *device_id,
    uint8_t data[static WIRELESS_PAYLOAD_LENGTH]);

/**
 * Enqueues a packet to be sent to a sensor.  The data is copied out during
 * this call so `data` can be reused as soon as this returns.
 */
void wireless_enqueue_outgoing(
    device_id_t device_id,
    uint8_t const data[static WIRELESS_PAYLOAD_LENGTH]);
