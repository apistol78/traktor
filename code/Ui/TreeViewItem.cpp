/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Ui/TreeViewItem.h"
#include "Core/Misc/Split.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TreeViewItem", TreeViewItem, Object)

Ref< TreeViewItem > TreeViewItem::findChild(const std::wstring& childPath)
{
	std::vector< std::wstring > childNames;
	if (Split< std::wstring >::any(childPath, L"/", childNames) <= 0)
		return 0;

	Ref< TreeViewItem > item = this;
	for (std::vector< std::wstring >::iterator i = childNames.begin(); i != childNames.end(); ++i)
	{
		RefArray< TreeViewItem > children;
		item->getChildren(children);

		Ref< TreeViewItem > next;
		for (RefArray< TreeViewItem >::iterator j = children.begin(); j != children.end(); ++j)
		{
			if ((*j)->getText() == *i)
			{
				next = *j;
				break;
			}
		}

		if (!(item = next))
			return 0;
	}

	return item;
}

std::wstring TreeViewItem::getPath() const
{
	return getParent() ? getParent()->getPath() + L'/' + getText() : L"";
}

	}
}

