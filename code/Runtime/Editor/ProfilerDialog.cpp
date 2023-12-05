/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include "Runtime/Editor/ProfilerDialog.h"
#include "Runtime/Editor/TargetConnection.h"
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/Splitter.h"
#include "Ui/TableLayout.h"
#include "Ui/BuildChart/BuildChartControl.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor::runtime
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

Ref< ui::GridRow > createPerformanceRow(const std::wstring& name, const std::wstring& value)
{
	Ref< ui::GridRow > row = new ui::GridRow();
	row->add(name);
	row->add(value);
	return row;
}

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
	if (!ui::Dialog::create(
		parent,
		L"Profiler",
		900_ut,
		700_ut,
		ui::Dialog::WsCenterParent | ui::Dialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut)
	))
		return false;

	m_toolBar = new ui::ToolBar();
	m_toolBar->create(this);
	m_toolBar->addItem(new ui::ToolBarButton(L"Start/Stop", ui::Command(L"Runtime.Profiler.Capture")));
	m_toolBar->addItem(new ui::ToolBarButton(L"Reset", ui::Command(L"Runtime.Profiler.Reset")));
	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &ProfilerDialog::eventToolClick);

	Ref< ui::Splitter > splitter = new ui::Splitter();
	splitter->create(this, true, 30_ut, true);

	m_performanceGrid = new ui::GridView();
	m_performanceGrid->create(splitter, ui::GridView::WsColumnHeader | ui::WsAccelerated);
	m_performanceGrid->addColumn(new ui::GridColumn(L"Name", 150_ut));
	m_performanceGrid->addColumn(new ui::GridColumn(L"Value", 150_ut));

	m_chart = new ui::BuildChartControl();
	m_chart->create(splitter, 4 * 8, ui::WsAccelerated);
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

void ProfilerDialog::receivedProfilerEvents(double currentTime, const Profiler::eventQueue_t& events)
{
	if (!m_recording)
		return;

	// Remove all tasks older than 10 seconds.
	const double c_eventAge = 10.0;
	m_chart->removeTasksOlderThan(currentTime - c_eventAge);

	// Quantize time to 60 Hz.
	currentTime = currentTime - std::fmod(currentTime, 1.0f / 60.0f);

	m_chart->showTime(currentTime);

	double syncTime = -1.0;
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
			// Track time to application update event.
			if (e.start > syncTime && it->second == L"Application update")
				syncTime = e.end;

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

	if (syncTime > 0.0)
		m_chart->showTime(syncTime);
}

