#include "Ui/Application.h"
#include "Ui/Custom/PreviewList/PreviewItem.h"
#include "Ui/Custom/PreviewList/PreviewItems.h"
#include "Ui/Custom/PreviewList/PreviewList.h"
#include "Ui/Custom/PreviewList/PreviewSelectionChangeEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int32_t c_marginX = 10;
const int32_t c_marginY = 10;
const int32_t c_itemWidth = 120;
const int32_t c_itemHeight = 100;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.PreviewList", PreviewList, AutoWidget)

bool PreviewList::create(Widget* parent, uint32_t style)
{
	if (!AutoWidget::create(parent, style))
		return false;

	setBackgroundColor(Color4ub(80, 80, 80));

	addEventHandler< MouseButtonDownEvent >(this, &PreviewList::eventButtonDown);
	return true;
}

void PreviewList::setItems(PreviewItems* items)
{
	if ((m_items = items) != 0)
	{
		for (int32_t i = 0; i < m_items->count(); ++i)
			m_items->get(i)->setSelected(false);
	}
	requestUpdate();
}

Ref< PreviewItems > PreviewList::getItems() const
{
	return m_items;
}

PreviewItem* PreviewList::getSelectedItem() const
{
	if (!m_items)
		return 0;
		
	for (int32_t i = 0; i < m_items->count(); ++i)
	{
		if (m_items->get(i)->isSelected())
			return m_items->get(i);
	}
	
	return 0;
}

void PreviewList::layoutCells(const Rect& rc)
{
	int32_t nitems = m_items ? m_items->count() : 0;
	if (nitems <= 0)
		return;

	int32_t ncolumns = (rc.getWidth() - c_marginX * 2) / c_itemWidth;
	if (ncolumns <= 0)
		return;

	// First layout non selected.
	for (int32_t i = 0; i < nitems; ++i)
	{
		if (m_items->get(i)->isSelected())
			continue;

		int32_t column = i % ncolumns;
		int32_t row = i / ncolumns;

		Rect rcItem(
			c_marginX + column * c_itemWidth,
			c_marginY + row * c_itemHeight,
			c_marginX + column * c_itemWidth + c_itemWidth,
			c_marginY + row * c_itemHeight + c_itemHeight
		);

		placeCell(m_items->get(i), rcItem);
	}

	// Then layout selected items.
	for (int32_t i = 0; i < nitems; ++i)
	{
		if (!m_items->get(i)->isSelected())
			continue;

		int32_t column = i % ncolumns;
		int32_t row = i / ncolumns;

		Rect rcItem(
			c_marginX + column * c_itemWidth,
			c_marginY + row * c_itemHeight,
			c_marginX + column * c_itemWidth + c_itemWidth,
			c_marginY + row * c_itemHeight + c_itemHeight
		);

		placeCell(m_items->get(i), rcItem);
	}
}

void PreviewList::eventButtonDown(MouseButtonDownEvent* event)
{
	if (event->getButton() != MbtLeft)
		return;

	const Point& position = event->getPosition();
	if (m_items)
	{
		for (int32_t i = 0; i < m_items->count(); ++i)
			m_items->get(i)->setSelected(false);

		AutoWidgetCell* cell = hitTest(position);
		if (PreviewItem* item = dynamic_type_cast< PreviewItem* >(cell))
		{
			item->setSelected(true);

			PreviewSelectionChangeEvent selectionChangeEvent(this, item);
			raiseEvent(&selectionChangeEvent);
		}
		else
		{
			PreviewSelectionChangeEvent selectionChangeEvent(this, 0);
			raiseEvent(&selectionChangeEvent);
		}
	}

	requestUpdate();
}

		}
	}
}
