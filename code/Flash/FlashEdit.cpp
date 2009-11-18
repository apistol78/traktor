#include "Flash/FlashEdit.h"
#include "Flash/FlashEditInstance.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashEdit", FlashEdit, FlashCharacter)

FlashEdit::FlashEdit(
	uint16_t id,
	uint16_t fontId,
	uint16_t fontHeight,
	const SwfRect& textBounds,
	const SwfColor& textColor,
	const std::wstring& initialText,
	Align align,
	uint16_t leftMargin,
	uint16_t rightMargin,
	bool renderHtml
)
:	FlashCharacter(id)
,	m_fontId(fontId)
,	m_fontHeight(fontHeight)
,	m_textBounds(textBounds)
,	m_textColor(textColor)
,	m_initialText(initialText)
,	m_align(align)
,	m_leftMargin(leftMargin)
,	m_rightMargin(rightMargin)
,	m_renderHtml(renderHtml)
{
}

Ref< FlashCharacterInstance > FlashEdit::createInstance(ActionContext* context, FlashCharacterInstance* parent) const
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

const SwfRect& FlashEdit::getTextBounds() const
{
	return m_textBounds;
}

const SwfColor& FlashEdit::getTextColor() const
{
	return m_textColor;
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

bool FlashEdit::renderHtml() const
{
	return m_renderHtml;
}

	}
}
