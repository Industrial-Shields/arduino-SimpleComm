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


#define PKT_LEN(dlen) (SIMPLECOMM_HDR_LEN + (dlen) + SIMPLECOMM_CRC_LEN)


static uint8_t _staticRxBuffer[SIMPLECOMM_BUFFER_SIZE];
static uint8_t _staticRxBufferLen = 0;


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
	uint8_t dlen;

	if (dlen > SIMPLECOMM_MAX_DATA_LEN) {
		return false;
	}

	const uint8_t *data = (const uint8_t*) packet.getData(dlen);
        uint8_t _txBuffer[SIMPLECOMM_SYN_LEN + SIMPLECOMM_LEN_LEN + PKT_LEN(dlen)];

	packet.setSource(_address);
	packet.setDestination(destination);


	uint8_t *ptr = _txBuffer;
	*ptr++ = SIMPLECOMM_SYN_VALUE;
	*ptr++ = PKT_LEN(dlen);
	*ptr++ = packet.getDestination();
	*ptr++ = packet.getSource();
	*ptr++ = packet.getType();
	if (dlen > 0) {
		memcpy(ptr, data, dlen);
		ptr += dlen;
	}
	*ptr++ = calcCRC(_txBuffer + SIMPLECOMM_SYN_LEN + SIMPLECOMM_LEN_LEN, SIMPLECOMM_HDR_LEN + dlen);

	size_t tlen = ptr - _txBuffer;
	return stream.write(_txBuffer, tlen) == tlen;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool SimpleCommClass::send(Stream &stream, SimplePacket &packet, uint8_t destination, uint8_t type) {
	packet.setType(type);
	return send(stream, packet, destination);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool SimpleCommClass::receive(Stream &stream, SimplePacket &packet, uint8_t* auxRxBuffer, uint8_t* auxRxBufferLen) {
	if (auxRxBuffer == nullptr || auxRxBufferLen == nullptr) {
		auxRxBuffer = _staticRxBuffer;
		auxRxBufferLen = &_staticRxBufferLen;
	}

	while (stream.available()) {
		uint8_t in = stream.read();

		if ((*auxRxBufferLen == 0) && (in != SIMPLECOMM_SYN_VALUE)) {
			// Unsynchronized
			continue;
		}

		if ((*auxRxBufferLen == SIMPLECOMM_SYN_LEN)
		    && (
			(in > (SIMPLECOMM_HDR_LEN + SIMPLECOMM_MAX_DATA_LEN + SIMPLECOMM_CRC_LEN))
			|| (in < (SIMPLECOMM_HDR_LEN + SIMPLECOMM_CRC_LEN))
			)) {
			// Invalid data length
			*auxRxBufferLen = 0;
			continue;
		}

		auxRxBuffer[(*auxRxBufferLen)++] = in;

		if (*auxRxBufferLen > SIMPLECOMM_SYN_LEN + SIMPLECOMM_LEN_LEN + SIMPLECOMM_HDR_LEN) {
			uint8_t tlen = auxRxBuffer[1];
			if (*auxRxBufferLen == (tlen + SIMPLECOMM_SYN_LEN + SIMPLECOMM_LEN_LEN)) {
				// Buffer complete

				// Check CRC
				if (auxRxBuffer[SIMPLECOMM_SYN_LEN + SIMPLECOMM_LEN_LEN + tlen - SIMPLECOMM_CRC_LEN] !=
				    calcCRC(auxRxBuffer + SIMPLECOMM_SYN_LEN + SIMPLECOMM_LEN_LEN, tlen - SIMPLECOMM_CRC_LEN)) {
					// Invalid CRC
					*auxRxBufferLen = 0;
					continue;
				}

				// Check destination
				// if my address is 0 then receive all messages
				// if destination address is 0 then it is a broadcast message
				if (_address != 0
				    && auxRxBuffer[SIMPLECOMM_SYN_LEN + SIMPLECOMM_LEN_LEN] != 0
				    && auxRxBuffer[SIMPLECOMM_SYN_LEN + SIMPLECOMM_LEN_LEN] != _address) {
					// It is not for me
					*auxRxBufferLen = 0;
					continue;
				}

				uint8_t *ptr = auxRxBuffer + SIMPLECOMM_SYN_LEN + SIMPLECOMM_LEN_LEN;
				packet.setDestination(*ptr++);
				packet.setSource(*ptr++);
				packet.setType(*ptr++);
				if (!packet.setData(ptr, tlen - SIMPLECOMM_HDR_LEN - SIMPLECOMM_CRC_LEN)) {
					// Internal error
					*auxRxBufferLen = 0;
					return false;
				}

				*auxRxBufferLen = 0;
				return true;
			}
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t SimpleCommClass::calcCRC(uint8_t *buffer, size_t len) {
	uint8_t ret = 0;
	while (len--) {
		ret += *buffer++;
	}
	return ret;
}

SimpleCommClass SimpleComm;
