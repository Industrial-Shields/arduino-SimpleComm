#ifndef __SimpleComm_H__
#define __SimpleComm_H__

#include <Arduino.h>


#include "SimplePacket.h"


// PACKET FORMAT:
//  _____________________________________________________________________
// |         |         |                                                 |
// |         |         |                       PKT                       |
// |_________|_________|_________________________________________________|
// |         |         |                             |         |         |
// |   SYN   |   LEN   |            HDR              |   DAT   |   CRC   |
// |_________|_________|_____________________________|_________|_________|
// |         |         |         |         |         |         |         |
// | SYN (1) | LEN (1) | DST (1) | SRC (1) | TYP (1) |   DAT   | CRC (1) |
// |_________|_________|_________|_________|_________|_________|_________|
//

#define SIMPLECOMM_SYN_LEN 1
#define SIMPLECOMM_LEN_LEN 1
#define SIMPLECOMM_DST_LEN 1
#define SIMPLECOMM_SRC_LEN 1
#define SIMPLECOMM_TYP_LEN 1
#define SIMPLECOMM_HDR_LEN (SIMPLECOMM_DST_LEN + SIMPLECOMM_SRC_LEN + SIMPLECOMM_TYP_LEN)
#define SIMPLECOMM_CRC_LEN 1

#define SIMPLECOMM_SYN_VALUE 0x02

#define SIMPLECOMM_MAX_DATA_LEN SIMPLEPACKET_DEFAULT_SIZE
#define SIMPLECOMM_BUFFER_SIZE (SIMPLECOMM_SYN_LEN + \
				SIMPLECOMM_LEN_LEN + \
				SIMPLECOMM_HDR_LEN + \
				SIMPLECOMM_MAX_DATA_LEN + \
				SIMPLECOMM_CRC_LEN)


class SimpleCommClass {
public:
	explicit SimpleCommClass();

public:
	void begin(uint8_t address = 0);

	bool send(Stream &stream, SimplePacket &packet, uint8_t destination = 0);
	bool send(Stream &stream, SimplePacket &packet, uint8_t destination, uint8_t type);
	// Buffers passed to this function must be at least of SIMPLEPACKET_DEFAULT_SIZE bytes (128 bytes)
	// When calling receive for the first time with a new auxiliar buffer, auxRxBufferLen must be 0
	bool receive(Stream &stream, SimplePacket &packet, uint8_t* auxRxBuffer = nullptr, uint8_t* auxRxBufferLen = nullptr);

private:
	uint8_t calcCRC(uint8_t *buffer, size_t len);

private:
	uint8_t _address;
};

extern SimpleCommClass SimpleComm;

#endif // __SimpleComm_H__
