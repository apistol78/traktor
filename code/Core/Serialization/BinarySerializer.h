/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Ref.h"
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

class IStream;

/*! Binary serializer.
 * \ingroup Core
 */
class T_DLLCLASS BinarySerializer : public Serializer
{
	T_RTTI_CLASS;

public:
	explicit BinarySerializer(IStream* stream);

	virtual Direction getDirection() const override final;

	virtual void operator>>(const Member< bool >& m) override final;

	virtual void operator>>(const Member< int8_t >& m) override final;

	virtual void operator>>(const Member< uint8_t >& m) override final;

	virtual void operator>>(const Member< int16_t >& m) override final;

	virtual void operator>>(const Member< uint16_t >& m) override final;

	virtual void operator>>(const Member< int32_t >& m) override final;

	virtual void operator>>(const Member< uint32_t >& m) override final;

	virtual void operator>>(const Member< int64_t >& m) override final;

	virtual void operator>>(const Member< uint64_t >& m) override final;

	virtual void operator>>(const Member< float >& m) override final;

	virtual void operator>>(const Member< double >& m) override final;

	virtual void operator>>(const Member< std::string >& m) override final;

	virtual void operator>>(const Member< std::wstring >& m) override final;

	virtual void operator>>(const Member< Guid >& m) override final;

	virtual void operator>>(const Member< Path >& m) override final;

	virtual void operator>>(const Member< Color4ub >& m) override final;

	virtual void operator>>(const Member< Color4f >& m) override final;

	virtual void operator>>(const Member< Scalar >& m) override final;

	virtual void operator>>(const Member< Vector2 >& m) override final;

	virtual void operator>>(const Member< Vector4 >& m) override final;

	virtual void operator>>(const Member< Matrix33 >& m) override final;

	virtual void operator>>(const Member< Matrix44 >& m) override final;

	virtual void operator>>(const Member< Quaternion >& m) override final;

	virtual void operator>>(const Member< ISerializable* >& m) override final;

	virtual void operator>>(const Member< void* >& m) override final;

	virtual void operator>>(const MemberArray& m) override final;

	virtual void operator>>(const MemberComplex& m) override final;

	virtual void operator>>(const MemberEnumBase& m) override final;

private:
	Ref< IStream > m_stream;
	Direction m_direction;
	SmallMap< uint64_t, Ref< ISerializable > > m_readCache;
	SmallMap< ISerializable*, uint64_t > m_writeCache;
	uint64_t m_nextCacheId;
	AlignedVector< const TypeInfo* > m_typeReadCache;
	SmallMap< const TypeInfo*, uint32_t > m_typeWriteCache;
	uint32_t m_nextTypeCacheId;
};

}
