#include <cstring>
#include <sstream>
#include "Core/Io/IStream.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Misc/Base64.h"
#include "Core/Log/Log.h"
#include "Xml/XmlDeserializer.h"

namespace traktor
{
	namespace xml
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

XmlDeserializer::XmlDeserializer(IStream* stream)
:	m_xpp(stream)
{
	T_ASSERT_M (stream->canRead(), L"Incorrect direction on input stream");
	m_values.reserve(16);
}

Serializer::Direction XmlDeserializer::getDirection() const
{
	return Serializer::SdRead;
}

bool XmlDeserializer::operator >> (const Member< bool >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m = bool(m_value == L"true");
	return true;
}

bool XmlDeserializer::operator >> (const Member< int8_t >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m = parseString< int32_t >(m_value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< uint8_t >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m = parseString< uint32_t >(m_value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< int16_t >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m = parseString< int16_t >(m_value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< uint16_t >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m = parseString< uint16_t >(m_value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< int32_t >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m = parseString< int32_t >(m_value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< uint32_t >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m = parseString< uint32_t >(m_value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< int64_t >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m = parseString< int64_t >(m_value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< uint64_t >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m = parseString< uint64_t >(m_value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< float >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m = parseString< float >(m_value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< double >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m = parseString< double >(m_value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< std::string >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m = wstombs(m_value);
	return true;
}

bool XmlDeserializer::operator >> (const Member< std::wstring >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m = m_value;
	return true;
}

bool XmlDeserializer::operator >> (const Member< Guid >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	Guid& guid = m;
	if (!guid.create(m_value))
		return false;

	return true;
}

bool XmlDeserializer::operator >> (const Member< Path >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	Path& path = m;
	path = Path(m_value);

	return true;
}

bool XmlDeserializer::operator >> (const Member< Color4ub >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m_values.resize(0); m_values.reserve(4);
	if (Split< std::wstring, float >::any(m_value, L",", m_values, true, 4) != 4)
		return false;

	m->r = uint8_t(m_values[0]);
	m->g = uint8_t(m_values[1]);
	m->b = uint8_t(m_values[2]);
	m->a = uint8_t(m_values[3]);

	return true;
}

bool XmlDeserializer::operator >> (const Member< Color4f >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m_values.resize(0); m_values.reserve(4);
	if (Split< std::wstring, float >::any(m_value, L",", m_values, true, 4) != 4)
		return false;

	m->set(
		m_values[0],
		m_values[1],
		m_values[2],
		m_values[3]
	);

	return true;
}

bool XmlDeserializer::operator >> (const Member< Scalar >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m = Scalar(parseString< float >(m_value));
	return true;
}

bool XmlDeserializer::operator >> (const Member< Vector2 >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m_values.resize(0); m_values.reserve(2);
	if (Split< std::wstring, float >::any(m_value, L",", m_values, true, 2) != 2)
		return false;

	m->x = m_values[0];
	m->y = m_values[1];

	return true;
}

bool XmlDeserializer::operator >> (const Member< Vector4 >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m_values.resize(0); m_values.reserve(4);
	if (Split< std::wstring, float >::any(m_value, L",", m_values, true, 4) != 4)
		return false;

	m->set(
		m_values[0],
		m_values[1],
		m_values[2],
		m_values[3]
	);

	return true;
}

bool XmlDeserializer::operator >> (const Member< Matrix33 >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m_values.resize(0); m_values.reserve(3 * 3);
	if (Split< std::wstring, float >::any(m_value, L",", m_values, true, 3 * 3) != 3 * 3)
		return false;

	for (int r = 0; r < 3; ++r)
	{
		for (int c = 0; c < 3; ++c)
		{
			m->e[r][c] = m_values[r * 3 + c];
		}
	}

	return true;
}

bool XmlDeserializer::operator >> (const Member< Matrix44 >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m_values.resize(0); m_values.reserve(4 * 4);
	if (Split< std::wstring, float >::any(m_value, L",", m_values, true, 4 * 4) != 4 * 4)
		return false;

	for (int r = 0; r < 4; ++r)
	{
		for (int c = 0; c < 4; ++c)
			(*m).set(r, c, Scalar(m_values[c + r * 4]));
	}

	return true;
}

bool XmlDeserializer::operator >> (const Member< Quaternion >& m)
{
	if (!nextElementValue(m.getName(), m_value))
		return false;

	m_values.resize(0); m_values.reserve(4);
	if (Split< std::wstring, float >::any(m_value, L",", m_values, true, 4) != 4)
		return false;

	m->e.set(
		m_values[0],
		m_values[1],
		m_values[2],
		m_values[3]
	);

	return true;
}

bool XmlDeserializer::operator >> (const Member< ISerializable* >& m)
{
	if (!enterElement(m.getName()))
		return false;

	const XmlPullParser::Attributes& attr = m_xpp.getEvent().attr;
	XmlPullParser::Attributes::const_iterator a;

	if ((a = findAttribute(attr, L"ref")) != attr.end())
	{
		std::map< std::wstring, Ref< ISerializable > >::iterator i = m_refs.find(a->second);
		if (i == m_refs.end())
		{
			log::error << L"No such reference, \"" << a->second << L"\"" << Endl;
			return 0;
		}
		m = m_refs[a->second];
	}
	else if ((a = findAttribute(attr, L"type")) != attr.end())
	{
		std::wstring typeName = a->second;

		const TypeInfo* type = TypeInfo::find(typeName);
		if (!type)
		{
			log::error << L"No such type, \"" << typeName << L"\"" << Endl;
			return false;
		}

		Ref< ISerializable > o = checked_type_cast< ISerializable* >(type->createInstance());
		if (!o)
		{
			log::error << L"Unable to instantiate type \"" << typeName << L"\"" << Endl;
			return false;
		}

		int32_t version = 0;
		if ((a = findAttribute(attr, L"version")) != attr.end())
			version = parseString< int >(a->second);

		int32_t typeVersion = type->getVersion();
		T_ASSERT_M (version <= typeVersion, L"Serialized data has a higher version number than supported by the code");

		rememberObject(o);
		if (serialize(o, version))
			m = o;
		else
		{
			log::error << L"Unable to serialize object of type \"" << typeName << L"\"" << Endl;
			return false;
		}
	}
	else
	{
		rememberObject(0);
		m = 0;
	}

	if (!leaveElement(m.getName()))
		return false;

	return true;
}

bool XmlDeserializer::operator >> (const Member< void* >& m)
{
	if (!enterElement(m.getName()))
		return false;

	if (m_xpp.next() != XmlPullParser::EtText)
		return false;

	std::vector< uint8_t > data = Base64().decode(m_xpp.getEvent().value);

	if (!data.empty())
	{
		m.setBlobSize(uint32_t(data.size()));
		memcpy(m.getBlob(), &data[0], data.size());
	}
	else
		m.setBlobSize(0);

	if (!leaveElement(m.getName()))
		return false;

	return true;
}

bool XmlDeserializer::operator >> (const MemberArray& m)
{
	if (!enterElement(m.getName()))
		return false;

	m.reserve(0, 0);

	for (size_t i = 0;; ++i)
	{
		while (m_xpp.next() != XmlPullParser::EtEndDocument)
		{
			if (m_xpp.getEvent().type == XmlPullParser::EtStartElement)
				break;
			if (
				m_xpp.getEvent().type == XmlPullParser::EtEndElement &&
				m_xpp.getEvent().value == m.getName()
			)
				break;
		}

		if (m_xpp.getEvent().type != XmlPullParser::EtStartElement)
			break;

		m_xpp.push();

		if (!m.read(*this))
			return false;
	}

	m_stack.pop_back();

	return true;
}

bool XmlDeserializer::operator >> (const MemberComplex& m)
{
	if (m.getCompound() == true)
	{
		if (!enterElement(m.getName()))
			return false;
	}

	if (!m.serialize(*this))
		return false;

	if (m.getCompound() == true)
	{
		if (!leaveElement(m.getName()))
			return false;
	}

	return true;
}

bool XmlDeserializer::operator >> (const MemberEnumBase& m)
{
	return this->operator >> (*(MemberComplex*)(&m));
}

std::wstring XmlDeserializer::stackPath()
{
	std::wstringstream ss;
	for (std::list< Entry >::const_iterator i = m_stack.begin(); i != m_stack.end(); ++i)
	{
		ss << L'/' << i->name;
		if (i->index > 0)
			ss << L'[' << i->index << L']';
	}
	return ss.str();
}

bool XmlDeserializer::enterElement(const std::wstring& name)
{
	Entry e =
	{
		name,
		m_stack.empty() ? 0 : m_stack.back().dups[name]++
	};
	m_stack.push_back(e);

	XmlPullParser::EventType eventType;
	while ((eventType = m_xpp.next()) != XmlPullParser::EtEndDocument)
	{
		if (
			eventType == XmlPullParser::EtStartElement &&
			m_xpp.getEvent().value == name
		)
			return true;
		else if (eventType == XmlPullParser::EtInvalid)
		{
			log::error << L"Invalid response from parser when entering element \"" << name << L"\"" << Endl;
			return false;
		}
	}

    log::error << L"No matching element \"" << name << L"\" until end of document" << Endl;
	return false;
}

bool XmlDeserializer::leaveElement(const std::wstring& name)
{
	T_ASSERT (m_stack.back().name == name);
	m_stack.pop_back();

	while (m_xpp.next() != XmlPullParser::EtEndDocument)
	{
		if (
			m_xpp.getEvent().type == XmlPullParser::EtEndElement &&
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
	if (m_xpp.getEvent().type == XmlPullParser::EtText)
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
}
