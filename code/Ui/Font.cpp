/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Ui/Font.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Font", Font, Object)

Font::Font()
{
	m_params.size = 0_ut;
	m_params.bold = false;
	m_params.italic = false;
	m_params.underline = false;
}

Font::Font(const Font& font)
:	m_face(font.m_face)
{
	m_params.size = font.m_params.size;
	m_params.bold = font.m_params.bold;
	m_params.italic = font.m_params.italic;
	m_params.underline = font.m_params.underline;
}

Font::Font(const std::wstring& face, Unit size, bool bold, bool italic, bool underline)
:	m_face(face)
{
	m_params.size = size;
	m_params.bold = bold;
	m_params.italic = italic;
	m_params.underline = underline;
}

void Font::setFace(const std::wstring& face)
{
	m_face = face;
}

std::wstring Font::getFace() const
{
	return m_face;
}

void Font::setSize(Unit size)
{
	m_params.size = size;
}

Unit Font::getSize() const
{
	return m_params.size;
}

void Font::setBold(bool bold)
{
	m_params.bold = bold;
}

bool Font::isBold() const
{
	return m_params.bold;
}

void Font::setItalic(bool italic)
{
	m_params.italic = italic;
}

bool Font::isItalic() const
{
	return m_params.italic;
}

void Font::setUnderline(bool underline)
{
	m_params.underline = underline;
}

bool Font::isUnderline() const
{
	return m_params.underline;
}

bool Font::operator == (const Font& r) const
{
	if (m_face != r.m_face)
		return false;

	return std::memcmp(&m_params, &r.m_params, sizeof(m_params)) == 0;
}

bool Font::operator != (const Font& r) const
{
	return !(*this == r);
}

bool Font::operator < (const Font& r) const
{
	if (m_face < r.m_face)
		return true;

	if (std::memcmp(&m_params, &r.m_params, sizeof(m_params)) < 0)
		return true;

	return false;
}

}
