/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <set>
#include <fontconfig/fontconfig.h>
#include <X11/Xresource.h>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/X11/BitmapX11.h"
#include "Ui/X11/ClipboardX11.h"
#include "Ui/X11/DialogX11.h"
#include "Ui/X11/EventLoopX11.h"
#include "Ui/X11/FormX11.h"
#include "Ui/X11/ToolFormX11.h"
#include "Ui/X11/UserWidgetX11.h"
#include "Ui/X11/WidgetFactoryX11.h"

namespace traktor::ui
{
	namespace
	{

int xerrorHandler(Display* display, XErrorEvent* ee)
{
	char msg[512] = { 0 };
	XGetErrorText(display, ee->error_code, msg, sizeof_array(msg));
	log::debug << L"X11 error: request_code " << (int32_t)ee->request_code << L", error_code " << (int32_t)ee->error_code << L" (" << mbstows(msg) << L")" << Endl;
	return 0;
}

	}

WidgetFactoryX11::WidgetFactoryX11()
{
	XIM xim;

	// Open connection to X display.
	Display* display = XOpenDisplay(nullptr);
	T_FATAL_ASSERT(display != nullptr);

    int screen = DefaultScreen(display);

	// Use our own X error handler.
	XSetErrorHandler(xerrorHandler);

	// Open input method.
	XSetLocaleModifiers("");
	if ((xim = XOpenIM(display, nullptr, nullptr, nullptr)) == 0)
	{
		XSetLocaleModifiers("@im=");
		if ((xim = XOpenIM(display, nullptr, nullptr, nullptr)) == 0)
		{
			log::error << L"Unable to open X11 input method." << Endl;
		}
	}

	// Create our context.
	m_context = new Context(display, screen, xim);
}

WidgetFactoryX11::~WidgetFactoryX11()
{
	// if (m_xim != 0)
	// {
	// 	XCloseIM(m_xim);
	// 	m_xim = 0;
	// }

	// if (m_context->getDisplay() != nullptr)
	// {
	// 	XCloseDisplay(m_context->getDisplay());
	// 	m_context->getDisplay() = nullptr;
	// }

	m_context = nullptr;
}

IEventLoop* WidgetFactoryX11::createEventLoop(EventSubject* owner)
{
	return new EventLoopX11(m_context);
}

IDialog* WidgetFactoryX11::createDialog(EventSubject* owner)
{
	return new DialogX11(m_context, owner);
}

IForm* WidgetFactoryX11::createForm(EventSubject* owner)
{
	return new FormX11(m_context, owner);
}

INotificationIcon* WidgetFactoryX11::createNotificationIcon(EventSubject* owner)
{
	return nullptr;
}

IPathDialog* WidgetFactoryX11::createPathDialog(EventSubject* owner)
{
	return nullptr;
}

IToolForm* WidgetFactoryX11::createToolForm(EventSubject* owner)
{
	return new ToolFormX11(m_context, owner);
}

IUserWidget* WidgetFactoryX11::createUserWidget(EventSubject* owner)
{
	return new UserWidgetX11(m_context, owner);
}

ISystemBitmap* WidgetFactoryX11::createBitmap()
{
	return new BitmapX11();
}

IClipboard* WidgetFactoryX11::createClipboard()
{
	return new ClipboardX11();
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

	std::set< std::wstring > fonts;
	for (int i = 0; fs && i < fs->nfont; ++i)
	{
		FcPattern* font = fs->fonts[i];
		if (font == nullptr)
			continue;

		FcChar8 *family;
		if (FcPatternGetString(font, FC_FAMILY, 0, &family) == FcResultMatch)
			fonts.insert(mbstows((const char*)family));
	}
	outFonts.insert(outFonts.begin(), fonts.begin(), fonts.end());

	FcFontSetDestroy(fs);
}

void WidgetFactoryX11::getDesktopRects(std::list< Rect >& outRects) const
{
	Atom type;
	int format;
	unsigned long num;
	unsigned long leftovers;
	uint8_t* data = nullptr;

	Atom atomWorkArea = XInternAtom(m_context->getDisplay(), "_NET_WORKAREA", False);
	Window win = XRootWindow(m_context->getDisplay(), m_context->getScreen());

	int result = XGetWindowProperty(
		m_context->getDisplay(),
		win,
		atomWorkArea,
		0,
		4 * 32,
		False,
		AnyPropertyType,
		&type,
		&format,
		&num,
		&leftovers,
		&data
	);

	if (result != Success)
	{
		int32_t width = DisplayWidth(m_context->getDisplay(), m_context->getScreen());
		int32_t height = DisplayHeight(m_context->getDisplay(), m_context->getScreen());
		outRects.push_back(Rect(
			0, 0,
			width, height
		));
		return;
	}

	const long* workareas = (const long*)data;
	for (int desktop = 0; desktop < num / 4; ++desktop)
	{
		outRects.push_back(Rect(
			workareas[desktop * 4],
			workareas[desktop * 4 + 1],
			workareas[desktop * 4 + 2],
			workareas[desktop * 4 + 3]
		));
	}
}

}
