/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Associative.h"
#include "Ui/EventSubject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class Widget;
class IBitmap;
class INotificationIcon;

/*! Notification icon.
 * \ingroup UI
 */
class T_DLLCLASS NotificationIcon
:	public EventSubject
,	public Associative
{
	T_RTTI_CLASS;

public:
	virtual ~NotificationIcon();

	bool create(const std::wstring& text, IBitmap* image);

	void destroy();

	void setImage(IBitmap* image);

private:
	INotificationIcon* m_ni = nullptr;
};

}
