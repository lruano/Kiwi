#include "gateway/modem.h"
#include "gateway/wireless.h"
#include "common/device.h"

/* SINGLE-BYTE COMMANDS LIST */
typedef enum
{
	PING = 0,
	RESET,

}T_Gateway_Commands;


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

/**
 * copyMessage
 *
 * Function to calulate a crc8 of the supplied data.
 *
 * @param 	  source Array containing the data to be copied
 * @param     destiny Destination array
 * @param     length Size of source array
 * @param     start Start reading point on source array
 *
 * @return    Nothing
 */
void copyMessage(uint8_t const *source, uint8_t const *destiny, uint8_t length, uint8_t start)
{
	uint8_t i;
	for(i = 0; i < length; ++i)
	{
		destiny = &(source[start + i]);
		++destiny;
	}
}



/**
 * prepareMessageToBackend
 *
 * Function to prepare the data array to be sent to the backend.
 *
 * @param     data_to_send Pointer to array to be sent
 * @param     T_Packet_Modem* packet Pointer to struct containing packet fields
 *
 * @return    Nothing
 */


void prepareMessageToBackend(uint8_t const* data_to_send, T_Packet_Modem* packet)
{
	uint8_t const* data_to_send_ptr = data_to_send;
	uint8_t crc8, i;

	packet->opening_flag = OPENING_FLAG_MODEM;
	data_to_send = &(packet->opening_flag);
	data_to_send++;
	data_to_send = &(packet->device);
	data_to_send++;
	data_to_send = &(packet->length);
	data_to_send++;

	for(i = 0; i < packet->length; ++i)
	{
		data_to_send = &(packet->message[i]);
		data_to_send++;
	}

	crc8 = calculateCrc8(data_to_send_ptr, PACKET_MODEM_HEADER_LENGTH);
	data_to_send = &crc8;
	data_to_send++;

	packet->closing_flag = CLOSING_FLAG_MODEM;
	data_to_send = &(packet->closing_flag);

	data_to_send = data_to_send_ptr;

}



/**
 * prepareMessageToSensor
 *
 * Function to prepare the data array to be sent to a sensor.
 *
 * @param     data_to_send Pointer to array to be sent
 * @param     T_Packet_Sensor* packet Pointer to struct containing packet fields
 *
 * @return    Nothing
 */


void prepareMessageToSensor(uint8_t const* data_to_send, T_Packet_Sensor* packet)
{
	uint8_t const* data_to_send_ptr = data_to_send;
	uint8_t crc8, i;

	packet->opening_flag = OPENING_FLAG_SENSOR;
	data_to_send = &(packet->opening_flag);
	data_to_send++;
	data_to_send = &(packet->length);
	data_to_send++;

	for(i = 0; i < packet->length; ++i)
	{
		data_to_send = &(packet->message[i]);
		data_to_send++;
	}

	crc8 = calculateCrc8(data_to_send_ptr,
						 PACKET_SENSOR_HEADER_LENGTH);
	data_to_send = &crc8;
	data_to_send++;

	packet->closing_flag = CLOSING_FLAG_SENSOR;
	data_to_send = &(packet->closing_flag);

	data_to_send = data_to_send_ptr;

}




/**
 * This function is polled by the main loop and should handle any packets coming
 * in over the modem or 868 MHz communication channel.
 */
