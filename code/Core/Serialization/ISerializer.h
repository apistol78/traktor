/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Core/Serialization/Member.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Color4f;
class Color4ub;
class Guid;
class Matrix33;
class Matrix44;
class MemberArray;
class MemberComplex;
class MemberEnumBase;
class Path;
class Quaternion;
class Scalar;
class Vector2;
class Vector4;

/*! Serializer interface.
 * \ingroup Core
 */
class T_DLLCLASS ISerializer : public Object
{
	T_RTTI_CLASS;

public:
	enum class Direction
	{
		Read,
		Write
	};

	/*! Get, serialized class, data version.
	 *
	 * \return Data version.
	 */
	virtual int32_t getVersion() const = 0;

	virtual int32_t getVersion(const TypeInfo& typeInfo) const = 0;

	virtual Direction getDirection() const = 0;

	virtual bool cloning() const { return false; }

	virtual void failure() = 0;

	virtual void operator >> (const Member< bool >& m) = 0;

	virtual void operator >> (const Member< int8_t >& m) = 0;

	virtual void operator >> (const Member< uint8_t >& m) = 0;

	virtual void operator >> (const Member< int16_t >& m) = 0;

	virtual void operator >> (const Member< uint16_t >& m) = 0;

	virtual void operator >> (const Member< int32_t >& m) = 0;

	virtual void operator >> (const Member< uint32_t >& m) = 0;

	virtual void operator >> (const Member< int64_t >& m) = 0;

	virtual void operator >> (const Member< uint64_t >& m) = 0;

	virtual void operator >> (const Member< float >& m) = 0;

	virtual void operator >> (const Member< double >& m) = 0;

	virtual void operator >> (const Member< std::string >& m) = 0;

	virtual void operator >> (const Member< std::wstring >& m) = 0;

	virtual void operator >> (const Member< Guid >& m) = 0;

	virtual void operator >> (const Member< Path >& m) = 0;

	virtual void operator >> (const Member< Color4ub >& m) = 0;

	virtual void operator >> (const Member< Color4f >& m) = 0;

	virtual void operator >> (const Member< Scalar >& m) = 0;

	virtual void operator >> (const Member< Vector2 >& m) = 0;

	virtual void operator >> (const Member< Vector4 >& m) = 0;

	virtual void operator >> (const Member< Matrix33 >& m) = 0;

	virtual void operator >> (const Member< Matrix44 >& m) = 0;

	virtual void operator >> (const Member< Quaternion >& m) = 0;

	virtual void operator >> (const Member< ISerializable* >& m) = 0;

	virtual void operator >> (const Member< void* >& m) = 0;

	virtual void operator >> (const MemberArray& m) = 0;

	virtual void operator >> (const MemberComplex& m) = 0;

	virtual void operator >> (const MemberEnumBase& m) = 0;

	bool ensure(bool condition)
	{
		if (!condition)
			failure();
		return condition;
	}

	template < typename T >
	int32_t getVersion() const
	{
		return getVersion(type_of< T >());
	}
};

}
