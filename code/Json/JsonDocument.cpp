#include <rapidjson/reader.h>
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Json/JsonArray.h"
#include "Json/JsonDocument.h"
#include "Json/JsonMember.h"
#include "Json/JsonObject.h"

namespace traktor
{
	namespace json
	{
		namespace
		{

template < typename Encoding >
class JsonReaderHandler
{
public:
	typedef typename Encoding::Ch Ch;

	JsonReaderHandler(JsonDocument* document)
	:	m_document(document)
	{
	}

	bool Null()
	{
		setValue(Any());
		return true;
	}

	bool Bool(bool b)
	{
		setValue(Any::fromBoolean(b));
		return true;
	}

	bool Int(int32_t i)
	{
		setValue(Any::fromInt32(i));
		return true;
	}

	bool Uint(uint32_t u)
	{
		setValue(Any::fromInt32(u));
		return true;
	}

	bool Int64(int64_t i)
	{
		setValue(Any::fromInt64(i));
		return true;
	}

	bool Uint64(uint64_t i)
	{
		setValue(Any::fromInt64(int64_t(i)));
		return true;
	}

	bool Double(double d)
	{
		setValue(Any::fromFloat(d));
		return true;
	}

	bool String(const Ch* str, size_t length, bool copy)
	{
		setValue(Any::fromString((const wchar_t*)str));
		return true;
	}

	bool StartObject()
	{
		Ref< JsonObject > object = new JsonObject();

		if (!m_scope.empty())
		{
			if (m_object)
				m_object->push(new JsonMember(m_member, Any::fromObject(object)));
			if (m_array)
				m_array->push(Any::fromObject(object));
		}
		else
			m_document->push(Any::fromObject(object));

		m_scope.push_back(object);

		m_array = 0;
		m_object = object;

		return true;
	}

	bool Key(const Ch* str, size_t length, bool copy)
	{
		m_member = (const wchar_t*)str;
		return true;
	}

	bool EndObject(size_t memberCount)
	{
		m_scope.pop_back();

		if (!m_scope.empty())
		{
			m_object = dynamic_type_cast< JsonObject* >(m_scope.back());
			m_array = dynamic_type_cast< JsonArray* >(m_scope.back());
		}
		else
		{
			m_object = 0;
			m_array = 0;
		}

		return true;
	}

	bool StartArray()
	{
		Ref< JsonArray > array = new JsonArray();

		if (!m_scope.empty())
		{
			if (m_object)
				m_object->push(new JsonMember(m_member, Any::fromObject(array)));
			if (m_array)
				m_array->push(Any::fromObject(array));
		}
		else
			m_document->push(Any::fromObject(array));

		m_scope.push_back(array);

		m_array = array;
		m_object = 0;

		return true;
	}

	bool EndArray(size_t elementCount)
	{
		m_scope.pop_back();

		if (!m_scope.empty())
		{
			m_object = dynamic_type_cast< JsonObject* >(m_scope.back());
			m_array = dynamic_type_cast< JsonArray* >(m_scope.back());
		}
		else
		{
			m_object = 0;
			m_array = 0;
		}

		return true;
	}

private:
	Ref< JsonDocument > m_document;
	RefArray< JsonNode > m_scope;
	Ref< JsonObject > m_object;
	Ref< JsonArray > m_array;
	std::wstring m_member;

	void setValue(const Any& value)
	{
		if (!m_scope.empty())
		{
			if (m_object)
				m_object->push(new JsonMember(m_member, value));
			if (m_array)
				m_array->push(value);
		}
		else
			m_document->push(value);
	}
};

template < typename Encoding >
class JsonStreamStream
{
public:
	typedef typename Encoding::Ch Ch;

	JsonStreamStream(IStream* stream)
	:	m_stream(stream)
	,	m_origin(stream->tell())
	{
		if (m_stream->read(&m_peek, sizeof(m_peek)) <= 0)
			m_peek = 0;
	}

    Ch Peek() const
	{
		return m_peek;
	}
    
