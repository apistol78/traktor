/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <set>
#include "Ui/EditValidator.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! Character set text edit validator.
 * \ingroup UI
 */
class T_DLLCLASS CharacterSetEditValidator : public EditValidator
{
	T_RTTI_CLASS;

public:
	void add(wchar_t ch);

	void addRange(wchar_t from, wchar_t to);

	virtual EditValidator::Result validate(const std::wstring& text) const override;

private:
	std::set< wchar_t > m_set;
};

	}
}

