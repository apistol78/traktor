#include "Ui/Custom/ProfileControl.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/PaintEvent.h"
#include "Core/Heap/HeapStats.h"
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
:	m_peekObjectReference(4000)
{
}

bool ProfileControl::create(Widget* parent, int time, int minSample, int maxSample, int style)
{
	if (!Widget::create(parent, style))
		return false;

	addPaintEventHandler(createMethodHandler(this, &ProfileControl::eventPaint));
	addTimerEventHandler(createMethodHandler(this, &ProfileControl::eventTimer));

	m_timer = gc_new< Timer >();
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

	canvas.setBackground(Color(80, 80, 80));
	canvas.fillRect(rc);

	canvas.setForeground(Color(120, 120, 100));
	for (int x = rc.left; x < rc.right; x += 16)
		canvas.drawLine(x, rc.top, x, rc.bottom);
	for (int y = rc.top; y < rc.bottom; y += 16)
		canvas.drawLine(rc.left, y, rc.right, y);

	HeapStats stats;
	Heap::getStats(stats);

	StringOutputStream ss1;
	ss1 << stats.objects << L" object(s)";

	StringOutputStream ss2;
	ss2 << stats.references << L" reference(s), " << stats.rootReferences << L" root(s)";

	canvas.setForeground(Color(200, 255, 200));
	canvas.drawText(Point(0,  0), ss1.str());

	canvas.setForeground(Color(200, 200, 255));
	canvas.drawText(Point(0, 16), ss2.str());

	if (!m_samples.empty())
	{
		double Ts = m_samples.front().at;

		std::list< Sample >::iterator i = m_samples.begin();

		int x1 = int(rc.right - rc.getWidth() * (c_profileTime - i->at + Ts) / c_profileTime);
		int yo1 = int(rc.bottom - (rc.getHeight() * i->objectCount) / m_peekObjectReference);
		int yr1 = int(rc.bottom - (rc.getHeight() * i->referenceCount) / m_peekObjectReference);

		for (++i; i != m_samples.end(); ++i)
		{
			int x2 = int(rc.right - rc.getWidth() * (c_profileTime - i->at + Ts) / c_profileTime);
			int yo2 = int(rc.bottom - (rc.getHeight() * i->objectCount) / m_peekObjectReference);
			int yr2 = int(rc.bottom - (rc.getHeight() * i->referenceCount) / m_peekObjectReference);

			canvas.setForeground(Color(200, 255, 200));
			canvas.drawLine(x1, yo1, x2, yo2);

			canvas.setForeground(Color(200, 200, 255));
			canvas.drawLine(x1, yr1, x2, yr2);

			x1 = x2;
			yo1 = yo2;
			yr1 = yr2;
		}
	}

	event->consume();
}

void ProfileControl::eventTimer(Event* event)
{
	double T = m_timer->getElapsedTime();

	while (!m_samples.empty() && m_samples.front().at < T - c_profileTime)
		m_samples.pop_front();

	HeapStats stats;
	Heap::getStats(stats);

	Sample sample =
	{
		T,
		stats.objects,
		stats.references,
	};
	m_samples.push_back(sample);

	m_peekObjectReference = std::max(sample.objectCount, m_peekObjectReference);
	m_peekObjectReference = std::max(sample.referenceCount, m_peekObjectReference);

	update();
}

		}
	}
}
