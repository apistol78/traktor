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

PreviewItem::PreviewItem()
:	m_selected(false)
{
	m_bitmapFrame = ui::Bitmap::load(c_ResourcePreviewFrame, sizeof(c_ResourcePreviewFrame), L"png");
}

PreviewItem::PreviewItem(const std::wstring& text)
:	m_text(text)
,	m_selected(false)
{
	m_bitmapFrame = ui::Bitmap::load(c_ResourcePreviewFrame, sizeof(c_ResourcePreviewFrame), L"png");
}

void PreviewItem::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& PreviewItem::getText() const
{
	return m_text;
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
	
	// Ensure text fit within boundaries.
	std::wstring text = m_text;
	
	Rect textRect = rect;
	textRect.top += frameSize.cy;

	Size textExtent = canvas.getTextExtent(text);
	if (textExtent.cx > textRect.getWidth())
	{
		if (!isSelected())
		{
			// Item not selected; cut text and add trailing ...
			while (!text.empty())
			{
				text = text.substr(0, text.length() - 1);
				textExtent = canvas.getTextExtent(text + L"...");
				if (textExtent.cx <= rect.getWidth())
					break;
			}
			text += L"...";
		}
		else
		{
			// Item is selected; enlarge text rectangle.
			int32_t excess = textExtent.cx - textRect.getWidth();
			textRect.left -= excess / 2 + 5;
			textRect.right += (excess + 1) / 2 + 5;
		}
	}

	textRect.bottom = textRect.top + textExtent.cy + 2;

	if (isSelected())
	{
		Color4ub background = canvas.getBackground();
		canvas.setBackground(Color4ub(80, 90, 120));
		canvas.fillRect(textRect);
		canvas.setBackground(background);
	}

	canvas.setForeground(Color4ub(255, 255, 255));
	canvas.drawText(
		Rect(
			textRect.left,
			textRect.top,
			textRect.right,
			textRect.bottom
		),
		text,
		AnCenter,
		AnTop
	);
}

		}
	}
}
