#include "Amalgam/Editor/ProfilerDialog.h"
#include "Amalgam/Editor/TargetConnection.h"
#include "Amalgam/Editor/Ui/ProfilerControl.h"
#include "Ui/TableLayout.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ProfilerDialog", ProfilerDialog, ui::Dialog)

ProfilerDialog::ProfilerDialog(TargetConnection* connection)
:	m_connection(connection)
{
}

bool ProfilerDialog::create(ui::Widget* parent)
{
	if (!ui::Dialog::create(parent, L"Profiler", 500, 400, ui::Dialog::WsDefaultResizable, new ui::TableLayout(L"100%", L"100%", 0, 0)))
		return false;

	m_control = new ProfilerControl();
	m_control->create(this);

	addEventHandler< ui::CloseEvent >(this, &ProfilerDialog::eventClose);
	addEventHandler< ui::TimerEvent >(this, &ProfilerDialog::eventTimer);

	startTimer(10);

	update();
	show();

	return true;
}

void ProfilerDialog::destroy()
{
	ui::Dialog::destroy();
}

void ProfilerDialog::eventClose(ui::CloseEvent* event)
{
	destroy();
}

void ProfilerDialog::eventTimer(ui::TimerEvent* event)
{
	m_control->addFrame(
		m_connection->getPerformance().time,
		m_connection->getPerformance().frameMarkers
	);
}

	}
}
