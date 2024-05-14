/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Timer/Timer.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class T_DLLCLASS BuildChartControl : public Widget
{
	T_RTTI_CLASS;

public:
	struct Task
	{
		double time0 = -1.0;
		double time1 = -1.0;
		std::wstring text;
		Color4ub color;
	};

	BuildChartControl();

	bool create(Widget* parent, uint32_t laneCount, uint32_t style = WsDoubleBuffer);

	void showRange(double fromTime, double toTime);

	void showTime(double time);

	double positionToTime(int32_t x) const;

	int32_t timeToPosition(double time) const;

	const Task* getTaskFromPosition(const Point& position) const;

	/*! \{ */

	void removeAllTasks();

	void removeTasksOlderThan(double since);

	void addTask(int32_t lane, const std::wstring& text, const Color4ub& color, double timeStart, double timeEnd);

	/*! \} */

	/*! \{ */

	void begin();

	void end();

	void beginTask(int32_t lane, const std::wstring& text, const Color4ub& color);

	void endTask(int32_t lane, const Color4ub& color);

	/*! \} */

private:
	typedef AlignedVector< Task > taskVector_t;

	Timer m_timer;
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
	std::wstring m_hover;

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

