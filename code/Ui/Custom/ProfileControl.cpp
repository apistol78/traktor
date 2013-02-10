#include "Ui/Custom/ProfileControl.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/PaintEvent.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Timer/Timer.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const double c_profileTime = 10.0;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ProfileControl", ProfileControl, Widget)

ProfileControl::ProfileControl()
:	m_peekCount(0)
,	m_callBack(0)
{
}

bool ProfileControl::create(Widget* parent, int time, int minSample, int maxSample, int style, IProfileCallback* callBack)
{
	if (!Widget::create(parent, style))
		return false;

	Sample s = { 0.0, 0 };
	m_samples.push_back(s);

	m_callBack = callBack;

	addPaintEventHandler(createMethodHandler(this, &ProfileControl::eventPaint));
	addTimerEventHandler(createMethodHandler(this, &ProfileControl::eventTimer));

	m_timer = new Timer();
	m_timer->start();

	startTimer(time);
	return true;
}

Size ProfileControl::getPreferedSize() const
{
	return Size(256, 256);
}

void ProfileControl::eventPaint(Event* event)
{
	PaintEvent* p = static_cast< PaintEvent* >(event);
	Canvas& canvas = p->getCanvas();
	Rect rc = getInnerRect();

	canvas.setBackground(Color4ub(80, 80, 80));
	canvas.fillRect(rc);

	canvas.setForeground(Color4ub(120, 120, 100));
	for (int x = rc.left; x < rc.right; x += 16)
		canvas.drawLine(x, rc.top, x, rc.bottom);
	for (int y = rc.top; y < rc.bottom; y += 16)
		canvas.drawLine(rc.left, y, rc.right, y);

	StringOutputStream ss1;
	ss1 << m_samples.back().count << L" object(s)";

	canvas.setForeground(Color4ub(200, 255, 200));
	canvas.drawText(Point(0,  0), ss1.str());

	if (!m_samples.empty())
	{
		double Ts = m_samples.front().at;

		std::list< Sample >::iterator i = m_samples.begin();

		int x1 = int(rc.right - rc.getWidth() * (c_profileTime - i->at + Ts) / c_profileTime);
		int y1 = int(rc.bottom - (rc.getHeight() * i->count) / (m_peekCount + 1));

		for (++i; i != m_samples.end(); ++i)
		{
			int x2 = int(rc.right - rc.getWidth() * (c_profileTime - i->at + Ts) / c_profileTime);
			int y2 = int(rc.bottom - (rc.getHeight() * i->count) / (m_peekCount + 1));

			canvas.setForeground(Color4ub(200, 255, 200));
			canvas.drawLine(x1, y1, x2, y2);

			x1 = x2;
			y1 = y2;
		}
	}

	event->consume();
}

void ProfileControl::eventTimer(Event* event)
{
	double T = m_timer->getElapsedTime();

	while (!m_samples.empty() && m_samples.front().at < T - c_profileTime)
		m_samples.pop_front();

	uint32_t count = m_callBack->getProfileValue();

	Sample sample =
	{
		T,
		count
	};
	m_samples.push_back(sample);

	m_peekCount = std::max(sample.count, m_peekCount);

	update();
}

		}
	}
}
