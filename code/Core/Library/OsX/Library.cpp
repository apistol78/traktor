/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <dlfcn.h>
#include "Core/Library/Library.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Library", Library, Object)

bool Library::open(const Path& libraryName)
{
	return open(libraryName, std::vector< Path >(), true);
}

bool Library::open(const Path& libraryName, const std::vector< Path >& searchPaths, bool includeDefaultPaths)
{
	m_handle = nullptr;

	std::wstring resolved = L"lib" + libraryName.getFileName() + L".dylib";
	std::wstring errors;

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
		else
			errors += mbstows(dlerror()) + L"\n";
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
		else
			errors += mbstows(dlerror()) + L"\n";
	}

	log::error << L"Failed to load library \"" << libraryName.getPathName() << L"\"" << Endl << IncreaseIndent << errors << DecreaseIndent;
	return false;
}

void Library::close()
{
	if (m_handle)
	{
		dlclose(m_handle);
		m_handle = nullptr;
	}
}

void Library::detach()
{
	m_handle = nullptr;
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
