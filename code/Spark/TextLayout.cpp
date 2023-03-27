/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <limits>
#include "Core/Math/Const.h"
#include "Core/Misc/StringSplit.h"
#include "Spark/CharacterInstance.h"
#include "Spark/Font.h"
#include "Spark/Shape.h"
#include "Spark/TextLayout.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

bool isWhiteSpace(wchar_t ch)
{
	return ch == 0 || ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'\r';
}

enum LineBreak
{
	BreakAny,
	BreakAfter,
	BreakBefore
};

// http://xml.ascc.net/en/utf-8/faq/zhl10n-faq-xsl.html#lb
int32_t isBreakableUnicode(wchar_t c)
{
	bool breakable = false;

	// Break after any punctuation or spaces characters...
	if (c >= 0x2000)
	{
		if (
			((c >= 0x2000) && (c <= 0x2006)) ||
			((c >= 0x2008) && (c <= 0x2010)) ||
			((c >= 0x2011) && (c <= 0x2046)) ||
			((c >= 0x207D) && (c <= 0x207E)) ||
			((c >= 0x208D) && (c <= 0x208E)) ||
			((c >= 0x2329) && (c <= 0x232A)) ||
			((c >= 0x3001) && (c <= 0x3003)) ||
			((c >= 0x3008) && (c <= 0x3011)) ||
			((c >= 0x3014) && (c <= 0x301F)) ||
			((c >= 0xFD3E) && (c <= 0xFD3F)) ||
			((c >= 0xFE30) && (c <= 0xFE44)) ||
			((c >= 0xFE49) && (c <= 0xFE52)) ||
			((c >= 0xFE54) && (c <= 0xFE61)) ||
			((c >= 0xFE6A) && (c <= 0xFE6B)) ||
			((c >= 0xFF01) && (c <= 0xFF03)) ||
			((c >= 0xFF05) && (c <= 0xFF0A)) ||
			((c >= 0xFF0C) && (c <= 0xFF0F)) ||
			((c >= 0xFF1A) && (c <= 0xFF1B)) ||
			((c >= 0xFF1F) && (c <= 0xFF20)) ||
			((c >= 0xFF3B) && (c <= 0xFF3D)) ||
			((c >= 0xFF61) && (c <= 0xFF65))
		)
		{
			breakable = true;
		}
		else
		{
			switch (c)
			{
			case 0xFE63: case 0xFE68: case 0x3030:
			case 0x30FB: case 0xFF3F: case 0xFF5B:
			case 0xFF5D:
				breakable=true;
			}
		}

		// ...but break before a left punctuation.
		if (breakable == true)
		{
			if (
				((c >= 0x201A) && ( c <= 0x201C )) ||
				((c >= 0x201E) && ( c <= 0x201F ))
			)
			{
					return BreakBefore;
			}
			else
			{
				switch (c)
				{
				case 0x2018: case 0x2039: case 0x2045:
				case 0x207D: case 0x208D: case 0x2329:
				case 0x3008: case 0x300A: case 0x300C:
				case 0x300E: case 0x3010: case 0x3014:
				case 0x3016: case 0x3018: case 0x301A:
				case 0x301D: case 0xFD3E: case 0xFE35:
				case 0xFE37: case 0xFE39: case 0xFE3B:
				case 0xFE3D: case 0xFE3F: case 0xFE41:
				case 0xFE43: case 0xFE59: case 0xFE5B:
				case 0xFE5D: case 0xFF08: case 0xFF3B:
				case 0xFF5B: case 0xFF62:
					return BreakBefore;
				}
			}
		}
		if (breakable == true)
			return BreakAfter;
		else
			return BreakAny;
	}
	if (isWhiteSpace(c))
		return BreakBefore;
	else
		return BreakAfter;
}


		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.TextLayout", TextLayout, Object)

TextLayout::TextLayout()
:	m_currentAttrib(0)
,	m_leading(40.0f)
,	m_fontHeight(0.0f)
,	m_wordWrap(false)
,	m_alignment(StaLeft)
,	m_cursorX(0.0f)
,	m_cursorY(0.0f)
,	m_width(0.0f)
,	m_height(0.0f)
{
}

void TextLayout::begin()
{
	m_attribs.resize(0);
	m_characters.resize(0);
	m_lines.resize(0);

	m_currentAttrib = 0;
	m_cursorX = 0.0f;
	m_cursorY = 0.0f;
	m_width = 0.0f;
	m_height = 0.0f;

	const Line ln = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_lines.push_back(ln);
}

