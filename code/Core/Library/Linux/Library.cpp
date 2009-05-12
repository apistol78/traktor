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
	std::string library = wstombs(std::wstring(L"lib") + libraryName.getPathName() + L".so");

	m_handle = dlopen(library.c_str(), RTLD_NOW | RTLD_GLOBAL);
	if (m_handle)
		return true;

	log::error << L"Unable to open library \"" << libraryName << L"\": " << mbstows(dlerror()) << Endl;
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


