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
	if (!ui::custom::AutoWidget::create(parent, ui::WsClientBorder | ui::WsDoubleBuffer))
		return false;

	setBackgroundColor(ui::getSystemColor(ui::ScButtonShadow));
	return true;
}

void GrainView::add(GrainViewItem* item)
{
	Ref< GrainViewCell > cell = new GrainViewCell(item);
	addCell(cell);
	requestLayout();
}

void GrainView::removeAll()
{
	removeAllCells();
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

	const RefArray< ui::custom::AutoWidgetCell >& cells = getCells();
	for (RefArray< ui::custom::AutoWidgetCell >::const_iterator i = cells.begin(); i != cells.end(); ++i)
	{
		GrainViewCell* cell = checked_type_cast< GrainViewCell*, false >(*i);

		uint32_t depth = 0;
		for (const GrainViewItem* item = cell->getItem(); item; item = item->getParent())
			++depth;

		ui::Rect cellRect = rowRect;
		cellRect.left += depth * 16;
		cellRect.right = cellRect.left + 128;

		cell->setRect(cellRect);

		rowRect = rowRect.offset(0, 32 + 8);
	}

	setClientSize(ui::Size(0, rowRect.bottom));
}

	}
}
