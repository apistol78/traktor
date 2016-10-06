#ifndef traktor_ui_custom_BuildChartControl_H
#define traktor_ui_custom_BuildChartControl_H

#include "Core/Containers/AlignedVector.h"
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

	bool create(Widget* parent, uint32_t laneCount, int style = WsDoubleBuffer);

	void showRange(double fromTime, double toTime);

	double positionToTime(int32_t x) const;

	int32_t timeToPosition(double time) const;

	/*! \{ */

	void removeAllTasks();

	void addTask(int32_t lane, const std::wstring& text, const Color4ub& color, double timeStart, double timeEnd);

	/*! \} */

	/*! \{ */

	void begin();

	void end();

	void beginTask(int32_t lane, const std::wstring& text, const Color4ub& color);

	void endTask(int32_t lane, const Color4ub& color);

	/*! \} */

private:
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

	typedef AlignedVector< Task > taskVector_t;

	Timer m_timer;
	Semaphore m_lanesLock;
	AlignedVector< taskVector_t > m_lanes;
	bool m_running;
	bool m_selecting;
	bool m_moving;
	double m_time;
	double m_fromTime;
	double m_toTime;
	int32_t m_lastSize;
	int32_t m_lastMouse;
	int32_t m_selectionTo;

	void eventPaint(PaintEvent* event);

	void eventSize(SizeEvent* event);

	void eventTimer(TimerEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventMouseWheel(MouseWheelEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_BuildChartControl_H
