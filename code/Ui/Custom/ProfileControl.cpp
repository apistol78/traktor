/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/StringOutputStream.h"
#include "Core/Timer/Timer.h"
#include "Ui/Application.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/ProfileControl.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const double c_profileTime = 10.0;

const Color4ub c_channelColors[] =
{
	Color4ub(200, 255, 200),
	Color4ub(255, 255, 200),
	Color4ub(200, 255, 255),
	Color4ub(255, 200, 255)
};

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ProfileControl", ProfileControl, Widget)

ProfileControl::ProfileControl()
:	m_callBack(0)
{
}

bool ProfileControl::create(Widget* parent, int channels, int time, int minSample, int maxSample, int style, IProfileCallback* callBack)
{
	if (!Widget::create(parent, style))
		return false;

	m_channels.resize(channels);
	for (int i = 0; i < channels; ++i)
	{
		Sample s = { 0.0, 0 };
		m_channels[i].peekCount = 0;
		m_channels[i].samples.push_back(s);
	}

	m_callBack = callBack;

	addEventHandler< PaintEvent >(this, &ProfileControl::eventPaint);
	addEventHandler< TimerEvent >(this, &ProfileControl::eventTimer);

	m_timer = new Timer();
	m_timer->start();

	startTimer(time);
	return true;
}

Size ProfileControl::getPreferedSize() const
{
	return Size(256, 256);
}

void ProfileControl::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	Rect rc = getInnerRect();

	const StyleSheet* ss = Application::getInstance()->getStyleSheet();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rc);

	canvas.setForeground(ss->getColor(this, L"grid-color"));
	for (int x = rc.left; x < rc.right; x += 16)
		canvas.drawLine(x, rc.top, x, rc.bottom);
	for (int y = rc.top; y < rc.bottom; y += 16)
		canvas.drawLine(rc.left, y, rc.right, y);

	for (int i = 0; i < int(m_channels.size()); ++i)
	{
		const std::list< Sample >& samples = m_channels[i].samples;

		StringOutputStream ss1;
		ss1 << samples.back().count << L" object(s)";

		canvas.setForeground(c_channelColors[i]);
		canvas.drawText(Point(0,  i * 14), ss1.str());

		if (!samples.empty())
		{
			double Ts = samples.front().at;

			std::list< Sample >::const_iterator it = samples.begin();

			int x1 = int(rc.right - rc.getWidth() * (c_profileTime - it->at + Ts) / c_profileTime);
			int y1 = int(rc.bottom - (rc.getHeight() * it->count) / (m_channels[i].peekCount + 1));

			for (++it; it != samples.end(); ++it)
			{
				int x2 = int(rc.right - rc.getWidth() * (c_profileTime - it->at + Ts) / c_profileTime);
				int y2 = int(rc.bottom - (rc.getHeight() * it->count) / (m_channels[i].peekCount + 1));

				canvas.drawLine(x1, y1, x2, y2);

				x1 = x2;
				y1 = y2;
			}
		}
	}

	event->consume();
}

void ProfileControl::eventTimer(TimerEvent* event)
{
	double T = m_timer->getElapsedTime();

	uint32_t values[4];
	m_callBack->getProfileValues(values);

	for (int i = 0; i < int(m_channels.size()); ++i)
	{
		std::list< Sample >& samples = m_channels[i].samples;

		while (!samples.empty() && samples.front().at < T - c_profileTime)
			samples.pop_front();

		Sample sample =
		{
			T,
			values[i]
		};
		samples.push_back(sample);

		m_channels[i].peekCount = std::max(sample.count, m_channels[i].peekCount);
	}

	update();
}

		}
	}
}
