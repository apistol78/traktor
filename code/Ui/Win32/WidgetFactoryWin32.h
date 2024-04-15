/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Itf/IWidgetFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_WIN32_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*!
 * \ingroup UIW32
 */
class T_DLLCLASS WidgetFactoryWin32 : public IWidgetFactory
{
public:
	virtual IEventLoop* createEventLoop(EventSubject* owner) override final;

	virtual IDialog* createDialog(EventSubject* owner) override final;

	virtual IForm* createForm(EventSubject* owner) override final;

	virtual INotificationIcon* createNotificationIcon(EventSubject* owner) override final;

	virtual IPathDialog* createPathDialog(EventSubject* owner) override final;

	virtual IToolForm* createToolForm(EventSubject* owner) override final;

	virtual IUserWidget* createUserWidget(EventSubject* owner) override final;

	virtual ISystemBitmap* createBitmap() override final;

	virtual IClipboard* createClipboard() override final;

	virtual void getSystemFonts(std::list< std::wstring >& outFonts) override final;

	virtual void getDesktopRects(std::list< Rect >& outRects) const override final;
};

}

