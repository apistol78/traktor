#pragma once

#include "Core/Containers/SmallMap.h"
#include "Runtime/Editor/TargetConnection.h"
#include "Ui/Dialog.h"

namespace traktor
{
	namespace ui
	{

class BuildChartControl;
class ToolBar;
class ToolBarButtonClickEvent;

	}

	namespace runtime
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
	ProfilerDialog(TargetConnection* connection);

	bool create(ui::Widget* parent);

	virtual void destroy() override;

private:
	Ref< TargetConnection > m_connection;
	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::BuildChartControl > m_chart;
	SmallMap< uint32_t, int32_t > m_threadIdToLane;
	SmallMap< uint16_t, std::wstring > m_dictionary;
	int32_t m_nextThreadLane;
	bool m_recording;

	virtual void receivedProfilerDictionary(const SmallMap< uint16_t, std::wstring >& dictionary) override final;

	virtual void receivedProfilerEvents(double currentTime, const AlignedVector< Profiler::Event >& events) override final;

	void eventToolClick(ui::ToolBarButtonClickEvent* event);

	void eventClose(ui::CloseEvent* event);
};

	}
}

