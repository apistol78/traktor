#include <fontconfig/fontconfig.h>
#include <X11/Xresource.h>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/X11/BitmapX11.h"
#include "Ui/X11/ContainerX11.h"
#include "Ui/X11/ClipboardX11.h"
#include "Ui/X11/DialogX11.h"
#include "Ui/X11/EventLoopX11.h"
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
,	m_dpi(96)
{
	// Open connection to X display.
	m_display = XOpenDisplay(nullptr);
	T_FATAL_ASSERT(m_display != nullptr);

    m_screen = DefaultScreen(m_display);

	// Use our own X error handler.
	XSetErrorHandler(xerrorHandler);

#if 0
	// Get DPI from X.
	float wdots = (float)XDisplayWidth(m_display, m_screen);
	float winch = (float)XDisplayWidthMM(m_display, m_screen) * 0.0393700787f;
	m_dpi = (int32_t)(wdots / winch + 0.5f);

	// Get DPI from Xft.dpi
    XrmInitialize();
    char* resourceString = XResourceManagerString(m_display);
    XrmDatabase db = XrmGetStringDatabase(resourceString);
    if (resourceString != nullptr)
	{
    	char* type = nullptr;
	    XrmValue value;

        if (XrmGetResource(db, "Xft.dpi", "String", &type, &value) == True)
		{
            if (value.addr != nullptr)
                m_dpi = (int32_t)(std::atof(value.addr) + 0.5f);
        }
    }
#else
	// Get DPI from X, as dpi handling in X is severly broken we deduce either small or large only.
	float wdots = (float)XDisplayWidth(m_display, m_screen);
	float winch = (float)XDisplayWidthMM(m_display, m_screen) * 0.0393700787f;
	if (wdots / winch > 96.0f)
		m_dpi = 192;
	else
		m_dpi = 96;
#endif
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
    return m_dpi;
}

void WidgetFactoryX11::getSystemFonts(std::list< std::wstring >& outFonts)
{
	FcConfig* config = FcInitLoadConfigAndFonts();
	if (config == nullptr)
		return;

	FcPattern* pat = FcPatternCreate();
	if (pat == nullptr)
		return;

	FcObjectSet* os = FcObjectSetBuild(FC_FAMILY, FC_STYLE, (char*)nullptr);
	if (os == nullptr)
		return;

	FcFontSet* fs = FcFontList(config, pat, os);
	if (fs == nullptr)
		return;

	for (int i = 0; fs && i < fs->nfont; ++i)
	{
		FcPattern* font = fs->fonts[i];
		if (font == nullptr)
			continue;

		FcChar8 *family;
		if (FcPatternGetString(font, FC_FAMILY, 0, &family) == FcResultMatch)
		{
			outFonts.push_back(mbstows((const char*)family));
		}
	}

	FcFontSetDestroy(fs);
}

void WidgetFactoryX11::getDesktopRects(std::list< Rect >& outRects) const
{
	int32_t width = DisplayWidth(m_display, m_screen);
	int32_t height = DisplayHeight(m_display, m_screen);
	outRects.push_back(Rect(
		0, 0,
		width, height
	));
}

	}
}

