#include "Ui/Win32/PathDialogWin32.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Enums.h"

namespace traktor
{
	namespace ui
	{

PathDialogWin32::PathDialogWin32(EventSubject* owner)
:	m_owner(owner)
{
}

bool PathDialogWin32::create(IWidget* parent, const std::wstring& title)
{
	_tcscpy_s(m_title, sizeof_array(m_title), wstots(title).c_str());

	std::memset(&m_bi, 0, sizeof(m_bi));
	m_bi.hwndOwner = parent ? (HWND)parent->getInternalHandle() : NULL;
	m_bi.pidlRoot = NULL;
	m_bi.pszDisplayName = m_title;
	m_bi.lpszTitle = m_title;
	m_bi.ulFlags = BIF_USENEWUI; 
	m_bi.lpfn = NULL;
	m_bi.lParam = NULL;
	m_bi.iImage = 0;

	return true;
}

void PathDialogWin32::destroy()
{
}

int PathDialogWin32::showModal(Path& outPath)
{
	PIDLIST_ABSOLUTE idl = SHBrowseForFolder(&m_bi);
	if (idl == NULL)
		return DrCancel;

	TCHAR path[MAX_PATH];
	if (!SHGetPathFromIDList(idl, path))
		return DrCancel;

	outPath = tstows(path);
	return DrOk;
}

	}
}
