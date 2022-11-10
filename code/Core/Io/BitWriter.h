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

/*! Bit stream writer.
 * \ingroup Core
 */
class T_DLLCLASS BitWriter : public Object
{
	T_RTTI_CLASS;

public:
	BitWriter(IStream* stream);

	virtual ~BitWriter();

	/*! Write single bit to stream. */
	void writeBit(bool bit);

	/*! Write variable bit-length unsigned number to stream. */
	void writeUnsigned(int32_t nbits, uint64_t value);

	/*! Write variable bit-length signed number to stream. */
	void writeSigned(int32_t nbits, int64_t value);

	/*! Write signed 8 bit number. */
	bool writeInt8(int8_t v);

	/*! Write unsigned 8 bit number. */
	bool writeUInt8(uint8_t v);

	/*! Write signed 16 bit number. */
	bool writeInt16(int16_t v);

	/*! Write unsigned 16 bit number. */
	bool writeUInt16(uint16_t v);

	/*! Write signed 32 bit number. */
	bool writeInt32(int32_t v);

	/*! Write unsigned 8 bit number. */
	bool writeUInt32(uint32_t v);

	/*! Write IEEE-32 float number. */
	bool writeFloat(float v);

	/*! Flush bits. */
	void flush();

	/*! Return current bit position. */
	int64_t tell() const;

	/*! Get reference to underlying stream. */
	Ref< IStream > getStream();

private:
	Ref< IStream > m_stream;
	uint8_t m_data;
	int8_t m_cnt;
};

}

