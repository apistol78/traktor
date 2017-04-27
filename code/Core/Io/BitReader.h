/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_BitReader_H
#define traktor_BitReader_H

#include "Core/Ref.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

/*! \brief Bit stream reader.
 * \ingroup Core
 *
 * Used to read individual bits from any stream.
 * It will not keep data (except a single byte)
 * locally as it will try to keep stream in
 * as much sync as possible.
 */
class T_DLLCLASS BitReader : public Object
{
	T_RTTI_CLASS;

public:
	BitReader(IStream* stream);

	/*! \brief Read single bit from stream. */
	bool readBit();

	/*! \brief Read variable bit-length unsigned number from stream. */
	uint32_t readUnsigned(int nbits);

	/*! \brief Read variable bit-length signed number from stream. */
	int32_t readSigned(int nbits);

	/*! \brief Read signed 8 bit number. */
	int8_t readInt8();

	/*! \brief Read unsigned 8 bit number. */
	uint8_t readUInt8();

	/*! \brief Read signed 16 bit number. */
	int16_t readInt16();

	/*! \brief Read unsigned 16 bit number. */
	uint16_t readUInt16();

	/*! \brief Read signed 32 bit number. */
	int32_t readInt32();

	/*! \brief Read unsigned 8 bit number. */
	uint32_t readUInt32();

	/*! \brief Align stream to next byte boundary. */
	void alignByte();

	/*! \brief Return current bit position. */
	uint32_t tell() const;

	/*! \brief Skip given number of bits. */
	void skip(uint32_t nbits);

	/*! \brief Check if end-of-stream has been reached. */
	bool eos() const;

	/*! \brief Get reference to underlying stream. */
	Ref< IStream > getStream();

private:
	Ref< IStream > m_stream;
	uint8_t m_data;
	int8_t m_cnt;
	bool m_eos;
};

}

#endif	// traktor_BitReader_H
