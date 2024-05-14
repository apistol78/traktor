/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/MathUtils.h"
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"
#include "Ui/BuildChart/BuildChartControl.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.BuildChartControl", BuildChartControl, Widget)

BuildChartControl::BuildChartControl()
:	m_running(false)
,	m_selecting(false)
,	m_moving(false)
,	m_time(0.0)
,	m_fromTime(-10.0)
,	m_toTime(3 * 60.0 + 20.0)
,	m_lastSize(0)
,	m_lastMouse(0)
,	m_selectionTo(0)
{
}

bool BuildChartControl::create(Widget* parent, uint32_t laneCount, uint32_t style)
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

void BuildChartControl::showTime(double time)
{
	double duration = m_toTime - m_fromTime;
	m_fromTime = time - duration;
	m_toTime = time;
	update();
}

double BuildChartControl::positionToTime(int32_t x) const
{
	return m_fromTime + (m_toTime - m_fromTime) * double(x) / m_lastSize;
}

int32_t BuildChartControl::timeToPosition(double time) const
{
	return (int32_t)(m_lastSize * (time - m_fromTime) / (m_toTime - m_fromTime) + 0.5f);
}

const BuildChartControl::Task* BuildChartControl::getTaskFromPosition(const Point& position) const
{
	const Rect rc = getInnerRect();
	const int32_t c_two = pixel(2_ut);
	const int32_t c_four = pixel(4_ut);

	Rect rcLane(rc.left, rc.top, rc.right, rc.top + pixel(24_ut));
	for (int32_t lane = 0; lane < m_lanes.size(); ++lane)
	{
		if (rcLane.inside(position))
		{
			for (const auto& task : m_lanes[lane])
			{
				int32_t x0 = timeToPosition(task.time0);
				int32_t x1 = timeToPosition(task.time1 >= 0.0 ? task.time1 : m_time);

				if (x1 < x0 + c_four)
					x1 = x0 + c_four;

				if (x0 >= rcLane.right || x1 <= rcLane.left)
					continue;

				Rect rcTask(
					rcLane.left + x0,
					rcLane.top + c_two,
					rcLane.left + x1,
					rcLane.bottom - c_two
				);
				if (rcTask.inside(position))
					return &task;
			}
		}
		rcLane = rcLane.offset(0, pixel(24_ut));
	}

	return nullptr;
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
		auto& ln = m_lanes[lane];
		while (!ln.empty() && ln.back().time1 < since)
			ln.pop_back();
	}
}

void BuildChartControl::addTask(int32_t lane, const std::wstring& text, const Color4ub& color, double timeStart, double timeEnd)
{
	if (lane < 0 || lane >= m_lanes.size())
		return;

	Task t;
	t.time0 = timeStart;
	t.time1 = timeEnd;
	t.text = text;
	t.color = color;

	auto& ln = m_lanes[lane];
	for (size_t i = ln.size(); i > 0; --i)
	{
		if (ln[i - 1].time1 <= timeEnd)
		{
			ln.insert(ln.begin() + i, t);
			return;
		}
	}
	ln.push_back(t);
}

void BuildChartControl::begin()
{
	for (int32_t lane = 0; lane < m_lanes.size(); ++lane)
		m_lanes[lane].resize(0);

	m_running = true;
	m_time = 0.0;

	m_timer.reset();
}

void BuildChartControl::end()
{
	m_running = false;
	m_time = m_timer.getElapsedTime();
}

void BuildChartControl::beginTask(int32_t lane, const std::wstring& text, const Color4ub& color)
{
	if (lane < 0 || lane >= m_lanes.size())
		return;

	auto& task = m_lanes[lane].push_back();
	task.time0 = m_timer.getElapsedTime();
	task.text = text;
	task.color = color;
}

void BuildChartControl::endTask(int32_t lane, const Color4ub& color)
{
	if (lane < 0 || lane >= m_lanes.size())
		return;

	auto& task = m_lanes[lane].back();
	task.time1 = m_timer.getElapsedTime();
	task.color = color;
}

void BuildChartControl::eventPaint(PaintEvent* event)
{
	const StyleSheet* ss = getStyleSheet();
	Canvas& canvas = event->getCanvas();

	if (m_running)
		m_time = m_timer.getElapsedTime();

	Rect rcUpdate = event->getUpdateRect();
	Rect rc = getInnerRect();

	const int32_t c_two = pixel(2_ut);
	const int32_t c_four = pixel(4_ut);
	const int32_t c_twenty = pixel(20_ut);

	// Clear background.
	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcUpdate);

	// Draw tasks.
	{
		Rect rcLane(rc.left, rc.top, rc.right, rc.top + pixel(24_ut));
		for (int32_t lane = 0; lane < m_lanes.size(); ++lane)
		{
			canvas.setForeground(ss->getColor(this, L"line-color"));
			canvas.drawLine(rcLane.left, rcLane.getCenter().y, rcLane.right, rcLane.getCenter().y);

			for (const auto& task : m_lanes[lane])
			{
				int32_t x0 = timeToPosition(task.time0);
				int32_t x1 = timeToPosition(task.time1 >= 0.0 ? task.time1 : m_time);

				if (x1 < x0 + c_four)
					x1 = x0 + c_four;

				if (x0 >= rcLane.right || x1 <= rcLane.left)
					continue;

				Rect rcTask(
					rcLane.left + x0,
					rcLane.top + c_two,
					rcLane.left + x1,
					rcLane.bottom - c_two
				);

				canvas.setBackground(task.color);
				canvas.fillRect(rcTask);

				canvas.setForeground(Color4ub(0, 0, 0, 80));
				canvas.drawRect(rcTask);

				if (rcTask.getWidth() > c_twenty)
				{
					canvas.setForeground(Color4ub(0, 0, 0, 128));
					canvas.setClipRect(rcTask);
					canvas.drawText(rcTask, task.text, AnLeft, AnCenter);
					canvas.resetClipRect();
				}
			}

			rcLane = rcLane.offset(0, pixel(24_ut));
		}
	}

	// Draw time axis.
	Rect rcTime(rc.left, rc.bottom - pixel(16_ut), rc.right, rc.bottom);

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcTime);

	canvas.setForeground(Color4ub(0, 0, 0, 100));
	canvas.drawLine(rcTime.left, rcTime.top, rcTime.right, rcTime.top);

	const double duration = m_toTime - m_fromTime;
	if (duration > 0.01)
	{
		const double l = std::log10(duration);
		const double f = std::floor(l);
		const double d = std::pow(10.0, f);
		const double a = std::fmod(m_fromTime, d);

		for (double t = m_fromTime - a; t <= m_toTime; t += d / 10.0)
		{
			const int32_t x = timeToPosition(t);
			canvas.drawLine(x, rcTime.bottom - pixel(8_ut), x, rcTime.bottom);
		}
	}

	canvas.setForeground(ss->getColor(this, L"color"));
	canvas.drawText(rcTime, str(L"%.3f", (float)m_fromTime), AnLeft, AnCenter);
	canvas.drawText(rcTime, str(L"%.3f", (float)m_toTime), AnRight, AnCenter);

	if (!m_hover.empty())
		canvas.drawText(rcTime, m_hover, AnCenter, AnCenter);

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

	if (m_running)
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
	else
	{
		// Hover over task to show detailed information.
		const Task* task = getTaskFromPosition(event->getPosition());
		if (task)
			m_hover = task->text;
		else
			m_hover = L"";
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
