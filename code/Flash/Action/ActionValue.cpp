#include <cstring>
#include "Core/Memory/Alloc.h"
#include "Core/Misc/String.h"
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/Avm1/ActionBoolean.h"
#include "Flash/Action/Avm1/ActionNumber.h"
#include "Flash/Action/Avm1/ActionString.h"

namespace traktor
{
	namespace flash
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

ActionValue::ActionValue()
:	m_type(AvtUndefined)
{
	m_value.n = 0;
}

ActionValue::ActionValue(const ActionValue& v)
:	m_type(v.m_type)
{
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
	m_value.b = b;
}

ActionValue::ActionValue(avm_number_t n)
:	m_type(AvtNumber)
{
	m_value.n = n;
}

ActionValue::ActionValue(const wchar_t* s)
:	m_type(AvtString)
{
	m_value.s = refStringCreate(s);
}

ActionValue::ActionValue(const std::wstring& s)
:	m_type(AvtString)
{
	m_value.s = refStringCreate(s.c_str());
}

ActionValue::~ActionValue()
{
	T_EXCEPTION_GUARD_BEGIN

	if (m_type == AvtString && m_value.s)
		refStringDec(m_value.s);
	else if (m_type == AvtObject && m_value.o)
		T_SAFE_RELEASE (m_value.o);

	T_EXCEPTION_GUARD_END
}

ActionValue ActionValue::fromObject(ActionObject* const o)
{
	T_SAFE_ADDREF(o);

	ActionValue value;
	value.m_type = AvtObject;
	value.m_value.o = o;

	return value;
}

ActionValue ActionValue::toBoolean() const
{
	return ActionValue(getBooleanSafe());
}

ActionValue ActionValue::toNumber() const
{
	return ActionValue(getNumberSafe());
}

ActionValue ActionValue::toString() const
{
	return ActionValue(getStringSafe());
}

bool ActionValue::getBooleanSafe() const
{
	switch (m_type)
	{
	case AvtBoolean:
		return m_value.b;
	case AvtNumber:
		return bool(m_value.n != 0.0);
	case AvtString:
		return bool(wcscmp(m_value.s, L"true") == 0);
	case AvtObject:
		return bool(m_value.o != 0);
	}
	return false;
}

avm_number_t ActionValue::getNumberSafe() const
{
	switch (m_type)
	{
	case AvtBoolean:
		return m_value.b ? avm_number_t(1) : avm_number_t(0);
	case AvtNumber:
		return m_value.n;
	}
	return avm_number_t(0);
}

std::wstring ActionValue::getStringSafe() const
{
	switch (m_type)
	{
	case AvtBoolean:
		return m_value.b ? L"true" : L"false";
	case AvtNumber:
		return traktor::toString(m_value.n);
	case AvtString:
		return m_value.s;
	case AvtObject:
		return m_value.o ? m_value.o->toString() : L"null";
	}
	return L"undefined";
}

Ref< ActionObject > ActionValue::getObjectSafe() const
{
	switch (m_type)
	{
	case AvtBoolean:
		return new ActionBoolean(m_value.b);
	case AvtNumber:
		return new ActionNumber(m_value.n);
	case AvtString:
		return new ActionString(m_value.s);
	case AvtObject:
		return m_value.o;
	}
	return 0;
}

ActionValue& ActionValue::operator = (const ActionValue& v)
{
	if (m_type == AvtString)
		refStringDec(m_value.s);
	else if (m_type == AvtObject)
	{
		T_SAFE_RELEASE (m_value.o);
		m_value.o = 0;
	}

	m_type = v.m_type;
	if (m_type == AvtString)
		m_value.s = refStringInc(v.m_value.s);
	else if (m_type == AvtObject)
	{
		m_value.o = v.m_value.o;
		T_SAFE_ADDREF (m_value.o);
	}
	else
		m_value = v.m_value;

	return *this;
}

	}
}
