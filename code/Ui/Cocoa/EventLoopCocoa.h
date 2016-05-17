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

	virtual bool process(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual int execute(EventSubject* owner) T_OVERRIDE T_FINAL;
	
	virtual void exit(int exitCode) T_OVERRIDE T_FINAL;

	virtual int getExitCode() const T_OVERRIDE T_FINAL;

	virtual int getAsyncKeyState() const T_OVERRIDE T_FINAL;
	
private:
	void* m_pool;
	bool m_launching;
	int m_exitCode;
	bool m_terminated;
	uint32_t m_modifierFlags;
	bool m_idleMode;
	
	bool handleGlobalEvents(EventSubject* owner, void* event);
};

	}
}

#endif	// traktor_ui_EventLoopCocoa_H
