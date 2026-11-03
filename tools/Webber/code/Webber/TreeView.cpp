/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Html/Element.h"
#include "Webber/TreeView.h"
#include "Webber/TreeViewItem.h"

namespace traktor::wbr
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.TreeView", TreeView, Widget)

TreeViewItem* TreeView::addChild(TreeViewItem* child)
{
	m_children.push_back(child);
	return child;
}

void TreeView::removeChild(TreeViewItem* child)
{
	m_children.remove(child);
}

const RefArray< TreeViewItem >& TreeView::getChildren() const
{
	return m_children;
}

void TreeView::build(html::Element* parent) const
{
	auto eul = createElement(L"ul", L"wtk_treeview", parent);
	for (auto c : m_children)
		c->build(eul);
}

void TreeView::consume(int32_t senderId, int32_t action)
{
	// Iterate a copy; handlers may remove items.
	const RefArray< TreeViewItem > children = m_children;
	for (auto c : children)
		c->consume(senderId, action);
}

}
