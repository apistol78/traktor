#include <cstring>
#include "Core/Class/Any.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Singleton/SingletonManager.h"

namespace traktor
{
	namespace
	{

char* refStringCreate(const char* s)
{
	size_t len = strlen(s);
	
	void* ptr = getAllocator()->alloc(sizeof(uint16_t) + (len + 1) * sizeof(char), 4, T_FILE_LINE);
	if (!ptr)
		return 0;

	uint16_t* base = static_cast< uint16_t* >(ptr);
	*base = 1;

	char* c = reinterpret_cast< char* >(base + 1);
	if (len > 0)
		std::memcpy(c, s, len * sizeof(char));

	c[len] = L'\0';
	return c;
}

char* refStringInc(char* s)
{
	uint16_t* base = reinterpret_cast< uint16_t* >(s) - 1;
	(*base)++;
	return s;
}

char* refStringDec(char* s)
{
	uint16_t* base = reinterpret_cast< uint16_t* >(s) - 1;
	if (--*base == 0)
	{
		getAllocator()->free(base);
		return 0;
	}
	return s;
}

	}

Any::Any()
:	m_type(AtVoid)
{
}

Any::Any(const Any& src)
:	m_type(src.m_type)
{
	if (m_type == AtString)
		m_data.m_string = refStringInc(src.m_data.m_string);
	else if (m_type == AtObject)
	{
		T_SAFE_ADDREF(src.m_data.m_object);
		m_data.m_object = src.m_data.m_object;
	}
	else
		m_data = src.m_data;
}

#if defined(T_CXX11)
Any::Any(Any&& src)
:	m_type(src.m_type)
,	m_data(src.m_data)
{
	src.m_type = AtVoid;
}
#endif

Any::~Any()
{
	T_EXCEPTION_GUARD_BEGIN
	
	if (m_type == AtString)
		refStringDec(m_data.m_string);
	else if (m_type == AtObject)
		T_SAFE_RELEASE(m_data.m_object);

	T_EXCEPTION_GUARD_END
}

Any Any::fromBoolean(bool value)
{
	Any any;
	any.m_type = AtBoolean;
	any.m_data.m_boolean = value;
	return any;
}

Any Any::fromInt32(int32_t value)
{
	Any any;
	any.m_type = AtInt32;
	any.m_data.m_int32 = value;
	return any;
}

Any Any::fromInt64(int64_t value)
{
	Any any;
	any.m_type = AtInt64;
	any.m_data.m_int64 = value;
	return any;
}

Any Any::fromFloat(float value)
{
	Any any;
	any.m_type = AtFloat;
	any.m_data.m_float = value;
	return any;
}

Any Any::fromString(const char* value)
{
	Any any;
	any.m_type = AtString;
	any.m_data.m_string = refStringCreate(value);
	return any;
}

Any Any::fromString(const std::string& value)
{
	Any any;
	any.m_type = AtString;
	any.m_data.m_string = refStringCreate(value.c_str());
	return any;
}

Any Any::fromString(const wchar_t* value)
{
	Any any;
	any.m_type = AtString;
	any.m_data.m_string = refStringCreate(wstombs(Utf8Encoding(), value).c_str());
	return any;
}

Any Any::fromString(const std::wstring& value)
{
	Any any;
	any.m_type = AtString;
	any.m_data.m_string = refStringCreate(wstombs(Utf8Encoding(), value).c_str());
	return any;
}

Any Any::fromObject(ITypedObject* value)
{
	T_SAFE_ANONYMOUS_ADDREF(value);
	Any any;
	any.m_type = AtObject;
	any.m_data.m_object = value;
	return any;
}

bool Any::getBoolean() const
{
	switch (m_type)
	{
	case AtBoolean:
		return m_data.m_boolean;
	case AtInt32:
		return m_data.m_int32 != 0;
	case AtInt64:
		return m_data.m_int64 != 0;
	case AtFloat:
		return m_data.m_float != 0.0f;
	case AtString:
		return parseString< int32_t >(m_data.m_string) != 0;
	case AtObject:
		return m_data.m_object != 0;
	default:
		break;
	}
	return false;
}

int32_t Any::getInt32() const
{
	switch (m_type)
	{
	case AtBoolean:
		return m_data.m_boolean ? 1 : 0;
	case AtInt32:
		return m_data.m_int32;
	case AtInt64:
		return int32_t(m_data.m_int64);
	case AtFloat:
		return int32_t(m_data.m_float);
	case AtString:
		return parseString< int32_t >(m_data.m_string);
	default:
		break;
	}
	return 0;
}

int64_t Any::getInt64() const
{
	switch (m_type)
	{
	case AtBoolean:
		return m_data.m_boolean ? 1 : 0;
	case AtInt32:
		return m_data.m_int32;
	case AtInt64:
		return m_data.m_int64;
	case AtFloat:
		return int64_t(m_data.m_float);
	case AtString:
		return parseString< int64_t >(m_data.m_string);
	default:
		break;
	}
	return 0;
}

float Any::getFloat() const
{
	switch (m_type)
	{
	case AtBoolean:
		return m_data.m_boolean ? 1.0f : 0.0f;
	case AtInt32:
		return float(m_data.m_int32);
	case AtInt64:
		return float(m_data.m_int64);
	case AtFloat:
		return m_data.m_float;
	case AtString:
		return parseString< float >(m_data.m_string);
	default:
		break;
	}
	return 0.0f;
}

std::string Any::getString() const
{
	switch (m_type)
	{
	case AtBoolean:
		return m_data.m_boolean ? "true" : "false";
	case AtInt32:
		return wstombs(Utf8Encoding(), toString(m_data.m_int32));
	case AtInt64:
		return wstombs(Utf8Encoding(), toString(m_data.m_int64));
	case AtFloat:
		return wstombs(Utf8Encoding(), toString(m_data.m_float));
	case AtString:
		return m_data.m_string;
	default:
		break;
	}
	return "";
}

std::wstring Any::getWideString() const
{
	switch (m_type)
	{
	case AtBoolean:
		return m_data.m_boolean ? L"true" : L"false";
	case AtInt32:
		return toString(m_data.m_int32);
	case AtInt64:
		return toString(m_data.m_int64);
	case AtFloat:
		return toString(m_data.m_float);
	case AtString:
		return mbstows(Utf8Encoding(), m_data.m_string);
	default:
		break;
	}
	return L"";
}

Any& Any::operator = (const Any& src)
{
	if (m_type == AtString)
		refStringDec(m_data.m_string);
	else if (m_type == AtObject)
		T_SAFE_RELEASE(m_data.m_object);

	m_type = src.m_type;
	m_data = src.m_data;

	if (m_type == AtString)
		refStringInc(m_data.m_string);
	else if (m_type == AtObject)
		T_SAFE_ADDREF(m_data.m_object);

	return *this;
}

#if defined(T_CXX11)
Any& Any::operator = (Any&& src)
{
	if (m_type == AtString)
	{
		if (src.m_type != AtString || m_data.m_string != src.m_data.m_string)
			refStringDec(m_data.m_string);
	}
	else if (m_type == AtObject)
	{
		if (src.m_type != AtObject || m_data.m_object != src.m_data.m_object)
			T_SAFE_RELEASE(m_data.m_object);
	}

	m_type = src.m_type;
	m_data = src.m_data;

	src.m_type = AtVoid;
	return *this;
}
#endif

}