void TextLayout::setBounds(const Aabb2& bounds)
{
	m_bounds = bounds;
}

void TextLayout::setLeading(float leading)
{
	m_leading = leading;
}

void TextLayout::setLetterSpacing(float letterSpacing)
{
	m_letterSpacing = letterSpacing;
}

void TextLayout::setFontHeight(float fontHeight)
{
	m_fontHeight = fontHeight;
}

void TextLayout::setWordWrap(bool wordWrap)
{
	m_wordWrap = wordWrap;
}

void TextLayout::setAlignment(SwfTextAlignType alignment)
{
	m_alignment = alignment;
}

void TextLayout::setAttribute(const Font* font, const Color4f& textColor)
{
	if (!m_attribs.empty())
	{
		const Attribute& current = m_attribs[m_currentAttrib];
		if (current.font == font && current.color == textColor)
			return;
	}

	m_currentAttrib = int32_t(m_attribs.size());

	const Attribute attrib = { font, textColor };
	m_attribs.push_back(attrib);
}

void TextLayout::insertText(const std::wstring& text)
{
	if (text.empty())
		return;

	const Attribute& attrib = m_attribs[m_currentAttrib];
	if (!attrib.font)
		return;

	const float coordScale = attrib.font->getCoordinateType() == Font::CtTwips ? 1.0f / 1000.0f : 1.0f / (20.0f * 1000.0f);
	const float fontScale = coordScale * m_fontHeight;
	const float letterSpacing = m_letterSpacing * 200.0f * 2000.0f / m_fontHeight;
	const float boundsWidth = m_bounds.mx.x - m_bounds.mn.x;

	const uint16_t spaceGlyphIndex = attrib.font->lookupIndex(L' ');
	const int16_t spaceWidth = attrib.font->getAdvance(spaceGlyphIndex);
	bool spaceInsert = false;

	if (m_wordWrap)
	{
		std::wstring::size_type start = 0;
		std::wstring::size_type end = 0;

		int32_t currentBreakable = isBreakableUnicode(text[start]);
		while (start < text.length())
		{
			end += 1;
			if (end < text.length())
			{
				const int32_t nextBreakable = isBreakableUnicode(text[end]);
				bool lineBreak = true;
				if (nextBreakable == BreakAfter)
					lineBreak = false;
				else if (currentBreakable == BreakBefore)
					lineBreak = false;
				currentBreakable = nextBreakable;
				if (lineBreak == false)
					continue;
			}

			std::wstring::size_type len = std::wstring::npos;
			if (end < text.length())
			{
				if (isWhiteSpace(text[end - 1]))
					len = end - start - 1;
				else
					len = end - start;
			}

			const std::wstring word = text.substr(start, len);

			if (end < text.length() && isWhiteSpace(text[end]))
				end += 1;

			start = end;

			if (word.empty())
				continue;

			float wordWidth = 0.0f;
			for (uint32_t j = 0; j < word.length(); ++j)
			{
				const uint16_t glyphIndex = attrib.font->lookupIndex(word[j]);
				int16_t glyphAdvance = attrib.font->getAdvance(glyphIndex);

				if (j < word.length() - 1)
					glyphAdvance += attrib.font->lookupKerning(word[j], word[j + 1]);

				wordWidth += (glyphAdvance + letterSpacing) * fontScale;
			}

			if (
				m_cursorX > FUZZY_EPSILON &&
				m_cursorX + wordWidth > boundsWidth
			)
			{
				newLine();
				spaceInsert = false;
			}

			if (spaceInsert)
				m_cursorX += spaceWidth * fontScale;

			Word w = { m_currentAttrib, 0 };

			float tailAdjust = 0.0f;
			for (uint32_t j = 0; j < word.length(); ++j)
			{
				const uint16_t glyphIndex = attrib.font->lookupIndex(word[j]);
				int16_t glyphAdvance = attrib.font->getAdvance(glyphIndex);

				if (j < word.length() - 1)
					glyphAdvance += attrib.font->lookupKerning(word[j], word[j + 1]);

				if (!isWhiteSpace(word[j]))
				{
					Character chr;
					chr.x = m_cursorX;
					chr.w = (glyphAdvance + letterSpacing) * fontScale;
					chr.ch = word[j];
					w.chars.push_back(chr);
				}

				const Aabb2* glyphBounds = attrib.font->getBounds(glyphIndex);
				if (glyphBounds)
					tailAdjust = (glyphAdvance - glyphBounds->getSize().x) * fontScale;
				else
					tailAdjust = 0.0f;

				m_cursorX += (glyphAdvance + letterSpacing) * fontScale;
			}

			if (!w.chars.empty())
				m_lines.back().words.push_back(w);

			const float lw = std::max(m_lines.back().width, m_cursorX - tailAdjust);

			m_lines.back().width = lw;
			m_width = std::max(m_width, lw);

			spaceInsert = true;
		}
	}
	else
	{
		const std::wstring& word = text;
		if (word.empty())
			return;

		Word w = { m_currentAttrib, 0 };

		float tail = 0.0f;
		for (uint32_t j = 0; j < word.length(); ++j)
		{
			const uint16_t glyphIndex = attrib.font->lookupIndex(word[j]);
			int16_t glyphAdvance = attrib.font->getAdvance(glyphIndex);

			if (j < word.length() - 1)
				glyphAdvance += attrib.font->lookupKerning(word[j], word[j + 1]);

			const Shape* glyphShape = attrib.font->getShape(glyphIndex);
			if (glyphShape)
			{
				if (j == 0)
					m_cursorX -= glyphShape->getShapeBounds().mn.x * fontScale;

				tail = m_cursorX + glyphShape->getShapeBounds().mx.x * fontScale;
			}

			Character chr;
			chr.x = m_cursorX;
			chr.w = (glyphAdvance + letterSpacing) * fontScale;
			chr.ch = !isWhiteSpace(word[j]) ? word[j] : 0;
			w.chars.push_back(chr);

			m_cursorX += (glyphAdvance + letterSpacing) * fontScale;
		}

		if (!w.chars.empty())
			m_lines.back().words.push_back(w);

		const float lw = std::max(m_lines.back().width, tail);
		m_lines.back().width = lw;
		m_width = std::max(m_width, lw);
	}
}

