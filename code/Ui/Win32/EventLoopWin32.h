/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	virtual bool process(EventSubject* owner) T_OVERRIDE T_FINAL;
	
	virtual int32_t execute(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual void exit(int32_t exitCode) T_OVERRIDE T_FINAL;

	virtual int32_t getExitCode() const T_OVERRIDE T_FINAL;

	virtual int32_t getAsyncKeyState() const T_OVERRIDE T_FINAL;

	virtual bool isKeyDown(VirtualKey vk) const T_OVERRIDE T_FINAL;

	virtual Size getDesktopSize() const T_OVERRIDE T_FINAL;

private:
	int32_t m_exitCode;
	bool m_terminate;

	bool preTranslateMessage(EventSubject* owner, const MSG& msg);
};

	}
}

#endif	// traktor_ui_EventLoopWin32_H