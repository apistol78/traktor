#include "Core/Misc/String.h"
#include "Html/Element.h"
#include "Html/Text.h"
#include "Webber/TextArea.h"
#include "Webber/Utilities.h"

namespace traktor
{
	namespace wbr
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.TextArea", TextArea, Widget)

TextArea::TextArea()
{
}

void TextArea::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& TextArea::getText() const
{
	return m_text;
}

void TextArea::build(html::Element* parent) const
{
	auto ecode = new html::Element(L"div");
	ecode->setAttribute(L"class", L"wtk_textarea");
	ecode->setAttribute(L"id", L"_" + toString(getId()));
	//ecode->addChild(new html::Text(escapeHtml(m_text)));
	ecode->addChild(new html::Text(m_text));
	parent->addChild(ecode);

	auto es2 = new html::Element(L"script");
	es2->addChild(new html::Text(L"init_ace_textarea(" + toString(getId()) + L");"));
	parent->addChild(es2);
}

void TextArea::consume(int32_t senderId, int32_t action)
{
}

	}
}
