#include "Core/Library/Library.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Library", Library, Object)

bool Library::open(const Path& libraryName)
{
	return false;
}

bool Library::open(const Path& libraryName, const std::vector< Path >& searchPaths, bool includeDefaultPaths)
{
	return false;
}

void Library::close()
{
	m_handle = nullptr;
}

void Library::detach()
{
	m_handle = nullptr;
}

void* Library::find(const std::wstring& symbol)
{
	return nullptr;
}

Path Library::getPath() const
{
    return Path();
}

}
