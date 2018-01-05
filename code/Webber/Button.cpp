#include "Core/Misc/String.h"
#include "Html/Element.h"
#include "Html/Text.h"
#include "Webber/Button.h"

namespace traktor
{
	namespace wbr
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Button", Button, Widget)

Button::Button(const std::wstring& text)
:	m_text(text)
{
}

void Button::build(html::Element* parent) const
{
	auto ebutton = new html::Element(L"button");
	ebutton->setAttribute(L"id", L"_" + toString(getId()));
	ebutton->setAttribute(L"onclick", L"post_event(" + toString(getId()) + L", 1);");
	ebutton->addChild(new html::Text(m_text));
	parent->addChild(ebutton);
}

void Button::consume(int32_t senderId, int32_t action)
{
	if (senderId == getId())
		m_clicked.raise();
}

Event& Button::clicked()
{
	return m_clicked;
}

	}
}
