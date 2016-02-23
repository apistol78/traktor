#include "Ui/Win32/FileDialogWin32.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Enums.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"

extern HINSTANCE g_hInstance;

namespace traktor
{
	namespace ui
	{

FileDialogWin32::FileDialogWin32(EventSubject* owner)
:	m_owner(owner)
,	m_save(false)
{
}

bool FileDialogWin32::create(IWidget* parent, const std::wstring& title, const std::wstring& filters, bool save)
{
	std::vector< std::wstring > tmp;
	Split< std::wstring >::any(filters, L";", tmp);

	T_ASSERT_M ((tmp.size() & 1) == 0, L"Incorrect number of filters");

	TCHAR* d = m_filters;
	for (size_t i = 0; i < tmp.size(); i += 2)
	{
		tstring name = wstots(tmp[i]);
		tstring ext = wstots(replaceAll(tmp[i + 1], L',', L';'));

		_tcscpy_s(d, sizeof_array(m_filters) - int(d - m_filters), name.c_str()); d += name.length();
		*d++ = _T('\0');

		_tcscpy_s(d, sizeof_array(m_filters) - int(d - m_filters), ext.c_str()); d += ext.length();
		*d++ = _T('\0');
	}
	*d++ = _T('\0');
	
	_tcscpy_s(m_title, sizeof_array(m_title), wstots(title).c_str());

	std::memset(m_fileName, 0, sizeof(m_fileName));

	std::memset(&m_ofn, 0, sizeof(m_ofn));
	m_ofn.lStructSize = sizeof(m_ofn);
	m_ofn.hwndOwner = parent ? (HWND)parent->getInternalHandle() : NULL;
	m_ofn.hInstance = g_hInstance;
	m_ofn.lpstrFilter = m_filters;
	m_ofn.lpstrCustomFilter = NULL;
	m_ofn.nMaxCustFilter = 0;
	m_ofn.nFilterIndex = 0;
	m_ofn.lpstrFile = m_fileName;
	m_ofn.nMaxFile = sizeof(m_fileName);
	m_ofn.lpstrFileTitle = NULL;
	m_ofn.nMaxFileTitle = 0;
	m_ofn.lpstrInitialDir = NULL;
	m_ofn.lpstrTitle = m_title;
	if (!save)
		m_ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
	else
		m_ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;

	m_save = save;

	return true;
}

void FileDialogWin32::destroy()
{
}

int FileDialogWin32::showModal(Path& outPath)
{
	m_ofn.Flags &= ~OFN_ALLOWMULTISELECT;
	wcscpy(m_fileName, outPath.getFileName().c_str());

	if (!m_save)
	{
		if (!GetOpenFileName(&m_ofn))
			return DrCancel;
	}
	else
	{
		if (!GetSaveFileName(&m_ofn))
			return DrCancel;
	}

	outPath = tstows(m_fileName);
	return DrOk;
}

int FileDialogWin32::showModal(std::vector< Path >& outPaths)
{
	m_ofn.Flags |= OFN_ALLOWMULTISELECT;
	if (!m_save)
	{
		if (!GetOpenFileName(&m_ofn))
			return DrCancel;
	}
	else
	{
		if (!GetSaveFileName(&m_ofn))
			return DrCancel;
	}

	std::wstring path = tstows(m_fileName);

	TCHAR* fileName = &m_fileName[m_ofn.nFileOffset];
	while (*fileName)
	{
		outPaths.push_back(path + L"\\" + tstows(fileName));
		fileName += _tcslen(fileName) + 1;
	}

	// Weird MS convention, in case user doesn't select multiple files
	// the convention is different, ie. same as if OFN_ALLOWMULTISELECT wasn't set.
	if (outPaths.size() == 1)
		outPaths[0] = path;

	return DrOk;
}

	}
}
