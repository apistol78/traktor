/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#define _WIN32_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
#	define _WIN32_WINNT 0x0501
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>
#include <tchar.h>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"
#include "Ui/Win32/SmartHandle.h"

namespace traktor
{
	namespace ui
	{

#define WM_REFLECTED_COMMAND			(WM_USER + 1000)
#define WM_REFLECTED_NOTIFY				(WM_USER + 1001)
#define WM_REFLECTED_HSCROLL			(WM_USER + 1002)
#define WM_REFLECTED_VSCROLL			(WM_USER + 1003)
#define WM_REFLECTED_CTLCOLORSTATIC		(WM_USER + 1004)
#define WM_REFLECTED_CTLCOLOREDIT		(WM_USER + 1005)
#define WM_REFLECTED_CTLCOLORBTN		(WM_USER + 1006)
#define WM_REFLECTED_CTLCOLORLISTBOX	(WM_USER + 1007)

/*!
 * \ingroup UIW32
 */
struct IMessageHandler : public Object
{
	virtual LRESULT handle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass) = 0;
};

/*!
 * \ingroup UIW32
 */
template < typename ClassType >
struct MethodMessageHandler : public IMessageHandler
{
	typedef LRESULT (ClassType::*MethodType)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	ClassType* m_object;
	MethodType m_method;

	MethodMessageHandler(ClassType* object, MethodType method)
	:	m_object(object)
	,	m_method(method)
	{
	}

	virtual LRESULT handle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
	{
		return (m_object->*m_method)(hWnd, message, wParam, lParam, pass);
	}
};

/*!
 * \ingroup UIW32
 */
class Window
{
public:
	Window();

	virtual ~Window();

	bool create(
		HWND hParentWnd,
		LPCTSTR className,
		LPCTSTR text,
		UINT style,
		UINT styleEx,
		int left,
		int top,
		int width,
		int height,
		int id = 0,
		bool subClass = false
	);

	bool subClass(HWND hWnd);

	void setFont(HFONT hFont);

	HFONT getFont() const;

	int32_t dpi() const;

	int32_t dpi96(int32_t measure) const;

	int32_t invdpi96(int32_t measure) const;

	LRESULT sendMessage(UINT message, WPARAM wParam, LPARAM lParam) const;

	operator HWND () const;

	void registerMessageHandler(UINT message, IMessageHandler* messageHandler);

	void registerMessageHandler(const wchar_t* message, IMessageHandler* messageHandler);

	static void registerDefaultClass();

	static void unregisterDefaultClass();

	static void registerDialogClass();

	static void unregisterDialogClass();

private:
	HWND m_hWnd;
	SmartFont m_hFont;
	WNDPROC m_originalWndProc;
	SmallMap< UINT, Ref< IMessageHandler > > m_messageHandlers;

	static LRESULT invokeMessageHandlers(HWND hWnd, DWORD dwIndex, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK dlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK wndProcSubClass(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

	}
}

