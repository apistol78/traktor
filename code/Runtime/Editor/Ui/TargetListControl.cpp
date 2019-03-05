#include "Runtime/Editor/Ui/TargetInstanceListItem.h"
#include "Runtime/Editor/Ui/TargetListControl.h"
#include "Ui/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetListControl", TargetListControl, ui::AutoWidget)

bool TargetListControl::create(ui::Widget* parent)
{
	if (!ui::AutoWidget::create(parent, ui::WsDoubleBuffer))
		return false;

	return true;
}

void TargetListControl::add(TargetInstanceListItem* item)
{
	m_items.push_back(item);
	requestUpdate();
}

void TargetListControl::removeAll()
{
	m_items.resize(0);
	requestUpdate();
}

void TargetListControl::layoutCells(const ui::Rect& rc)
{
	ui::Rect targetRect = rc;
	for (RefArray< TargetInstanceListItem >::const_iterator i = m_items.begin(); i != m_items.end(); ++i)
	{
		ui::Size itemSize = (*i)->getSize();

		targetRect.bottom = targetRect.top + itemSize.cy;
		placeCell(*i, targetRect);

		targetRect.top = targetRect.bottom;
	}
}

	}
}
