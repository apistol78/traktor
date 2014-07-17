#include "Amalgam/Editor/Ui/ProgressCell.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"

// Resources
#include "Resources/ProgressBar.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

void drawSkin(ui::Canvas& canvas, const ui::Rect& rc, ui::Bitmap* bitmap, int pieceOffset)
{
	int w = rc.getWidth();
	int h = rc.getHeight();

	if (w < 8)
		return;

	canvas.drawBitmap(
		ui::Point(rc.left, rc.top),
		ui::Size(4, h),
		ui::Point(pieceOffset, 0),
		ui::Size(4, 16),
		bitmap
	);

	canvas.drawBitmap(
		ui::Point(rc.left + 4, rc.top),
		ui::Size(w - 8, h),
		ui::Point(pieceOffset + 4, 0),
		ui::Size(8, 16),
		bitmap
	);

	canvas.drawBitmap(
		ui::Point(rc.right - 4, rc.top),
		ui::Size(4, h),
		ui::Point(pieceOffset + 12, 0),
		ui::Size(4, 16),
		bitmap
	);
}

		}

ProgressCell::ProgressCell()
:	m_progress(-1)
{
	m_imageProgressBar = ui::Bitmap::load(c_ResourceProgressBar, sizeof(c_ResourceProgressBar), L"png");
}

void ProgressCell::setText(const std::wstring& text)
{
	m_text = text;
}

void ProgressCell::setProgress(int32_t progress)
{
	m_progress = progress;
	if (m_progress < 0)
		m_progress = 0;
	else if (m_progress > 100)
		m_progress = 100;
}

void ProgressCell::paint(ui::Canvas& canvas, const ui::Rect& rect)
{
	if (m_progress >= 0)
	{
		drawSkin(canvas, rect, m_imageProgressBar, 16);

		int32_t x = (rect.getWidth() * m_progress) / 100;
		if (x > 0)
		{
			ui::Rect rect2 = rect;
			rect2.right = rect2.left + x;
			drawSkin(canvas, rect2, m_imageProgressBar, 0);
		}
	}
	if (!m_text.empty())
	{
		ui::Rect rect2 = rect; rect2.left += 2;
		canvas.drawText(rect2, m_text, ui::AnLeft, ui::AnCenter);
	}
}

	}
}
