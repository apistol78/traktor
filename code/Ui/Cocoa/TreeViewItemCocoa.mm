#include "Ui/Cocoa/TreeViewItemCocoa.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TreeViewItemCocoa", TreeViewItemCocoa, TreeViewItem)

TreeViewItemCocoa::TreeViewItemCocoa()
{
}

TreeViewItemCocoa::~TreeViewItemCocoa()
{
}

void TreeViewItemCocoa::setText(const std::wstring& text)
{
}

std::wstring TreeViewItemCocoa::getText() const
{
	return L"";
}

void TreeViewItemCocoa::setImage(int image)
{
}

int TreeViewItemCocoa::getImage() const
{
	return 0;
}

void TreeViewItemCocoa::setExpandedImage(int expandedImage)
{
}

int TreeViewItemCocoa::getExpandedImage() const
{
	return 0;
}

bool TreeViewItemCocoa::isExpanded() const
{
	return false;
}

void TreeViewItemCocoa::expand()
{
}

bool TreeViewItemCocoa::isCollapsed() const
{
	return true;
}

void TreeViewItemCocoa::collapse()
{
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
	return 0;
}

bool TreeViewItemCocoa::hasChildren() const
{
	return false;
}

int TreeViewItemCocoa::getChildren(RefArray< TreeViewItem >& outChildren) const
{
	return 0;
}

	}
}
