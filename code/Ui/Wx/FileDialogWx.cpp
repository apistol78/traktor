/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <wx/wx.h>
#include "Ui/Wx/FileDialogWx.h"
#include "Ui/Itf/IWidget.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace ui
	{

FileDialogWx::FileDialogWx(EventSubject* owner)
:	m_owner(owner)
,	m_fileDialog(0)
{
}

FileDialogWx::~FileDialogWx()
{
	T_ASSERT (!m_fileDialog);
}

bool FileDialogWx::create(IWidget* parent, const std::wstring& title, const std::wstring& filters, bool save)
{
	m_fileDialog = new wxFileDialog(
		parent ? static_cast< wxWindow* >(parent->getInternalHandle()) : 0,
		wstots(title).c_str(),
		_T(""),
		_T(""),
		wstots(replaceAll(filters, ';', '|')).c_str(),
		save ? (wxFD_SAVE | wxFD_OVERWRITE_PROMPT) : (wxFD_OPEN | wxFD_FILE_MUST_EXIST)
	);
	return true;
}

void FileDialogWx::destroy()
{
	if (m_fileDialog)
	{
		delete m_fileDialog;
		m_fileDialog = 0;
	}
}

int FileDialogWx::showModal(Path& outPath)
{
	T_ASSERT (m_fileDialog);

	if (m_fileDialog->ShowModal() != wxID_OK)
		return DrCancel;

	outPath = tstows((const wxChar*)m_fileDialog->GetPath());

	return DrOk;
}

int FileDialogWx::showModal(std::vector< Path >& outPaths)
{
	return DrCancel;
}

	}
}
