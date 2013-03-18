#include "Core/Math/MathUtils.h"
#include "Core/Thread/Acquire.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/PaintEvent.h"
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
,	m_time(0.0)
,	m_offset(0.0)
,	m_scale(32.0)
,	m_lastMouse(0)
{
}

bool BuildChartControl::create(Widget* parent, int style)
{
	if (!Widget::create(parent, style))
		return false;

	addButtonDownEventHandler(createMethodHandler(this, &BuildChartControl::eventButtonDown));
	addButtonUpEventHandler(createMethodHandler(this, &BuildChartControl::eventButtonUp));
	addMouseMoveEventHandler(createMethodHandler(this, &BuildChartControl::eventMouseMove));
	addMouseWheelEventHandler(createMethodHandler(this, &BuildChartControl::eventMouseWheel));
	addPaintEventHandler(createMethodHandler(this, &BuildChartControl::eventPaint));
	addTimerEventHandler(createMethodHandler(this, &BuildChartControl::eventTimer));

	startTimer(100);
	return true;
}

void BuildChartControl::begin()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lanesLock);
	for (int32_t lane = 0; lane < sizeof_array(m_lanes); ++lane)
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
	m_lanes[lane].push_back(Task());
	m_lanes[lane].back().time0 = m_timer.getElapsedTime();
	m_lanes[lane].back().text = text;
	m_lanes[lane].back().color = color;
}

void BuildChartControl::endTask(int32_t lane, const Color4ub& color)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lanesLock);
	m_lanes[lane].back().time1 = m_timer.getElapsedTime();
	m_lanes[lane].back().color = color;
}

void BuildChartControl::eventPaint(Event* event)
{
	PaintEvent* p = static_cast< PaintEvent* >(event);
	Canvas& canvas = p->getCanvas();

	if (m_running)
		m_time = m_timer.getElapsedTime();

	Rect rc = getInnerRect();

	// Clear background.
	canvas.setBackground(Color4ub(255, 255, 255, 255));
	canvas.fillRect(rc);

	// Draw tasks.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lanesLock);
		Rect rcLane(rc.left, rc.top, rc.right, rc.top + 24);
		for (int32_t lane = 0; lane < 32; ++lane)
		{
			canvas.setForeground(Color4ub(0, 0, 0, 40));
			canvas.drawLine(rcLane.left, rcLane.getCenter().y, rcLane.right, rcLane.getCenter().y);

			const std::vector< Task >& tasks = m_lanes[lane];
			for (std::vector< Task >::const_iterator i = tasks.begin(); i != tasks.end(); ++i)
			{
				int32_t x0 = int32_t((i->time0 - m_offset) * m_scale) + 1;
				int32_t x1 = int32_t((m_time - m_offset) * m_scale) - 1;

				if (i->time1 >= 0.0)
					x1 = int32_t((i->time1 - m_offset) * m_scale);

				if (x1 < x0 + 4)
					x1 = x0 + 4;

				if (x0 >= rcLane.right || x1 <= rcLane.left)
					continue;

				Rect rcTask(
					rcLane.left + x0,
					rcLane.top + 2,
					rcLane.left + x1,
					rcLane.bottom - 2
				);

				canvas.setForeground(Color4ub(220, 255, 220, 255));
				canvas.setBackground(i->color);
				canvas.fillGradientRect(rcTask);

				canvas.setForeground(Color4ub(0, 0, 0, 80));
				canvas.drawRect(rcTask);

				canvas.setForeground(Color4ub(0, 0, 0, 128));
				canvas.drawText(rcTask, i->text, AnLeft, AnCenter);
			}

			rcLane = rcLane.offset(0, 24);
		}
	}

	// Draw time axis.
	Rect rcTime(rc.left, rc.bottom - 16, rc.right, rc.bottom);
	
	canvas.setBackground(Color4ub(255, 255, 255, 255));
	canvas.fillRect(rcTime);
	
	canvas.setForeground(Color4ub(0, 0, 0, 80));
	canvas.drawLine(rcTime.left, rcTime.top, rcTime.right, rcTime.top);

	double time = (rcTime.right - rcTime.left) / m_scale;
	
	int32_t x0 = 0;
	int32_t x1 = int32_t(ceil(time));
	int32_t xs = max(1, (x1 - x0) / 10);

	for (int32_t i = x0; i < x1; i += xs)
	{
		int32_t x = int32_t(i * m_scale);
		canvas.drawLine(x, rcTime.bottom - 4, x, rcTime.bottom);
	}

	// Animate back to zero.
	if (m_offset < 0.0 && !hasCapture())
		m_offset += -m_offset / 10.0;

	event->consume();
}

void BuildChartControl::eventTimer(Event* event)
{
	if (m_running && !hasCapture())
	{
		Rect rc = getInnerRect();
		int32_t x = int32_t((m_time - m_offset) * m_scale);
		if (x >= rc.right)
		{
			x = (rc.left + rc.right) / 2;
			m_offset = -(x / m_scale - m_time);
		}
	}

	update();
}

void BuildChartControl::eventButtonDown(Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	if (mouseEvent->getButton() == ui::MouseEvent::BtLeft)
	{
		m_lastMouse = mouseEvent->getPosition().x;
		setCapture();
	}
}

void BuildChartControl::eventButtonUp(Event* event)
{
	releaseCapture();
}

void BuildChartControl::eventMouseMove(Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	if (hasCapture())
	{
		int32_t delta = mouseEvent->getPosition().x - m_lastMouse;
		m_offset -= delta / m_scale;
		m_lastMouse = mouseEvent->getPosition().x;
		update();
	}
}

void BuildChartControl::eventMouseWheel(Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);

	double pivot = mouseEvent->getPosition().x / m_scale + m_offset;

	m_scale += mouseEvent->getWheelRotation() * 4.0;
	m_scale = clamp(m_scale, 1.0, 1000.0);

	m_offset = pivot - mouseEvent->getPosition().x / m_scale;

	update();
}

		}
	}
}
