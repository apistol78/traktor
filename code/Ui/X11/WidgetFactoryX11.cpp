#include "Core/Log/Log.h"
#include "Ui/X11/BitmapX11.h"
#include "Ui/X11/ContainerX11.h"
#include "Ui/X11/ClipboardX11.h"
#include "Ui/X11/DialogX11.h"
#include "Ui/X11/EventLoopX11.h"
#include "Ui/X11/FileDialogX11.h"
#include "Ui/X11/FormX11.h"
#include "Ui/X11/ToolFormX11.h"
#include "Ui/X11/UserWidgetX11.h"
#include "Ui/X11/WidgetFactoryX11.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

int xerrorHandler(Display*, XErrorEvent* ee)
{
	log::error << L"X11 error occured, major code " << (int32_t)ee->error_code << Endl;
	return 0;
}

		}

WidgetFactoryX11::WidgetFactoryX11()
:	m_display(nullptr)
,	m_screen(0)
{
	m_display = XOpenDisplay(nullptr);
	T_FATAL_ASSERT(m_display != nullptr);

    m_screen = DefaultScreen(m_display);

	XSetErrorHandler(xerrorHandler);
}

WidgetFactoryX11::~WidgetFactoryX11()
{
	if (m_display != nullptr)
	{
		XCloseDisplay(m_display);
		m_display = nullptr;
	}
}

IEventLoop* WidgetFactoryX11::createEventLoop(EventSubject* owner)
{
	return new EventLoopX11(m_display, m_screen);
}

IContainer* WidgetFactoryX11::createContainer(EventSubject* owner)
{
	return new ContainerX11(owner, m_display, m_screen);
}

IDialog* WidgetFactoryX11::createDialog(EventSubject* owner)
{
	return new DialogX11(owner, m_display, m_screen);
}

IFileDialog* WidgetFactoryX11::createFileDialog(EventSubject* owner)
{
	return new FileDialogX11(owner);
}

IForm* WidgetFactoryX11::createForm(EventSubject* owner)
{
	return new FormX11(owner, m_display, m_screen);
}

INotificationIcon* WidgetFactoryX11::createNotificationIcon(EventSubject* owner)
{
	return 0;
}

IPathDialog* WidgetFactoryX11::createPathDialog(EventSubject* owner)
{
	return 0;
}

IToolForm* WidgetFactoryX11::createToolForm(EventSubject* owner)
{
	return new ToolFormX11(owner, m_display, m_screen);
}

IUserWidget* WidgetFactoryX11::createUserWidget(EventSubject* owner)
{
	return new UserWidgetX11(owner, m_display, m_screen);
}

IWebBrowser* WidgetFactoryX11::createWebBrowser(EventSubject* owner)
{
	return 0;
}

ISystemBitmap* WidgetFactoryX11::createBitmap()
{
	return new BitmapX11();
}

IClipboard* WidgetFactoryX11::createClipboard()
{
	return new ClipboardX11();
}

int32_t WidgetFactoryX11::getSystemDPI() const
{
	return 96 * 2;
}

bool WidgetFactoryX11::getSystemColor(SystemColor systemColor, Color4ub& outColor)
{
	return false;
}

void WidgetFactoryX11::getSystemFonts(std::list< std::wstring >& outFonts)
{
}

	}
}

