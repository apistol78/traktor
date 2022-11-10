/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

namespace traktor
{
	namespace i18n
	{

class Translator : public Object
{
	T_RTTI_CLASS;

public:
	Translator(const std::wstring& from, const std::wstring& to);

	bool translate(const std::wstring& text, std::wstring& outText) const;

private:
	std::wstring m_from;
	std::wstring m_to;
};

	}
}

