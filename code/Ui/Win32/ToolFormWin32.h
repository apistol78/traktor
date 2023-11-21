/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Itf/IToolForm.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor::ui
{

/*!
 * \ingroup UIW32
 */
class ToolFormWin32 : public WidgetWin32Impl< IToolForm >
{
public:
	explicit ToolFormWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) override final;

	virtual void setIcon(ISystemBitmap* icon) override final;

	virtual DialogResult showModal() override final;

	virtual void endModal(DialogResult result) override final;

private:
	bool m_modal;
	DialogResult m_result;

	LRESULT eventNcButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventNcButtonUp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventNcMouseMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventMouseActivate(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventEndModal(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);
};

}
