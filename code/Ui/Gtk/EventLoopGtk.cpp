#include "Ui/Gtk/EventLoopGtk.h"

namespace traktor
{
	namespace ui
	{

EventLoopGtk::EventLoopGtk()
{
	gtk_init(0, nullptr);
}

EventLoopGtk::~EventLoopGtk()
{
}

bool EventLoopGtk::process(EventSubject* owner)
{
	return false;
}

int32_t EventLoopGtk::execute(EventSubject* owner)
{
	gtk_main();
	return 0;
}

void EventLoopGtk::exit(int32_t exitCode)
{
	gtk_main_quit();
}

int32_t EventLoopGtk::getExitCode() const
{
	return 0;
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
	return Size(0, 0);
}

	}
}

