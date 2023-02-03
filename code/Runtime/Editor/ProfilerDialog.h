/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Math/Variance.h"
#include "Runtime/Editor/TargetConnection.h"
#include "Ui/Dialog.h"

namespace traktor::ui
{

class BuildChartControl;
class GridView;
class ToolBar;
class ToolBarButtonClickEvent;

}

namespace traktor::runtime
{

/*! Visual profiler dialog.
 * \ingroup Runtime
 */
class ProfilerDialog
:	public ui::Dialog
,	public TargetConnection::IProfilerEventsCallback
{
	T_RTTI_CLASS;

public:
	explicit ProfilerDialog(TargetConnection* connection);

	bool create(ui::Widget* parent);

	virtual void destroy() override;

private:
	Ref< TargetConnection > m_connection;
	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::GridView > m_performanceGrid;
	Ref< ui::BuildChartControl > m_chart;
	SmallMap< uint32_t, int32_t > m_threadIdToLane;
	SmallMap< uint16_t, std::wstring > m_dictionary;
	Variance m_varianceUpdate;
	Variance m_varianceBuild;
	Variance m_varianceRenderCPU;
	Variance m_varianceRenderGPU;
	int32_t m_nextThreadLane;
	bool m_recording;

	virtual void receivedProfilerDictionary(const SmallMap< uint16_t, std::wstring >& dictionary) override final;

	virtual void receivedProfilerEvents(double currentTime, const Profiler::eventQueue_t& events) override final;

	virtual void receivedPerfSets() override final;

	void eventToolClick(ui::ToolBarButtonClickEvent* event);

	void eventClose(ui::CloseEvent* event);
};

}
