#ifndef traktor_ui_EventLoopGtk_H
#define traktor_ui_EventLoopGtk_H

#include "Ui/Itf/IEventLoop.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_WIN32_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace Gtk
{

	class Main;

}

namespace traktor
{
	namespace ui
	{

class T_DLLCLASS EventLoopGtk : public IEventLoop
{
public:
	EventLoopGtk();

	virtual ~EventLoopGtk();

	virtual bool process(EventSubject* owner);

	virtual int execute(EventSubject* owner);

	virtual void exit(int exitCode);

	virtual int getExitCode() const;

	virtual int getAsyncKeyState() const;

private:
	Gtk::Main* m_main;
};

	}
}

#endif	// traktor_ui_EventLoopGtk_H

