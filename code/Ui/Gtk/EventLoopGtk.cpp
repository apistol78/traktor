#include <gtkmm.h>
#include "Ui/Gtk/EventLoopGtk.h"

namespace traktor
{
	namespace ui
	{

EventLoopGtk::EventLoopGtk()
{
	char* argv[] = { 0, 0 };
	int argc = 0;

	//gtk_init(&argc, (char***)&argv);
	char** dummy = (char**)argv;
	m_main = new Gtk::Main(argc, dummy, false);
}

EventLoopGtk::~EventLoopGtk()
{
	delete m_main;
}

bool EventLoopGtk::process()
{
	return false;
}

int EventLoopGtk::execute()
{
	//gtk_main();
	m_main->run();
	return 0;
}

void EventLoopGtk::exit(int exitCode)
{
	//gtk_main_quit();
}

int EventLoopGtk::getExitCode() const
{
	return 0;
}

int EventLoopGtk::getAsyncKeyState() const
{
	return 0;
}

	}
}

