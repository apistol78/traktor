#include <cstring>
#include <limits>
#include "Core/Math/Const.h"
#include "Core/Misc/StringSplit.h"
#include "Flash/FlashFont.h"
#include "Flash/TextLayout.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

bool isWhiteSpace(wchar_t ch)
{
	return ch == 0 || ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'\r';
}

bool operator == (const SwfColor& a, const SwfColor& b)
{
	return std::memcmp(&a, &b, sizeof(SwfColor)) == 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.TextLayout", TextLayout, Object)

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
	Line ln = { 0.0f, 0.0f };
	m_lines.push_back(Line());
}

void TextLayout::begin()
{
	m_lines.clear();
	m_cursorX = 0.0f;
	m_cursorY = 0.0f;
	m_width = 0.0f;
	m_height = 0.0f;

	Line ln = { 0.0f, 0.0f, 0.0f };
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

void TextLayout::setAttribute(const FlashFont* font, const SwfColor& textColor)
{
	if (!m_attribs.empty())
	{
		const Attribute& current = m_attribs[m_currentAttrib];
		if (current.font == font && current.color == textColor)
			return;
	}

	m_currentAttrib = m_attribs.size();

	Attribute attrib = { font, textColor };
	m_attribs.push_back(attrib);
}

void TextLayout::insertText(const std::wstring& text)
{
	const Attribute& attrib = m_attribs[m_currentAttrib];

	float coordScale = attrib.font->getCoordinateType() == FlashFont::CtTwips ? 1.0f / 1000.0f : 1.0f / (20.0f * 1000.0f);
	float fontScale = coordScale * m_fontHeight;
	float letterSpacing = m_letterSpacing * 200.0f * 2000.0f / m_fontHeight;
	float boundsWidth = m_bounds.mx.x - m_bounds.mn.x;

	uint16_t spaceGlyphIndex = attrib.font->lookupIndex(L' ');
	int16_t spaceWidth = attrib.font->getAdvance(spaceGlyphIndex);

	if (m_wordWrap)
	{
		StringSplit< std::wstring > split(text, L" \t");
		for (StringSplit< std::wstring >::const_iterator i = split.begin(); i != split.end(); ++i)
		{
			const std::wstring& word = *i;
			if (word.empty())
				continue;

			float wordWidth = 0.0f;

			for (uint32_t j = 0; j < word.length(); ++j)
			{
				uint16_t glyphIndex = attrib.font->lookupIndex(word[j]);
				int16_t glyphAdvance = attrib.font->getAdvance(glyphIndex);

				if (j < word.length() - 1)
					glyphAdvance += attrib.font->lookupKerning(word[j], word[j + 1]);

				wordWidth += (glyphAdvance + letterSpacing) * fontScale;
			}

			if (m_wordWrap)
			{
				if (
					m_cursorX > FUZZY_EPSILON &&
					m_cursorX + wordWidth > boundsWidth
				)
					newLine();
			}

			if (m_cursorX > FUZZY_EPSILON)
				m_cursorX += spaceWidth * fontScale;

			Word w = { m_currentAttrib };

			for (uint32_t j = 0; j < word.length(); ++j)
			{
				uint16_t glyphIndex = attrib.font->lookupIndex(word[j]);
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

				m_cursorX += (glyphAdvance + letterSpacing) * fontScale;
			}

			if (!w.chars.empty())
				m_lines.back().words.push_back(w);

			m_lines.back().width = std::max(m_lines.back().width, m_cursorX);
			m_width = std::max(m_width, m_cursorX);
		}
	}
	else
	{
		const std::wstring& word = text;
		if (word.empty())
			return;

		float wordWidth = 0.0f;

		for (uint32_t j = 0; j < word.length(); ++j)
		{
			uint16_t glyphIndex = attrib.font->lookupIndex(word[j]);
			int16_t glyphAdvance = attrib.font->getAdvance(glyphIndex);

			if (j < word.length() - 1)
				glyphAdvance += attrib.font->lookupKerning(word[j], word[j + 1]);

			wordWidth += (glyphAdvance + letterSpacing) * fontScale;
		}

		Word w = { m_currentAttrib };

		for (uint32_t j = 0; j < word.length(); ++j)
		{
			uint16_t glyphIndex = attrib.font->lookupIndex(word[j]);
			int16_t glyphAdvance = attrib.font->getAdvance(glyphIndex);

			if (j < word.length() - 1)
				glyphAdvance += attrib.font->lookupKerning(word[j], word[j + 1]);

			Character chr;
			chr.x = m_cursorX;
			chr.w = (glyphAdvance + letterSpacing) * fontScale;
			chr.ch = !isWhiteSpace(word[j]) ? word[j] : 0;
			w.chars.push_back(chr);

			m_cursorX += (glyphAdvance + letterSpacing) * fontScale;
		}

		if (!w.chars.empty())
			m_lines.back().words.push_back(w);

		m_lines.back().width = std::max(m_lines.back().width, m_cursorX);
		m_width = std::max(m_width, m_cursorX);
	}
}

void TextLayout::newLine()
{
	float lineHeight = m_fontHeight + m_leading;
	/*
	if (!m_attribs.empty())
	{
		const Attribute& attrib = m_attribs[m_currentAttrib];
		if (attrib.font)
		{
			lineHeight = (attrib.font->getAscent() + attrib.font->getDescent() + attrib.font->getLeading());
			if (attrib.font->getCoordinateType() == FlashFont::CtEMSquare)
				lineHeight /= 20.0f * 4.0f;
		}
	}
	*/
	m_cursorX = 0.0f;
	m_cursorY += lineHeight;
	m_height = std::max(m_height, m_cursorY + lineHeight);

	Line ln = { 0.0f, 0.0f, m_cursorY };
	m_lines.push_back(ln);
}

void TextLayout::end()
{
	float boundsWidth = m_bounds.mx.x - m_bounds.mn.x;
	for (AlignedVector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
	{
		i->x = m_bounds.mn.x;
		i->y += m_fontHeight;

		if (m_alignment == StaRight || m_alignment == StaCenter)
		{
			float offset = boundsWidth - i->width;

			if (m_alignment == StaCenter)
				offset /= 2.0f;

			i->x = m_bounds.mn.x + offset;
		}
	}
}

	}
}
