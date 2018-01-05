#include "Core/Misc/String.h"
#include "Html/Element.h"
#include "Webber/Container.h"
#include "Webber/ILayout.h"

namespace traktor
{
	namespace wbr
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Container", Container, Widget)

void Container::setLayout(ILayout* layout)
{
	m_layout = layout;
}

void Container::removeChild(Widget* child)
{
}

const RefArray< Widget >& Container::getChildren() const
{
	return m_children;
}

void Container::build(html::Element* parent) const
{
	auto ediv = new html::Element(L"div");
	ediv->setAttribute(L"class", L"wtk_container");
	ediv->setAttribute(L"id", L"_" + toString(getId()));

	if (m_layout)
		m_layout->build(this, m_children, ediv);
	else
	{
		for (auto c : m_children)
			c->build(ediv);
	}

	parent->addChild(ediv);
}

void Container::consume(int32_t senderId, int32_t action)
{
	for (auto c : m_children)
		c->consume(senderId, action);
}

	}
}
