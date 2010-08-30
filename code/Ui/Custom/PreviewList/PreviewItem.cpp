#include "Ui/Bitmap.h"
#include "Ui/Canvas.h"
#include "Ui/Custom/PreviewList/PreviewItem.h"

// Resources
#include "Resources/PreviewFrame.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.PreviewItem", PreviewItem, AutoWidgetCell)

PreviewItem::PreviewItem(const std::wstring& text)
:	m_text(text)
,	m_selected(false)
{
	m_bitmapFrame = ui::Bitmap::load(c_ResourcePreviewFrame, sizeof(c_ResourcePreviewFrame), L"png");
}

void PreviewItem::setImage(ui::Bitmap* image)
{
	m_bitmapImage = image;
}

ui::Bitmap* PreviewItem::getImage() const
{
	return m_bitmapImage;
}

void PreviewItem::setSelected(bool selected)
{
	m_selected = selected;
}

bool PreviewItem::isSelected() const
{
	return m_selected;
}

void PreviewItem::paint(AutoWidget* widget, Canvas& canvas, const Rect& rect)
{
	Size frameSize(74, 74); // = m_bitmapFrame->getSize();
	Point framePosition(rect.left + (rect.getWidth() - frameSize.cx) / 2, rect.top);

	canvas.drawBitmap(
		framePosition,
		Point(m_selected ? 74 : 0, 0),
		frameSize,
		m_bitmapFrame,
		BmAlpha
	);

	if (m_bitmapImage)
	{
		Size thumbSize = m_bitmapImage->getSize();
		Point thumbPosition(
			framePosition.x + 5,
			framePosition.y + 5
		);

		canvas.drawBitmap(
			thumbPosition,
			Point(0, 0),
			thumbSize,
			m_bitmapImage,
			BmAlpha
		);
	}

	canvas.setForeground(Color(255, 255, 255));
	canvas.drawText(
		Rect(
			rect.left,
			rect.top + frameSize.cy,
			rect.right,
			rect.bottom
		),
		m_text,
		AnCenter,
		AnTop
	);
}

		}
	}
}
