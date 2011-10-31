#include "I18N/Format.h"
#include "I18N/I18N.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace i18n
	{

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
	m_value.s = s.c_str();
}

Format::Argument::Argument(const wchar_t* s)
:	m_type(AtString)
{
	m_value.s = s;
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
