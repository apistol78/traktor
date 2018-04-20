/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Editor/ProfilerDialog.h"
#include "Amalgam/Editor/TargetConnection.h"
#include "Core/Io/StringOutputStream.h"
#include "Ui/Application.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/BuildChart/BuildChartControl.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{
		
const Color4ub c_threadColors[] =
{
	Color4ub(255, 255, 0, 255),
	Color4ub(0, 255, 255, 255),
	Color4ub(255, 0, 255, 255),
	Color4ub(255, 0, 0, 255),
	Color4ub(0, 255, 0, 255),
	Color4ub(0, 0, 255, 255)
};
		
		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ProfilerDialog", ProfilerDialog, ui::Dialog)

ProfilerDialog::ProfilerDialog(TargetConnection* connection)
:	m_connection(connection)
,	m_nextThreadLane(0)
,	m_recording(true)
{
}

bool ProfilerDialog::create(ui::Widget* parent)
{
	if (!ui::Dialog::create(parent, L"Profiler", ui::dpi96(800), ui::dpi96(350), ui::Dialog::WsDefaultResizable, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	m_toolBar = new ui::custom::ToolBar();
	m_toolBar->create(this);
	m_toolBar->addItem(new ui::custom::ToolBarButton(L"Start/Stop", ui::Command(L"Amalgam.Profiler.Capture")));
	m_toolBar->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &ProfilerDialog::eventToolClick);

	m_chart = new ui::custom::BuildChartControl();
	m_chart->create(this, 4 * 8);
	m_chart->begin();

	addEventHandler< ui::CloseEvent >(this, &ProfilerDialog::eventClose);

	update();
	show();

	m_connection->setProfilerEventsCallback(this);
	return true;
}

void ProfilerDialog::destroy()
{
	m_connection->setProfilerEventsCallback(0);
	ui::Dialog::destroy();
}

void ProfilerDialog::receivedProfilerEvents(double currentTime, const AlignedVector< Profiler::Event >& events)
{
	if (!m_recording)
		return;

	const double c_width = 8.0 / 60.0;
	const double c_eventAge = 2.0;

	m_chart->showRange(currentTime - c_width, currentTime);
	m_chart->removeTasksOlderThan(currentTime - c_eventAge);

	for (size_t i = 0; i < events.size(); ++i)
	{
		const Profiler::Event& e = events[i];

		if (m_threadIdToLane.find(e.threadId) == m_threadIdToLane.end())
		{
			m_threadIdToLane[e.threadId] = m_nextThreadLane;
			m_nextThreadLane++;
		}

		StringOutputStream ss;
		ss.setDecimals(1);
		ss << e.name << L" (" << ((e.end - e.start) * 1000.0f) << L" ms , " << e.alloc << L")";

		m_chart->addTask(
			m_threadIdToLane[e.threadId] * 8 + e.depth,
			ss.str(),
			c_threadColors[m_threadIdToLane[e.threadId] % sizeof_array(c_threadColors)],
			e.start,
			e.end
		);
	}
}

void ProfilerDialog::eventToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	if (event->getCommand() == L"Amalgam.Profiler.Capture")
		m_recording = !m_recording;
}

void ProfilerDialog::eventClose(ui::CloseEvent* event)
{
	destroy();
}

	}
}
