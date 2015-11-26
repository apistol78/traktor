#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Thread/Acquire.h"
#include "Flash/FlashDictionary.h"
#include "Flash/FlashEdit.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/FlashFont.h"
#include "Flash/FlashTextFormat.h"
#include "Flash/TextLayout.h"
#include "Flash/Action/ActionContext.h"
#include "Html/Attribute.h"
#include "Html/Document.h"
#include "Html/Element.h"
#include "Html/Text.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const struct { const wchar_t* name; SwfColor color; } c_colorTable[] =
{
	L"black",	{ 0x00, 0x00, 0x00, 0xff },
	L"red",		{ 0xff, 0x00, 0x00, 0xff },
	L"green",	{ 0x00, 0xff, 0x00, 0xff },
	L"blue",	{ 0x00, 0x00, 0xff, 0xff },
	L"yellow",	{ 0xff, 0xff, 0x00, 0xff },
	L"white",	{ 0xff, 0xff, 0xff, 0xff },
	L"lime",	{ 0x40, 0x80, 0xff, 0xff }
};

bool parseColor(const std::wstring& color, SwfColor& outColor)
{
	if (startsWith< std::wstring >(color, L"#"))
	{
		int32_t red, green, blue;
		swscanf(color.c_str(), L"#%02x%02x%02x", &red, &green, &blue);
		outColor.red = uint8_t(red);
		outColor.green = uint8_t(green);
		outColor.blue = uint8_t(blue);
		outColor.alpha = 0xff;
		return true;
	}
	else if (startsWith< std::wstring >(color, L"rgb"))
	{
		int32_t red, green, blue;
		swscanf(color.c_str(), L"rgb(%d,%d,%d)", &red, &green, &blue);
		outColor.red = uint8_t(red);
		outColor.green = uint8_t(green);
		outColor.blue = uint8_t(blue);
		outColor.alpha = 0xff;
		return true;
	}
	else if (toLower(color) == L"none")
		return false;

	for (int32_t i = 0; i < sizeof_array(c_colorTable); ++i)
	{
		if (toLower(color) == c_colorTable[i].name)
		{
			outColor = c_colorTable[i].color;
			return true;
		}
	}

	return false;
}

void traverseHtmlDOM(const html::Element* element, const FlashFont* font, const SwfColor& textColor, TextLayout* layout, StringOutputStream& text)
{
	SwfColor color = textColor;

	if (element->getName() == L"font")
	{
		Ref< const html::Attribute > colorAttribute = element->getAttribute(L"color");
		if (colorAttribute)
		{
			if (!parseColor(colorAttribute->getValue(), color))
				color = textColor;
		}
	}
	else if (element->getName() == L"br")
	{
		layout->newLine();
		text << Endl;
	}

	layout->setAttribute(font, color);

	for (const html::Node* child = element->getFirstChild(); child; child = child->getNextSibling())
	{
		if (const html::Element* childElement = dynamic_type_cast< const html::Element* >(child))
			traverseHtmlDOM(childElement, font, color, layout, text);
		else
		{
			layout->insertText(child->getValue());
			text << child->getValue();
		}
	}

	if (element->getName() == L"p")
		layout->newLine();

	layout->setAttribute(font, textColor);
}

Aabb2 adjustForGutter(const Aabb2& aabb)
{
	return Aabb2(
		Vector2(
			aabb.mn.x + 2.0f * 20.0f,
			aabb.mn.y + 2.0f * 20.0f
		),
		Vector2(
			aabb.mx.x - 2.0f * 20.0f,
			aabb.mx.y - 2.0f * 20.0f
		)
	);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashEditInstance", FlashEditInstance, FlashCharacterInstance)

FlashEditInstance::FlashEditInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashEdit* edit, const std::wstring& html)
:	FlashCharacterInstance(context, "TextField", parent)
,	m_edit(edit)
,	m_textColor(edit->getTextColor())
,	m_letterSpacing(0.0f)
,	m_align(edit->getAlign())
,	m_fontHeight(edit->getFontHeight())
,	m_html(false)
,	m_caret(0)
,	m_scroll(0)
,	m_layout(new TextLayout())
{
	if (m_edit->renderHtml())
		parseHtml(html);
	else
		parseText(html);
}

