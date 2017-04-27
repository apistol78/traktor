/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <dlfcn.h>
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Library", Library, Object)

Library::Library()
:   m_handle(0)
{
}

Library::~Library()
{
}

bool Library::open(const Path& libraryName)
{
	return open(libraryName, std::vector< Path >(), true);
}

bool Library::open(const Path& libraryName, const std::vector< Path >& searchPaths, bool includeDefaultPaths)
{
#if !defined(_DEBUG)
	std::wstring resolved = std::wstring(L"lib") + libraryName.getPathName() + L".so";
#else
	std::wstring resolved = std::wstring(L"lib") + libraryName.getPathName() + L"_d.so";
#endif
	std::wstring errors;

    // Prefer executable path first.
    {
        std::wstring library = L"$ORIGIN/" + resolved;
        m_handle = dlopen(wstombs(library).c_str(), RTLD_LAZY | RTLD_GLOBAL);
        if (m_handle)
        {
            T_DEBUG(L"Library \"" << library << L"\" loaded");
            return true;
        }
        else
            errors += mbstows(dlerror()) + L"\n";
    }

	// Try working directory second.
	{
		std::wstring library = L"./" + resolved;
		m_handle = dlopen(wstombs(library).c_str(), RTLD_LAZY | RTLD_GLOBAL);
		if (m_handle)
		{
			T_DEBUG(L"Library \"" << library << L"\" loaded");
			return true;
		}
		else
			errors += mbstows(dlerror()) + L"\n";
	}

    // Try default search paths.
    {
        std::wstring library = resolved;
        m_handle = dlopen(wstombs(library).c_str(), RTLD_LAZY | RTLD_GLOBAL);
        if (m_handle)
        {
            T_DEBUG(L"Library \"" << library << L"\" loaded");
            return true;
        }
        else
             errors += mbstows(dlerror()) + L"\n";
   }

	log::error << L"Failed to load library \"" << libraryName.getPathName() << L"\"" << Endl << IncreaseIndent << errors << DecreaseIndent;
	return false;
}

void Library::close()
{
	dlclose(m_handle);
}

void Library::detach()
{
}

void* Library::find(const std::wstring& symbol)
{
	std::string symb = wstombs(symbol);
	return dlsym(m_handle, symb.c_str());
}

Path Library::getPath() const
{
    return Path();
}

}
