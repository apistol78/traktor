#include "Core/Misc/String.h"
#include "Html/Element.h"
#include "Webber/TreeView.h"
#include "Webber/TreeViewItem.h"

namespace traktor
{
	namespace wbr
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.TreeView", TreeView, Widget)

TreeView::TreeView()
{
}

TreeViewItem* TreeView::addChild(TreeViewItem* child)
{
	m_children.push_back(child);
	return child;
}

void TreeView::removeChild(TreeViewItem* child)
{
}

const RefArray< TreeViewItem >& TreeView::getChildren() const
{
	return m_children;
}

void TreeView::build(html::Element* parent) const
{
	auto eul = new html::Element(L"ul");
	eul->setAttribute(L"class", L"wtk_treeview");
	eul->setAttribute(L"id", L"_" + toString(getId()));

	for (auto c : m_children)
		c->build(eul);

	parent->addChild(eul);
}

void TreeView::consume(int32_t senderId, int32_t action)
{
	for (auto c : m_children)
		c->consume(senderId, action);
}

	}
}
