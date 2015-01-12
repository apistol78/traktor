#include <cstring>
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Misc/String.h"
#include "I18N/Format.h"
#include "I18N/I18N.h"

namespace traktor
{
	namespace i18n
	{
		namespace
		{

wchar_t* refStringCreate(const wchar_t* s)
{
	size_t len = wcslen(s);

	void* ptr = getAllocator()->alloc(sizeof(uint16_t) + (len + 1) * sizeof(wchar_t), 4, T_FILE_LINE);
	if (!ptr)
		return 0;

	uint16_t* base = static_cast< uint16_t* >(ptr);
	*base = 1;

	wchar_t* c = reinterpret_cast< wchar_t* >(base + 1);
	if (len > 0)
		std::memcpy(c, s, len * sizeof(wchar_t));

	c[len] = L'\0';
	return c;
}

wchar_t* refStringInc(wchar_t* s)
{
	uint16_t* base = reinterpret_cast< uint16_t* >(s) - 1;
	(*base)++;
	return s;
}

wchar_t* refStringDec(wchar_t* s)
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.i18n.Format", Format, Object)

Format::Argument Format::Argument::ms_void;

Format::Argument::Argument(void)
:	m_type(AtVoid)
{
}

Format::Argument::Argument(wchar_t c)
:	m_type(AtChar)
{
	m_value.c = c;
}

Format::Argument::Argument(int32_t i)
:	m_type(AtInt32)
{
	m_value.i = i;
}

Format::Argument::Argument(float f)
:	m_type(AtFloat)
{
	m_value.f = f;
}

Format::Argument::Argument(double d)
:	m_type(AtDouble)
{
	m_value.d = d;
}

Format::Argument::Argument(const std::wstring& s)
:	m_type(AtString)
{
	m_value.s = refStringCreate(s.c_str());
}

Format::Argument::Argument(const wchar_t* s)
:	m_type(AtString)
{
	m_value.s = refStringCreate(s);
}

Format::Argument::Argument(const Argument& s)
:	m_type(s.m_type)
,	m_value(s.m_value)
{
	if (m_type == AtString)
		refStringInc(m_value.s);
}

Format::Argument::~Argument()
{
	if (m_type == AtString)
		refStringDec(m_value.s);
}

std::wstring Format::Argument::format() const
{
	switch (m_type)
	{
	case AtChar:
		{
			wchar_t tmp[] = { m_value.c, L'\0' };
			return std::wstring(tmp);
		}
	case AtInt32:
		{
			return toString< int32_t >(m_value.i);
		}
	case AtFloat:
		{
			return toString< float >(m_value.f);
		}
	case AtDouble:
		{
			return toString< double >(m_value.d);
		}
	case AtString:
		{
			return m_value.s ? m_value.s : L"";
		}
	default:
		break;
	}
	return L"";
}

Format::Format(
	const std::wstring& id,
	const Argument& arg1,
	const Argument& arg2,
	const Argument& arg3,
	const Argument& arg4
)
{
	m_text = I18N::getInstance().get(id, id);
	for (;;)
	{
		size_t s = m_text.find(L"{");
		if (s == std::string::npos)
			break;

		size_t e = m_text.find(L"}", s + 1);
		if (e == std::string::npos)
			break;

		std::wstring argument = m_text.substr(s + 1, e - s - 1);
		std::wstring value = L"";

		switch (parseString< int >(argument))
		{
		case 0:
			value = arg1.format();
			break;
		case 1:
			value = arg2.format();
			break;
		case 2:
			value = arg3.format();
			break;
		case 3:
			value = arg4.format();
			break;
		}

		m_text = m_text.substr(0, s) + value + m_text.substr(e + 1);
	}
}

const std::wstring& Format::str() const
{
	return m_text;
}

Format::operator const std::wstring& () const
{
	return m_text;
}

	}
}
