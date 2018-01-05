#include "Html/Element.h"
#include "Webber/Splitter.h"
#include "Webber/Widget.h"

namespace traktor
{
	namespace wbr
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Splitter", Splitter, ILayout)

Splitter::Splitter(Direction direction)
:	m_direction(direction)
{
}

void Splitter::build(const Widget* widget, const RefArray< Widget >& children, html::Element* parent) const
{
	if (children.size() < 2)
		return;

	if (m_direction == Horizontal)
	{
		auto ediv1 = new html::Element(L"div");
		ediv1->setAttribute(L"class", L"wtk_layout_splitter_top");
		children[0]->build(ediv1);
		parent->addChild(ediv1);

		auto ediv2 = new html::Element(L"div");
		ediv2->setAttribute(L"class", L"wtk_layout_splitter_bottom");
		children[1]->build(ediv2);
		parent->addChild(ediv2);
	}
	else if (m_direction == Vertical)
	{
		auto ediv1 = new html::Element(L"div");
		ediv1->setAttribute(L"class", L"wtk_layout_splitter_left");
		children[0]->build(ediv1);
		parent->addChild(ediv1);

		auto ediv2 = new html::Element(L"div");
		ediv2->setAttribute(L"class", L"wtk_layout_splitter_right");
		children[1]->build(ediv2);
		parent->addChild(ediv2);
	}
}

	}
}
