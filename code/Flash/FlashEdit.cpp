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
,	m_textColor(0.0f, 0.0f, 0.0f, 0.0f)
,	m_maxLength(std::numeric_limits< uint16_t >::max())
,	m_align(StaLeft)
,	m_leftMargin(0)
,	m_rightMargin(0)
,	m_indent(0)
,	m_leading(0)
,	m_readOnly(true)
,	m_wordWrap(false)
,	m_multiLine(false)
,	m_password(false)
,	m_renderHtml(false)
{
}

FlashEdit::FlashEdit(
	uint16_t id,
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
)
:	FlashCharacter(id)
,	m_fontId(fontId)
,	m_fontHeight(fontHeight)
,	m_textBounds(textBounds)
,	m_textColor(textColor)
,	m_initialText(initialText)
,	m_maxLength(maxLength)
,	m_align(align)
,	m_leftMargin(leftMargin)
,	m_rightMargin(rightMargin)
,	m_indent(indent)
,	m_leading(leading)
,	m_readOnly(readOnly)
,	m_wordWrap(wordWrap)
,	m_multiLine(multiLine)
,	m_password(password)
,	m_renderHtml(renderHtml)
{
}

Ref< FlashCharacterInstance > FlashEdit::createInstance(
	ActionContext* context,
	FlashDictionary* dictionary,
	FlashCharacterInstance* parent,
	const std::string& name,
	const Matrix33& transform,
	const ActionObject* initObject,
	const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
) const
{
	return new FlashEditInstance(context, dictionary, parent, this, m_initialText);
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

const Color4f& FlashEdit::getTextColor() const
{
	return m_textColor;
}

uint16_t FlashEdit::getMaxLength() const
{
	return m_maxLength;
}

void FlashEdit::setInitialText(const std::wstring& initialText)
{
	m_initialText = initialText;
}

const std::wstring& FlashEdit::getInitialText() const
{
	return m_initialText;
}

SwfTextAlignType FlashEdit::getAlign() const
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

bool FlashEdit::password() const
{
	return m_password;
}

bool FlashEdit::renderHtml() const
{
	return m_renderHtml;
}

void FlashEdit::serialize(ISerializer& s)
{
	const MemberEnum< SwfTextAlignType >::Key kAlign[] =
	{
		{ L"StaLeft", StaLeft },
		{ L"StaRight", StaRight },
		{ L"StaCenter", StaCenter },
		{ L"StaJustify", StaJustify },
		{ 0, 0 }
	};

	FlashCharacter::serialize(s);

	s >> Member< uint16_t >(L"fontId", m_fontId);
	s >> Member< uint16_t >(L"fontHeight", m_fontHeight);
	s >> MemberAabb2(L"textBounds", m_textBounds);
	s >> Member< Color4f >(L"textColor", m_textColor);
	s >> Member< uint16_t >(L"maxLength", m_maxLength);
	s >> Member< std::wstring >(L"initialText", m_initialText);
	s >> MemberEnum< SwfTextAlignType >(L"align", m_align, kAlign);
	s >> Member< uint16_t >(L"leftMargin", m_leftMargin);
	s >> Member< uint16_t >(L"rightMargin", m_rightMargin);
	s >> Member< int16_t >(L"indent", m_indent);
	s >> Member< int16_t >(L"leading", m_leading);
	s >> Member< bool >(L"readOnly", m_readOnly);
	s >> Member< bool >(L"wordWrap", m_wordWrap);
	s >> Member< bool >(L"multiLine", m_multiLine);
	s >> Member< bool >(L"password", m_password);
	s >> Member< bool >(L"renderHtml", m_renderHtml);
}

	}
}
