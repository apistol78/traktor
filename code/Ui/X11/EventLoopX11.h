#ifndef traktor_ui_EventLoopX11_H
#define traktor_ui_EventLoopX11_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "Ui/Itf/IEventLoop.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_X11_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class T_DLLCLASS EventLoopX11 : public IEventLoop
{
public:
	EventLoopX11(Display* display, int32_t screen);

	virtual ~EventLoopX11();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool process(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual int32_t execute(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual void exit(int32_t exitCode) T_OVERRIDE T_FINAL;

	virtual int32_t getExitCode() const T_OVERRIDE T_FINAL;

	virtual int32_t getAsyncKeyState() const T_OVERRIDE T_FINAL;

	virtual bool isKeyDown(VirtualKey vk) const T_OVERRIDE T_FINAL;

	virtual Size getDesktopSize() const T_OVERRIDE T_FINAL;

private:
	Display* m_display;
	int32_t m_screen;
	XIM m_xim;
	XIC m_xic;
	bool m_terminated;
	int32_t m_exitCode;
	int32_t m_keyState;

	bool preTranslateEvent(EventSubject* owner, XEvent& e);
};

	}
}

#endif	// traktor_ui_EventLoopX11_H

