#include "Core/Math/MathUtils.h"
#include "Core/Thread/Acquire.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Custom/BuildChart/BuildChartControl.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.BuildChartControl", BuildChartControl, Widget)

BuildChartControl::BuildChartControl()
:	m_running(false)
,	m_selecting(false)
,	m_moving(false)
,	m_time(0.0)
,	m_fromTime(0.0)
,	m_toTime(1.0)
,	m_lastSize(0)
,	m_lastMouse(0)
,	m_selectionTo(0)
{
}

bool BuildChartControl::create(Widget* parent, uint32_t laneCount, int style)
{
	if (!Widget::create(parent, style))
		return false;

	addEventHandler< MouseButtonDownEvent >(this, &BuildChartControl::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &BuildChartControl::eventButtonUp);
	addEventHandler< MouseMoveEvent >(this, &BuildChartControl::eventMouseMove);
	addEventHandler< MouseWheelEvent >(this, &BuildChartControl::eventMouseWheel);
	addEventHandler< PaintEvent >(this, &BuildChartControl::eventPaint);
	addEventHandler< SizeEvent >(this, &BuildChartControl::eventSize);
	addEventHandler< TimerEvent >(this, &BuildChartControl::eventTimer);

	m_lanes.resize(laneCount);

	startTimer(100);
	return true;
}

void BuildChartControl::showRange(double fromTime, double toTime)
{
	m_fromTime = fromTime;
	m_toTime = toTime;
	update();
}

double BuildChartControl::positionToTime(int32_t x) const
{
	return m_fromTime + (m_toTime - m_fromTime) * double(x) / m_lastSize;
}

int32_t BuildChartControl::timeToPosition(double time) const
{
	return int32_t(m_lastSize * (time - m_fromTime) / (m_toTime - m_fromTime) + 0.5f);
}

void BuildChartControl::removeAllTasks()
{
	for (int32_t lane = 0; lane < m_lanes.size(); ++lane)
		m_lanes[lane].resize(0);

	m_time = 0.0;
	m_running = false;
}

void BuildChartControl::removeTasksOlderThan(double since)
{
	for (int32_t lane = 0; lane < m_lanes.size(); ++lane)
	{
		while (!m_lanes[lane].empty())
		{
			if (m_lanes[lane].front().time1 >= since)
				break;
			m_lanes[lane].erase(m_lanes[lane].begin());
		}
	}
}

void BuildChartControl::addTask(int32_t lane, const std::wstring& text, const Color4ub& color, double timeStart, double timeEnd)
{
	if (lane < 0 || lane >= m_lanes.size())
		return;

	m_lanes[lane].push_back(Task());
	m_lanes[lane].back().time0 = timeStart;
	m_lanes[lane].back().time1 = timeEnd;
	m_lanes[lane].back().text = text;
	m_lanes[lane].back().color = color;
}

void BuildChartControl::begin()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lanesLock);
	for (int32_t lane = 0; lane < m_lanes.size(); ++lane)
		m_lanes[lane].resize(0);

	m_running = true;
	m_time = 0.0;

	m_timer.start();
}

void BuildChartControl::end()
{
	m_running = false;
	m_time = m_timer.getElapsedTime();
}

void BuildChartControl::beginTask(int32_t lane, const std::wstring& text, const Color4ub& color)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lanesLock);

	if (lane < 0 || lane >= m_lanes.size())
		return;

	m_lanes[lane].push_back(Task());
	m_lanes[lane].back().time0 = m_timer.getElapsedTime();
	m_lanes[lane].back().text = text;
	m_lanes[lane].back().color = color;
}

void BuildChartControl::endTask(int32_t lane, const Color4ub& color)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lanesLock);

	if (lane < 0 || lane >= m_lanes.size())
		return;

	m_lanes[lane].back().time1 = m_timer.getElapsedTime();
	m_lanes[lane].back().color = color;
}

