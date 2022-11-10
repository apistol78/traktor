/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Spark/CharacterInstance.h"
#include "Spark/Event.h"
#include "Spark/Swf/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class Edit;
class TextFormat;
class TextLayout;

/*! Dynamic text character instance.
 * \ingroup Spark
 */
class T_DLLCLASS EditInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	explicit EditInstance(Context* context, Dictionary* dictionary, CharacterInstance* parent, const Edit* edit, const std::wstring& html);

	virtual ~EditInstance();

	virtual void destroy() override;

	/*! Get edit character. */
	const Edit* getEdit() const { return m_edit; }

	/*! Parse text. */
	bool parseText(const std::wstring& text);

	/*! Parse html formatted text. */
	bool parseHtml(const std::wstring& html);

	/*! Measure text (without replacing) using edit field's format. */
	Vector2 measureText(const std::wstring& text) const;

	/*! Measure text (without replacing) using edit field's format. */
	Vector2 measureText(const std::wstring& text, float width) const;

	/*! Set text bounds. */
	void setTextBounds(const Aabb2& textBounds);

	/*! Get text bounds. */
	const Aabb2& getTextBounds() const { return m_textBounds; }

	/*! Set text color. */
	void setTextColor(const Color4f& textColor);

	/*! Get text color. */
	const Color4f& getTextColor() const { return m_textColor; }

	/*! Set letter spacing. */
	void setLetterSpacing(float letterSpacing);

	/*! Get letter spacing. */
	float getLetterSpacing() const { return m_letterSpacing; }

	/*! Set text format. */
	void setTextFormat(const TextFormat* textFormat);

	/*! Get text format. */
	Ref< TextFormat > getTextFormat() const;

	/*! Set text format on text range. */
	void setTextFormat(const TextFormat* textFormat, int32_t beginIndex, int32_t endIndex);

	/*! Get text format from text range. */
	Ref< TextFormat > getTextFormat(int32_t beginIndex, int32_t endIndex) const;

	/*! Get text. */
	const std::wstring& getText() const { return m_text; }

	/*! Get html formatted text. */
	const std::wstring& getHtmlText() const;

	/*! Set multiline. */
	void setMultiLine(bool multiLine);

	/*! Get multiline. */
	bool getMultiLine() const;

	/*! Set word wrapping. */
	void setWordWrap(bool wordWrap);

	/*! Get word wrapping. */
	bool getWordWrap() const { return m_wordWrap; }

	/*! Set password mode. */
	void setPassword(bool password);

	/*! Get password mode. */
	bool getPassword() const { return m_password; }

	/*! Get caret position in text. */
	int32_t getCaret() const { return m_caret; }

	/*! Set scroll offset. */
	void setScroll(int32_t scroll);

	/*! Get scroll offset. */
	int32_t getScroll() const { return m_scroll; }

	/*! Get maximum scroll offset. */
	int32_t getMaxScroll() const;

	/*! Prepare a text layout.
	 *
	 * Preparing a text layout with edit field's font and attributes.
	 * Useful for custom layout of field's content.
	 *
	 * \return Prepared text layout.
	 */
	Ref< TextLayout > prepareTextLayout() const;

	/*! Override text layout.
	 *
	 * \note
	 * Layout is modified if text is changed in this field.
	 *
	 * \param layout Text layout.
	 */
	void setTextLayout(TextLayout* layout);

	/*! Get current text layout. */
	const TextLayout* getTextLayout() const { return m_layout; }

	/*! Set render clip mask enable. */
	void setRenderClipMask(bool renderClipMask);

	/*! Get render clip mask enable. */
	bool getRenderClipMask() const { return m_renderClipMask; }

	virtual Aabb2 getBounds() const override final;

	virtual void eventKey(wchar_t unicode) override final;

	virtual void eventMouseDown(int32_t x, int32_t y, int32_t button) override final;

	/*! \group Convenience methods, commonly used by scripting. */
	//@{

	void setPosition(const Vector2& position);

	Vector2 getPosition() const;

	void setX(float x);

	float getX() const;

	void setY(float y);

	float getY() const;

	void setSize(const Vector2& size);

	Vector2 getSize() const;

	void setWidth(float width);

	float getWidth() const;

	void setHeight(float height);

	float getHeight() const;

	void setRotation(float rotation);

	float getRotation() const;

	void setScale(const Vector2& scale);

	Vector2 getScale() const;

	void setXScale(float xscale);

	float getXScale() const;

	void setYScale(float yscale);

	float getYScale() const;

	Vector2 getTextSize() const;

	float getTextWidth() const;

	float getTextHeight() const;

	//@}

	/*! \group Events */
	//@{

	Event* getEventChanged() { return &m_eventChanged; }

	//@}

private:
	Ref< const Edit > m_edit;
	Aabb2 m_textBounds;
	Color4f m_textColor;
	float m_letterSpacing;
	SwfTextAlignType m_align;
	float m_fontHeight;
	std::wstring m_text;
	std::wstring m_htmlText;
	bool m_html;
	bool m_multiLine;
	bool m_wordWrap;
	bool m_password;
	int32_t m_caret;
	int32_t m_scroll;
	Ref< TextLayout > m_layout;
	bool m_renderClipMask;
	Event m_eventChanged;

	bool internalParseText(const std::wstring& text);

	bool internalParseHtml(const std::wstring& html);

	void updateLayout();
};

	}
}

