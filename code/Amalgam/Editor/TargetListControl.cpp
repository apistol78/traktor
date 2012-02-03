#include "Amalgam/Editor/TargetCell.h"
#include "Amalgam/Editor/TargetListControl.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/Auto/AutoWidgetCell.h"

// Resources
#include "Resources/TargetControl.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetListControl", TargetListControl, ui::custom::AutoWidget)

TargetListControl::TargetListControl(HostEnumerator* hostEnumerator)
:	m_hostEnumerator(hostEnumerator)
{
}

bool TargetListControl::create(ui::Widget* parent)
{
	if (!ui::custom::AutoWidget::create(parent, ui::WsDoubleBuffer))
		return false;

	m_bitmapTargetControl = ui::Bitmap::load(c_ResourceTargetControl, sizeof(c_ResourceTargetControl), L"png");
	return true;
}

void TargetListControl::add(TargetInstance* instance)
{
	m_cells.push_back(new TargetCell(m_bitmapTargetControl, m_hostEnumerator, instance));
	requestLayout();
}

void TargetListControl::removeAll()
{
	m_cells.resize(0);
	requestLayout();
}

void TargetListControl::addPlayEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(ui::EiUser + 1, eventHandler);
}

void TargetListControl::addStopEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(ui::EiUser + 2, eventHandler);
}

void TargetListControl::layoutCells(const ui::Rect& rc)
{
	ui::Rect targetRect = rc;
	for (RefArray< TargetCell >::const_iterator i = m_cells.begin(); i != m_cells.end(); ++i)
	{
		targetRect.bottom = targetRect.top + (*i)->getHeight();
		placeCell(*i, targetRect);
		targetRect.top = targetRect.bottom;
	}
}

	}
}
