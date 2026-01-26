/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <set>
#include <fontconfig/fontconfig.h>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/WL/DialogWL.h"
#include "Ui/WL/FormWL.h"
#include "Ui/WL/UserWidgetWL.h"
#include "Ui/WL/WidgetFactoryWL.h"

namespace traktor::ui
{
	namespace
	{

wl_compositor *compositor;
wl_shm *shm;
wl_shell *shell;

void registry_global_handler
(
	void *data,
	struct wl_registry *registry,
	uint32_t name,
	const char *interface,
	uint32_t version
)
{
	// printf("interface: '%s', version: %u, name: %u\n", interface, version, name);
    if (strcmp(interface, "wl_compositor") == 0)
	{
        compositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 3);
    }
	else if (strcmp(interface, "wl_shm") == 0)
	{
        shm = (wl_shm*)wl_registry_bind(registry, name, &wl_shm_interface, 1);
    }
	else if (strcmp(interface, "wl_shell") == 0)
	{
        shell = (wl_shell*)wl_registry_bind(registry, name, &wl_shell_interface, 1);
    }
}

void registry_global_remove_handler
(
	void *data,
	struct wl_registry *registry,
	uint32_t name
)
{
	// printf("removed: %u\n", name);
}

	}

WidgetFactoryWL::WidgetFactoryWL()
{
	m_display = wl_display_connect(NULL);

	wl_registry* registry = wl_display_get_registry(m_display);
	wl_registry_listener registry_listener =
	{
		.global = registry_global_handler,
		.global_remove = registry_global_remove_handler
	};
	wl_registry_add_listener(registry, &registry_listener, nullptr);

	wl_display_roundtrip(m_display);
}

WidgetFactoryWL::~WidgetFactoryWL()
{
	wl_display_disconnect(m_display);
}

IEventLoop* WidgetFactoryWL::createEventLoop(EventSubject* owner)
{
	return nullptr;
}

IDialog* WidgetFactoryWL::createDialog(EventSubject* owner)
{
	return new DialogWL(owner);
}

IForm* WidgetFactoryWL::createForm(EventSubject* owner)
{
	return new FormWL(owner);
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
	return new UserWidgetWL(owner);
}

ISystemBitmap* WidgetFactoryWL::createBitmap()
{
	return nullptr;
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
