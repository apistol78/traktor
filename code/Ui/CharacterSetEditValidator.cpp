/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
	for (auto ch : text)
	{
		if (m_set.find(ch) == m_set.end())
			return VrInvalid;
	}
	return VrOk;
}

	}
}
