#include "Core/Platform.h"
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/TString.h"
#include "Core/Misc/String.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Library", Library, Object)

Library::Library()
:	m_handle(0)
{
}

Library::~Library()
{
}

bool Library::open(const Path& libraryName)
{
	tstring path;

	path = wstots(libraryName.getPathName() + L".dll");
	m_handle = (void*)LoadLibrary(path.c_str());

	if (!m_handle)
	{
		path = wstots(libraryName.getPathName());
		m_handle = (void*)LoadLibrary(path.c_str());
	}
	if (m_handle == NULL)
	{
		DWORD errorCode = GetLastError();
		log::warning << L"Unable to load module \"" << libraryName.getPathName() << L"\"; error code " << int32_t(errorCode) << Endl;
	}	
	return bool(m_handle != NULL);
}

bool Library::open(const Path& libraryName, const std::vector< Path >& searchPaths, bool includeDefaultPaths)
{
#if !defined(_XBOX)
	TCHAR currentPath[32767];

	if (!GetEnvironmentVariable(_T("PATH"), currentPath, sizeof_array(currentPath)))
		return false;

	StringOutputStream newPathStream;

	std::vector< Path >::const_iterator i = searchPaths.begin();
	if (i != searchPaths.end())
	{
		newPathStream << i->getPathName();
		for (++i; i != searchPaths.end(); ++i)
			newPathStream << L";" << i->getPathName();
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
		m_handle = 0;
	}
}

void Library::detach()
{
	m_handle = 0;
}

void* Library::find(const std::wstring& symbol)
{
	return (void*)GetProcAddress((HMODULE)m_handle, wstombs(symbol).c_str());
}

}
