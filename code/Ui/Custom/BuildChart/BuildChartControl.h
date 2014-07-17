#ifndef traktor_ui_custom_BuildChartControl_H
#define traktor_ui_custom_BuildChartControl_H

#include "Core/Thread/Semaphore.h"
#include "Core/Timer/Timer.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class T_DLLCLASS BuildChartControl : public Widget
{
	T_RTTI_CLASS;

public:
	BuildChartControl();

	bool create(Widget* parent, int style = WsDoubleBuffer);

	void begin();

	void end();

	void beginTask(int32_t lane, const std::wstring& text, const Color4ub& color);

	void endTask(int32_t lane, const Color4ub& color);

private:
	enum { MaxLaneCount = 32 };

	struct Task
	{
		double time0;
		double time1;
		std::wstring text;
		Color4ub color;

		Task()
		:	time0(-1.0)
		,	time1(-1.0)
		{
		}
	};

	Timer m_timer;
	Semaphore m_lanesLock;
	std::vector< Task > m_lanes[MaxLaneCount];
	bool m_running;
	double m_time;
	double m_offset;
	double m_scale;
	int32_t m_lastMouse;

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventMouseWheel(MouseWheelEvent* event);

	void eventPaint(PaintEvent* event);

	void eventTimer(TimerEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_BuildChartControl_H
