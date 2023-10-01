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
#include "Core/Containers/StaticVector.h"
#include "Core/Serialization/Serializer.h"
#include "Xml/XmlPullParser.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_XML_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::xml
{

/*! XML based de-serializer.
 * \ingroup XML
 */
class T_DLLCLASS XmlDeserializer : public Serializer
{
	T_RTTI_CLASS;

public:
	explicit XmlDeserializer(IStream* stream, const std::wstring& name = L"");

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
	XmlPullParser m_xpp;

	struct Entry
	{
		std::wstring name;
		int32_t index = 0;
		SmallMap< std::wstring, int32_t > dups;
	};

	AlignedVector< Entry > m_stack;
	uint32_t m_stackPointer;
	SmallMap< std::wstring, Ref< ISerializable > > m_refs;
	std::wstring m_value;
	StaticVector< float, 16 > m_values;

	std::wstring stackPath();

	bool enterElement(const std::wstring& name);

	bool leaveElement(const std::wstring& name);

	void rememberObject(ISerializable* object);

	bool nextElementValue(const std::wstring& name, std::wstring& value);
};

}
