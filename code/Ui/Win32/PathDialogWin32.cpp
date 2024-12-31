/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Win32/PathDialogWin32.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Enums.h"

namespace traktor::ui
{

PathDialogWin32::PathDialogWin32(EventSubject* owner)
	: m_owner(owner)
{
}

bool PathDialogWin32::create(IWidget* parent, const std::wstring& title)
{
	m_hWnd = (HWND)parent->getInternalHandle();
	_tcscpy_s(m_title, sizeof_array(m_title), wstots(title).c_str());

	if (!SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pFileDialog))))
	{
		return false;
	}

	// Set the options to select folders
	m_pFileDialog->GetOptions(&m_openDialogOptions);
	m_pFileDialog->SetOptions(m_openDialogOptions | FOS_PICKFOLDERS);
	m_pFileDialog->SetTitle(m_title);

	return true;
}

void PathDialogWin32::destroy()
{
	if (m_pFileDialog)
	{
		m_pFileDialog->Release();
		m_pFileDialog = nullptr;
	}
}

DialogResult PathDialogWin32::showModal(Path& outPath)
{
	if (!SUCCEEDED(m_pFileDialog->Show(m_hWnd)))
	{
		return DialogResult::Cancel;
	}

	// Retrieve the selected folder
	IShellItem* pItem = nullptr;

	if (!SUCCEEDED(m_pFileDialog->GetResult(&pItem)))
	{
		return DialogResult::Cancel;
	}

	PWSTR pszFolderPath = nullptr;

	if (!SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath)))
	{
		pItem->Release();
		return DialogResult::Cancel;
	}

	outPath = tstows(pszFolderPath);

	CoTaskMemFree(pszFolderPath);
	pItem->Release();

	return DialogResult::Ok;
}

}
