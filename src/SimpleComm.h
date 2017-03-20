#ifndef __SimpleComm_H__
#define __SimpleComm_H__

#include <Arduino.h>

#include "SimplePacket.h"

class SimpleCommClass {
	public:
		explicit SimpleCommClass();

	public:
		void begin(uint8_t address);

		bool send(Stream &stream, SimplePacket &packet, uint8_t destination);
		bool send(Stream &stream, SimplePacket &packet, uint8_t destination, uint8_t type);
		bool receive(Stream &stream, SimplePacket &packet);

	private:
		uint8_t calcCRC(uint8_t *buffer, size_t len);

	private:
		uint8_t _address;
};

extern SimpleCommClass SimpleComm;

#endif // __SimplePacket_H__
