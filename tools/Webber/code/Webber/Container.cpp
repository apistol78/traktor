/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Html/Element.h"
#include "Webber/Container.h"
#include "Webber/ILayout.h"

namespace traktor::wbr
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Container", Container, Widget)

void Container::setLayout(ILayout* layout)
{
	m_layout = layout;
}

void Container::removeChild(Widget* child)
{
	m_children.remove(child);
}

void Container::removeAllChildren()
{
	m_children.clear();
}

const RefArray< Widget >& Container::getChildren() const
{
	return m_children;
}

void Container::build(html::Element* parent) const
{
	auto ediv = createElement(L"div", L"wtk_container", parent);
	if (m_layout)
		m_layout->build(this, m_children, ediv);
	else
	{
		for (auto c : m_children)
			c->build(ediv);
	}
}

void Container::consumeValues(const std::map< int32_t, std::wstring >& values)
{
	for (auto c : m_children)
		c->consumeValues(values);
}

void Container::consume(int32_t senderId, int32_t action)
{
	// Iterate a copy; handlers may replace children, keeps them alive until dispatched.
	const RefArray< Widget > children = m_children;
	for (auto c : children)
		c->consume(senderId, action);
}

}