const FlashEdit* FlashEditInstance::getEdit() const
{
	return m_edit;
}

bool FlashEditInstance::parseText(const std::wstring& text)
{
	if (!internalParseText(text))
		return false;
	
	m_caret = 0;

	if (m_layout)
	{
		const AlignedVector< TextLayout::Line >& lines = m_layout->getLines();
		for (AlignedVector< TextLayout::Line >::const_iterator i = lines.begin(); i != lines.end(); ++i)
		{
			const AlignedVector< TextLayout::Word >& words = lines.back().words;
			for (AlignedVector< TextLayout::Word >::const_iterator j = words.begin(); j != words.end(); ++j)
				m_caret += j->chars.size();
		}
	}

	return true;
}

bool FlashEditInstance::parseHtml(const std::wstring& html)
{
	if (!internalParseHtml(html))
		return false;

	m_caret = 0;

	if (m_layout)
	{
		const AlignedVector< TextLayout::Line >& lines = m_layout->getLines();
		for (AlignedVector< TextLayout::Line >::const_iterator i = lines.begin(); i != lines.end(); ++i)
		{
			const AlignedVector< TextLayout::Word >& words = lines.back().words;
			for (AlignedVector< TextLayout::Word >::const_iterator j = words.begin(); j != words.end(); ++j)
				m_caret += j->chars.size();
		}
	}

	return true;
}

const SwfColor& FlashEditInstance::getTextColor() const
{
	return m_textColor;
}

void FlashEditInstance::setTextColor(const SwfColor& textColor)
{
	m_textColor = textColor;
	updateLayout();
}

float FlashEditInstance::getLetterSpacing() const
{
	return m_letterSpacing;
}

void FlashEditInstance::setLetterSpacing(float letterSpacing)
{
	m_letterSpacing = letterSpacing;
	updateLayout();
}

Ref< FlashTextFormat > FlashEditInstance::getTextFormat() const
{
	return new FlashTextFormat(m_letterSpacing, m_align, m_fontHeight);
}

Ref< FlashTextFormat > FlashEditInstance::getTextFormat(int32_t beginIndex, int32_t endIndex) const
{
	return new FlashTextFormat(m_letterSpacing, m_align, m_fontHeight);
}

void FlashEditInstance::setTextFormat(const FlashTextFormat* textFormat)
{
	m_letterSpacing = textFormat->getLetterSpacing();
	m_align = textFormat->getAlign();
	m_fontHeight = textFormat->getSize();
	updateLayout();
}

void FlashEditInstance::setTextFormat(const FlashTextFormat* textFormat, int32_t beginIndex, int32_t endIndex)
{
	m_letterSpacing = textFormat->getLetterSpacing();
	m_align = textFormat->getAlign();
	m_fontHeight = textFormat->getSize();
	updateLayout();
}

std::wstring FlashEditInstance::getText() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return m_text;
}

std::wstring FlashEditInstance::getHtmlText() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return m_html ? m_htmlText : m_text;
}

int32_t FlashEditInstance::getCaret() const
{
	return m_caret;
}

void FlashEditInstance::setScroll(int32_t scroll)
{
	m_scroll = scroll;
}

int32_t FlashEditInstance::getScroll() const
{
	return m_scroll;
}

int32_t FlashEditInstance::getMaxScroll() const
{
	if (m_layout)
	{
		const AlignedVector< TextLayout::Line >& lines = m_layout->getLines();
		Aabb2 textBounds = m_edit->getTextBounds();

		float lineHeight = m_layout->getFontHeight() + m_layout->getLeading();
		float editHeight = textBounds.mx.y - textBounds.mn.y;

		int maxScroll = lines.size() - int32_t(editHeight / lineHeight + 0.5f);
		return std::max(maxScroll, 0);
	}
	else
		return 0;
}

