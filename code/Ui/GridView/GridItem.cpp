#include <algorithm>
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/IBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Auto/AutoWidget.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridItem", GridItem, AutoWidgetCell)

GridItem::GridItem()
:	m_row(nullptr)
{
}

GridItem::GridItem(const std::wstring& text)
:	m_row(nullptr)
,	m_text(text)
{
}

GridItem::GridItem(const std::wstring& text, Font* font)
:	m_row(nullptr)
,	m_text(text)
,	m_font(font)
{
}

GridItem::GridItem(const std::wstring& text, IBitmap* image)
:	m_row(nullptr)
,	m_text(text)
,	m_image(image)
{
}

GridItem::GridItem(IBitmap* image)
:	m_row(nullptr)
,	m_image(image)
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
	getWidget< GridView >()->beginEdit(this);
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

int32_t GridItem::getHeight()
{
	int32_t height = dpi96(19);

	if (m_font)
	{
		int32_t lines = std::max< int32_t >(1, (int32_t)std::count(m_text.begin(), m_text.end(), L'\n'));
		height = std::max(height, lines * m_font->getPixelSize() + dpi96(10));
	}
	else if (getWidget< GridView >())
		height = std::max(height, getWidget< GridView >()->getFontMetric().getHeight());

	if (m_image)
		height = std::max(height, m_image->getSize().cy + dpi96(4));

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
	return nullptr;
}

void GridItem::paint(Canvas& canvas, const Rect& rect)
{
	const StyleSheet* ss = getWidget< GridView >()->getStyleSheet();

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

		if (getWidget< GridView >()->isEnable() && getRow())
			canvas.setForeground(ss->getColor(this, (getRow()->getState() & GridRow::RsSelected) ? L"color-selected" : L"color"));
		else
			canvas.setForeground(ss->getColor(this, L"color-disabled"));

		canvas.setClipRect(rcText);
		canvas.drawText(rcText, m_text, AnLeft, AnCenter);
		canvas.resetClipRect();

		if (m_font)
			canvas.setFont(getWidget< GridView >()->getFont());
	}
}

	}
}
