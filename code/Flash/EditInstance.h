/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_EditInstance_H
#define traktor_flash_EditInstance_H

#include "Flash/CharacterInstance.h"
#include "Flash/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class Edit;
class TextFormat;
class TextLayout;

/*! \brief Dynamic text character instance.
 * \ingroup Flash
 */
class T_DLLCLASS EditInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	EditInstance(ActionContext* context, Dictionary* dictionary, CharacterInstance* parent, const Edit* edit, const std::wstring& html);

	/*! \brief Get edit character. */
	const Edit* getEdit() const { return m_edit; }

	/*! \brief Parse text. */
	bool parseText(const std::wstring& text);

	/*! \brief Parse html formatted text. */
	bool parseHtml(const std::wstring& html);

	/*! \brief Measure text (without replacing) using edit field's format. */
	Vector2 measureText(const std::wstring& text) const;

	/*! \brief Set text bounds. */
	void setTextBounds(const Aabb2& textBounds);

	/*! \brief Get text bounds. */
	const Aabb2& getTextBounds() const { return m_textBounds; }

	/*! \brief Set text color. */
	void setTextColor(const Color4f& textColor);

	/*! \brief Get text color. */
	const Color4f& getTextColor() const { return m_textColor; }

	/*! \brief Set letter spacing. */
	void setLetterSpacing(float letterSpacing);

	/*! \brief Get letter spacing. */
	float getLetterSpacing() const { return m_letterSpacing; }

	/*! \brief Set text format. */
	void setTextFormat(const TextFormat* textFormat);

	/*! \brief Get text format. */
	Ref< TextFormat > getTextFormat() const;

	/*! \brief Set text format on text range. */
	void setTextFormat(const TextFormat* textFormat, int32_t beginIndex, int32_t endIndex);

	/*! \brief Get text format from text range. */
	Ref< TextFormat > getTextFormat(int32_t beginIndex, int32_t endIndex) const;

	/*! \brief Get text. */
	const std::wstring& getText() const { return m_text; }

	/*! \brief Get html formatted text. */
	const std::wstring& getHtmlText() const;

	/*! \brief Set word wrapping. */
	void setWordWrap(bool wordWrap);

	/*! \brief Get word wrapping. */
	bool getWordWrap() const { return m_wordWrap; }

	/*! \brief Set password mode. */
	void setPassword(bool password);

	/*! \brief Get password mode. */
	bool getPassword() const { return m_password; }

	/*! \brief Get caret position in text. */
	int32_t getCaret() const { return m_caret; }

	/*! \brief Set scroll offset. */
	void setScroll(int32_t scroll);

	/*! \brief Get scroll offset. */
	int32_t getScroll() const { return m_scroll; }

	/*! \brief Get maximum scroll offset. */
	int32_t getMaxScroll() const;

	/*! \brief Prepare a text layout.
	 *
	 * Preparing a text layout with edit field's font and attributes.
	 * Useful for custom layout of field's content.
	 *
	 * \return Prepared text layout.
	 */
	Ref< TextLayout > prepareTextLayout() const;

	/*! \brief Override text layout.
	 *
	 * \note
	 * Layout is modified if text is changed in this field.
	 *
	 * \param layout Text layout.
	 */
	void setTextLayout(TextLayout* layout);

	/*! \brief Get current text layout. */
	const TextLayout* getTextLayout() const { return m_layout; }

	/*! \brief Set render clip mask enable. */
	void setRenderClipMask(bool renderClipMask);

	/*! \brief Get render clip mask enable. */
	bool getRenderClipMask() const { return m_renderClipMask; }

	virtual Aabb2 getBounds() const T_OVERRIDE T_FINAL;

	virtual void eventKey(wchar_t unicode) T_OVERRIDE T_FINAL;

	virtual void eventMouseDown(int32_t x, int32_t y, int32_t button) T_OVERRIDE T_FINAL;

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

protected:
	virtual void trace(visitor_t visitor) const T_OVERRIDE;

	virtual void dereference() T_OVERRIDE;

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
	bool m_wordWrap;
	bool m_password;
	int32_t m_caret;
	int32_t m_scroll;
	Ref< TextLayout > m_layout;
	bool m_renderClipMask;

	bool internalParseText(const std::wstring& text);

	bool internalParseHtml(const std::wstring& html);

	void updateLayout();
};

	}
}

#endif	// traktor_flash_EditInstance_H
