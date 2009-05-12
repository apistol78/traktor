#ifndef traktor_ui_EventLoopWx_H
#define traktor_ui_EventLoopWx_H

#include "Ui/Itf/IEventLoop.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_WX_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class T_DLLCLASS EventLoopWx : public IEventLoop
{
public:
	EventLoopWx();

	virtual bool process(EventSubject* owner);

	virtual int execute(EventSubject* owner);
	
	virtual void exit(int exitCode);

	virtual int getExitCode() const;

	virtual int getAsyncKeyState() const;

private:
	int m_exitCode;
	bool m_terminate;
};

	}
}

#endif	// traktor_ui_EventLoopWx_H