const TextLayout* FlashEditInstance::getTextLayout() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return m_layout;
}

Aabb2 FlashEditInstance::getBounds() const
{
	return getTransform() * m_edit->getTextBounds();
}

void FlashEditInstance::eventKey(wchar_t unicode)
{
	if (getContext()->getFocus() != this)
		return;

	if (unicode == L'\n' || unicode == L'\r' || unicode == L'\t')
		return;

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (m_html)
		m_text = L"";

	if (unicode == L'\b')
	{
		if (!m_text.empty())
		{
			m_text = m_text.substr(0, m_text.length() - 1);
			m_caret--;
		}
	}
	else if (m_text.length() < m_edit->getMaxLength())
	{
		m_text += unicode;
		m_caret++;
	}
	
	internalParseText(m_text);

	executeScriptEvent(ActionContext::IdOnChanged, ActionValue(getAsObject()));
}

void FlashEditInstance::eventMouseDown(int32_t x, int32_t y, int32_t button)
{
	if (!m_edit->readOnly())
	{
		Vector2 xy = getFullTransform().inverse() * Vector2(float(x), float(y));
		Aabb2 bounds = m_edit->getTextBounds();

		bool inside = (xy.x >= bounds.mn.x && xy.y >= bounds.mn.y && xy.x <= bounds.mx.x && xy.y <= bounds.mx.y);
		if (inside)
			getContext()->setFocus(this);
		else
			getContext()->setFocus(0);
	}

	FlashCharacterInstance::eventMouseDown(x, y, button);
}

bool FlashEditInstance::internalParseText(const std::wstring& text)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	
	const FlashDictionary* dictionary = getContext()->getDictionary();
	T_ASSERT (dictionary);

	const FlashFont* font = dictionary->getFont(m_edit->getFontId());
	T_ASSERT (font);

	m_layout->begin();

	m_layout->setBounds(adjustForGutter(m_edit->getTextBounds()));
	m_layout->setLeading(m_edit->getLeading());
	m_layout->setLetterSpacing(m_letterSpacing);
	m_layout->setFontHeight(m_fontHeight);
	m_layout->setWordWrap(m_edit->wordWrap());
	m_layout->setAlignment(m_align);
	m_layout->setAttribute(font, m_textColor);

	if (m_edit->multiLine())
	{
		StringSplit< std::wstring > ss(text, L"\n");
		for (StringSplit< std::wstring >::const_iterator i = ss.begin(); i != ss.end(); ++i)
		{
			m_layout->insertText(*i);
			m_layout->newLine();
		}
	}
	else
		m_layout->insertText(text);
	
	m_layout->end();

	m_text = text;
	m_htmlText.clear();
	m_html = false;

	return true;
}

bool FlashEditInstance::internalParseHtml(const std::wstring& html)
{
	html::Document document(false);
	if (!document.loadFromText(html))
		return false;

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	const FlashDictionary* dictionary = getContext()->getDictionary();
	T_ASSERT (dictionary);

	const FlashFont* font = dictionary->getFont(m_edit->getFontId());
	T_ASSERT (font);

	m_layout->begin();

	m_layout->setBounds(adjustForGutter(m_edit->getTextBounds()));
	m_layout->setLeading(m_edit->getLeading());
	m_layout->setLetterSpacing(m_letterSpacing);
	m_layout->setFontHeight(m_fontHeight);
	m_layout->setWordWrap(m_edit->wordWrap());
	m_layout->setAlignment(m_align);
	m_layout->setAttribute(font, m_textColor);

	const html::Element* element = document.getDocumentElement();
	T_ASSERT (element);

	StringOutputStream text;
	traverseHtmlDOM(element, font, m_textColor, m_layout, text);

	m_layout->end();

	m_text = text.str();
	m_htmlText = html;
	m_html = true;

	return true;
}

void FlashEditInstance::updateLayout()
{
	if (m_html)
		internalParseHtml(m_text);
	else
		internalParseText(m_text);
}

	}
}
