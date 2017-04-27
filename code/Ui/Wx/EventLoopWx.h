/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_EventLoopWx_H
#define traktor_ui_EventLoopWx_H

#include "Ui/Itf/IEventLoop.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_WX_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class T_DLLCLASS EventLoopWx : public IEventLoop
{
public:
	EventLoopWx();

	virtual bool process(EventSubject* owner) T_OVERRIDE T_FINAL;

	virtual int32_t execute(EventSubject* owner) T_OVERRIDE T_FINAL;
	
	virtual void exit(int32_t exitCode) T_OVERRIDE T_FINAL;

	virtual int32_t getExitCode() const T_OVERRIDE T_FINAL;

	virtual int32_t getAsyncKeyState() const T_OVERRIDE T_FINAL;

	virtual bool isKeyDown(VirtualKey vk) const T_OVERRIDE T_FINAL;

private:
	int32_t m_exitCode;
	bool m_terminate;
};

	}
}

#endif	// traktor_ui_EventLoopWx_H
