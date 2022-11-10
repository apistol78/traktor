/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <map>
#include "Core/Guid.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Path.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Half.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quaternion.h"
#include "Core/Misc/Base64.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"
#include "Xml/XmlDeserializer.h"

namespace traktor::xml
{
	namespace
	{

inline
XmlPullParser::Attributes::const_iterator findAttribute(const XmlPullParser::Attributes& attr, const std::wstring& name)
{
	for (XmlPullParser::Attributes::const_iterator i = attr.begin(); i != attr.end(); ++i)
	{
		if (i->first == name)
			return i;
	}
	return attr.end();
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.xml.XmlDeserializer", XmlDeserializer, Serializer)

#define T_CHECK_STATUS \
	if (failed()) return;

XmlDeserializer::XmlDeserializer(IStream* stream, const std::wstring& name)
:	m_xpp(stream, name)
,	m_stackPointer(0)
{
	T_ASSERT_M (stream->canRead(), L"Incorrect direction on input stream");
	m_stack.reserve(32);
}

Serializer::Direction XmlDeserializer::getDirection() const
{
	return Serializer::Direction::Read;
}

void XmlDeserializer::operator >> (const Member< bool >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);
	m = bool(m_value == L"true");
}

void XmlDeserializer::operator >> (const Member< int8_t >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);
	m = parseString< int32_t >(m_value);
}

void XmlDeserializer::operator >> (const Member< uint8_t >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);
	m = parseString< uint32_t >(m_value);
}

void XmlDeserializer::operator >> (const Member< int16_t >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);
	m = parseString< int16_t >(m_value);
}

void XmlDeserializer::operator >> (const Member< uint16_t >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);
	m = parseString< uint16_t >(m_value);
}

void XmlDeserializer::operator >> (const Member< int32_t >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);
	m = parseString< int32_t >(m_value);
}

void XmlDeserializer::operator >> (const Member< uint32_t >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);
	m = parseString< uint32_t >(m_value);
}

void XmlDeserializer::operator >> (const Member< int64_t >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);
	m = parseString< int64_t >(m_value);
}

void XmlDeserializer::operator >> (const Member< uint64_t >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);
	m = parseString< uint64_t >(m_value);
}

void XmlDeserializer::operator >> (const Member< float >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);
	m = parseString< float >(m_value);
}

void XmlDeserializer::operator >> (const Member< double >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);
	m = parseString< double >(m_value);
}

void XmlDeserializer::operator >> (const Member< std::string >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);
	m = wstombs(m_value);
}

void XmlDeserializer::operator >> (const Member< std::wstring >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);
	m = m_value;
}

void XmlDeserializer::operator >> (const Member< Guid >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);
	((Guid&)m).create(m_value);
}

void XmlDeserializer::operator >> (const Member< Path >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);
	((Path&)m) = Path(m_value);
}

void XmlDeserializer::operator >> (const Member< Color4ub >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);

	m_values.resize(0);
	Split< std::wstring, float >::any(m_value, L",", m_values, true, 4);

	m->r = uint8_t(m_values[0]);
	m->g = uint8_t(m_values[1]);
	m->b = uint8_t(m_values[2]);
	m->a = uint8_t(m_values[3]);
}

void XmlDeserializer::operator >> (const Member< Color4f >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);

	m_values.resize(0);
	Split< std::wstring, float >::any(m_value, L",", m_values, true, 4);

	m->set(
		m_values[0],
		m_values[1],
		m_values[2],
		m_values[3]
	);
}

void XmlDeserializer::operator >> (const Member< Scalar >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);
	m = Scalar(parseString< float >(m_value));
}

void XmlDeserializer::operator >> (const Member< Vector2 >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);

	m_values.resize(0);
	Split< std::wstring, float >::any(m_value, L",", m_values, true, 2);

	m->x = m_values[0];
	m->y = m_values[1];
}

void XmlDeserializer::operator >> (const Member< Vector4 >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);

	m_values.resize(0);
	Split< std::wstring, float >::any(m_value, L",", m_values, true, 4);

	m->set(
		m_values[0],
		m_values[1],
		m_values[2],
		m_values[3]
	);
}

