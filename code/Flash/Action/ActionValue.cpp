#include <cstring>
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Misc/String.h"
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/Classes/Boolean.h"
#include "Flash/Action/Classes/Number.h"
#include "Flash/Action/Classes/String.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

#if defined(_DEBUG)
#	define T_VALIDATE(av) \
	T_ASSERT ((av).getType() >= ActionValue::AvtUndefined && (av).getType() <= ActionValue::AvtObject);
#else
#	define T_VALIDATE(av)
#endif

static int32_t s_stringCount = 0;

char* refStringCreate(const char* s)
{
	uint32_t len = strlen(s);
	T_ASSERT (len < 4096);
	
	void* ptr = getAllocator()->alloc(sizeof(uint16_t) + (len + 1) * sizeof(char), 4, T_FILE_LINE);
	if (!ptr)
		return 0;

	uint16_t* base = static_cast< uint16_t* >(ptr);
	*base = 1;

	char* c = reinterpret_cast< char* >(base + 1);
	if (len > 0)
		std::memcpy(c, s, len * sizeof(char));

	c[len] = '\0';

	++s_stringCount;
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
	if (--(*base) == 0)
	{
		getAllocator()->free(base);
		--s_stringCount;
		return 0;
	}
	return s;
}

		}

ActionValue::ActionValue()
:	m_type(AvtUndefined)
{
	m_value.o = 0;
}

ActionValue::ActionValue(const ActionValue& v)
:	m_type(v.m_type)
{
	T_VALIDATE(v);
	m_value.o = 0;
	if (m_type == AvtString)
		m_value.s = refStringInc(v.m_value.s);
	else if (m_type == AvtObject)
	{
		T_SAFE_ADDREF (v.m_value.o);
		m_value.o = v.m_value.o;
	}
	else
		m_value = v.m_value;
}

ActionValue::ActionValue(bool b)
:	m_type(AvtBoolean)
{
	m_value.o = 0;
	m_value.b = b;
}

ActionValue::ActionValue(avm_number_t n)
:	m_type(AvtNumber)
{
	m_value.o = 0;
	m_value.n = n;
}

ActionValue::ActionValue(const char* s)
:	m_type(AvtString)
{
	m_value.s = refStringCreate(s);
}

ActionValue::ActionValue(const std::string& s)
:	m_type(AvtString)
{
	m_value.s = refStringCreate(s.c_str());
}

ActionValue::ActionValue(const wchar_t* s)
:	m_type(AvtString)
{
	m_value.s = refStringCreate(wstombs(Utf8Encoding(), s).c_str());
}

ActionValue::ActionValue(const std::wstring& s)
:	m_type(AvtString)
{
	m_value.s = refStringCreate(wstombs(Utf8Encoding(), s).c_str());
}

ActionValue::ActionValue(ActionObject* o)
:	m_type(AvtObject)
{
	T_SAFE_ADDREF(o);
	m_value.o = o;
}

ActionValue::~ActionValue()
{
	T_EXCEPTION_GUARD_BEGIN

	T_VALIDATE(*this);
	if (m_type == AvtString && m_value.s)
		refStringDec(m_value.s);
	else if (m_type == AvtObject && m_value.o)
		T_SAFE_RELEASE (m_value.o);

	T_EXCEPTION_GUARD_END
}

bool ActionValue::getBoolean() const
{
	T_VALIDATE(*this);
	switch (m_type)
	{
	case AvtBoolean:
		return m_value.b;
	case AvtNumber:
		return bool(m_value.n != 0.0);
	case AvtString:
		return bool(strcmp(m_value.s, "true") == 0);
	case AvtObject:
		return bool(m_value.o != 0);
	}
	return false;
}

avm_number_t ActionValue::getNumber() const
{
	T_VALIDATE(*this);
	switch (m_type)
	{
	case AvtBoolean:
		return m_value.b ? avm_number_t(1) : avm_number_t(0);
	case AvtNumber:
		return m_value.n;
	}
	return avm_number_t(0);
}

std::string ActionValue::getString() const
{
	T_VALIDATE(*this);
	switch (m_type)
	{
	case AvtBoolean:
		return m_value.b ? "true" : "false";
	case AvtNumber:
		return wstombs(traktor::toString(m_value.n));
	case AvtString:
		return m_value.s;
	case AvtObject:
		return m_value.o ? m_value.o->toString().getString() : "null";
	}
	return "undefined";
}

std::wstring ActionValue::getWideString() const
{
	return mbstows(Utf8Encoding(), getString());
}

Ref< ActionObject > ActionValue::getObject() const
{
	T_VALIDATE(*this);
	switch (m_type)
	{
	case AvtBoolean:
		return new Boolean(m_value.b);
	case AvtNumber:
		return new Number(m_value.n);
	case AvtString:
		return new String(m_value.s);
	case AvtObject:
		return m_value.o;
	}
	return 0;
}

ActionValue& ActionValue::operator = (const ActionValue& v)
{
	T_VALIDATE(*this);
	T_VALIDATE(v);

	if (v.m_type == AvtString)
		refStringInc(v.m_value.s);
	else if (v.m_type == AvtObject)
		T_SAFE_ADDREF(v.m_value.o);

	if (m_type == AvtString)
		refStringDec(m_value.s);
	else if (m_type == AvtObject)
		T_SAFE_RELEASE(m_value.o);

	m_type = v.m_type;
	m_value = v.m_value;

	return *this;
}

	}
}
