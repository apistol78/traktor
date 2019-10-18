#include "Ui/ToolBar/ToolBarItem.h"
#include "Ui/ToolBar/ToolBarItemGroup.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolBarItemGroup", ToolBarItemGroup, Object)

ToolBarItem* ToolBarItemGroup::addItem(ToolBarItem* item)
{
	m_items.push_back(item);
	return item;
}

void ToolBarItemGroup::removeItem(ToolBarItem* item)
{
	m_items.remove(item);
}

void ToolBarItemGroup::setEnable(bool enable)
{
	for (auto item : m_items)
		item->setEnable(enable);
}

	}
}
