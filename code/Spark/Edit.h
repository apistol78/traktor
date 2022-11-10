/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Spark/Character.h"
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

#if defined (_MSC_VER)
#	pragma warning( disable:4324 )
#endif

/*! Dynamic text field.
 * \ingroup Spark
 */
class T_DLLCLASS Edit : public Character
{
	T_RTTI_CLASS;

public:
	Edit();

	explicit Edit(
		uint16_t fontId,
		uint16_t fontHeight,
		const Aabb2& textBounds,
		const Color4f& textColor,
		uint16_t maxLength,
		const std::wstring& initialText,
		SwfTextAlignType align,
		uint16_t leftMargin,
		uint16_t rightMargin,
		int16_t indent,
		int16_t leading,
		bool readOnly,
		bool wordWrap,
		bool multiLine,
		bool password,
		bool renderHtml
	);

	virtual Ref< CharacterInstance > createInstance(
		Context* context,
		Dictionary* dictionary,
		CharacterInstance* parent,
		const std::string& name,
		const Matrix33& transform
	) const override final;

	/*! Get font identifier.
	 *
	 * \return Font identifier.
	 */
	uint16_t getFontId() const;

	/*! Get font height.
	 *
	 * \return Font height.
	 */
	uint16_t getFontHeight() const;

	/*! Get text character bounding box.
	 *
	 * \return Bounding box.
	 */
	const Aabb2& getTextBounds() const;

	/*! Get text color.
	 *
	 * \return Text color.
	 */
	const Color4f& getTextColor() const;

	/*! Get max length of input text.
	 *
	 * \return Max length.
	 */
	uint16_t getMaxLength() const;

	/*! Set initial text string.
	 */
	void setInitialText(const std::wstring& initialText);

	/*! Initial text string.
	 *
	 * \return Initial text, can be HTML.
	 */
	const std::wstring& getInitialText() const;

	/*! Get text alignment within bounding box.
	 *
	 * \return Alignment.
	 */
	SwfTextAlignType getAlign() const;

	/*! Get left margin.
	 *
	 * \return Left margin.
	 */
	uint16_t getLeftMargin() const;

	/*! Get right margin.
	 *
	 * \return Right margin.
	 */
	uint16_t getRightMargin() const;

	/*! Get indent.
	 *
	 * \return Indent.
	 */
	int16_t getIndent() const;

	/*! Get leading.
	 *
	 * \return Leading.
	 */
	int16_t getLeading() const;

	/*! Read only.
	 *
	 * \return True if read-only.
	 */
	bool readOnly() const;

	/*! Word wrap enabled.
	 *
	 * \return True if word wrap is enabled.
	 */
	bool wordWrap() const;

	/*! Multiline text field.
	 *
	 * \return True if multiline.
	 */
	bool multiLine() const;

	/*! Password text field.
	 *
	 * \return True if password field.
	 */
	bool password() const;

	/*! Render HTML content.
	 *
	 * \return True if text is HTML.
	 */
	bool renderHtml() const;

	virtual void serialize(ISerializer& s) override final;

private:
	uint16_t m_fontId;
	uint16_t m_fontHeight;
	Aabb2 m_textBounds;
	Color4f m_textColor;
	std::wstring m_initialText;
	uint16_t m_maxLength;
	SwfTextAlignType m_align;
	uint16_t m_leftMargin;
	uint16_t m_rightMargin;
	int16_t m_indent;
	int16_t m_leading;
	bool m_readOnly;
	bool m_wordWrap;
	bool m_multiLine;
	bool m_password;
	bool m_renderHtml;
};

#if defined (_MSC_VER)
#	pragma warning( default:4324 )
#endif

	}
}

