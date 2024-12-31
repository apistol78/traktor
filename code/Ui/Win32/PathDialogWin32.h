/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Misc/TString.h"
#include "Ui/Itf/IPathDialog.h"
#include "Ui/Win32/Window.h"

namespace traktor::ui
{

class EventSubject;

/*!
 * \ingroup UIW32
 */
class PathDialogWin32 : public IPathDialog
{
public:
	explicit PathDialogWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& title);

	virtual void destroy();

	virtual DialogResult showModal(Path& outPath);

private:
	EventSubject* m_owner;
	HWND m_hWnd = NULL;

	DWORD m_openDialogOptions = 0;
	IFileDialog* m_pFileDialog = nullptr;
	TCHAR m_title[64] = {};
};

}
