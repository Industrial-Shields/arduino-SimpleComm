/*
  Copyright (c) 2017 Boot&Work Corp., S.L. All rights reserved

  This library is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SimpleComm.h"


// #define SIMPLECOMM_DEBUG

#ifdef SIMPLECOMM_DEBUG
static void printBuff(const uint8_t* buff, uint8_t len) {
	for (uint8_t c = 0; c < len; c++) {
		uint8_t ch = buff[c];
		if (isAlphaNumeric(ch)) {
			Serial.write(ch);
		}
		else {
			Serial.print(F("\\x"));
			Serial.print(ch, HEX);
		}
	}
}
#endif


#define PKT_LEN(dlen) (SP_HDR_LEN + (dlen) + SP_CRC_LEN)


////////////////////////////////////////////////////////////////////////////////////////////////////
SimpleCommClass::SimpleCommClass() {
	_address = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void SimpleCommClass::begin(uint8_t address) {
	_address = address;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool SimpleCommClass::send(Stream &stream, SimplePacket &packet, uint8_t destination) {
	packet._buff.syn = SP_SYN_VALUE;

	uint8_t dataLength = packet.getDataLength();
	if (dataLength > SP_MAX_DATA_LEN) {
		return false;
	}

	packet._exhausted = true;

	packet._buff.expectedLen = PKT_LEN(dataLength);
	packet.setSource(_address);
	packet.setDestination(destination);

	packet._buff.data[dataLength] = calcCRC(&packet._buff.source, SP_HDR_LEN + dataLength);

	uint8_t totalLength = SP_SYN_LEN + SP_LEN_LEN + PKT_LEN(dataLength);

	bool ret = stream.write(&packet._buff.syn, totalLength) == totalLength;
#ifdef SIMPLECOMM_DEBUG
	Serial.print(F("Sent package with len "));
	Serial.print(totalLength);
        Serial.print(F(": "));
	printBuff(&packet._buff.syn, totalLength);
	Serial.println();
#endif
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool SimpleCommClass::send(Stream &stream, SimplePacket &packet, uint8_t destination, uint8_t type) {
	packet.setType(type);
	return send(stream, packet, destination);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool SimpleCommClass::receive(Stream &stream, SimplePacket &packet) {
	uint8_t* rxBuffer = &packet._buff.syn;
	uint8_t* rxBufferLen = &packet._dataLen;

	if (packet._exhausted) {
#ifdef SIMPLECOMM_DEBUG
		Serial.println(F("Packet is exhausted, clearing it..."));
#endif
		packet.clear();
	}

	while (stream.available()) {
		uint8_t in = stream.read();

		if ((*rxBufferLen == 0) && (in != SP_SYN_VALUE)) {
#ifdef SIMPLECOMM_DEBUG
			Serial.print(F("Unsynchronized: "));
			Serial.println(in, HEX);
#endif
			continue;
		}

		if ((*rxBufferLen == SP_SYN_LEN)
		    && (
			(in > (SP_HDR_LEN + SP_MAX_DATA_LEN + SP_CRC_LEN))
			|| (in < (SP_HDR_LEN + SP_CRC_LEN))
			)) {
#ifdef SIMPLECOMM_DEBUG
			Serial.println(F("Invalid data length"));
#endif
			packet.clear();
			continue;
		}

		rxBuffer[(*rxBufferLen)++] = in;

		if (*rxBufferLen > SP_SYN_LEN + SP_LEN_LEN + SP_HDR_LEN) {
			uint8_t tlen = rxBuffer[1];
			if (*rxBufferLen == (tlen + SP_SYN_LEN + SP_LEN_LEN)) {
				// Buffer complete

				// Check CRC
				uint8_t expectedCrc = calcCRC(rxBuffer + SP_SYN_LEN + SP_LEN_LEN, tlen - SP_CRC_LEN);
				if (rxBuffer[SP_SYN_LEN + SP_LEN_LEN + tlen - SP_CRC_LEN] != expectedCrc) {
#ifdef SIMPLECOMM_DEBUG
					Serial.print(F("Invalid CRC: "));
					Serial.print(rxBuffer[SP_SYN_LEN + SP_LEN_LEN + tlen - SP_CRC_LEN], HEX);
					Serial.print(F(" != "));
					Serial.print(expectedCrc, HEX);
					Serial.println();
					printBuff(rxBuffer, SP_SYN_LEN + SP_LEN_LEN + tlen);
#endif
					packet.clear();
					continue;
				}

				// Check destination
				// if my address is 0 then receive all messages
				// if destination address is 0 then it is a broadcast message
				if (_address != 0
				    && rxBuffer[SP_SYN_LEN + SP_LEN_LEN] != 0
				    && rxBuffer[SP_SYN_LEN + SP_LEN_LEN] != _address) {
#ifdef SIMPLECOMM_DEBUG
					Serial.println(F("Received package it's not for me"));
#endif
					packet.clear();
					continue;
				}

				packet._dataLen -= SP_SYN_LEN + SP_LEN_LEN + SP_HDR_LEN + SP_CRC_LEN;
#ifdef SIMPLECOMM_DEBUG
				Serial.print(F("Good package with len "));
				Serial.print(packet._dataLen);
				Serial.print(F(": "));
				printBuff(packet._buff.data, packet._dataLen);
				Serial.println();
#endif
				packet._exhausted = true;

				return true;
			}
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t SimpleCommClass::calcCRC(const uint8_t *buffer, size_t len) {
	uint8_t ret = 0;
	while (len--) {
		ret += *buffer++;
	}
	return ret;
}

SimpleCommClass SimpleComm;
