#include "Ui/Application.h"
#include "World/Editor/PostProcess/PostProcessStepCell.h"
#include "World/Editor/PostProcess/PostProcessStepItem.h"
#include "World/Editor/PostProcess/PostProcessView.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessView", PostProcessView, ui::custom::AutoWidget)

bool PostProcessView::create(ui::Widget* parent)
{
	if (!ui::custom::AutoWidget::create(parent, ui::WsClientBorder | ui::WsDoubleBuffer))
		return false;

	setBackgroundColor(ui::getSystemColor(ui::ScButtonShadow));
	return true;
}

void PostProcessView::add(PostProcessStepItem* item)
{
	m_cells.push_back(new PostProcessStepCell(item));
	requestUpdate();
}

void PostProcessView::removeAll()
{
	m_cells.clear();
	requestUpdate();
}

void PostProcessView::layoutCells(const ui::Rect& rc)
{
	ui::Rect rowRect = rc;
	rowRect.top += 16;
	rowRect.bottom = rowRect.top + 32;

	for (RefArray< PostProcessStepCell >::const_iterator i = m_cells.begin(); i != m_cells.end(); ++i)
	{
		uint32_t depth = 0;
		for (const PostProcessStepItem* item = (*i)->getItem(); item; item = item->getParent())
			++depth;

		ui::Rect cellRect = rowRect;
		cellRect.left += 16 + depth * 16;
		cellRect.right = cellRect.left + 220;

		placeCell(*i, cellRect);

		rowRect = rowRect.offset(0, 32 + 8);
	}
}

	}
}
