/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include <string>
#include "Core/Config.h"
#include "Ui/Enums.h"
#include "Ui/Rect.h"

namespace traktor::ui
{

class EventSubject;

class IDialog;
class IEventLoop;
class IForm;
class INotificationIcon;
class IPathDialog;
class IToolForm;
class IUserWidget;
class ISystemBitmap;
class IClipboard;

/*! Widget factory interface.
 * \ingroup UI
 */
class IWidgetFactory
{
public:
	virtual ~IWidgetFactory() {}

	virtual IEventLoop* createEventLoop(EventSubject* owner) = 0;

	virtual IDialog* createDialog(EventSubject* owner) = 0;

	virtual IForm* createForm(EventSubject* owner) = 0;

	virtual INotificationIcon* createNotificationIcon(EventSubject* owner) = 0;

	virtual IPathDialog* createPathDialog(EventSubject* owner) = 0;

	virtual IToolForm* createToolForm(EventSubject* owner) = 0;

	virtual IUserWidget* createUserWidget(EventSubject* owner) = 0;

	virtual ISystemBitmap* createBitmap() = 0;

	virtual IClipboard* createClipboard() = 0;

	virtual void getSystemFonts(std::list< std::wstring >& outFonts) = 0;

	virtual void getDesktopRects(std::list< Rect >& outRects) const = 0;
};

}
