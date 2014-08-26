#include "Amalgam/Editor/Ui/ProfilerControl.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

const Color4ub c_markerColors[] =
{
	Color4ub(255, 255, 120),
	Color4ub(200, 200, 80),
	Color4ub(255, 120, 255),
	Color4ub(200, 80, 200),
	Color4ub(120, 255, 255),
	Color4ub(80, 200, 200),
	Color4ub(255, 120, 120),
	Color4ub(200, 80, 80),
	Color4ub(120, 255, 120),
	Color4ub(80, 200, 80),
	Color4ub(120, 120, 255),
	Color4ub(80, 80, 200)
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ProfilerControl", ProfilerControl, ui::Widget)

ProfilerControl::ProfilerControl()
:	m_maxTime(1.0f / 60.0f)
{
}

bool ProfilerControl::create(ui::Widget* parent)
{
	if (!ui::Widget::create(parent, ui::WsDoubleBuffer))
		return false;

	addEventHandler< ui::PaintEvent >(this, &ProfilerControl::eventPaint);

	return true;
}

void ProfilerControl::addFrame(float time, const AlignedVector< TargetPerformance::FrameMarker >& markers)
{
	if (!m_frames.empty())
	{
		if (std::abs(m_frames.back().time - time) <= FUZZY_EPSILON)
			return;
	}

	Frame& f = m_frames.push_back();
	f.time = time;
	f.markers = markers;

	update();
}

void ProfilerControl::eventPaint(ui::PaintEvent* event)
{
	ui::Canvas& canvas = event->getCanvas();
	ui::Rect rc = getInnerRect();

	canvas.setBackground(Color4ub(255, 255, 255, 255));
	canvas.fillRect(rc);

	int32_t x = 0;
	int32_t y = rc.bottom;
	int32_t h = rc.getHeight();

	float maxTime = 0.0f;

	for (int32_t x = 0; x < rc.getWidth(); ++x)
	{
		int32_t i = x * m_frames.capacity() / rc.getWidth();
		if (i >= m_frames.size())
			break;

		const Frame& f = m_frames[i];

		uint32_t level = 0;
		for (;;)
		{
			uint32_t ndraw = 0;
			for (uint32_t j = 0; j < f.markers.size(); ++j)
			{
				const TargetPerformance::FrameMarker& fm = f.markers[j];

				if (fm.level != level)
					continue;

				int32_t yb = y - int32_t(fm.begin * h / m_maxTime);
				int32_t ye = y - int32_t(fm.end * h / m_maxTime);

				if (yb > ye)
				{
					canvas.setForeground(c_markerColors[j % sizeof_array(c_markerColors)]);
					canvas.drawLine(ui::Point(x, yb), ui::Point(x, ye));
				}

				++ndraw;
			}
			if (!ndraw)
				break;
			else
				++level;
		}

		maxTime = std::max(maxTime, f.markers.back().end);
	}

	// Draw frame time limits.
	for (int32_t i = 1; ; ++i)
	{
		const float f = i / 60.0f;

		int32_t yf = y - int32_t(f * h / m_maxTime);
		if (yf < 0)
			break;

		canvas.setForeground(Color4ub(0, 0, 0, 80));
		canvas.drawLine(rc.left, yf, rc.right, yf);
	}

	m_maxTime = m_maxTime * 0.2f * maxTime * 0.8f;
	m_maxTime = std::max(m_maxTime, 1.0f / 60.0f);

	event->consume();
}

	}
}
