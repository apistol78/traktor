/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Win32/ContainerWin32.h"

namespace traktor::ui
{

ContainerWin32::ContainerWin32(EventSubject* owner)
:	WidgetWin32Impl< IContainer >(owner)
{
}

bool ContainerWin32::create(IWidget* parent, int style)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	DWORD dwStyleEx = WS_EX_CONTROLPARENT;

	if (style & WsBorder)
		dwStyle |= WS_BORDER;
	if (style & WsClientBorder)
		dwStyleEx |= WS_EX_CLIENTEDGE;

	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		_T("TraktorWin32Class"),
		_T(""),
		dwStyle,
		dwStyleEx,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0
	))
		return false;

	if (!WidgetWin32Impl::create(style))
		return false;

	m_ownCursor = true;
	return true;
}

}
