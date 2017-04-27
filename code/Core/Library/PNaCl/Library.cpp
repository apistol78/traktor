/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	return false;
}

bool Library::open(const Path& libraryName, const std::vector< Path >& searchPaths, bool includeDefaultPaths)
{
	return false;
}

void Library::close()
{
}

void Library::detach()
{
}

void* Library::find(const std::wstring& symbol)
{
	return 0;
}

Path Library::getPath() const
{
    return Path();
}

}
