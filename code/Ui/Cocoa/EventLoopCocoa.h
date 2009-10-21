#ifndef traktor_ui_EventLoopCocoa_H
#define traktor_ui_EventLoopCocoa_H

#include "Ui/Itf/IEventLoop.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_COCOA_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class T_DLLCLASS EventLoopCocoa : public IEventLoop
{
public:
	EventLoopCocoa();
	
	virtual ~EventLoopCocoa();

	virtual bool process(EventSubject* owner);

	virtual int execute(EventSubject* owner);
	
	virtual void exit(int exitCode);

	virtual int getExitCode() const;

	virtual int getAsyncKeyState() const;
	
private:
	struct NSAutoreleasePool* m_pool;
	bool m_launching;
	int m_exitCode;
	bool m_terminated;
	uint32_t m_modifierFlags;
};

	}
}

#endif	// traktor_ui_EventLoopCocoa_H