	Ch Take()
	{
		Ch ch = m_peek;
		if (m_stream->read(&m_peek, sizeof(m_peek)) <= 0)
			m_peek = 0;
		return ch;
	}

    size_t Tell() const
	{
		return m_stream->tell() - m_origin;
	}

	Ch* PutBegin()
	{
		return 0;
	}

	void Put(Ch c)
	{
	}

	void Flush()
	{
	}

	size_t PutEnd(Ch* begin)
	{
		return 0;
	}

private:
	IStream* m_stream;
	int32_t m_origin;
	Ch m_peek;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.json.JsonDocument", JsonDocument, JsonArray)

bool JsonDocument::loadFromFile(const Path& fileName)
{
	Ref< IStream > f = FileSystem::getInstance().open(fileName, File::FmRead);
	return f ? loadFromStream(f) : false;
}
	
bool JsonDocument::loadFromStream(IStream* stream)
{
	JsonStreamStream< rapidjson::UTF8< > > ss(stream);
	if (sizeof(wchar_t) == 4)
	{
		JsonReaderHandler< rapidjson::UTF32< > > handler(this);
		rapidjson::GenericReader< rapidjson::UTF8< >, rapidjson::UTF32< > > r;
		r.Parse(ss, handler);
	}
	else if (sizeof(wchar_t) == 2)
	{
		JsonReaderHandler< rapidjson::UTF16< > > handler(this);
		rapidjson::GenericReader< rapidjson::UTF8< >, rapidjson::UTF16< > > r;
		r.Parse(ss, handler);
	}
	return true;
}
	
bool JsonDocument::loadFromText(const std::wstring& text)
{
	MemoryStream ms(
		(void*)text.c_str(),
		int(text.length() * sizeof(wchar_t)),
		true,
		false
	);
	if (sizeof(wchar_t) == 4)
	{
		JsonStreamStream< rapidjson::UTF32< > > ss(&ms);
		JsonReaderHandler< rapidjson::UTF32< > > handler(this);
		rapidjson::GenericReader< rapidjson::UTF32< >, rapidjson::UTF32< > > r;
		r.Parse(ss, handler);
	}
	else if (sizeof(wchar_t) == 2)
	{
		JsonStreamStream< rapidjson::UTF16< > > ss(&ms);
		JsonReaderHandler< rapidjson::UTF16< > > handler(this);
		rapidjson::GenericReader< rapidjson::UTF16< >, rapidjson::UTF16< > > r;
		r.Parse(ss, handler);
	}
	return true;
}

bool JsonDocument::saveToFile(const Path& fileName)
{
	Ref< IStream > f = FileSystem::getInstance().open(fileName, File::FmWrite);
	return f ? saveToStream(f) : false;
}

bool JsonDocument::saveToStream(IStream* stream)
{
	if (!stream->canWrite())
		return false;

	Utf8Encoding encoding;
	FileOutputStream os(stream, &encoding);

	for (AlignedVector< Any >::const_iterator i = get().begin(); i != get().end(); ++i)
	{
		if (i != get().begin())
			os << L"," << Endl;

		switch (i->getType())
		{
		case Any::AtVoid:
			os << L"nil" << Endl;
			break;

		case Any::AtBoolean:
			os << (i->getBooleanUnsafe() ? L"true" : L"false");
			break;

		case Any::AtInt32:
			os << i->getInt32Unsafe();
			break;

		case Any::AtInt64:
			os << i->getInt64Unsafe();
			break;

		case Any::AtFloat:
			os << i->getFloatUnsafe();
			break;

		case Any::AtString:
			os << L"\"" << i->getWideString() << L"\"";
			break;

		case Any::AtObject:
			{
				if (const JsonNode* node = dynamic_type_cast< const JsonNode* >(i->getObjectUnsafe()))
					node->write(os);
				else
					os << L"nil" << Endl;
			}
			break;
		}
	}
	os << Endl;
	return true;
}

	}
}
