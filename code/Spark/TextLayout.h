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
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Spark/Swf/SwfTypes.h"

namespace traktor
{
	namespace spark
	{

class CharacterInstance;
class Font;

#if defined (_MSC_VER)
#	pragma warning( disable:4324 )
#endif

/*! Text layout engine.
 * \ingroup Spark
 */
class TextLayout : public Object
{
	T_RTTI_CLASS;

public:
	struct Attribute
	{
		const Font* font;
		Color4f color;
	};

	struct Character
	{
		float x;
		float w;
		wchar_t ch;
	};

	struct Word
	{
		int32_t a;
		int32_t c;
		AlignedVector< Character > chars;
	};

	struct Line
	{
		float width;
		float x;
		float y;
		float offset;
		AlignedVector< Word > words;
	};

	TextLayout();

	void begin();

	void setBounds(const Aabb2& bounds);

	void setLeading(float leading);

	void setLetterSpacing(float letterSpacing);

	void setFontHeight(float fontHeight);

	void setWordWrap(bool wordWrap);

	void setAlignment(SwfTextAlignType alignment);

	void setAttribute(const Font* font, const Color4f& textColor);

	void insertText(const std::wstring& text);

	void insertCharacter(CharacterInstance* characterInstance);

	void insertBlank(int32_t width);

	void newLine();

	void end();

	const AlignedVector< Attribute >& getAttributes() const { return m_attribs; }

	const RefArray< CharacterInstance >& getCharacters() const { return m_characters; }

	const AlignedVector< Line >& getLines() const { return m_lines; }

	const Aabb2& getBounds() const { return m_bounds; }

	float getLeading() const { return m_leading; }

	float getFontHeight() const { return m_fontHeight; }

	SwfTextAlignType getAlignment() const { return m_alignment; }

	float getCursorX() const { return m_cursorX; }

	float getCursorY() const { return m_cursorY; }

	float getWidth() const { return m_width / 20.0f; }

	float getHeight() const { return m_height / 20.0f; }

private:
	AlignedVector< Attribute > m_attribs;
	RefArray< CharacterInstance > m_characters;
	AlignedVector< Line > m_lines;
	int32_t m_currentAttrib;
	Aabb2 m_bounds;
	float m_leading;
	float m_letterSpacing;
	float m_fontHeight;
	bool m_wordWrap;
	SwfTextAlignType m_alignment;
	float m_cursorX;
	float m_cursorY;
	float m_width;
	float m_height;
};

#if defined (_MSC_VER)
#	pragma warning( default:4324 )
#endif

	}
}

