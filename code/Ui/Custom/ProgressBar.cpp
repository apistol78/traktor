#include "Ui/Bitmap.h"
#include "Ui/Custom/ProgressBar.h"

// Resources
#include "Resources/ProgressBar.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ProgressBar", ProgressBar, Widget)

ProgressBar::ProgressBar()
:	m_minProgress(0)
,	m_maxProgress(100)
,	m_progress(0)
,	m_loop(0)
{
}

bool ProgressBar::create(Widget* parent, int style, int minProgress, int maxProgress)
{
	if (!Widget::create(parent, style))
		return false;

	m_imageProgressBar = Bitmap::load(c_ResourceProgressBar, sizeof(c_ResourceProgressBar), L"png");
	m_minProgress = minProgress;
	m_maxProgress = maxProgress;
	m_progress = minProgress;

	addEventHandler< PaintEvent >(this, &ProgressBar::eventPaint);

	return true;
}

void ProgressBar::setRange(int minProgress, int maxProgress)
{
	m_minProgress = minProgress;
	m_maxProgress = maxProgress;
}

int ProgressBar::getMinRange() const
{
	return m_minProgress;
}

int ProgressBar::getMaxRange() const
{
	return m_maxProgress;
}

void ProgressBar::setProgress(int progress)
{
	if (m_progress != progress)
	{
		m_progress = progress;
		update();
	}
}

int ProgressBar::getProgress() const
{
	return m_progress;
}

Size ProgressBar::getPreferedSize() const
{
	return Size(256, 16);
}

namespace
{

	void drawSkin(Canvas& canvas, const Rect& rc, Bitmap* bitmap, int pieceOffset)
	{
		int w = rc.getWidth();
		int h = rc.getHeight();

		if (w < 8)
			return;

		canvas.drawBitmap(
			Point(rc.left, rc.top),
			Size(4, h),
			Point(pieceOffset, 0),
			Size(4, 16),
			bitmap
		);

		canvas.drawBitmap(
			Point(rc.left + 4, rc.top),
			Size(w - 8, h),
			Point(pieceOffset + 4, 0),
			Size(8, 16),
			bitmap
		);

		canvas.drawBitmap(
			Point(rc.right - 4, rc.top),
			Size(4, h),
			Point(pieceOffset + 12, 0),
			Size(4, 16),
			bitmap
		);
	}

}

void ProgressBar::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();

	Rect rc = getInnerRect();

	drawSkin(canvas, rc, m_imageProgressBar, 16);

	int32_t range = m_maxProgress - m_minProgress;
	if (range > 0)
	{
		int32_t x = rc.left + rc.getWidth() * (m_progress - m_minProgress) / (m_maxProgress - m_minProgress);
		if (x > 0)
		{
			Rect rc2 = rc; rc2.right = rc2.left + x;
			drawSkin(canvas, rc2, m_imageProgressBar, 0);
		}
	}
	else	// No range; looping progressbar.
	{
		int32_t w = rc.getWidth() / 4;
		int32_t x1 = std::max< int32_t >(m_loop - w, 0);
		int32_t x2 = std::min< int32_t >(m_loop, rc.getWidth());

		Rect rc2 = rc;
		rc2.left = x1;
		rc2.right = x2;

		drawSkin(canvas, rc2, m_imageProgressBar, 0);

		m_loop += std::max< int32_t >(rc.getWidth() / 16, 1);
		if (m_loop >= (rc.getWidth() * 5) / 4)
			m_loop = 0;
	}

	event->consume();
}

		}
	}
}
