/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Config.h"
#include "Ui/Size.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*!
 * \ingroup UI
 */
class T_DLLCLASS IFontMetric
{
public:
	virtual void getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const = 0;

	virtual int32_t getAdvance(wchar_t ch, wchar_t next) const = 0;

	virtual int32_t getLineSpacing() const = 0;

	virtual Size getExtent(const std::wstring& text) const = 0;
};

}
