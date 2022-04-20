#include <dlfcn.h>
#include <limits.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Library", Library, Object)

bool Library::open(const Path& libraryName)
{
	return open(libraryName, std::vector< Path >(), true);
}

bool Library::open(const Path& libraryName, const std::vector< Path >& searchPaths, bool includeDefaultPaths)
{
	std::wstring resolved = std::wstring(L"lib") + libraryName.getPathName() + L".so";
	std::wstring errors;

	// Prefer executable path first.
	{
		char exepath[PATH_MAX];
		if (readlink("/proc/self/exe", exepath, PATH_MAX) != -1)
		{
			std::wstring library = mbstows(dirname(exepath)) + L"/" + resolved;
			m_handle = dlopen(wstombs(library).c_str(), RTLD_LAZY | RTLD_GLOBAL);
			if (m_handle)
			{
				T_DEBUG(L"Library \"" << library << L"\" loaded");
				return true;
			}
			else
				errors += mbstows(dlerror()) + L"\n";
		}
 	}

	// Try loading from specified search paths.
	for (std::vector< Path >::const_iterator i = searchPaths.begin(); i != searchPaths.end(); ++i)
	{
		std::wstring library = i->getPathNameNoVolume() + L"/" + resolved;
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
