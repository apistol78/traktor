/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
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

namespace traktor::render
{

/*!
 * \ingroup Render
 */
class Window : public Object
{
public:
	struct IListener
	{
		virtual bool windowListenerEvent(class Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult) = 0;
	};

	Window();

	virtual ~Window();

	bool create(uint32_t display, int32_t width, int32_t height, bool fullscreen);

	void setTitle(const wchar_t* title);

	void setWindowedStyle(int32_t width, int32_t height);

	void setFullScreenStyle(int32_t width, int32_t height);

	void show();

	void hide();

	bool isActive() const;

	/*! Get width of client area; authoritative size of the renderable surface. */
	int32_t getWidth() const;

	/*! Get height of client area; authoritative size of the renderable surface. */
	int32_t getHeight() const;

	uint32_t getDisplay() const;

	operator HWND () const;

	void addListener(IListener* listener);

	void removeListener(IListener* listener);

	bool haveWindowedStyle() const { return !m_fullScreen; }

	bool haveFullScreenStyle() const { return m_fullScreen; }

private:
	HWND m_hWnd;
	bool m_fullScreen;
	std::set< IListener* > m_listeners;

	// Windowed presentation saved when entering fullscreen, so leaving it restores
	// the window exactly as it was; position alone is not enough as the fullscreen
	// window has taken over the size and the frame styles.
	RECT m_windowedRect;
	LONG m_windowedStyle;
	LONG m_windowedStyleEx;

	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

}