void handle_communication(void)
{
	  uint8_t const *packet_from_backend = NULL;
	  size_t packet_from_backend_length;
	  uint8_t data_to_backend[MODEM_MAX_PAYLOAD_LENGTH], data_to_sensor[WIRELESS_PAYLOAD_LENGTH];
	  uint8_t pos_in_packet = 0, command, crc8, message_length, *packet_from_sensor = NULL;
	  device_id_t id_device;
	  T_Packet_Modem packet_backend;
	  T_Packet_Sensor packet_sensor;
	  bool send_packet_to_backend = FALSE, send_packet_to_sensor = FALSE;


	  /* Checks if a message over the Internet came in */
	  if(modem_dequeue_incoming(&packet_from_backend, &packet_from_backend_length))
	  {
		  /* Packet first verification: packet length */
		  if(packet_from_backend_length == MODEM_MAX_PAYLOAD_LENGTH
				  || packet_from_backend_length < MODEM_MAX_PAYLOAD_LENGTH)
		  {
			  message_length = MESSAGE_LENGTH_FIELD_MODEM(packet_from_backend[pos_in_packet + MESSAGE_FIELD_MODEM_POS]);

			  /* Packet second verification: opening and closing flags */
			  if(packet_from_backend[pos_in_packet] == OPENING_FLAG_MODEM
					  && packet_from_backend[pos_in_packet + PACKET_MODEM_HEADER_LENGTH + message_length + PACKET_MODEM_TRAILER_LENGTH] == CLOSING_FLAG_MODEM)
			  {
				  /* Packet third verification: crc8 */
			  	  crc8 = calculateCrc8(packet_from_backend,
			  			  	  	  	   PACKET_MODEM_HEADER_LENGTH);

			  	  if(CRC_FIELD_MODEM(packet_from_backend[pos_in_packet + PACKET_MODEM_HEADER_LENGTH + message_length]) == crc8)
			  	  {
			  		  /* Verify target device: gateway or sensor */
			  		  if(DEVICE_IS_GATEWAY(packet_from_backend[pos_in_packet + DEVICE_FIELD_POS]))
			  		  {
	  					  command = packet_from_backend[pos_in_packet + MESSAGE_FIELD_MODEM_POS];
	  					  switch(command)
			  			  {
			  			  case PING:
			  				  packet_backend.device = GATEWAY;
			  				  packet_backend.length = 1;
			  				  packet_backend.message[0] = STILL_ALIVE;
			  				  send_packet_to_backend = TRUE;
			  				  break;
			  			  case RESET:
			  				  reset_device();
			  				  break;
			  			  default:
			  				  packet_backend.device = GATEWAY;
			  				  packet_backend.length = 1;
			  				  packet_backend.message[0] = NACK_INVALID_COMMAND;
			  				  send_packet_to_backend = TRUE;
			  				  break;
			  			  }

			  		  }
			  		  /* If a packet targeted to a sensor came in */
			  		  else if(!DEVICE_IS_GATEWAY(packet_from_backend[pos_in_packet + DEVICE_FIELD_POS]))
			  		  {
			  			  /* If message body bigger than 28 bytes, message invalid */
			  			  if(message_length > MAX_MESSAGE_FIELD_SENSOR_SIZE)
			  			  {
				  			packet_backend.device = GATEWAY;
				  			packet_backend.length = 1;
				  			packet_backend.message[0] = NACK_LENGTH_INVALID;
				  			send_packet_to_backend = TRUE;
			  			  }
		  				  else
		  				  {
		  					packet_sensor.length = 1;
		  					copyMessage(packet_from_backend, packet_sensor.message, message_length, MESSAGE_FIELD_MODEM_POS);
		  					send_packet_to_sensor = TRUE;
		  				  }
			  		  }
			  		  else
			  		  {
		  				packet_backend.device = GATEWAY;
		  				packet_backend.length = 1;
		  				packet_backend.message[0] = NACK_LENGTH_INVALID;
		  				send_packet_to_backend = TRUE;
			  		  }
			  	  }
			  	  else
			  	  {
			  		packet_backend.device = GATEWAY;
	  				packet_backend.length = 1;
	  				packet_backend.message[0] = NACK_CRC8_INVALID;
	  				send_packet_to_backend = TRUE;
			  	  }
			  }
			  else
			  {
  				packet_backend.device = GATEWAY;
  				packet_backend.length = 1;
  				packet_backend.message[0] = NACK_PACKET_INVALID;
  				send_packet_to_backend = TRUE;
			  }
		  }
		  else
		  {
			packet_backend.device = GATEWAY;
			packet_backend.length = 1;
			packet_backend.message[0] = NACK_PACKET_INVALID;
			send_packet_to_backend = TRUE;
		  }

	  }
	  /* If a packet is received from a sensor */
	  else if(wireless_dequeue_incoming(&id_device, packet_from_sensor))
	  {
		  /* Packet first verification: message length */
		  message_length = MESSAGE_LENGTH_SENSOR_FIELD(packet_from_sensor[pos_in_packet + MESSAGE_LENGTH_SENSOR_FIELD_POS]);
		  if(message_length < MAX_MESSAGE_FIELD_SENSOR_SIZE
				  || message_length == MAX_MESSAGE_FIELD_SENSOR_SIZE)
		  {
			  /* Packet second verification: opening and closing flags */
			  if(packet_from_sensor[pos_in_packet] == OPENING_FLAG_SENSOR
					  && packet_from_sensor[pos_in_packet + PACKET_SENSOR_HEADER_LENGTH + message_length] == CLOSING_FLAG_SENSOR)
			  {
				  /* Packet third verification: crc8 */
				  if( packet_from_sensor[pos_in_packet + PACKET_SENSOR_HEADER_LENGTH + message_length + CRC_SENSOR_FIELD_SIZE] == calculateCrc8(packet_from_sensor, PACKET_SENSOR_HEADER_LENGTH) )
				  {
					  /* If packet is valid, extract message and send it to backend */
					  packet_backend.device = SENSOR;
					  packet_backend.length = message_length;
					  copyMessage(packet_from_sensor, packet_backend.message, message_length, MESSAGE_SENSOR_FIELD_POS);
					  send_packet_to_backend = TRUE;
				  }
				  else
				  {
					  /*
					   * Not clear if the sensor will re send the message after a timeout so no implementation here.
					   * If the sensor will re send the message in case of error, a NACK response should be sent (to be implemented here)
					   */
				  }
			  }
			  else
			  {
				  /*
				   * Not clear if the sensor will re send the message after a timeout so no implementation here.
				   * If the sensor will re send the message in case of error, a NACK response should be sent (to be implemented here)
				   */
			  }
		  }
		  else
		  {
			  /*
			   * Not clear if the sensor will re send the message after a timeout so no implementation here.
			   * If the sensor will re send the message in case of error, a NACK response should be sent (to be implemented here)
			   */
		  }
	  }
	  else
	  {
		packet_backend.device = GATEWAY;
		packet_backend.length = 1;
		packet_backend.message[0] = NACK_PACKET_INVALID;
		send_packet_to_backend = TRUE;
	  }



	  /** SEND PACKET IF READY **/
	  if(send_packet_to_backend)
	  {
		prepareMessageToBackend(data_to_backend, &packet_backend);
		modem_enqueue_outgoing(data_to_backend,
					           PACKET_MODEM_HEADER_LENGTH + packet_backend.length + PACKET_MODEM_TRAILER_LENGTH);
		send_packet_to_backend = FALSE;
	  }

	  if(send_packet_to_sensor)
	  {
			prepareMessageToSensor(data_to_sensor, &packet_sensor);
			wireless_enqueue_outgoing(get_device_id(), data_to_sensor);
			send_packet_to_sensor = FALSE;
	  }

}







