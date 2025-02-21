/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Xml/XmlSerializer.h"

#include "Core/Guid.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Path.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Half.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quaternion.h"
#include "Core/Misc/Base64.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/AttributeMultiLine.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"

namespace traktor::xml
{
namespace
{

std::wstring characterEntity(const std::wstring& str)
{
	const struct
	{
		const wchar_t* needle;
		const wchar_t* escape;
	} c_entities[] = {
		{ L"&", L"&amp;" },
		{ L"<", L"&lt;" },
		{ L">", L"&gt;" },
		{ L"\"", L"&quot;" },
		{ L"\'", L"&apos;" }
	};

	std::wstring result = str;
	for (int32_t i = 0; i < sizeof_array(c_entities); ++i)
		result = replaceAll(result, c_entities[i].needle, c_entities[i].escape);

	return result;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.xml.XmlSerializer", XmlSerializer, Serializer)

#define T_CHECK_STATUS \
	if (failed()) \
		return;

XmlSerializer::XmlSerializer(IStream* stream)
	: m_xml(stream, new Utf8Encoding())
{
	T_ASSERT_M(stream->canWrite(), L"Incorrect direction on output stream");
	m_xml << m_indent << L"<?xml version=\"1.0\" encoding=\"utf-8\"?>" << Endl;
	m_indent = L"";
}

Serializer::Direction XmlSerializer::getDirection() const
{
	return ISerializer::Direction::Write;
}

void XmlSerializer::operator>>(const Member< bool >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << (m ? L"true" : L"false") << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< int8_t >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << int32_t(m) << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< uint8_t >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << int32_t(m) << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< int16_t >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << int32_t(m) << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< uint16_t >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << int32_t(m) << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< int32_t >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << m << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< uint32_t >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << m << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< int64_t >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << m << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< uint64_t >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << m << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< float >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << m << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< double >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << m << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< std::string >& m)
{
	T_CHECK_STATUS;
	if (!m->empty())
		m_xml << m_indent << L"<" << m.getName() << L">" << characterEntity(mbstows((const std::string&)m)) << L"</" << m.getName() << L">" << Endl;
	else
		m_xml << m_indent << L"<" << m.getName() << L"/>" << Endl;
}

void XmlSerializer::operator>>(const Member< std::wstring >& m)
{
	T_CHECK_STATUS;
	if (!m->empty())
	{
		const Attribute* attributes = m.getAttributes();
		if (attributes)
			attributes = attributes->find< AttributeMultiLine >();

		if (!attributes)
			m_xml << m_indent << L"<" << m.getName() << L">" << characterEntity(m) << L"</" << m.getName() << L">" << Endl;
		else
			m_xml << m_indent << L"<" << m.getName() << L"><![CDATA[" << m << L"]]></" << m.getName() << L">" << Endl;
	}
	else
		m_xml << m_indent << L"<" << m.getName() << L"/>" << Endl;
}

void XmlSerializer::operator>>(const Member< Guid >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << m->format() << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< Path >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << m->getOriginal() << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< Color4ub >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << int32_t(m->r) << L", " << int32_t(m->g) << L", " << int32_t(m->b) << L", " << int32_t(m->a) << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< Color4f >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << m->getRed() << L", " << m->getGreen() << L", " << m->getBlue() << L", " << m->getAlpha() << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< Scalar >& m)
{
	T_CHECK_STATUS;
	Scalar& v = m;
	m_xml << m_indent << L"<" << m.getName() << L">" << float(v) << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< Vector2 >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << m->x << L", " << m->y << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< Vector4 >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << m->x() << L", " << m->y() << L", " << m->z() << L", " << m->w() << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< Matrix33 >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << Endl;
	for (int r = 0; r < 3; ++r)
	{
		m_xml << m_indent << L"\t";
		for (int c = 0; c < 3; ++c)
		{
			m_xml << m->e[r][c];
			if (r < 2 || c < 2)
				m_xml << L", ";
		}
		m_xml << Endl;
	}
	m_xml << m_indent << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< Matrix44 >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << Endl;
	for (int r = 0; r < 4; ++r)
	{
		m_xml << m_indent << L"\t";
		for (int c = 0; c < 4; ++c)
		{
			m_xml << (*m).get(r, c);
			if (r < 3 || c < 3)
				m_xml << L", ";
		}
		m_xml << Endl;
	}
	m_xml << m_indent << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< Quaternion >& m)
{
	T_CHECK_STATUS;
	m_xml << m_indent << L"<" << m.getName() << L">" << m->e.x() << L", " << m->e.y() << L", " << m->e.z() << L", " << m->e.w() << L"</" << m.getName() << L">" << Endl;
}

void XmlSerializer::operator>>(const Member< ISerializable* >& m)
{
	T_CHECK_STATUS;

	ISerializable* o = *m;
	auto it = m_refs.find(o);
	if (it != m_refs.end())
	{
		m_xml << m_indent << L"<" << m.getName() << L" ref=\"" << it->second << L"\"/>" << Endl;

		enterElement(m.getName());
		leaveElement();
	}
	else if (o)
	{
		m_xml << m_indent << L"<" << m.getName() << L" type=\"" << type_name(o) << L"\"";

		StringOutputStream v2;

		const TypeInfo* ti = &type_of(o);
		const int32_t version = ti->getVersion();

		if (version > 0)
			v2 << version;

		for (ti = ti->getSuper(); ti != 0; ti = ti->getSuper())
		{
			const int32_t typeVersion = ti->getVersion();
			if (typeVersion > 0)
			{
				if (!v2.empty())
					v2 << L",";
				v2 << ti->getName() << L":" << typeVersion;
			}
		}
		if (!v2.empty())
			m_xml << L" version=\"" << v2.str() << L"\"";

		m_xml << L">" << Endl;

		enterElement(m.getName());

		rememberObject(o);
		serialize(o);

		leaveElement();

		m_xml << m_indent << L"</" << m.getName() << L">" << Endl;
	}
	else
	{
		m_xml << m_indent << L"<" << m.getName() << L"/>" << Endl;

		enterElement(m.getName());
		leaveElement();
	}
}

void XmlSerializer::operator>>(const Member< void* >& m)
{
	T_CHECK_STATUS;

	const size_t size = m.getBlobSize();
	uint8_t* blob = reinterpret_cast< uint8_t* >(m.getBlob());

	if (size > 0)
	{
		m_xml << m_indent << L"<" << m.getName() << L">" << Endl;

		enterElement(m.getName());
		m_xml << Base64().encode(blob, (uint32_t)size, true) << Endl;
		leaveElement();

		m_xml << m_indent << L"</" << m.getName() << L">" << Endl;
	}
	else
	{
		m_xml << m_indent << L"<" << m.getName() << L"/>" << Endl;

		enterElement(m.getName());
		leaveElement();
	}
}

void XmlSerializer::operator>>(const MemberArray& m)
{
	T_CHECK_STATUS;

	if (m.size() > 0)
	{
		m_xml << m_indent << L"<" << m.getName() << L">" << Endl;

		enterElement(m.getName());
		m.write(*this, m.size());
		leaveElement();

		m_xml << m_indent << L"</" << m.getName() << L">" << Endl;
	}
	else
	{
		m_xml << m_indent << L"<" << m.getName() << L"/>" << Endl;

		enterElement(m.getName());
		leaveElement();
	}
}

void XmlSerializer::operator>>(const MemberComplex& m)
{
	T_CHECK_STATUS;

	if (m.getCompound() == true)
	{
		m_xml << m_indent << L"<" << m.getName() << L">" << Endl;
		enterElement(m.getName());
	}

	m.serialize(*this);

	if (m.getCompound() == true)
	{
		leaveElement();
		m_xml << m_indent << L"</" << m.getName() << L">" << Endl;
	}
}

void XmlSerializer::operator>>(const MemberEnumBase& m)
{
	T_CHECK_STATUS;
	this->operator>>(*(MemberComplex*)(&m));
}

std::wstring XmlSerializer::stackPath()
{
	StringOutputStream ss;
	for (const auto& entry : m_stack)
	{
		ss << L'/' << entry.name;
		if (entry.index > 0)
			ss << L'[' << entry.index << L']';
	}
	return ss.str();
}

void XmlSerializer::enterElement(const std::wstring& name)
{
	const Entry e = {
		name,
		m_stack.empty() ? 0 : m_stack.back().dups[name]++
	};
	m_stack.push_back(e);
	incrementIndent();
}

void XmlSerializer::leaveElement()
{
	m_stack.pop_back();
	decrementIndent();
}

void XmlSerializer::rememberObject(ISerializable* object)
{
	m_refs[object] = stackPath();
}

void XmlSerializer::incrementIndent()
{
	m_indent += L'\t';
}

void XmlSerializer::decrementIndent()
{
	T_ASSERT(!m_indent.empty());
	m_indent.erase(m_indent.end() - 1);
}

}