void TextLayout::insertCharacter(CharacterInstance* characterInstance)
{
	m_characters.push_back(characterInstance);

	Aabb2 bounds = characterInstance->getBounds();

	Word w = { m_currentAttrib, int32_t(m_characters.size()) };
	m_lines.back().words.push_back(w);

	m_cursorX += bounds.getSize().x;
	m_lines.back().width = std::max(m_lines.back().width, m_cursorX);
	m_width = std::max(m_width, m_cursorX);
}

void TextLayout::insertBlank(int32_t width)
{
	m_cursorX += width;
	m_lines.back().width = std::max(m_lines.back().width, m_cursorX);
	m_width = std::max(m_width, m_cursorX);
}

void TextLayout::newLine()
{
	const float lineHeight = m_fontHeight + m_leading;

	m_cursorX = 0.0f;
	m_cursorY += lineHeight;
	m_height = std::max(m_height, m_cursorY + lineHeight);

	Line ln = { 0.0f, 0.0f, m_cursorY, 0.0f };
	m_lines.push_back(ln);
}

void TextLayout::end()
{
	// Calculate height of last line.
	float lineHeight = m_fontHeight + m_leading;
	if (!m_attribs.empty())
	{
		const Attribute& attrib = m_attribs[m_currentAttrib];
		if (attrib.font)
		{
			const float coordScale = attrib.font->getCoordinateType() == Font::CtTwips ? 1.0f / 1000.0f : 1.0f / (20.0f * 1000.0f);
			const float fontScale = coordScale * m_fontHeight;
			lineHeight = attrib.font->getDescent() * fontScale;
		}
	}

	const float boundsWidth = m_bounds.mx.x - m_bounds.mn.x;
	for (auto& line : m_lines)
	{
		line.x = m_bounds.mn.x;
		line.y += lineHeight;
		line.offset = 0.0f;

		if (line.words.empty())
			continue;

		// Calculate alignment.
		if (m_alignment == StaRight || m_alignment == StaCenter)
		{
			float offset = boundsWidth - line.width;

			if (m_alignment == StaCenter)
				offset /= 2.0f;

			line.x = m_bounds.mn.x + offset;
		}
	}

	m_height = std::max(m_height, m_cursorY + lineHeight);
}

	}
}
