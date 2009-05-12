#include <dlfcn.h>
#include "Core/Library/Library.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

namespace traktor
{

Library::~Library()
{
}

bool Library::open(const Path& libraryName)
{
	std::wstring resolved = libraryName;

#if !defined(_DEBUG)
	std::wstring library = L"@executable_path/lib" + resolved + L".dylib";
#else
	std::wstring library = L"@executable_path/lib" + resolved + L"_d.dylib";
#endif

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