void ProfilerDialog::receivedPerfSets()
{
	m_performanceGrid->removeAllRows();

	const TpsRuntime& runtime = m_connection->getPerformance< TpsRuntime >();

	m_varianceUpdate.insert(runtime.update);
	m_varianceBuild.insert(runtime.build);
	m_varianceRenderCPU.insert(runtime.renderCPU);
	m_varianceRenderGPU.insert(runtime.renderGPU);

	m_performanceGrid->addRow(createPerformanceRow(L"FPS", str(L"%.2f", runtime.fps)));
	m_performanceGrid->addRow(createPerformanceRow(L"Update", str(L"%.2f ms (%.2f ms, %.2f)", runtime.update * 1000.0f, m_varianceUpdate.getMean() * 1000.0f, m_varianceUpdate.getVariance() * 1000.0f)));
	m_performanceGrid->addRow(createPerformanceRow(L"Build", str(L"%.2f ms (%.2f ms, %.2f)", runtime.build * 1000.0f, m_varianceBuild.getMean() * 1000.0f, m_varianceBuild.getVariance() * 1000.0f)));
	m_performanceGrid->addRow(createPerformanceRow(L"Render (CPU)", str(L"%.2f ms (%.2f ms, %.2f)", runtime.renderCPU * 1000.0f, m_varianceRenderCPU.getMean() * 1000.0f, m_varianceRenderCPU.getVariance() * 1000.0f)));
	m_performanceGrid->addRow(createPerformanceRow(L"Render (GPU)", str(L"%.2f ms (%.2f ms, %.2f)", runtime.renderGPU * 1000.0f, m_varianceRenderGPU.getMean() * 1000.0f, m_varianceRenderGPU.getVariance() * 1000.0f)));
	m_performanceGrid->addRow(createPerformanceRow(L"Render Interval", str(L"%.1f", runtime.renderInterval)));
	m_performanceGrid->addRow(createPerformanceRow(L"Physics", str(L"%.2f ms", runtime.physics * 1000.0f)));
	m_performanceGrid->addRow(createPerformanceRow(L"Input", str(L"%.2f ms", runtime.input * 1000.0f)));
	m_performanceGrid->addRow(createPerformanceRow(L"Garbage Collect", str(L"%.2f ms", runtime.garbageCollect * 1000.0f)));
	m_performanceGrid->addRow(createPerformanceRow(L"Simulation Steps", str(L"%d", runtime.steps)));
	m_performanceGrid->addRow(createPerformanceRow(L"Simulation Interval", str(L"%.1f", runtime.simulationInterval)));

	const TpsMemory& memory = m_connection->getPerformance< TpsMemory >();
	m_performanceGrid->addRow(createPerformanceRow(L"Memory (Native)", str(L"%d KiB", memory.memInUse / 1024)));
	m_performanceGrid->addRow(createPerformanceRow(L"Memory (Script)", str(L"%d KiB", memory.memInUseScript / 1024)));
	m_performanceGrid->addRow(createPerformanceRow(L"Heap Objects", str(L"%d", memory.heapObjects)));

	const TpsRender& render = m_connection->getPerformance< TpsRender >();
	m_performanceGrid->addRow(createPerformanceRow(L"Memory (GPU) Available", str(L"%d KiB", render.renderSystemStats.memoryAvailable / 1024)));
	m_performanceGrid->addRow(createPerformanceRow(L"Memory (GPU) Usage", str(L"%d KiB", render.renderSystemStats.memoryUsage / 1024)));
	m_performanceGrid->addRow(createPerformanceRow(L"Buffers", str(L"%d", render.renderSystemStats.buffers)));
	m_performanceGrid->addRow(createPerformanceRow(L"Simple Textures", str(L"%d", render.renderSystemStats.simpleTextures)));
	m_performanceGrid->addRow(createPerformanceRow(L"Cube Textures", str(L"%d", render.renderSystemStats.cubeTextures)));
	m_performanceGrid->addRow(createPerformanceRow(L"Volume Textures", str(L"%d", render.renderSystemStats.volumeTextures)));
	m_performanceGrid->addRow(createPerformanceRow(L"Render Target Sets", str(L"%d", render.renderSystemStats.renderTargetSets)));
	m_performanceGrid->addRow(createPerformanceRow(L"Programs", str(L"%d", render.renderSystemStats.programs)));
	m_performanceGrid->addRow(createPerformanceRow(L"Render Passes", str(L"%d", render.renderViewStats.passCount)));
	m_performanceGrid->addRow(createPerformanceRow(L"Draw Calls", str(L"%d", render.renderViewStats.drawCalls)));
	m_performanceGrid->addRow(createPerformanceRow(L"Primitives", str(L"%d", render.renderViewStats.primitiveCount)));

	const TpsResource& resource = m_connection->getPerformance< TpsResource >();
	m_performanceGrid->addRow(createPerformanceRow(L"Resident Resources", str(L"%d", resource.residentResourcesCount)));
	m_performanceGrid->addRow(createPerformanceRow(L"Exclusive Resources", str(L"%d", resource.exclusiveResourcesCount)));

	const TpsPhysics& physics = m_connection->getPerformance< TpsPhysics >();
	m_performanceGrid->addRow(createPerformanceRow(L"Bodies", str(L"%d", physics.bodyCount)));
	m_performanceGrid->addRow(createPerformanceRow(L"Active Bodies", str(L"%d", physics.activeBodyCount)));
	m_performanceGrid->addRow(createPerformanceRow(L"Collision Manifolds", str(L"%d", physics.manifoldCount)));
	m_performanceGrid->addRow(createPerformanceRow(L"Queries", str(L"%d", physics.queryCount)));

	const TpsAudio& audio = m_connection->getPerformance< TpsAudio >();
	m_performanceGrid->addRow(createPerformanceRow(L"Active Sound Channels", str(L"%d", audio.activeSoundChannels)));
}

void ProfilerDialog::eventToolClick(ui::ToolBarButtonClickEvent* event)
{
	if (event->getCommand() == L"Runtime.Profiler.Capture")
		m_recording = !m_recording;
	else if (event->getCommand() == L"Runtime.Profiler.Reset")
	{
		double currentTime = m_chart->positionToTime(getInnerRect().getWidth() / 2);
		m_chart->showRange(currentTime - 0.5f / 60.0f, currentTime + 0.5f / 60.0f);

		m_varianceUpdate = Variance();
		m_varianceBuild = Variance();
		m_varianceRenderCPU = Variance();
		m_varianceRenderGPU = Variance();
	}
}

void ProfilerDialog::eventClose(ui::CloseEvent* event)
{
	destroy();
}

}
