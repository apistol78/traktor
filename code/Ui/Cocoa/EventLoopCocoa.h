#pragma once

#include <vector>
#include "Ui/Itf/IEventLoop.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_COCOA_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

#if __OBJC__
@class NSEvent;
@class NSWindow;
#else
typedef void NSEvent;
typedef void NSWindow;
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

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool process(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual int32_t execute(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual void exit(int32_t exitCode) T_OVERRIDE T_FINAL;

	virtual int32_t getExitCode() const T_OVERRIDE T_FINAL;

	virtual int32_t getAsyncKeyState() const T_OVERRIDE T_FINAL;

	virtual bool isKeyDown(VirtualKey vk) const T_OVERRIDE T_FINAL;

	void pushModal(NSWindow* modalWindow);

	void popModal();

private:
	bool m_launching;
	int32_t m_exitCode;
	bool m_terminated;
	uint32_t m_modifierFlags;
	bool m_idleMode;
	std::vector< NSWindow* > m_modalWindows;

	bool handleGlobalEvents(EventSubject* owner, NSEvent* event);
};

	}
}

