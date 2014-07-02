#include <dlfcn.h>
#include "Core/Library/Library.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Library", Library, Object)

Library::Library()
:	m_handle(0)
{
}

Library::~Library()
{
	T_ASSERT(m_handle == 0);
}

bool Library::open(const Path& libraryName)
{
	return open(libraryName, std::vector< Path >(), true);
}

bool Library::open(const Path& libraryName, const std::vector< Path >& searchPaths, bool includeDefaultPaths)
{
#if !defined(_DEBUG)
	std::wstring resolved = std::wstring(L"lib") + libraryName.getPathName() + L".dylib";
#else
	std::wstring resolved = std::wstring(L"lib") + libraryName.getPathName() + L"_d.dylib";
#endif
	
	m_handle = 0;
	
	// Use executable path first.
	{
		std::wstring library = L"@executable_path/" + resolved;
		std::string tmp1 = wstombs(library);
		m_handle = dlopen(tmp1.c_str(), RTLD_LAZY | RTLD_GLOBAL);
		if (m_handle)
		{
            T_DEBUG(L"Library \"" << library << L"\" loaded");
			return true;
		}
		log::error << L"Unable to open library \"" << resolved << L"\", " << mbstows(dlerror()) << L", trying default paths..." << Endl;
	}
	
	// Try default paths second.
	{
		std::wstring library = resolved;
		std::string tmp1 = wstombs(library);
		m_handle = dlopen(tmp1.c_str(), RTLD_LAZY | RTLD_GLOBAL);
		if (m_handle)
		{
            T_DEBUG(L"Library \"" << library << L"\" loaded");
			return true;
		}
		log::error << L"Unable to open library \"" << resolved << L"\", " << mbstows(dlerror()) << L", trying default paths..." << Endl;
	}
	
	return false;
}

void Library::close()
{
	if (m_handle)
	{
		dlclose(m_handle);
		m_handle = 0;
	}
}

void Library::detach()
{
	m_handle = 0;
}

void* Library::find(const std::wstring& symbol)
{
	std::string tmp = wstombs(symbol);
	return dlsym(m_handle, tmp.c_str());
}

}


