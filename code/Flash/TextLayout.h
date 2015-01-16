#ifndef traktor_flash_TextLayout_H
#define traktor_flash_TextLayout_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Flash/SwfTypes.h"

namespace traktor
{
	namespace flash
	{

class FlashFont;

class TextLayout : public Object
{
	T_RTTI_CLASS;

public:
	enum Align
	{
		AnLeft = 0x00,
		AnRight = 0x01,
		AnCenter = 0x02,
		AnJustify = 0x03
	};

	struct Attribute
	{
		const FlashFont* font;
		SwfColor color;
	};

	struct Character
	{
		float x;
		float w;
		wchar_t ch;
	};

	struct Word
	{
		int32_t attrib;
		AlignedVector< Character > chars;
	};

	struct Line
	{
		float width;
		float x;
		float y;
		AlignedVector< Word > words;
	};

	TextLayout();

	void begin();

	void setBounds(const Aabb2& bounds);

	void setLeading(float leading);

	void setLetterSpacing(float letterSpacing);

	void setFontHeight(float fontHeight);

	void setWordWrap(bool wordWrap);

	void setAlignment(Align alignment);

	void setAttribute(const FlashFont* font, const SwfColor& textColor);

	void insertText(const std::wstring& text);

	void newLine();

	void end();

	const AlignedVector< Attribute >& getAttributes() const { return m_attribs; }

	const AlignedVector< Line >& getLines() const { return m_lines; }

	const Aabb2& getBounds() const { return m_bounds; }

	float getLeading() const { return m_leading; }

	float getFontHeight() const { return m_fontHeight; }

	Align getAlignment() const { return m_alignment; }

	float getWidth() const { return m_width / 20.0f; }

	float getHeight() const { return m_height / 20.0f; }

private:
	AlignedVector< Attribute > m_attribs;
	AlignedVector< Line > m_lines;
	int32_t m_currentAttrib;
	Aabb2 m_bounds;
	float m_leading;
	float m_letterSpacing;
	float m_fontHeight;
	bool m_wordWrap;
	Align m_alignment;
	float m_cursorX;
	float m_cursorY;
	float m_width;
	float m_height;
};

	}
}

#endif	// traktor_flash_TextLayout_H
