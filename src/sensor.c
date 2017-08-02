#include "sensor/wireless.h"
#include "sensor/ki_store.h"
#include "sensor/door.h"
#include "common/device.h"

#define NULL 0  /* I assumed I can't add a library so I declare NULL */

/* SINGLE-BYTE COMMANDS LIST */
typedef enum
{
	PING = 0,
	RESET,
	ADD_KI,
	REMOVE_KI,
	OPEN_DOOR,

}T_Sensor_Commands;


/* Table used in calculating CRC8 */
static const uint8_t m_crc8_table[256] = {
    0x00, 0x91, 0xE3, 0x72, 0x07, 0x96, 0xE4, 0x75,
    0x0E, 0x9F, 0xED, 0x7C, 0x09, 0x98, 0xEA, 0x7B,
    0x1C, 0x8D, 0xFF, 0x6E, 0x1B, 0x8A, 0xF8, 0x69,
    0x12, 0x83, 0xF1, 0x60, 0x15, 0x84, 0xF6, 0x67,
    0x38, 0xA9, 0xDB, 0x4A, 0x3F, 0xAE, 0xDC, 0x4D,
    0x36, 0xA7, 0xD5, 0x44, 0x31, 0xA0, 0xD2, 0x43,
    0x24, 0xB5, 0xC7, 0x56, 0x23, 0xB2, 0xC0, 0x51,
    0x2A, 0xBB, 0xC9, 0x58, 0x2D, 0xBC, 0xCE, 0x5F,
    0x70, 0xE1, 0x93, 0x02, 0x77, 0xE6, 0x94, 0x05,
    0x7E, 0xEF, 0x9D, 0x0C, 0x79, 0xE8, 0x9A, 0x0B,
    0x6C, 0xFD, 0x8F, 0x1E, 0x6B, 0xFA, 0x88, 0x19,
    0x62, 0xF3, 0x81, 0x10, 0x65, 0xF4, 0x86, 0x17,
    0x48, 0xD9, 0xAB, 0x3A, 0x4F, 0xDE, 0xAC, 0x3D,
    0x46, 0xD7, 0xA5, 0x34, 0x41, 0xD0, 0xA2, 0x33,
    0x54, 0xC5, 0xB7, 0x26, 0x53, 0xC2, 0xB0, 0x21,
    0x5A, 0xCB, 0xB9, 0x28, 0x5D, 0xCC, 0xBE, 0x2F,
    0xE0, 0x71, 0x03, 0x92, 0xE7, 0x76, 0x04, 0x95,
    0xEE, 0x7F, 0x0D, 0x9C, 0xE9, 0x78, 0x0A, 0x9B,
    0xFC, 0x6D, 0x1F, 0x8E, 0xFB, 0x6A, 0x18, 0x89,
    0xF2, 0x63, 0x11, 0x80, 0xF5, 0x64, 0x16, 0x87,
    0xD8, 0x49, 0x3B, 0xAA, 0xDF, 0x4E, 0x3C, 0xAD,
    0xD6, 0x47, 0x35, 0xA4, 0xD1, 0x40, 0x32, 0xA3,
    0xC4, 0x55, 0x27, 0xB6, 0xC3, 0x52, 0x20, 0xB1,
    0xCA, 0x5B, 0x29, 0xB8, 0xCD, 0x5C, 0x2E, 0xBF,
    0x90, 0x01, 0x73, 0xE2, 0x97, 0x06, 0x74, 0xE5,
    0x9E, 0x0F, 0x7D, 0xEC, 0x99, 0x08, 0x7A, 0xEB,
    0x8C, 0x1D, 0x6F, 0xFE, 0x8B, 0x1A, 0x68, 0xF9,
    0x82, 0x13, 0x61, 0xF0, 0x85, 0x14, 0x66, 0xF7,
    0xA8, 0x39, 0x4B, 0xDA, 0xAF, 0x3E, 0x4C, 0xDD,
    0xA6, 0x37, 0x45, 0xD4, 0xA1, 0x30, 0x42, 0xD3,
    0xB4, 0x25, 0x57, 0xC6, 0xB3, 0x22, 0x50, 0xC1,
    0xBA, 0x2B, 0x59, 0xC8, 0xBD, 0x2C, 0x5E, 0xCF
};



/**
 * calculateCrc8
 *
 * Function to calulate a crc8 of the supplied data.
 *
 * @param 	  data source data to calulate crc
 * @param     len Length of data to CRC
 *
 * @return    Returns CRC8 calculation of data.
 */


static uint8_t calculateCrc8( const uint8_t const *data, uint32_t len )
{
    uint32_t sum = 0xFF;

    while ( len > 0 )
    {
        sum = m_crc8_table[( sum ^ *data++ ) & 0xFF];
        --len;
    }
    return ( ~sum & 0xFF );
}


uint8_t const * getToken(uint8_t *data_from_gateway)
{
	uint8_t i;
	uint8_t const *token_data;

	for(i = 0; i < KI_TOKEN_LENGTH; ++i)
	{
		token_data = &(data_from_gateway[PACKET_SENSOR_HEADER_LENGTH + 1 + i]);
		++token_data;
	}
	return token_data;
}



