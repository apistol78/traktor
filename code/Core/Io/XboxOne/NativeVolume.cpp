/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/XboxOne/NativeVolume.h"
#include "Core/Io/XboxOne/NativeStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.NativeVolume", NativeVolume, IVolume)

NativeVolume::NativeVolume(const Path& currentDirectory)
:	m_currentDirectory(currentDirectory)
{
}

std::wstring NativeVolume::getDescription() const
{
	return L"";
}

Ref< File > NativeVolume::get(const Path& path)
{
	return 0;
}

int NativeVolume::find(const Path& mask, RefArray< File >& out)
{
	return 0;
}

bool NativeVolume::modify(const Path& fileName, uint32_t flags)
{
	return false;
}

Ref< IStream > NativeVolume::open(const Path& fileName, uint32_t mode)
{
	return 0;
}

bool NativeVolume::exist(const Path& fileName)
{
	return false;
}

bool NativeVolume::remove(const Path& fileName)
{
	return false;
}

bool NativeVolume::move(const Path& fileName, const std::wstring& newName, bool overwrite)
{
	return false;
}

bool NativeVolume::copy(const Path& fileName, const std::wstring& newName, bool overwrite)
{
	return false;
}

bool NativeVolume::makeDirectory(const Path& directory)
{
	return false;
}

bool NativeVolume::removeDirectory(const Path& directory)
{
	return false;
}

bool NativeVolume::renameDirectory(const Path& directory, const std::wstring& newName)
{
	return false;
}

bool NativeVolume::setCurrentDirectory(const Path& directory)
{
	return false;

}

Path NativeVolume::getCurrentDirectory() const
{
	return L"";
}

void NativeVolume::mountVolumes(FileSystem& fileSystem)
{
}

}
