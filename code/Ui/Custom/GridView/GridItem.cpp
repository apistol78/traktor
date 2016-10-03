#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/IBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/Auto/AutoWidget.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GridItem", GridItem, AutoWidgetCell)

GridItem::GridItem()
:	m_editMode(0)
{
}

GridItem::GridItem(const std::wstring& text)
:	m_text(text)
,	m_editMode(0)
{
}

GridItem::GridItem(const std::wstring& text, Font* font)
:	m_text(text)
,	m_font(font)
,	m_editMode(0)
{
}

GridItem::GridItem(const std::wstring& text, IBitmap* image)
:	m_text(text)
,	m_image(image)
,	m_editMode(0)
{
}

GridItem::GridItem(IBitmap* image)
:	m_image(image)
,	m_editMode(0)
{
}

void GridItem::setText(const std::wstring& text)
{
	m_text = text;
}

std::wstring GridItem::getText() const
{
	return m_text;
}

bool GridItem::edit()
{
	GridView* gridView = checked_type_cast< GridView*, false >(getWidget());
	gridView->beginEdit(this);
	return true;
}

void GridItem::setFont(Font* font)
{
	m_font = font;
}

Font* GridItem::getFont() const
{
	return m_font;
}

void GridItem::setImage(IBitmap* image)
{
	m_image = image;
}

IBitmap* GridItem::getImage() const
{
	return m_image;
}

int32_t GridItem::getHeight() const
{
	int32_t height = scaleBySystemDPI(19);

	if (m_font)
		height = std::max(height, m_font->getPixelSize() + scaleBySystemDPI(10));
	if (m_image)
		height = std::max(height, m_image->getSize().cy + scaleBySystemDPI(4));

	return height;
}

GridRow* GridItem::getRow() const
{
	return m_row;
}

AutoWidgetCell* GridItem::hitTest(const Point& position)
{
	// Not allowed to pick items; entire row must be picked as selection
	// is handled by the GridView class.
	return 0;
}

void GridItem::interval()
{
	// Cancel pending edit.
	if (m_editMode != 0)
		m_editMode = 0;
}

void GridItem::mouseDown(MouseButtonDownEvent* event, const Point& position)
{
	m_mouseDownPosition = position;

	if (true /*m_editable*/)
	{
		if (m_editMode == 0)
		{
			// Wait for next tap; cancel wait after 2 seconds.
			getWidget()->requestInterval(this, 2000);
			m_editMode = 1;
		}
		else if (m_editMode == 1)
		{
			// Double tap detected; begin edit after mouse is released.
			getWidget()->requestInterval(this, 1000);
			m_editMode = 2;
		}
	}

	getWidget()->requestUpdate();
}

void GridItem::mouseUp(MouseButtonUpEvent* event, const Point& position)
{
	if (m_editMode == 2)
	{
		//if (m_view->m_autoEdit)
		//	m_view->beginEdit(this);
		m_editMode = 0;
	}
}

void GridItem::mouseDoubleClick(MouseDoubleClickEvent* event, const Point& position)
{
	// Ensure edit isn't triggered.
	m_editMode = 0;

	//// Raise activation event.
	//TreeViewItemActivateEvent activateEvent(m_view, this);
	//m_view->raiseEvent(&activateEvent);
}

void GridItem::mouseMove(MouseMoveEvent* event, const Point& position)
{
	Size d = position - m_mouseDownPosition;
	if (abs(d.cx) > scaleBySystemDPI(2) || abs(d.cy) > scaleBySystemDPI(2))
	{
		// Ensure edit isn't triggered if mouse moved during edit state tracking.
		m_editMode = 0;
	}
}

void GridItem::paint(Canvas& canvas, const Rect& rect)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();

	Rect rcText(rect.left + 2, rect.top, rect.right, rect.bottom);

	if (m_image)
	{
		Size szImage = m_image->getSize();

		Point pntImage(
			rcText.left,
			rcText.top + (rcText.getHeight() - szImage.cy) / 2
		);
		if (m_text.empty())
			pntImage.x = rcText.left + (rcText.getWidth() - szImage.cx) / 2;

		canvas.drawBitmap(
			pntImage,
			Point(0, 0),
			szImage,
			m_image,
			BmAlpha
		);

		rcText.left += szImage.cx + 2;
	}

	if (!m_text.empty())
	{
		if (m_font)
			canvas.setFont(*m_font);

		if (getWidget()->isEnable() && getRow())
			canvas.setForeground(ss->getColor(getWidget(), (getRow()->getState() & GridRow::RsSelected) ? L"item-color-selected" : L"color"));
		else
			canvas.setForeground(ss->getColor(getWidget(), L"color-disabled"));

		canvas.drawText(rcText, m_text, AnLeft, AnCenter);

		if (m_font)
			canvas.setFont(getWidget()->getFont());
	}
}

		}
	}
}