/**
 * prepareMessageToGateway
 *
 * Function to prepare the data array to be sent to a sensor.
 *
 * @param     data_to_send Pointer to array to be sent
 * @param     T_Packet_Gateway* packet Pointer to struct containing packet fields
 *
 * @return    Nothing
 */


void prepareMessageToGateway(uint8_t const* data_to_send, T_Packet_Gateway* packet)
{
	uint8_t const* data_to_send_ptr = data_to_send;
	uint8_t crc8, i;

	packet->op_flag = OPENING_FLAG_SENSOR;
	data_to_send = &(packet->op_flag);
	data_to_send++;
	data_to_send = &(packet->message_size);
	data_to_send++;

	for(i = 0; i < packet->message_size; ++i)
	{
		data_to_send = &(packet->message_body[i]);
		data_to_send++;
	}

	crc8 = calculateCrc8(data_to_send_ptr,
						 PACKET_SENSOR_HEADER_LENGTH);
	data_to_send = &crc8;
	data_to_send++;

	packet->close_flag = CLOSING_FLAG_SENSOR;
	data_to_send = &(packet->close_flag);

	data_to_send = data_to_send_ptr;

}



/**
 * This function is polled by the main loop and should handle any packets coming
 * in over the 868 MHz communication channel.
 */
void handle_communication2(void)
{
  uint8_t *packet_from_gateway = NULL, *data_to_gateway = NULL;
  uint8_t command, message_length, pos_in_packet = 0;
  T_Packet_Gateway packet_to_gateway;
  bool send_packet_to_gateway = FALSE;

  if(wireless_dequeue_incoming(packet_from_gateway))
  {
	  /* Packet first verification: message length */
	  message_length = MESSAGE_LENGTH_SENSOR_FIELD(packet_from_gateway[pos_in_packet + MESSAGE_LENGTH_SENSOR_FIELD_POS]);
	  if(message_length < MAX_MESSAGE_FIELD_SENSOR_SIZE
			  || message_length == MAX_MESSAGE_FIELD_SENSOR_SIZE)
	  {
		  /* Packet second verification: opening and closing flags */
		  if(packet_from_gateway[pos_in_packet] == OPENING_FLAG_SENSOR
				  && packet_from_gateway[pos_in_packet + PACKET_SENSOR_HEADER_LENGTH + message_length] == CLOSING_FLAG_SENSOR)
		  {
			  /* Packet third verification: crc8 */
			  if( packet_from_gateway[pos_in_packet + PACKET_SENSOR_HEADER_LENGTH + message_length + CRC_SENSOR_FIELD_SIZE] == calculateCrc8(packet_from_gateway, PACKET_SENSOR_HEADER_LENGTH) )
			  {
				  command = packet_from_gateway[pos_in_packet + MESSAGE_SENSOR_FIELD_POS];
				  switch(command)
				  {
				  case PING:
					  packet_to_gateway.message_size = 1;
				  	  packet_to_gateway.message_body[0] = STILL_ALIVE_SENSOR;
				  	  send_packet_to_gateway = TRUE;
				  	  break;
				  case RESET:
					  reset_device();
				  	  break;
				  case ADD_KI:
				  	  packet_to_gateway.message_size = 1;
				  	  packet_to_gateway.message_body[0] = ki_store_add(getToken(packet_from_gateway));
				  	  send_packet_to_gateway = TRUE;
				  	  break;
				   case REMOVE_KI:
				  	  packet_to_gateway.message_size = 1;
				  	  packet_to_gateway.message_body[0] = ki_store_remove(getToken(packet_from_gateway));
				  	  send_packet_to_gateway = TRUE;
				  	  break;
				   case OPEN_DOOR:
				  	  door_trigger();
				  	  packet_to_gateway.message_size = 1;
				  	  packet_to_gateway.message_body[0] = ACK_SENSOR;
				  	  send_packet_to_gateway = TRUE;
				  	  break;
				   default:
				  	  packet_to_gateway.message_size = 1;
				  	  packet_to_gateway.message_body[0] = NACK_INVALID_COMMAND_SENSOR;
				  	  send_packet_to_gateway = TRUE;
				  	  break;
				   }

			  }
			  else
			  {
				  packet_to_gateway.message_size = 1;
				  packet_to_gateway.message_body[0] = NACK_CRC8_INVALID_SENSOR;
				  send_packet_to_gateway = TRUE;

			  }
		  }
		  else
		  {
			  packet_to_gateway.message_size = 1;
			  packet_to_gateway.message_body[0] = NACK_PACKET_INVALID_SENSOR;
			  send_packet_to_gateway = TRUE;
		  }
	  }
	  else
	  {
		  packet_to_gateway.message_size = 1;
		  packet_to_gateway.message_body[0] = NACK_LENGTH_INVALID_SENSOR;
		  send_packet_to_gateway = TRUE;
	  }
  }

  /** SEND PACKET IF READY **/
  if(send_packet_to_gateway)
  {
	  prepareMessageToGateway(data_to_gateway, &packet_to_gateway);
	  wireless_enqueue_outgoing(data_to_gateway);
	  send_packet_to_gateway = FALSE;
  }
}
