#include "Ui/Application.h"
#include "Sound/Editor/Resound/GrainView.h"
#include "Sound/Editor/Resound/GrainViewCell.h"
#include "Sound/Editor/Resound/GrainViewItem.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.GrainView", GrainView, ui::custom::AutoWidget)

bool GrainView::create(ui::Widget* parent)
{
	if (!ui::custom::AutoWidget::create(parent, ui::WsDoubleBuffer))
		return false;

	setBackgroundColor(ui::getSystemColor(ui::ScButtonShadow));
	return true;
}

void GrainView::add(GrainViewItem* item)
{
	m_cells.push_back(new GrainViewCell(item));
	requestUpdate();
}

void GrainView::removeAll()
{
	m_cells.clear();
	requestUpdate();
}

GrainViewItem* GrainView::getSelected() const
{
	GrainViewCell* cell = checked_type_cast< GrainViewCell* >(getFocusCell());
	return cell ? cell->getItem() : 0;
}

void GrainView::layoutCells(const ui::Rect& rc)
{
	ui::Rect rowRect = rc;
	rowRect.top += 16;
	rowRect.bottom = rowRect.top + 32;

	for (RefArray< GrainViewCell >::const_iterator i = m_cells.begin(); i != m_cells.end(); ++i)
	{
		uint32_t depth = 0;
		for (const GrainViewItem* item = (*i)->getItem(); item; item = item->getParent())
			++depth;

		ui::Rect cellRect = rowRect;
		cellRect.left += depth * 16;
		cellRect.right = cellRect.left + 128;
		placeCell(*i, cellRect);

		rowRect = rowRect.offset(0, 32 + 8);
	}
}

	}
}
