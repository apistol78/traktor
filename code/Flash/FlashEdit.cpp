#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberEnum.h"
#include "Flash/FlashEdit.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/SwfMembers.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashEdit", 0, FlashEdit, FlashCharacter)

FlashEdit::FlashEdit()
:	m_fontId(0)
,	m_fontHeight(0)
,	m_maxLength(std::numeric_limits< uint16_t >::max())
,	m_align(AnLeft)
,	m_leftMargin(0)
,	m_rightMargin(0)
,	m_indent(0)
,	m_leading(0)
,	m_readOnly(true)
,	m_wordWrap(false)
,	m_multiLine(false)
,	m_renderHtml(false)
{
	m_textColor.red =
	m_textColor.green =
	m_textColor.blue =
	m_textColor.alpha = 0;
}

FlashEdit::FlashEdit(
	uint16_t id,
	uint16_t fontId,
	uint16_t fontHeight,
	const Aabb2& textBounds,
	const SwfColor& textColor,
	uint16_t maxLength,
	const std::wstring& initialText,
	Align align,
	uint16_t leftMargin,
	uint16_t rightMargin,
	int16_t indent,
	int16_t leading,
	bool readOnly,
	bool wordWrap,
	bool multiLine,
	bool renderHtml
)
:	FlashCharacter(id)
,	m_fontId(fontId)
,	m_fontHeight(fontHeight)
,	m_textBounds(textBounds)
,	m_textColor(textColor)
,	m_maxLength(maxLength)
,	m_initialText(initialText)
,	m_align(align)
,	m_leftMargin(leftMargin)
,	m_rightMargin(rightMargin)
,	m_indent(indent)
,	m_leading(leading)
,	m_readOnly(readOnly)
,	m_wordWrap(wordWrap)
,	m_multiLine(multiLine)
,	m_renderHtml(renderHtml)
{
}

Ref< FlashCharacterInstance > FlashEdit::createInstance(
	ActionContext* context,
	FlashCharacterInstance* parent,
	const std::string& name,
	const Matrix33& transform,
	const ActionObject* initObject,
	const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
) const
{
	return new FlashEditInstance(context, parent, this, m_initialText);
}

uint16_t FlashEdit::getFontId() const
{
	return m_fontId;
}

uint16_t FlashEdit::getFontHeight() const
{
	return m_fontHeight;
}

const Aabb2& FlashEdit::getTextBounds() const
{
	return m_textBounds;
}

const SwfColor& FlashEdit::getTextColor() const
{
	return m_textColor;
}

uint16_t FlashEdit::getMaxLength() const
{
	return m_maxLength;
}

const std::wstring& FlashEdit::getInitialText() const
{
	return m_initialText;
}

FlashEdit::Align FlashEdit::getAlign() const
{
	return m_align;
}

uint16_t FlashEdit::getLeftMargin() const
{
	return m_leftMargin;
}

uint16_t FlashEdit::getRightMargin() const
{
	return m_rightMargin;
}

int16_t FlashEdit::getIndent() const
{
	return m_indent;
}

int16_t FlashEdit::getLeading() const
{
	return m_leading;
}

bool FlashEdit::readOnly() const
{
	return m_readOnly;
}

bool FlashEdit::wordWrap() const
{
	return m_wordWrap;
}

bool FlashEdit::multiLine() const
{
	return m_multiLine;
}

bool FlashEdit::renderHtml() const
{
	return m_renderHtml;
}

void FlashEdit::serialize(ISerializer& s)
{
	const MemberEnum< Align >::Key kAlign[] =
	{
		{ L"AnLeft", AnLeft },
		{ L"AnRight", AnRight },
		{ L"AnCenter", AnCenter },
		{ L"AnJustify", AnJustify },
		{ 0, 0 }
	};

	FlashCharacter::serialize(s);

	s >> Member< uint16_t >(L"fontId", m_fontId);
	s >> Member< uint16_t >(L"fontHeight", m_fontHeight);
	s >> MemberAabb2(L"textBounds", m_textBounds);
	s >> MemberSwfColor(L"textColor", m_textColor);
	s >> Member< uint16_t >(L"maxLength", m_maxLength);
	s >> Member< std::wstring >(L"initialText", m_initialText);
	s >> MemberEnum< Align >(L"align", m_align, kAlign);
	s >> Member< uint16_t >(L"leftMargin", m_leftMargin);
	s >> Member< uint16_t >(L"rightMargin", m_rightMargin);
	s >> Member< int16_t >(L"indent", m_indent);
	s >> Member< int16_t >(L"leading", m_leading);
	s >> Member< bool >(L"readOnly", m_readOnly);
	s >> Member< bool >(L"wordWrap", m_wordWrap);
	s >> Member< bool >(L"multiLine", m_multiLine);
	s >> Member< bool >(L"renderHtml", m_renderHtml);
}

	}
}
