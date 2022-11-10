/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Itf/IDialog.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*!
 * \ingroup UIW32
 */
class DialogWin32 : public WidgetWin32Impl< IDialog >
{
public:
	explicit DialogWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) override;

	virtual void setIcon(ISystemBitmap* icon) override;

	virtual DialogResult showModal() override;

	virtual void endModal(DialogResult result) override;

	virtual void setMinSize(const Size& minSize) override;

	virtual void setVisible(bool visible) override;

	virtual Rect getRect() const override;

private:
	bool m_modal;
	Size m_minSize;
	int32_t m_centerStyle;
	bool m_keepCentered;
	DialogResult m_result;

	LRESULT eventInitDialog(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);

	LRESULT eventSizing(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);

	LRESULT eventClose(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);

	LRESULT eventEndModal(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);
};

	}
}

