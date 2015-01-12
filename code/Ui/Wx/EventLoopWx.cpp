#include <wx/wx.h>
#include <wx/evtloop.h>
#include "Ui/Wx/EventLoopWx.h"
#include "Ui/Enums.h"

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

int EventLoopWx::execute(EventSubject* owner)
{
	return wxGetApp().MainLoop();
}

void EventLoopWx::exit(int exitCode)
{
	m_exitCode = exitCode;
	m_terminate = true;

	wxEventLoopBase* eventLoop = wxEventLoop::GetActive();

	if (eventLoop)
		eventLoop->Exit(exitCode);
}

int EventLoopWx::getExitCode() const
{
	return m_exitCode;
}

int EventLoopWx::getAsyncKeyState() const
{
	int keyState = KsNone;

	if (wxGetKeyState(WXK_CONTROL))
		keyState |= KsControl | KsCommand;
	if (wxGetKeyState(WXK_MENU))
		keyState |= KsMenu;
	if (wxGetKeyState(WXK_SHIFT))
		keyState |= KsShift;

	return keyState;
}

	}
}
