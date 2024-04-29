/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Ui/Font.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Graph visual settings.
 * \ingroup UI
 */
class T_DLLCLASS PaintSettings : public Object
{
	T_RTTI_CLASS;

public:
	PaintSettings() = default;

	explicit PaintSettings(const ui::Font& font);

	const ui::Font& getFont() const;

	const ui::Font& getFontBold() const;

	const ui::Font& getFontUnderline() const;

	const ui::Font& getFontLabel() const;

	const ui::Font& getFontGroup() const;

private:
	ui::Font m_font;
	ui::Font m_fontBold;
	ui::Font m_fontUnderline;
	ui::Font m_fontLabel;
	ui::Font m_fontGroup;
};

}
