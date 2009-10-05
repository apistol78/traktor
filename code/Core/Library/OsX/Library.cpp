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
	std::wstring resolved = libraryName.getPathName();
	std::wstring library = L"@executable_path/" + resolved + L".dylib";

	std::string tmp1 = wstombs(library);
	m_handle = dlopen(tmp1.c_str(), RTLD_LAZY | RTLD_GLOBAL);
	if (!m_handle)
	{
		log::error << L"Unable to open library, " << mbstows(dlerror()) << Endl;
		return false;
	}
	
	return true;
}

void Library::close()
{
	dlclose(m_handle);
}

void* Library::find(const std::wstring& symbol)
{
	std::string tmp = wstombs(symbol);
	return dlsym(m_handle, tmp.c_str());
}

}


