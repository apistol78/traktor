/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "I18N/Text.h"
#include "I18N/I18N.h"

namespace traktor
{
	namespace i18n
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.i18n.Text", Text, Object)

Text::Text(const std::wstring& id, const std::wstring& defaultText)
{
	m_text = I18N::getInstance().get(id, !defaultText.empty() ? defaultText : id);
}

const std::wstring& Text::str() const
{
	return m_text;
}

Text::operator const std::wstring& () const
{
	return m_text;
}

	}
}
