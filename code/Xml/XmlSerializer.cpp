#include <sstream>
#include "Xml/XmlSerializer.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Misc/TString.h"
#include "Core/Misc/String.h"
#include "Core/Misc/Base64.h"

namespace traktor
{
	namespace xml
	{
		namespace
		{

std::wstring characterEntity(const std::wstring& str)
{
	struct { const wchar_t* needle; const wchar_t* escape; } c_entities[] =
	{
		{ L"&", L"&amp;" },
		{ L"<", L"&lt;" },
		{ L">", L"&gt;" },
		{ L"\"", L"&quot;" },
		{ L"\'", L"&apos;" }
	};

	std::wstring result = str;
	for (int i = 0; i < sizeof_array(c_entities); ++i)
		result = replaceAll< std::wstring >(result, c_entities[i].needle, c_entities[i].escape);

	return result;
}

		}
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.xml.XmlSerializer", XmlSerializer, Serializer)

XmlSerializer::XmlSerializer(IStream* stream)
:	m_xml(stream, new Utf8Encoding())
{
	T_ASSERT_M (stream->canWrite(), L"Incorrect direction on output stream");
	m_xml << m_indent << L"<?xml version=\"1.0\" encoding=\"utf-8\"?>" << Endl;
	m_indent = L"";
}

Serializer::Direction XmlSerializer::getDirection()
{
	return ISerializer::SdWrite;
}

bool XmlSerializer::operator >> (const Member< bool >& m)
{
	m_xml << m_indent << L"<" << m.getName() << L">" << (m ? L"true" : L"false") << L"</" << m.getName() << L">" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< int8_t >& m)
{
	m_xml << m_indent << L"<" << m.getName() << L">" << int32_t(m) << L"</" << m.getName() << L">" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< uint8_t >& m)
{
	m_xml << m_indent << L"<" << m.getName() << L">" << int32_t(m) << L"</" << m.getName() << L">" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< int16_t >& m)
{
	m_xml << m_indent << L"<" << m.getName() << L">" << int32_t(m) << L"</" << m.getName() << L">" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< uint16_t >& m)
{
	m_xml << m_indent << L"<" << m.getName() << L">" << int32_t(m) << L"</" << m.getName() << L">" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< int32_t >& m)
{
	m_xml << m_indent << L"<" << m.getName() << L">" << m << L"</" << m.getName() << L">" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< uint32_t >& m)
{
	m_xml << m_indent << L"<" << m.getName() << L">" << m << L"</" << m.getName() << L">" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< int64_t >& m)
{
	m_xml << m_indent << L"<" << m.getName() << L">" << m << L"</" << m.getName() << L">" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< uint64_t >& m)
{
	m_xml << m_indent << L"<" << m.getName() << L">" << m << L"</" << m.getName() << L">" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< float >& m)
{
	m_xml << m_indent << L"<" << m.getName() << L">" << m << L"</" << m.getName() << L">" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< double >& m)
{
	m_xml << m_indent << L"<" << m.getName() << L">" << m << L"</" << m.getName() << L">" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< std::string >& m)
{
	if (!m->empty())
		m_xml << m_indent << L"<" << m.getName() << L">" << characterEntity(mbstows(m)) << L"</" << m.getName() << L">" << Endl;
	else
		m_xml << m_indent << L"<" << m.getName() << L"/>" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< std::wstring >& m)
{
	if (!m->empty())
	{
		if (!m.isMultiLine())
			m_xml << m_indent << L"<" << m.getName() << L">" << characterEntity(m) << L"</" << m.getName() << L">" << Endl;
		else
		{
			m_xml << m_indent << L"<" << m.getName() << L">" << Endl;
			m_xml << m_indent << L"<![CDATA[" << Endl;
			m_xml << m << Endl;
			m_xml << m_indent << L"]]>" << Endl;
			m_xml << m_indent << L"</" << m.getName() << L">" << Endl;
		}
	}
	else
	{
		m_xml << m_indent << L"<" << m.getName() << L"/>" << Endl;
	}
	return true;
}

bool XmlSerializer::operator >> (const Member< Guid >& m)
{
	m_xml << m_indent << L"<" << m.getName() << L">" << m->format() << L"</" << m.getName() << L">" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< Path >& m)
{
	m_xml << m_indent << L"<" << m.getName() << L">" << m->getOriginal() << L"</" << m.getName() << L">" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< Color >& m)
{
	m_xml << m_indent << L"<" << m.getName() << L">" << int32_t(m->r) << L", " << int32_t(m->g) << L", " << int32_t(m->b) << L", " << int32_t(m->a) << L"</" << m.getName() << L">" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< Scalar >& m)
{
	Scalar& v = m;
	m_xml << m_indent << L"<" << m.getName() << L">" << float(v) << L"</" << m.getName() << L">" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< Vector2 >& m)
{
	m_xml << m_indent << L"<" << m.getName() << L">" << m->x << L", " << m->y << L"</" << m.getName() << L">" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< Vector4 >& m)
{
	m_xml << m_indent << L"<" << m.getName() << L">" << m->x() << L", " << m->y() << L", " << m->z() << L", " << m->w() << L"</" << m.getName() << L">" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< Matrix33 >& m)
{
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
	return true;
}

bool XmlSerializer::operator >> (const Member< Matrix44 >& m)
{
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
	return true;
}

bool XmlSerializer::operator >> (const Member< Quaternion >& m)
{
	m_xml << m_indent << L"<" << m.getName() << L">" << m->e.x() << L", " << m->e.y() << L", " << m->e.z() << L", " << m->e.w() << L"</" << m.getName() << L">" << Endl;
	return true;
}

bool XmlSerializer::operator >> (const Member< ISerializable >& m)
{
	ISerializable* o = &static_cast< ISerializable& >(m);
	bool result;

	int32_t version = type_of(o).getVersion();
	if (version > 0)
		m_xml << m_indent << L"<" << m.getName() << L" version=\"" << version << L"\">" << Endl;
	else
		m_xml << m_indent << L"<" << m.getName() << L">" << Endl;

	enterElement(m.getName());

	rememberObject(o);
	result = serialize(o, version, 0);

	leaveElement();
	
	m_xml << m_indent << L"</" << m.getName() << L">" << Endl;

	return result;
}

bool XmlSerializer::operator >> (const Member< ISerializable* >& m)
{
	ISerializable* o = m;
	bool result = true;

	std::map< ISerializable*, std::wstring >::iterator i = m_refs.find(o);
	if (i != m_refs.end())
	{
		m_xml << m_indent << L"<" << m.getName() << L" ref=\"" << i->second << L"\"/>" << Endl;

		enterElement(m.getName());
		leaveElement();
	}
	else if (o)
	{
		int32_t version = type_of(o).getVersion();
		if (version > 0)
			m_xml << m_indent << L"<" << m.getName() << L" type=\"" << type_name(o) << L"\" version=\"" << version << L"\">" << Endl;
		else
			m_xml << m_indent << L"<" << m.getName() << L" type=\"" << type_name(o) << L"\">" << Endl;

		enterElement(m.getName());

		rememberObject(o);
		result = serialize(o, version, 0);

		leaveElement();

		m_xml << m_indent << L"</" << m.getName() << L">" << Endl;
	}
	else
	{
		m_xml << m_indent << L"<" << m.getName() << L"/>" << Endl;

		enterElement(m.getName());
		leaveElement();
	}
	
	return result;
}

bool XmlSerializer::operator >> (const Member< void* >& m)
{
	uint32_t size = m.getBlobSize();
	uint8_t* blob = reinterpret_cast< uint8_t* >(m.getBlob());

	if (size > 0)
	{
		m_xml << m_indent << L"<" << m.getName() << L">" << Endl;
		
		enterElement(m.getName());
		m_xml << Base64().encode(blob, size, true) << Endl;
		leaveElement();

		m_xml << m_indent << L"</" << m.getName() << L">" << Endl;
	}
	else
	{
		m_xml << m_indent << L"<" << m.getName() << L"/>" << Endl;

		enterElement(m.getName());
		leaveElement();
	}

	return true;
}

bool XmlSerializer::operator >> (const MemberArray& m)
{
	if (m.size() > 0)
	{
		m_xml << m_indent << L"<" << m.getName() << L">" << Endl;
		
		enterElement(m.getName());

		for (size_t i = 0, size = m.size(); i < size; ++i)
		{
			if (!m.serialize(*this, i))
				return false;
		}

		leaveElement();

		m_xml << m_indent << L"</" << m.getName() << L">" << Endl;
	}
	else
	{
		m_xml << m_indent << L"<" << m.getName() << L"/>" << Endl;

		enterElement(m.getName());
		leaveElement();
	}

	return true;
}

bool XmlSerializer::operator >> (const MemberComplex& m)
{
	if (m.getCompound() == true)
	{
		m_xml << m_indent << L"<" << m.getName() << L">" << Endl;
		enterElement(m.getName());
	}

	bool result = m.serialize(*this);

	if (m.getCompound() == true)
	{
		leaveElement();
		m_xml << m_indent << L"</" << m.getName() << L">" << Endl;
	}

	return result;
}

bool XmlSerializer::operator >> (const MemberEnumBase& m)
{
	return this->operator >> (*(MemberComplex*)(&m));
}

std::wstring XmlSerializer::stackPath()
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

void XmlSerializer::enterElement(const std::wstring& name)
{
	Entry e =
	{
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
	T_ASSERT (!m_indent.empty());
	m_indent.erase(m_indent.end() - 1);
}

	}
}
