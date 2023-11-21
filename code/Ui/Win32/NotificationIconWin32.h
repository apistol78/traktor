/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Itf/INotificationIcon.h"
#include "Ui/Win32/Window.h"

namespace traktor::ui
{

class EventSubject;

/*!
 * \ingroup UIW32
 */
class NotificationIconWin32 : public INotificationIcon
{
public:
	explicit NotificationIconWin32(EventSubject* owner);

	virtual bool create(const std::wstring& text, ISystemBitmap* image);

	virtual void destroy();

	virtual void setImage(ISystemBitmap* image);

private:
	EventSubject* m_owner;
	Window m_hWnd;
	NOTIFYICONDATA m_nid;

	LRESULT eventNotification(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventTaskbarCreated(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);
};

}
