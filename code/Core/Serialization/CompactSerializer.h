/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef traktor_CompactSerializer_H
#define traktor_CompactSerializer_H

#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Core/Serialization/Serializer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class T_DLLCLASS CompactSerializer : public Serializer
{
	T_RTTI_CLASS;

public:
	CompactSerializer(IStream* stream, const TypeInfo** types, uint32_t ntypes);

	void flush();

	virtual Direction getDirection() const override final;

	virtual void operator >> (const Member< bool >& m) override final;

	virtual void operator >> (const Member< int8_t >& m) override final;

	virtual void operator >> (const Member< uint8_t >& m) override final;

	virtual void operator >> (const Member< int16_t >& m) override final;

	virtual void operator >> (const Member< uint16_t >& m) override final;

	virtual void operator >> (const Member< int32_t >& m) override final;

	virtual void operator >> (const Member< uint32_t >& m) override final;

	virtual void operator >> (const Member< int64_t >& m) override final;

	virtual void operator >> (const Member< uint64_t >& m) override final;

	virtual void operator >> (const Member< float >& m) override final;

	virtual void operator >> (const Member< double >& m) override final;

	virtual void operator >> (const Member< std::string >& m) override final;

	virtual void operator >> (const Member< std::wstring >& m) override final;

	virtual void operator >> (const Member< Guid >& m) override final;

	virtual void operator >> (const Member< Path >& m) override final;

	virtual void operator >> (const Member< Color4ub >& m) override final;

	virtual void operator >> (const Member< Color4f >& m) override final;

	virtual void operator >> (const Member< Scalar >& m) override final;

	virtual void operator >> (const Member< Vector2 >& m) override final;

	virtual void operator >> (const Member< Vector4 >& m) override final;

	virtual void operator >> (const Member< Matrix33 >& m) override final;

	virtual void operator >> (const Member< Matrix44 >& m) override final;

	virtual void operator >> (const Member< Quaternion >& m) override final;

	virtual void operator >> (const Member< ISerializable* >& m) override final;

	virtual void operator >> (const Member< void* >& m) override final;

	virtual void operator >> (const MemberArray& m) override final;

	virtual void operator >> (const MemberComplex& m) override final;

	virtual void operator >> (const MemberEnumBase& m) override final;

private:
	const TypeInfo** m_types;
	uint32_t m_ntypes;
	Direction m_direction;
	BitReader m_reader;
	BitWriter m_writer;
};

}

#endif	// traktor_net_CompactSerializer_H