void XmlDeserializer::operator >> (const Member< Matrix33 >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);

	m_values.resize(0);
	Split< std::wstring, float >::any(m_value, L",", m_values, true, 3 * 3);

	for (int r = 0; r < 3; ++r)
	{
		for (int c = 0; c < 3; ++c)
			m->e[r][c] = m_values[r * 3 + c];
	}
}

void XmlDeserializer::operator >> (const Member< Matrix44 >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);

	m_values.resize(0);
	Split< std::wstring, float >::any(m_value, L",", m_values, true, 4 * 4);

	for (int r = 0; r < 4; ++r)
	{
		for (int c = 0; c < 4; ++c)
			(*m).set(r, c, Scalar(m_values[c + r * 4]));
	}
}

void XmlDeserializer::operator >> (const Member< Quaternion >& m)
{
	T_CHECK_STATUS;
	nextElementValue(m.getName(), m_value);

	m_values.resize(0);
	Split< std::wstring, float >::any(m_value, L",", m_values, true, 4);

	m->e.set(
		m_values[0],
		m_values[1],
		m_values[2],
		m_values[3]
	);
}

void XmlDeserializer::operator >> (const Member< ISerializable* >& m)
{
	T_CHECK_STATUS;

	if (!ensure(enterElement(m.getName())))
		return;

	const XmlPullParser::Attributes& attr = m_xpp.getEvent().attr;
	XmlPullParser::Attributes::const_iterator a;

	if ((a = findAttribute(attr, L"ref")) != attr.end())
	{
		SmallMap< std::wstring, Ref< ISerializable > >::const_iterator i = m_refs.find(a->second);
		if (!ensure(i != m_refs.end()))
			return;

		m = m_refs[a->second];
	}
	else if ((a = findAttribute(attr, L"type")) != attr.end())
	{
		std::wstring typeName = a->second;

		const TypeInfo* type = TypeInfo::find(typeName.c_str());
		if (!ensure(type != 0))
			return;

		Ref< ISerializable > o = checked_type_cast< ISerializable* >(type->createInstance());
		if (!ensure(o != 0))
			return;

		Serializer::dataVersionMap_t dataVersions;

		if ((a = findAttribute(attr, L"version")) != attr.end())
		{
			StringSplit< std::wstring > ss(a->second, L",");
			for (StringSplit< std::wstring >::const_iterator i = ss.begin(); i != ss.end(); ++i)
			{
				const std::wstring& s = *i;
				size_t p = s.find(L':');
				if (p != s.npos)
				{
					std::wstring dataTypeName = s.substr(0, p);
					int32_t dataTypeVersion = parseString< int32_t >(s.substr(p + 1));

					const TypeInfo* dataType = TypeInfo::find(dataTypeName.c_str());
					if (!ensure(dataType != 0))
						return;

					if (!ensure(dataType->getVersion() >= dataTypeVersion))
						return;

					if (dataTypeVersion > 0)
					{
						dataVersions.insert(std::make_pair(
							dataType,
							dataTypeVersion
						));
					}
				}
				else
				{
					int32_t dataTypeVersion = parseString< int32_t >(s);
					if (dataTypeVersion > 0)
					{
						dataVersions.insert(std::make_pair(
							type,
							dataTypeVersion
						));
					}
				}
			}
		}

		rememberObject(o);
		serialize(o, dataVersions);

		m = o;
	}
	else
	{
		rememberObject(0);
		m = 0;
	}

	leaveElement(m.getName());
}

void XmlDeserializer::operator >> (const Member< void* >& m)
{
	T_CHECK_STATUS;

	if (!ensure(enterElement(m.getName())))
		return;

	if (!ensure(m_xpp.next() == XmlPullParser::EventType::Text))
		return;

	AlignedVector< uint8_t > data = Base64().decode(m_xpp.getEvent().value);

	if (!data.empty())
	{
		m.setBlobSize(uint32_t(data.size()));
		memcpy(m.getBlob(), &data[0], data.size());
	}
	else
		m.setBlobSize(0);

	leaveElement(m.getName());
}

