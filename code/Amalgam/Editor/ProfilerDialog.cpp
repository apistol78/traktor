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

const struct
{
	const wchar_t* name;
	Color4ub color;
}
c_markers[] =
{
	{ 0/*L"Frame"*/, Color4ub(255, 255, 120) },		// FptEndFrame
	{ L"Render update", Color4ub(200, 200, 80) },	// FptRenderServerUpdate
	{ L"Session", Color4ub(255, 120, 255) },		// FptSessionManagerUpdate
	{ L"Script GC", Color4ub(200, 80, 200) },		// FptScriptGC
	{ L"Audio", Color4ub(120, 255, 255) },			// FptAudioServerUpdate
	{ L"Rumble", Color4ub(80, 200, 200) },			// FptRumbleUpdate
	{ L"Input", Color4ub(255, 120, 120) },			// FptInputServerUpdate
	{ L"State", Color4ub(200, 80, 80) },			// FptStateUpdate
	{ L"Physics", Color4ub(120, 255, 120) },		// FptPhysicsServerUpdate
	{ L"Build", Color4ub(80, 200, 80) },			// FptBuildFrame
	{ L"Audio update", Color4ub(120, 120, 255) },	// FptAudioLayerUpdate
	{ L"Flash update", Color4ub(80, 80, 200) },		// FptFlashLayerUpdate
	{ L"Flash build", Color4ub(80, 200, 80) },		// FptFlashLayerBuild
	{ L"Video update", Color4ub(200, 80, 80) },		// FptVideoLayerUpdate
	{ L"World update", Color4ub(200, 80, 200) },	// FptWorldLayerUpdate
	{ L"World build", Color4ub(200, 200, 80) },		// FptWorldLayerBuild
	{ L"Script", Color4ub(255, 80, 128) }			// FptScript
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ProfilerDialog", ProfilerDialog, ui::Dialog)

ProfilerDialog::ProfilerDialog(TargetConnection* connection)
:	m_connection(connection)
{
}

bool ProfilerDialog::create(ui::Widget* parent)
{
	if (!ui::Dialog::create(parent, L"Profiler", ui::scaleBySystemDPI(800), ui::scaleBySystemDPI(180), ui::Dialog::WsDefaultResizable, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	m_toolBar = new ui::custom::ToolBar();
	m_toolBar->create(this);
	m_toolBar->addItem(new ui::custom::ToolBarButton(L"Capture", ui::Command(L"Amalgam.Profiler.Capture")));
	m_toolBar->addItem(new ui::custom::ToolBarButton(L"Show All", ui::Command(L"Amalgam.Profiler.ShowAll")));
	m_toolBar->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &ProfilerDialog::eventToolClick);

	m_chart = new ui::custom::BuildChartControl();
	m_chart->create(this, 3);

	addEventHandler< ui::CloseEvent >(this, &ProfilerDialog::eventClose);

	update();
	show();

	return true;
}

void ProfilerDialog::destroy()
{
	ui::Dialog::destroy();
}

void ProfilerDialog::eventToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	if (event->getCommand() == L"Amalgam.Profiler.Capture")
	{
		const AlignedVector< TargetPerformance::FrameMarker >& markers = m_connection->getPerformance().frameMarkers;
		float time = m_connection->getPerformance().time;

		float timeMin = 0.0f;
		float timeMax = 0.0f;

		for (uint32_t i = 0; i < markers.size(); ++i)
		{
			const TargetPerformance::FrameMarker& fm = markers[i];
			timeMin = std::min(timeMin, fm.begin);
			timeMax = std::max(timeMax, fm.end);
		}

		float duration = std::max(timeMax - timeMin, 1.0f / 60.0f);
		m_chart->showRange(0.0, duration);

		m_chart->removeAllTasks();
		for (uint32_t i = 0; i < markers.size(); ++i)
		{
			const TargetPerformance::FrameMarker& fm = markers[i];
			if (!c_markers[fm.id].name)
				continue;

			StringOutputStream ss;
			ss.setDecimals(1);
			ss << c_markers[fm.id].name << L" (" << ((fm.end - fm.begin) * 1000.0f) << L" ms)";

			m_chart->addTask(
				fm.level,
				ss.str(),
				c_markers[fm.id].color,
				fm.begin - timeMin,
				fm.end - timeMin
			);
		}
	}
	else if (event->getCommand() == L"Amalgam.Profiler.ShowAll")
	{
		const AlignedVector< TargetPerformance::FrameMarker >& markers = m_connection->getPerformance().frameMarkers;
		float time = m_connection->getPerformance().time;

		float timeMin = 0.0f;
		float timeMax = 0.0f;

		for (uint32_t i = 0; i < markers.size(); ++i)
		{
			const TargetPerformance::FrameMarker& fm = markers[i];
			timeMin = std::min(timeMin, fm.begin);
			timeMax = std::max(timeMax, fm.end);
		}

		float duration = std::max(timeMax - timeMin, 1.0f / 60.0f);
		m_chart->showRange(0.0, duration);
	}
}

void ProfilerDialog::eventClose(ui::CloseEvent* event)
{
	destroy();
}

	}
}
