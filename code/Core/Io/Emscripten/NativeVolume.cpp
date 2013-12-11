#include <cstdio>
#include "Core/Io/FileSystem.h"
#include "Core/Io/Linux/NativeVolume.h"
#include "Core/Io/Linux/NativeStream.h"
#include "Core/Misc/TString.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.NativeVolume", NativeVolume, IVolume)

NativeVolume::NativeVolume(const Path& currentDirectory)
:	m_currentDirectory(currentDirectory)
{
}

std::wstring NativeVolume::getDescription() const
{
	return L"Native volume";
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

Ref< IStream > NativeVolume::open(const Path& filename, uint32_t mode)
{
	FILE* fp = fopen(
		wstombs(getSystemPath(filename)).c_str(),
		bool(mode == File::FmRead) ? "rb" : "wb"
	);
	return bool(fp != 0) ? new NativeStream(fp, mode) : 0;
}

bool NativeVolume::exist(const Path& filename)
{
	return false;
}

bool NativeVolume::remove(const Path& filename)
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
	if (directory.isRelative())
	{
		m_currentDirectory = m_currentDirectory + directory;
	}
	else
	{
		if (m_currentDirectory.getVolume() != directory.getVolume())
			return false;

		m_currentDirectory = directory;
	}
	return true;
}

Path NativeVolume::getCurrentDirectory() const
{
	return m_currentDirectory;
}

void NativeVolume::mountVolumes(FileSystem& fileSystem)
{
	Ref< IVolume > volume = new NativeVolume(L"C:\\");
	fileSystem.mount(L"C", volume);
	fileSystem.setCurrentVolume(volume);
}

std::wstring NativeVolume::getSystemPath(const Path& path) const
{
	return path.getPathNameNoVolume();
}

}
