/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_TextLayout_H
#define traktor_flash_TextLayout_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Flash/SwfTypes.h"

namespace traktor
{
	namespace flash
	{

class FlashCharacterInstance;
class FlashFont;

#pragma warning( disable:4324 )

/*! \brief Text layout engine.
 * \ingroup Flash
 */
class TextLayout : public Object
{
	T_RTTI_CLASS;

public:
	struct Attribute
	{
		const FlashFont* font;
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

	void setAttribute(const FlashFont* font, const Color4f& textColor);

	void insertText(const std::wstring& text);

	void insertCharacter(FlashCharacterInstance* characterInstance);

	void newLine();

	void end();

	const AlignedVector< Attribute >& getAttributes() const { return m_attribs; }

	const RefArray< FlashCharacterInstance >& getCharacters() const { return m_characters; }

	const AlignedVector< Line >& getLines() const { return m_lines; }

	const Aabb2& getBounds() const { return m_bounds; }

	float getLeading() const { return m_leading; }

	float getFontHeight() const { return m_fontHeight; }

	SwfTextAlignType getAlignment() const { return m_alignment; }

	float getWidth() const { return m_width / 20.0f; }

	float getHeight() const { return m_height / 20.0f; }

private:
	AlignedVector< Attribute > m_attribs;
	RefArray< FlashCharacterInstance > m_characters;
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

	}
}

#endif	// traktor_flash_TextLayout_H
