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
#if !defined(_XBOX)
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
#else
	return false;
#endif
}

bool Library::open(const Path& libraryName, const std::vector< Path >& searchPaths, bool includeDefaultPaths)
{
#if !defined(_XBOX)
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
#else
	return false;
#endif
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
#if !defined(_XBOX)
	return (void*)GetProcAddress((HMODULE)m_handle, wstombs(symbol).c_str());
#else
	return nullptr;
#endif
}

Path Library::getPath() const
{
	wchar_t fileName[MAX_PATH + 1] = { 0 };
#if !defined(_XBOX)
	GetModuleFileName((HMODULE)m_handle, fileName, sizeof_array(fileName));
#endif
	return Path(fileName);
}

}
