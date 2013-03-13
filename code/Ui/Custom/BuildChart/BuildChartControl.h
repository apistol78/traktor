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

	void beginTask(int32_t lane, const std::wstring& text);

	void endTask(int32_t lane);

private:
	enum { MaxLaneCount = 32 };

	struct Task
	{
		double time0;
		double time1;
		std::wstring text;

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

	void eventButtonDown(Event* event);

	void eventButtonUp(Event* event);

	void eventMouseMove(Event* event);

	void eventMouseWheel(Event* event);

	void eventPaint(Event* event);

	void eventTimer(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_BuildChartControl_H
