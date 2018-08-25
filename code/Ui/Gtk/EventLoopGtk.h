#ifndef traktor_ui_EventLoopGtk_H
#define traktor_ui_EventLoopGtk_H

#include <gtk/gtk.h>
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

	virtual int32_t execute(EventSubject* owner);

	virtual void exit(int32_t exitCode);

	virtual int32_t getExitCode() const;

	virtual int32_t getAsyncKeyState() const;

	virtual bool isKeyDown(VirtualKey vk) const;

	virtual Size getDesktopSize() const;
};

	}
}

#endif	// traktor_ui_EventLoopGtk_H

