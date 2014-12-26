#include "Ui/CharacterSetEditValidator.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.CharacterSetEditValidator", CharacterSetEditValidator, EditValidator)

void CharacterSetEditValidator::add(wchar_t ch)
{
	m_set.insert(ch);
}

void CharacterSetEditValidator::addRange(wchar_t from, wchar_t to)
{
	for (wchar_t ch = from; ch <= to; ++ch)
		add(ch);
}

EditValidator::Result CharacterSetEditValidator::validate(const std::wstring& text) const
{
	for (std::wstring::const_iterator i = text.begin(); i != text.end(); ++i)
	{
		if (m_set.find(*i) == m_set.end())
			return VrInvalid;
	}
	return VrOk;
}

	}
}
