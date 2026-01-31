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
#include "Ui/WL/BitmapWL.h"
#include "Ui/WL/Context.h"
#include "Ui/WL/DialogWL.h"
#include "Ui/WL/EventLoopWL.h"
#include "Ui/WL/FormWL.h"
#include "Ui/WL/UserWidgetWL.h"
#include "Ui/WL/WidgetFactoryWL.h"

namespace traktor::ui
{

WidgetFactoryWL::WidgetFactoryWL()
{
	m_context = Context::create();
	T_FATAL_ASSERT(m_context);
}

WidgetFactoryWL::~WidgetFactoryWL()
{
}

IEventLoop* WidgetFactoryWL::createEventLoop(EventSubject* owner)
{
	return new EventLoopWL(m_context);
}

IDialog* WidgetFactoryWL::createDialog(EventSubject* owner)
{
	return new DialogWL(m_context, owner);
}

IForm* WidgetFactoryWL::createForm(EventSubject* owner)
{
	return new FormWL(m_context, owner);
}

INotificationIcon* WidgetFactoryWL::createNotificationIcon(EventSubject* owner)
{
	return nullptr;
}

IPathDialog* WidgetFactoryWL::createPathDialog(EventSubject* owner)
{
	return nullptr;
}

IToolForm* WidgetFactoryWL::createToolForm(EventSubject* owner)
{
	return nullptr;
}

IUserWidget* WidgetFactoryWL::createUserWidget(EventSubject* owner)
{
	return new UserWidgetWL(m_context, owner);
}

ISystemBitmap* WidgetFactoryWL::createBitmap()
{
	return new BitmapWL();
}

IClipboard* WidgetFactoryWL::createClipboard()
{
	return nullptr;
}

void WidgetFactoryWL::getSystemFonts(std::list< std::wstring >& outFonts)
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

void WidgetFactoryWL::getDesktopRects(std::list< Rect >& outRects) const
{
	return;
}

}
