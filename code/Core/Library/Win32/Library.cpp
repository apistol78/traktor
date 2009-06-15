#include "Core/Platform.h"
#include "Core/Library/Library.h"
#include "Core/Misc/TString.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Library", Library, Object)

Library::~Library()
{
}

bool Library::open(const Path& libraryName)
{
#if !defined(_DEBUG)
	const wchar_t suffix[] = L".dll";
#else
	const wchar_t suffix[] = L"_d.dll";
#endif

	tstring path;

	path = wstots(libraryName.getPathName() + suffix);
	m_handle = (void*)LoadLibrary(path.c_str());

	if (!m_handle)
	{
		path = wstots(libraryName.getPathName());
		m_handle = (void*)LoadLibrary(path.c_str());
	}
	
	return bool(m_handle != NULL);
}

void Library::close()
{
	FreeLibrary((HMODULE)m_handle);
}

void* Library::find(const std::wstring& symbol)
{
#if !defined(WINCE)
	return (void*)GetProcAddress((HMODULE)m_handle, wstombs(symbol).c_str());
#else
	return (void*)GetProcAddress((HMODULE)m_handle, symbol.c_str());
#endif
}

}
