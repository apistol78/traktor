/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