void BuildChartControl::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();

	if (m_running)
		m_time = m_timer.getElapsedTime();

	Rect rc = getInnerRect();

	// Clear background.
	canvas.setBackground(Color4ub(255, 255, 255, 255));
	canvas.fillRect(rc);

	// Draw tasks.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lanesLock);
		Rect rcLane(rc.left, rc.top, rc.right, rc.top + scaleBySystemDPI(24));
		for (int32_t lane = 0; lane < m_lanes.size(); ++lane)
		{
			canvas.setForeground(Color4ub(0, 0, 0, 40));
			canvas.drawLine(rcLane.left, rcLane.getCenter().y, rcLane.right, rcLane.getCenter().y);

			const taskVector_t& tasks = m_lanes[lane];
			for (taskVector_t::const_iterator i = tasks.begin(); i != tasks.end(); ++i)
			{
				int32_t x0 = timeToPosition(i->time0);
				int32_t x1 = timeToPosition(i->time1 >= 0.0 ? i->time1 : m_time);

				if (x1 < x0 + scaleBySystemDPI(4))
					x1 = x0 + scaleBySystemDPI(4);

				if (x0 >= rcLane.right || x1 <= rcLane.left)
					continue;

				Rect rcTask(
					rcLane.left + x0,
					rcLane.top + scaleBySystemDPI(2),
					rcLane.left + x1,
					rcLane.bottom - scaleBySystemDPI(2)
				);

				canvas.setForeground(Color4ub(220, 255, 220, 255));
				canvas.setBackground(i->color);
				canvas.fillGradientRect(rcTask);

				canvas.setForeground(Color4ub(0, 0, 0, 80));
				canvas.drawRect(rcTask);

				canvas.setForeground(Color4ub(0, 0, 0, 128));
				canvas.drawText(rcTask, i->text, AnLeft, AnCenter);
			}

			rcLane = rcLane.offset(0, scaleBySystemDPI(24));
		}
	}

	// Draw time axis.
	Rect rcTime(rc.left, rc.bottom - scaleBySystemDPI(16), rc.right, rc.bottom);
	
	canvas.setBackground(Color4ub(255, 255, 255, 255));
	canvas.fillRect(rcTime);
	
	canvas.setForeground(Color4ub(0, 0, 0, 100));
	canvas.drawLine(rcTime.left, rcTime.top, rcTime.right, rcTime.top);

	double duration = m_toTime - m_fromTime;

	double l = std::log10(duration);
	double f = std::floor(l);
	double d = std::pow(10.0, f);
	double a = std::fmod(m_fromTime, d);

	for (double t = m_fromTime - a; t <= m_toTime; t += d / 10.0)
	{
		int32_t x = timeToPosition(t);
		canvas.drawLine(x, rcTime.bottom - scaleBySystemDPI(8), x, rcTime.bottom);
	}

	if (m_selecting)
	{
		Rect rcSelection(
			std::min(m_lastMouse, m_selectionTo), rc.top,
			std::max(m_lastMouse, m_selectionTo), rc.bottom
		);
	
		canvas.setBackground(Color4ub(150, 150, 255, 50));
		canvas.fillRect(rcSelection);

		canvas.setForeground(Color4ub(150, 150, 255, 100));
		canvas.drawLine(rcSelection.getTopLeft(), rcSelection.getBottomLeft());
		canvas.drawLine(rcSelection.getTopRight(), rcSelection.getBottomRight());
	}

	event->consume();
}

void BuildChartControl::eventSize(SizeEvent* event)
{
	if (event->getSize().cx <= 0)
		return;

	if (m_lastSize > 0)
		m_toTime = positionToTime(event->getSize().cx);

	m_lastSize = event->getSize().cx;
}

void BuildChartControl::eventTimer(TimerEvent* event)
{
	if (m_running && !hasCapture())
	{
		if (m_time > m_toTime)
		{
			double d = m_time - m_toTime;
			m_fromTime += d;
			m_toTime += d;
		}
	}

	update();
}

void BuildChartControl::eventButtonDown(MouseButtonDownEvent* event)
{
	if (event->getButton() == MbtLeft)
	{
		m_selectionTo =
		m_lastMouse = event->getPosition().x;
		setCapture();
	}
}

void BuildChartControl::eventButtonUp(MouseButtonUpEvent* event)
{
	if (hasCapture())
	{
		if (m_selecting)
		{
			int32_t x0 = m_lastMouse;
			int32_t x1 = event->getPosition().x;

			if (x1 < x0)
				std::swap(x0, x1);

			double t0 = positionToTime(x0);
			double t1 = positionToTime(x1);

			showRange(t0, t1);
		}

		m_moving = false;
		m_selecting = false;

		releaseCapture();
	}
}

void BuildChartControl::eventMouseMove(MouseMoveEvent* event)
{
	if (hasCapture())
	{
		if (!m_selecting && !m_moving)
		{
			if ((event->getKeyState() & ui::KsMenu) != 0)
				m_moving = true;
			else
				m_selecting = true;
		}

		if (m_moving)
		{
			int32_t x0 = m_selectionTo;
			int32_t x1 = event->getPosition().x;

			double t0 = positionToTime(x0);
			double t1 = positionToTime(x1);

			m_fromTime -= (t1 - t0);
			m_toTime -= (t1 - t0);
		}

		m_selectionTo = event->getPosition().x;
		update();
	}
}

void BuildChartControl::eventMouseWheel(MouseWheelEvent* event)
{
	double pivot = positionToTime(event->getPosition().x);
	double duration = m_toTime - m_fromTime;

	int32_t r = event->getRotation();
	double expand = duration * r * 0.1;

	double w = clamp((pivot - m_fromTime) / duration, 0.0, 1.0);
	m_fromTime -= expand * w;
	m_toTime += expand * (1.0 - w);

	update();
}

		}
	}
}
