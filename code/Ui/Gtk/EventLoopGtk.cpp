#include "Ui/Gtk/EventLoopGtk.h"

namespace traktor
{
	namespace ui
	{

EventLoopGtk::EventLoopGtk()
:	m_terminated(false)
,	m_exitCode(0)
{
	gtk_init(0, nullptr);
}

EventLoopGtk::~EventLoopGtk()
{
}

bool EventLoopGtk::process(EventSubject* owner)
{
	if (m_terminated)
		return false;

	gtk_main_iteration_do(FALSE);
	return true;
}

int32_t EventLoopGtk::execute(EventSubject* owner)
{
	gtk_main();
	return m_exitCode;
}

void EventLoopGtk::exit(int32_t exitCode)
{
	m_exitCode = exitCode;
	m_terminated = true;
	gtk_main_quit();
}

int32_t EventLoopGtk::getExitCode() const
{
	return m_exitCode;
}

int32_t EventLoopGtk::getAsyncKeyState() const
{
	return 0;
}

bool EventLoopGtk::isKeyDown(VirtualKey vk) const
{
	return false;
}

Size EventLoopGtk::getDesktopSize() const
{
	GdkDisplay* display = gdk_display_get_default();
	T_FATAL_ASSERT(display != nullptr);

	GdkMonitor* monitor = gdk_display_get_primary_monitor(display);
	T_FATAL_ASSERT(monitor != nullptr);

	GdkRectangle wa;
	gdk_monitor_get_workarea(monitor, &wa);
	
	return Size(wa.width, wa.height);
}

	}
}

