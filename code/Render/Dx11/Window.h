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
#include <tchar.h>
#include <set>
#include "Core/Object.h"

namespace traktor
{
	namespace render
	{

struct IWindowListener
{
	virtual bool windowListenerEvent(class Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult) = 0;
};

class Window : public Object
{
public:
	Window();

	virtual ~Window();

	bool create();

	void setTitle(const wchar_t* title);

	void setWindowedStyle(int32_t width, int32_t height);

	void setFullScreenStyle(int32_t width, int32_t height);

	void hide();

	operator HWND () const;

	void addListener(IWindowListener* listener);

	void removeListener(IWindowListener* listener);

	bool haveWindowedStyle() const { return !m_fullScreen; }

	bool haveFullScreenStyle() const { return m_fullScreen; }

private:
	HWND m_hWnd;
	bool m_fullScreen;
	std::set< IWindowListener* > m_listeners;
	POINT m_windowPosition;

	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

	}
}

