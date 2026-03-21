/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <set>
#include <fontconfig/fontconfig.h>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/Wl/BitmapWl.h"
#include "Ui/Wl/ClipboardWl.h"
#include "Ui/Wl/DialogWl.h"
#include "Ui/Wl/EventLoopWl.h"
#include "Ui/Wl/FormWl.h"
#include "Ui/Wl/ToolFormWl.h"
#include "Ui/Wl/UserWidgetWl.h"
#include "Ui/Wl/WidgetFactoryWl.h"

namespace traktor::ui
{

WidgetFactoryWl::WidgetFactoryWl()
{
	m_context = new ContextWl();
	if (!m_context->initialize())
	{
		log::error << L"Failed to initialize Wayland context." << Endl;
		m_context = nullptr;
	}
}

WidgetFactoryWl::~WidgetFactoryWl()
{
	m_context = nullptr;
}

IEventLoop* WidgetFactoryWl::createEventLoop(EventSubject* owner)
{
	return new EventLoopWl(m_context);
}

IDialog* WidgetFactoryWl::createDialog(EventSubject* owner)
{
	return new DialogWl(m_context, owner);
}

IForm* WidgetFactoryWl::createForm(EventSubject* owner)
{
	return new FormWl(m_context, owner);
}

INotificationIcon* WidgetFactoryWl::createNotificationIcon(EventSubject* owner)
{
	return nullptr;
}

IPathDialog* WidgetFactoryWl::createPathDialog(EventSubject* owner)
{
	return nullptr;
}

IToolForm* WidgetFactoryWl::createToolForm(EventSubject* owner)
{
	return new ToolFormWl(m_context, owner);
}

IUserWidget* WidgetFactoryWl::createUserWidget(EventSubject* owner)
{
	return new UserWidgetWl(m_context, owner);
}

ISystemBitmap* WidgetFactoryWl::createBitmap()
{
	return new BitmapWl();
}

IClipboard* WidgetFactoryWl::createClipboard()
{
	return new ClipboardWl();
}

void WidgetFactoryWl::getSystemFonts(std::list< std::wstring >& outFonts)
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

void WidgetFactoryWl::getDesktopRects(std::list< Rect >& outRects) const
{
	// Return device-pixel dimensions.  In a complete implementation we'd
	// track actual output geometry from wl_output events.
	const int32_t scale = m_context->getOutputScale();
	outRects.push_back(Rect(0, 0, 1920 * scale, 1080 * scale));
}

}
