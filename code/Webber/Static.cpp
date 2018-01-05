#include "Core/Misc/String.h"
#include "Html/Element.h"
#include "Html/Text.h"
#include "Webber/Static.h"

namespace traktor
{
	namespace wbr
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Static", Static, Widget)

Static::Static(const std::wstring& text)
:	m_text(text)
{
}

void Static::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& Static::getText() const
{
	return m_text;
}

void Static::build(html::Element* parent) const
{
	auto ediv = new html::Element(L"div");
	ediv->setAttribute(L"class", L"wtk_static");
	ediv->setAttribute(L"id", L"_" + toString(getId()));
	ediv->addChild(new html::Text(m_text));
	parent->addChild(ediv);
}

void Static::consume(int32_t senderId, int32_t action)
{
}

	}
}
