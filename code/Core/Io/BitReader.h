/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

/*! Bit stream reader.
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

	/*! Read single bit from stream. */
	bool readBit();

	/*! Read variable bit-length unsigned number from stream. */
	uint32_t readUnsigned(int nbits);

	/*! Read variable bit-length signed number from stream. */
	int32_t readSigned(int nbits);

	/*! Read signed 8 bit number. */
	int8_t readInt8();

	/*! Read unsigned 8 bit number. */
	uint8_t readUInt8();

	/*! Read signed 16 bit number. */
	int16_t readInt16();

	/*! Read unsigned 16 bit number. */
	uint16_t readUInt16();

	/*! Read signed 32 bit number. */
	int32_t readInt32();

	/*! Read unsigned 8 bit number. */
	uint32_t readUInt32();

	/*! Align stream to next byte boundary. */
	void alignByte();

	/*! Return current bit position. */
	uint32_t tell() const;

	/*! Skip given number of bits. */
	void skip(uint32_t nbits);

	/*! Check if end-of-stream has been reached. */
	bool eos() const;

	/*! Get reference to underlying stream. */
	Ref< IStream > getStream();

private:
	Ref< IStream > m_stream;
	uint8_t m_data;
	int8_t m_cnt;
	bool m_eos;
};

}

