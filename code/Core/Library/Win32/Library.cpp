/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/StringOutputStream.h"
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/System.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Library", Library, Object)

bool Library::open(const Path& libraryName)
{
	std::wstring ln = libraryName.getPathName();
	if (!endsWith(toLower(ln), L".dll"))
		ln += L".dll";

	tstring path = wstots(ln);
	m_handle = (void*)LoadLibrary(path.c_str());

	if (m_handle == NULL)
	{
		DWORD errorCode = GetLastError();
		LPWSTR errorMessage = NULL;
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER,
			NULL,
			errorCode,
			0,
			(LPWSTR)&errorMessage,
			0,
			NULL
		);
		if (errorMessage)
		{
			log::warning << L"Unable to load module \"" << tstows(path) << L"\"; error code " << int32_t(errorCode) << L", " << errorMessage;
			HeapFree(GetProcessHeap(), LMEM_FIXED, errorMessage);
		}
		else
			log::warning << L"Unable to load module \"" << tstows(path) << L"\"; error code " << int32_t(errorCode) << Endl;
	}

	return bool(m_handle != NULL);
}

bool Library::open(const Path& libraryName, const std::vector< Path >& searchPaths, bool includeDefaultPaths)
{
	TCHAR currentPath[32767];
	if (!GetEnvironmentVariable(_T("PATH"), currentPath, sizeof_array(currentPath)))
		return false;

	StringOutputStream newPathStream;

	auto it = searchPaths.begin();
	if (it != searchPaths.end())
	{
		newPathStream << it->getPathName();
		for (++it; it != searchPaths.end(); ++it)
			newPathStream << L";" << it->getPathName();
	}

	if (includeDefaultPaths)
		newPathStream << L";" << currentPath;

	std::wstring newPath = newPathStream.str();
	if (!SetEnvironmentVariable(_T("PATH"), newPath.c_str()))
		return false;

	bool result = open(libraryName);
	SetEnvironmentVariable(_T("PATH"), currentPath);

	return result;
}

void Library::close()
{
	if (m_handle)
	{
		FreeLibrary((HMODULE)m_handle);
		m_handle = nullptr;
	}
}

void Library::detach()
{
	m_handle = nullptr;
}

void* Library::find(const std::wstring& symbol)
{
	return (void*)GetProcAddress((HMODULE)m_handle, wstombs(symbol).c_str());
}

Path Library::getPath() const
{
	wchar_t fileName[MAX_PATH + 1] = { 0 };
	GetModuleFileName((HMODULE)m_handle, fileName, sizeof_array(fileName));
	return Path(fileName);
}

}
