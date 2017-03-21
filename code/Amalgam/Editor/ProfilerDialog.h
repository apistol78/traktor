#ifndef traktor_amalgam_ProfilerDialog_H
#define traktor_amalgam_ProfilerDialog_H

#include "Amalgam/Editor/TargetConnection.h"
#include "Ui/Dialog.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
		
class BuildChartControl;
class ToolBar;
class ToolBarButtonClickEvent;

		}
	}

	namespace amalgam
	{

/*! \brief Visual profiler dialog.
 * \ingroup Amalgam
 */
class ProfilerDialog
:	public ui::Dialog
,	public TargetConnection::IProfilerEventsCallback
{
	T_RTTI_CLASS;

public:
	ProfilerDialog(TargetConnection* connection);

	bool create(ui::Widget* parent);

	virtual void destroy() T_OVERRIDE;

private:
	Ref< TargetConnection > m_connection;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::BuildChartControl > m_chart;
	std::map< uint32_t, int32_t > m_threadIdToLane;
	int32_t m_nextThreadLane;
	bool m_recording;

	virtual void receivedProfilerEvents(double currentTime, const AlignedVector< Profiler::Event >& events) T_OVERRIDE T_FINAL;

	void eventToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventClose(ui::CloseEvent* event);
};

	}
}

#endif	// traktor_amalgam_ProfilerDialog_H
