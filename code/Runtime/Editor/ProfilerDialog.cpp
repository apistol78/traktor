#include "Runtime/Editor/ProfilerDialog.h"
#include "Runtime/Editor/TargetConnection.h"
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/TableLayout.h"
#include "Ui/BuildChart/BuildChartControl.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor
{
	namespace runtime
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.ProfilerDialog", ProfilerDialog, ui::Dialog)

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

	m_toolBar = new ui::ToolBar();
	m_toolBar->create(this);
	m_toolBar->addItem(new ui::ToolBarButton(L"Start/Stop", ui::Command(L"Runtime.Profiler.Capture")));
	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &ProfilerDialog::eventToolClick);

	m_chart = new ui::BuildChartControl();
	m_chart->create(this, 4 * 8, ui::WsAccelerated);
	m_chart->showRange(0.0, 1.0 / 60.0);

	addEventHandler< ui::CloseEvent >(this, &ProfilerDialog::eventClose);

	update();
	show();

	m_connection->setProfilerEventsCallback(this);
	return true;
}

void ProfilerDialog::destroy()
{
	m_connection->setProfilerEventsCallback(nullptr);
	ui::Dialog::destroy();
}

void ProfilerDialog::receivedProfilerDictionary(const SmallMap< uint16_t, std::wstring >& dictionary)
{
	m_dictionary = dictionary;
}

void ProfilerDialog::receivedProfilerEvents(double currentTime, const AlignedVector< Profiler::Event >& events)
{
	if (!m_recording)
		return;

	const double c_eventAge = 10.0;

	m_chart->showTime(currentTime);
	m_chart->removeTasksOlderThan(currentTime - c_eventAge);

	for (size_t i = 0; i < events.size(); ++i)
	{
		const Profiler::Event& e = events[i];

		if (m_threadIdToLane.find(e.threadId) == m_threadIdToLane.end())
		{
			m_threadIdToLane[e.threadId] = m_nextThreadLane;
			m_nextThreadLane++;
		}

		auto it = m_dictionary.find(e.name);
		if (it != m_dictionary.end())
		{
			m_chart->addTask(
				m_threadIdToLane[e.threadId] * 8 + e.depth,
				str(L"%ls (%.2f ms)", it->second.c_str(), (e.end - e.start) * 1000.0f),
				c_threadColors[m_threadIdToLane[e.threadId] % sizeof_array(c_threadColors)],
				e.start,
				e.end
			);
		}
		else
		{
			m_chart->addTask(
				m_threadIdToLane[e.threadId] * 8 + e.depth,
				str(L"%d (%.2f ms)", e.name, (e.end - e.start) * 1000.0f),
				c_threadColors[m_threadIdToLane[e.threadId] % sizeof_array(c_threadColors)],
				e.start,
				e.end
			);
		}
	}
}

void ProfilerDialog::eventToolClick(ui::ToolBarButtonClickEvent* event)
{
	if (event->getCommand() == L"Runtime.Profiler.Capture")
		m_recording = !m_recording;
}

void ProfilerDialog::eventClose(ui::CloseEvent* event)
{
	destroy();
}

	}
}
