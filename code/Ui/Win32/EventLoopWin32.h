#ifndef traktor_ui_EventLoopWin32_H
#define traktor_ui_EventLoopWin32_H

#include "Ui/Itf/IEventLoop.h"
#include "Ui/Win32/Window.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_WIN32_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class T_DLLCLASS EventLoopWin32 : public IEventLoop
{
public:
	EventLoopWin32();

	virtual ~EventLoopWin32();

	virtual bool process(EventSubject* owner);
	
	virtual int execute(EventSubject* owner);

	virtual void exit(int exitCode);

	virtual int getExitCode() const;

	virtual int getAsyncKeyState() const;

private:
	int m_exitCode;
	bool m_terminate;

	bool preTranslateMessage(EventSubject* owner, const MSG& msg);
};

	}
}

#endif	// traktor_ui_EventLoopWin32_H