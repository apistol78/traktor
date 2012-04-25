#include "Ui/Application.h"
#include "World/Editor/PostProcess/PostProcessDefineCell.h"
#include "World/Editor/PostProcess/PostProcessDefineItem.h"
#include "World/Editor/PostProcess/PostProcessDefineView.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessDefineView", PostProcessDefineView, ui::custom::AutoWidget)

bool PostProcessDefineView::create(ui::Widget* parent)
{
	if (!ui::custom::AutoWidget::create(parent, ui::WsClientBorder | ui::WsDoubleBuffer))
		return false;

	setBackgroundColor(ui::getSystemColor(ui::ScButtonShadow));
	return true;
}

void PostProcessDefineView::add(PostProcessDefineItem* item)
{
	m_cells.push_back(new PostProcessDefineCell(item));
	requestUpdate();
}

void PostProcessDefineView::removeAll()
{
	m_cells.clear();
	requestUpdate();
}

void PostProcessDefineView::layoutCells(const ui::Rect& rc)
{
	ui::Rect columnRect = rc;
	columnRect.top += 16;
	columnRect.bottom = columnRect.top + 32;

	for (RefArray< PostProcessDefineCell >::const_iterator i = m_cells.begin(); i != m_cells.end(); ++i)
	{
		ui::Rect itemRect = columnRect;
		itemRect.setSize(ui::Size(64, 64));
		placeCell(*i, itemRect);
		columnRect = columnRect.offset(64 + 16, 0);
	}
}

	}
}
