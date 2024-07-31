#ifndef __SimplePacket_H__
#define __SimplePacket_H__

#include "SimplePacketConfig.h"


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

#define SP_SYN_LEN 1
#define SP_LEN_LEN 1
#define SP_DST_LEN 1
#define SP_SRC_LEN 1
#define SP_TYP_LEN 1
#define SP_HDR_LEN (SP_DST_LEN + SP_SRC_LEN + SP_TYP_LEN)
#define SP_MAX_DATA_LEN 128
#define SP_CRC_LEN 1

#define SP_SYN_VALUE 0x02

#define SP_BUFFER_SIZE (SP_SYN_LEN +		\
			SP_LEN_LEN +		\
			SP_HDR_LEN +		\
			SP_MAX_DATA_LEN +	\
			SP_CRC_LEN)

#define SP_BUFF_READ_OFFSET (SP_SYN_LEN + SP_LEN_LEN + SP_HDR_LEN)


class SimplePacket {
public:
	friend class SimpleCommClass;

	explicit SimplePacket();

	// Packet functions
	void clear();
	void setSource(uint8_t source);
	void setDestination(uint8_t destination);
	void setType(uint8_t type);

	uint8_t getSource() const;
	uint8_t getDestination() const;
	uint8_t getType() const;

	// Payload (data) functions
#if !defined(UNIVERSAL_CPP) && !defined(CUSTOM_TYPES)
	bool setData(SP_BOOL data);
#endif
	bool setData(SP_CHAR data);
	bool setData(SP_UCHAR data);
	bool setData(SP_INT data);
	bool setData(SP_UINT data);
	bool setData(SP_LONG data);
	bool setData(SP_ULONG data);
	bool setData(SP_DOUBLE data);
#ifdef SP_STRING_TYPE
	bool setData(const String &data);
#endif
	bool setData(const char *data);
	bool setData(const __FlashStringHelper* data);
	bool setData(const __FlashStringHelper* data, uint8_t expectedLength);
	bool setData(const void *data, uint8_t len);

	#if !defined(UNIVERSAL_CPP) && !defined(CUSTOM_TYPES)
	bool addData(SP_BOOL data);
#endif
	bool addData(SP_CHAR data);
	bool addData(SP_UCHAR data);
	bool addData(SP_INT data);
	bool addData(SP_UINT data);
	bool addData(SP_LONG data);
	bool addData(SP_ULONG data);
	bool addData(SP_DOUBLE data);
#ifdef SP_STRING_TYPE
	bool addData(const String &data);
#endif
	bool addData(const char *data);
	bool addData(const __FlashStringHelper* data);
	bool addData(const __FlashStringHelper* data, uint8_t expectedLength);
	bool addData(const void *data, uint8_t len);

	bool getBool() const;
	char getChar() const;
	unsigned char getUChar() const;
	int getInt() const;
	unsigned int getUInt() const;
	long getLong() const;
	unsigned long getULong() const;
	double getDouble() const;
	const char *getString() const;
	const void *getData() const;
	const void *getData(uint8_t &len) const;

	uint8_t getDataLength() const;

private:
        struct {
		uint8_t syn;
		uint8_t expectedLen;
		uint8_t source;
		uint8_t destination;
		uint8_t type;
		uint8_t data[SP_MAX_DATA_LEN + SP_CRC_LEN];
	} _buff;
	uint8_t _dataLen;
	uint8_t _exhausted;
};

#endif // __SimplePacket_H__
