/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Itf/IUserWidget.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor::ui
{

/*!
 * \ingroup UIW32
 */
class UserWidgetWin32 : public WidgetWin32Impl< IUserWidget >
{
public:
	explicit UserWidgetWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

private:
	LRESULT eventButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);
};

}
