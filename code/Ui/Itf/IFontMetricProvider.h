/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Config.h"
#include "Ui/Font.h"
#include "Ui/Size.h"

namespace traktor::ui
{

/*! Font metric provider interface.
 *
 * Font-parameterized text measurement, provided by top-level widget peers
 * so widgets without a native canvas of their own can measure text.
 *
 * \ingroup UI
 */
class IFontMetricProvider
{
public:
	virtual ~IFontMetricProvider() {}

	virtual void getAscentAndDescent(const Font& font, int32_t& outAscent, int32_t& outDescent) const = 0;

	virtual int32_t getAdvance(const Font& font, wchar_t ch, wchar_t next) const = 0;

	virtual int32_t getLineSpacing(const Font& font) const = 0;

	virtual Size getExtent(const Font& font, const std::wstring& text) const = 0;
};

}
