/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Graph/PaintSettings.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PaintSettings", PaintSettings, Object)

PaintSettings::PaintSettings(const ui::Font& font)
:	m_font(font.getFace(), font.getSize(), false, false, false)
,	m_fontBold(font.getFace(), font.getSize(), true, false, false)
,	m_fontUnderline(font.getFace(), font.getSize(), false, false, true)
,	m_fontLabel(font.getFace(), font.getSize() / 2_ut, false, false, false)
,	m_fontGroup(font.getFace(), font.getSize() * 3_ut, true, false, false)
{
}

const ui::Font& PaintSettings::getFont() const
{
	return m_font;
}

const ui::Font& PaintSettings::getFontBold() const
{
	return m_fontBold;
}

const ui::Font& PaintSettings::getFontUnderline() const
{
	return m_fontUnderline;
}

const ui::Font& PaintSettings::getFontLabel() const
{
	return m_fontLabel;
}

const ui::Font& PaintSettings::getFontGroup() const
{
	return m_fontGroup;
}

}