void XmlDeserializer::operator >> (const MemberArray& m)
{
	T_CHECK_STATUS;

	if (!ensure(enterElement(m.getName())))
		return;

	m.reserve(0, 0);

	for (size_t i = 0;; ++i)
	{
		T_CHECK_STATUS;

		while (m_xpp.next() != XmlPullParser::EventType::EndDocument)
		{
			if (m_xpp.getEvent().type == XmlPullParser::EventType::StartElement)
				break;
			if (
				m_xpp.getEvent().type == XmlPullParser::EventType::EndElement &&
				m_xpp.getEvent().value == m.getName()
			)
				break;
		}

		if (m_xpp.getEvent().type != XmlPullParser::EventType::StartElement)
			break;

		m_xpp.push();

		m.read(*this);
	}

	m_stack[--m_stackPointer].dups.reset();
}

void XmlDeserializer::operator >> (const MemberComplex& m)
{
	T_CHECK_STATUS;

	if (m.getCompound() == true)
	{
		if (!ensure(enterElement(m.getName())))
			return;
	}

	m.serialize(*this);

	if (m.getCompound() == true)
	{
		if (!ensure(leaveElement(m.getName())))
			return;
	}
}

void XmlDeserializer::operator >> (const MemberEnumBase& m)
{
	T_CHECK_STATUS;
	this->operator >> (*(MemberComplex*)(&m));
}

std::wstring XmlDeserializer::stackPath()
{
	StringOutputStream ss;
	for (uint32_t i = 0; i < m_stackPointer; ++i)
	{
		const Entry& e = m_stack[i];
		ss << L'/' << e.name;
		if (e.index > 0)
			ss << L'[' << e.index << L']';
	}
	return ss.str();
}

bool XmlDeserializer::enterElement(const std::wstring& name)
{
	int32_t index = (m_stackPointer > 0) ? m_stack[m_stackPointer - 1].dups[name]++ : 0;

	if (m_stackPointer >= m_stack.size())
		m_stack.resize(m_stackPointer + 16);

	Entry& e = m_stack[m_stackPointer++];
	e.name = name;
	e.index = index;

	XmlPullParser::EventType eventType;
	while ((eventType = m_xpp.next()) != XmlPullParser::EventType::EndDocument)
	{
		if (
			eventType == XmlPullParser::EventType::StartElement &&
			m_xpp.getEvent().value == name
		)
			return true;
		else if (eventType == XmlPullParser::EventType::Invalid)
		{
			log::error << L"Invalid response from parser when entering element \"" << name << L"\"" << Endl;
			m_stack[--m_stackPointer].dups.reset();
			return false;
		}
	}

    log::error << L"No matching element \"" << name << L"\" until end of document" << Endl;
	m_stack[--m_stackPointer].dups.reset();
	return false;
}

bool XmlDeserializer::leaveElement(const std::wstring& name)
{
	T_ASSERT(m_stackPointer > 0);
	T_ASSERT(m_stack[m_stackPointer - 1].name == name);
	m_stack[--m_stackPointer].dups.reset();

	while (m_xpp.next() != XmlPullParser::EventType::EndDocument)
	{
		if (
			m_xpp.getEvent().type == XmlPullParser::EventType::EndElement &&
			m_xpp.getEvent().value == name
		)
			return true;
	}

	return false;
}

void XmlDeserializer::rememberObject(ISerializable* object)
{
	m_refs[stackPath()] = object;
}

bool XmlDeserializer::nextElementValue(const std::wstring& name, std::wstring& value)
{
	if (!enterElement(name))
		return false;

	m_xpp.next();
	if (m_xpp.getEvent().type == XmlPullParser::EventType::Text)
		value = m_xpp.getEvent().value;
	else
	{
		m_xpp.push();
		value = L"";
	}

	if (!leaveElement(name))
		return false;

	return true;
}

}
