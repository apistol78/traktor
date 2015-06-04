#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/Auto/AutoWidget.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GridItem", GridItem, AutoWidgetCell)

GridItem::GridItem()
{
}

GridItem::GridItem(const std::wstring& text)
:	m_text(text)
{
}

GridItem::GridItem(const std::wstring& text, Font* font)
:	m_text(text)
,	m_font(font)
{
}

GridItem::GridItem(const std::wstring& text, Bitmap* image)
:	m_text(text)
,	m_image(image)
{
}

GridItem::GridItem(Bitmap* image)
:	m_image(image)
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

void GridItem::setFont(Font* font)
{
	m_font = font;
}

Font* GridItem::getFont() const
{
	return m_font;
}

void GridItem::setImage(Bitmap* image)
{
	m_image = image;
}

Bitmap* GridItem::getImage() const
{
	return m_image;
}

int32_t GridItem::getHeight() const
{
	int32_t height = 19;

	if (m_font)
		height = std::max(height, m_font->getSize() + 10);
	if (m_image)
		height = std::max(height, m_image->getSize().cy + 4);

	return height;
}

AutoWidgetCell* GridItem::hitTest(const Point& position)
{
	// Not allowed to pick items; entire row must be picked as selection
	// is handled by the GridView class.
	return 0;
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

		if (getWidget()->isEnable())
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
