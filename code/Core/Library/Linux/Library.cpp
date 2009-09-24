#include <dlfcn.h>
#include "Core/Library/Library.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Library", Library, Object)

Library::~Library()
{
}

bool Library::open(const Path& libraryName)
{
	return open(libraryName, std::vector< Path >(), true);
}

bool Library::open(const Path& libraryName, const std::vector< Path >& searchPaths, bool includeDefaultPaths)
{
	std::string library = wstombs(std::wstring(L"lib") + libraryName.getPathName() + L".so");

	m_handle = dlopen(library.c_str(), RTLD_NOW | RTLD_GLOBAL);
	if (m_handle)
		return true;

	log::error << L"Unable to open library \"" << libraryName.getPathName() << L"\": " << mbstows(dlerror()) << Endl;
	return false;
}

void Library::close()
{
	dlclose(m_handle);
}

void* Library::find(const std::wstring& symbol)
{
	std::string symb = wstombs(symbol);
	return dlsym(m_handle, symb.c_str());
}

}


