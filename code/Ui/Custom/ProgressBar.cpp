#include "Ui/Application.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/ProgressBar.h"

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

bool ProgressBar::create(Widget* parent, int32_t style, int32_t minProgress, int32_t maxProgress)
{
	if (!Widget::create(parent, style))
		return false;

	m_minProgress = minProgress;
	m_maxProgress = maxProgress;
	m_progress = minProgress;

	addEventHandler< PaintEvent >(this, &ProgressBar::eventPaint);
	return true;
}

void ProgressBar::setRange(int32_t minProgress, int32_t maxProgress)
{
	m_minProgress = minProgress;
	m_maxProgress = maxProgress;
}

int32_t ProgressBar::getMinRange() const
{
	return m_minProgress;
}

int32_t ProgressBar::getMaxRange() const
{
	return m_maxProgress;
}

void ProgressBar::setProgress(int32_t progress)
{
	if (m_progress != progress)
	{
		m_progress = progress;
		update();
	}
}

int32_t ProgressBar::getProgress() const
{
	return m_progress;
}

Size ProgressBar::getPreferedSize() const
{
	return Size(scaleBySystemDPI(256), scaleBySystemDPI(16));
}

void ProgressBar::eventPaint(PaintEvent* event)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	Canvas& canvas = event->getCanvas();

	Rect rc = getInnerRect();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rc);

	int32_t range = m_maxProgress - m_minProgress;
	if (range > 0)
	{
		int32_t x = rc.left + rc.getWidth() * (m_progress - m_minProgress) / (m_maxProgress - m_minProgress);
		if (x > 0)
		{
			Rect rc2 = rc; rc2.right = rc2.left + x;

			canvas.setBackground(ss->getColor(this, L"progress-color"));
			canvas.fillRect(rc2);
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

		canvas.setBackground(ss->getColor(this, L"progress-color"));
		canvas.fillRect(rc2);

		m_loop += std::max< int32_t >(rc.getWidth() / 16, 1);
		if (m_loop >= (rc.getWidth() * 5) / 4)
			m_loop = 0;
	}

	event->consume();
}

		}
	}
}
