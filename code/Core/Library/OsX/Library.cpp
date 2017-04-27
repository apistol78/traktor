/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	m_handle = 0;

	// In case we're running debug build we first need to look for equivialent libraries.
#if defined(_DEBUG)
	{
		std::wstring resolved = L"lib" + libraryName.getFileName() + L"_d.dylib";

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
		}
	}
#endif

	{
		std::wstring resolved = L"lib" + libraryName.getFileName() + L".dylib";

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
		}
	}

	log::error << L"Unable to open library \"" << libraryName.getFileName() << L"\"" << Endl;
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

Path Library::getPath() const
{
    return Path();
}

}
