/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <wx/wx.h>
#include <wx/evtloop.h>
#include "Ui/Enums.h"
#include "Ui/Wx/EventLoopWx.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

class CustomApp : public wxApp
{
public:
	virtual bool OnInit();
};

IMPLEMENT_APP_NO_MAIN(CustomApp)

bool CustomApp::OnInit()
{
	return true;
}

		}

EventLoopWx::EventLoopWx()
:	m_exitCode(0)
,	m_terminate(false)
{
	wxInitialize();

	wxEventLoop* eventLoop = new wxEventLoop();
	wxEventLoop::SetActive(eventLoop);
}

bool EventLoopWx::process(EventSubject* owner)
{
	if (m_terminate)
		return false;

	wxGetApp().Yield();

	return true;
}

int32_t EventLoopWx::execute(EventSubject* owner)
{
	return wxGetApp().MainLoop();
}

void EventLoopWx::exit(int32_t exitCode)
{
	m_exitCode = exitCode;
	m_terminate = true;

	wxEventLoopBase* eventLoop = wxEventLoop::GetActive();

	if (eventLoop)
		eventLoop->Exit(exitCode);
}

int32_t EventLoopWx::getExitCode() const
{
	return m_exitCode;
}

int32_t EventLoopWx::getAsyncKeyState() const
{
	int32_t keyState = KsNone;

	if (wxGetKeyState(WXK_CONTROL))
		keyState |= KsControl | KsCommand;
	if (wxGetKeyState(WXK_MENU))
		keyState |= KsMenu;
	if (wxGetKeyState(WXK_SHIFT))
		keyState |= KsShift;

	return keyState;
}

bool EventLoopWx::isKeyDown(VirtualKey vk) const
{
	return false;	
}

Size EventLoopWx::getDesktopSize() const
{
	return Size(1280, 720);
}

	}
}
