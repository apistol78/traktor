#include <cstring>
#include "Script/Any.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

wchar_t* refStringCreate(const wchar_t* s)
{
	uint32_t len = wcslen(s);
	
	void* ptr = Alloc::acquire(sizeof(uint16_t) + (len + 1) * sizeof(wchar_t));
	if (!ptr)
		return 0;

	uint16_t* base = static_cast< uint16_t* >(ptr);
	*base = 0;

	wchar_t* c = reinterpret_cast< wchar_t* >(base + 1);
	if (len > 0)
		std::memcpy(c, s, len * sizeof(wchar_t));

	c[len] = L'\0';
	return c;
}

wchar_t* refStringInc(wchar_t* s)
{
	uint16_t* base = reinterpret_cast< uint16_t* >(s) - 1; *base++;
	return s;
}

wchar_t* refStringDec(wchar_t* s)
{
	uint16_t* base = reinterpret_cast< uint16_t* >(s) - 1;
	if (--*base == 0)
	{
		Alloc::free(base);
		return 0;
	}
	return s;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.Any", Any, Object)

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
		m_data.m_object = new ref_object_t(*src.m_data.m_object);
	else
		m_data = src.m_data;
}

Any::Any(bool value)
:	m_type(AtBoolean)
{
	m_data.m_boolean = value;
}

Any::Any(int32_t value)
:	m_type(AtInteger)
{
	m_data.m_integer = value;
}

Any::Any(float value)
:	m_type(AtFloat)
{
	m_data.m_float = value;
}

Any::Any(const std::wstring& value)
:	m_type(AtString)
{
	m_data.m_string = refStringCreate(value.c_str());
}

Any::Any(Object* value)
:	m_type(AtObject)
{
	m_data.m_object = new ref_object_t(value);
}

Any::~Any()
{
	T_EXCEPTION_GUARD_BEGIN
	
	if (m_type == AtString)
		refStringDec(m_data.m_string);
	else if (m_type == AtObject)
		delete m_data.m_object;

	T_EXCEPTION_GUARD_END
}

bool Any::getBoolean() const
{
	switch (m_type)
	{
	case AtBoolean:
		return m_data.m_boolean;
	case AtInteger:
		return m_data.m_integer != 0;
	case AtFloat:
		return m_data.m_float != 0.0f;
	case AtString:
		return parseString< int32_t >(m_data.m_string) != 0;
	case AtObject:
		return *m_data.m_object != 0;
	}
	return false;
}

int32_t Any::getInteger() const
{
	switch (m_type)
	{
	case AtBoolean:
		return m_data.m_boolean ? 1 : 0;
	case AtInteger:
		return m_data.m_integer;
	case AtFloat:
		return int32_t(m_data.m_float);
	case AtString:
		return parseString< int32_t >(m_data.m_string);
	}
	return 0;
}

float Any::getFloat() const
{
	switch (m_type)
	{
	case AtBoolean:
		return m_data.m_boolean ? 1.0f : 0.0f;
	case AtInteger:
		return float(m_data.m_integer);
	case AtFloat:
		return m_data.m_float;
	case AtString:
		return parseString< float >(m_data.m_string);
	}
	return 0.0f;
}

std::wstring Any::getString() const
{
	switch (m_type)
	{
	case AtBoolean:
		return m_data.m_boolean ? L"true" : L"false";
	case AtInteger:
		return toString(m_data.m_integer);
	case AtFloat:
		return toString(m_data.m_float);
	case AtString:
		return m_data.m_string;
	}
	return L"";
}

Ref< Object > Any::getObject() const
{
	return m_type == AtObject ? *m_data.m_object : 0;
}

Any& Any::operator = (const Any& src)
{
	if (m_type == AtString)
		refStringDec(m_data.m_string);
	else if (m_type == AtObject)
		delete m_data.m_object;

	m_type = src.m_type;
	m_data = src.m_data;

	if (m_type == AtString)
		refStringInc(m_data.m_string);
	else if (m_type == AtObject)
		m_data.m_object = new Ref< Object >(*m_data.m_object);

	return *this;
}

	}
}
