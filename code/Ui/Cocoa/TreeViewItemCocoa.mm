#include "Ui/Cocoa/TreeViewItemCocoa.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TreeViewItemCocoa", TreeViewItemCocoa, TreeViewItem)

TreeViewItemCocoa::TreeViewItemCocoa(TreeViewItemCocoa* parent)
:	m_image(-1)
,	m_expandedImage(-1)
,	m_expanded(false)
,	m_parent(parent)
{
}

TreeViewItemCocoa::~TreeViewItemCocoa()
{
}

void TreeViewItemCocoa::setText(const std::wstring& text)
{
	m_text = text;
}

std::wstring TreeViewItemCocoa::getText() const
{
	return m_text;
}

void TreeViewItemCocoa::setImage(int image)
{
	m_image = image;
}

int TreeViewItemCocoa::getImage() const
{
	return m_image;
}

void TreeViewItemCocoa::setExpandedImage(int expandedImage)
{
	m_expandedImage = expandedImage;
}

int TreeViewItemCocoa::getExpandedImage() const
{
	return m_expandedImage;
}

bool TreeViewItemCocoa::isExpanded() const
{
	return m_expanded;
}

void TreeViewItemCocoa::expand()
{
	m_expanded = true;
}

bool TreeViewItemCocoa::isCollapsed() const
{
	return !m_expanded;
}

void TreeViewItemCocoa::collapse()
{
	m_expanded = false;
}

bool TreeViewItemCocoa::isSelected() const
{
	return false;
}

void TreeViewItemCocoa::select()
{
}

bool TreeViewItemCocoa::edit()
{
	return false;
}

TreeViewItem* TreeViewItemCocoa::getParent() const
{
	return m_parent;
}

bool TreeViewItemCocoa::hasChildren() const
{
	return !m_children.empty();
}

int TreeViewItemCocoa::getChildren(RefArray< TreeViewItem >& outChildren) const
{
	for (RefArray< TreeViewItemCocoa >::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
		outChildren.push_back(*i);
	return int(outChildren.size());
}

	}
}
