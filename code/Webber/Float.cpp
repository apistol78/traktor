#include "Html/Element.h"
#include "Webber/Float.h"
#include "Webber/Widget.h"

namespace traktor
{
	namespace wbr
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Float", Float, ILayout);

void Float::build(const Widget* widget, const RefArray< Widget >& children, html::Element* parent) const
{
	auto ediv = new html::Element(L"div");
	ediv->setAttribute(L"class", L"wtk_layout_float");
	for (auto c : children)
		c->build(ediv);
	parent->addChild(ediv);
}

	}
}
