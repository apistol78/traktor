/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Html/Element.h"
#include "Html/Text.h"
#include "Webber/TreeViewItem.h"

namespace traktor::wbr
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.TreeViewItem", TreeViewItem, Widget)

TreeViewItem::TreeViewItem(const std::wstring& text)
:	m_text(text)
{
}

void TreeViewItem::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& TreeViewItem::getText() const
{
	return m_text;
}

void TreeViewItem::setExpanded(bool expanded)
{
	m_expanded = expanded;
}

bool TreeViewItem::isExpanded() const
{
	return m_expanded;
}

TreeViewItem* TreeViewItem::addChild(TreeViewItem* child)
{
	m_children.push_back(child);
	return child;
}

void TreeViewItem::removeChild(TreeViewItem* child)
{
	m_children.remove(child);
}

const RefArray< TreeViewItem >& TreeViewItem::getChildren() const
{
	return m_children;
}

Event& TreeViewItem::clicked()
{
	return m_clicked;
}

void TreeViewItem::build(html::Element* parent) const
{
	auto eli = createElement(L"li", L"wtk_treeviewitem", parent);

	auto ediv = new html::Element(L"div");
	ediv->setAttribute(L"class", L"wtk_treeviewitemtext");
	ediv->setAttribute(L"onclick", L"post_event(" + toString(getId()) + L", 1);");
	ediv->addChild(new html::Text(m_text));
	eli->addChild(ediv);

	if (m_expanded && !m_children.empty())
	{
		auto eul = new html::Element(L"ul");
		eul->setAttribute(L"class", L"wtk_treeviewitemchildren");

		for (auto c : m_children)
			c->build(eul);

		eli->addChild(eul);
	}
}

void TreeViewItem::consume(int32_t senderId, int32_t action)
{
	if (senderId == getId())
		m_clicked.raise();
	else
	{
		// Iterate a copy; handlers may remove items.
		const RefArray< TreeViewItem > children = m_children;
		for (auto c : children)
			c->consume(senderId, action);
	}
}

}
