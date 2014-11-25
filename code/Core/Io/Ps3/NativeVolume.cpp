#include <cell/cell_fs.h>
#include "Core/Io/FileSystem.h"
#include "Core/Io/Ps3/NativeVolume.h"
#include "Core/Io/Ps3/NativeStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Misc/WildCompare.h"

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
	std::wstring maskPath = mask.getPathOnly();
	std::wstring systemPath = getSystemPath(maskPath);
	std::wstring fileMask = mask.getFileName();

	if (fileMask == L"*.*")
		fileMask = L"*";
		
	WildCompare maskCompare(fileMask);

	int32_t dd;
	if (cellFsOpendir(systemPath.empty() ? "." : wstombs(systemPath).c_str(), &dd) != CELL_FS_SUCCEEDED)
		return 0;
	
	if (!maskPath.empty())
		maskPath += L"/";
		
	for (;;)
	{
		CellFsDirent dirent;
		uint64_t nread;

		if (cellFsReaddir(dd, &dirent, &nread) != CELL_FS_SUCCEEDED || nread == 0)
			break;

		std::wstring fileName = mbstows(dirent.d_name);
		if (maskCompare.match(fileName))
		{
			int flags = 0;
			int size = 0;
			
			if (dirent.d_type == CELL_FS_TYPE_DIRECTORY)
				flags = File::FfDirectory;
			else	// Assumes it's a normal file.
				flags = File::FfNormal;
			
			out.push_back(new File(
				maskPath + fileName,
				size,
				flags
			));
		}
	}

	cellFsClosedir(dd);
	
	return int(out.size());
}

bool NativeVolume::modify(const Path& fileName, uint32_t flags)
{
	return false;
}

Ref< IStream > NativeVolume::open(const Path& filename, uint32_t mode)
{
	int32_t fd;

	CellFsErrno err = cellFsOpen(
		wstombs(getSystemPath(filename)).c_str(),
		bool(mode == File::FmRead) ? CELL_FS_O_RDONLY : (CELL_FS_O_CREAT | CELL_FS_O_TRUNC | CELL_FS_O_WRONLY),
		&fd,
		NULL,
		0
	);
	if (err != CELL_FS_SUCCEEDED)
		return 0;

	return new NativeStream(fd, mode);
}

bool NativeVolume::exist(const Path& filename)
{
	CellFsStat sb;
	return cellFsStat(wstombs(getSystemPath(filename)).c_str(), &sb) == CELL_FS_SUCCEEDED;
}

bool NativeVolume::remove(const Path& filename)
{
	return cellFsUnlink(wstombs(getSystemPath(filename)).c_str()) == CELL_FS_SUCCEEDED;
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
	CellFsErrno err = cellFsMkdir(
		wstombs(getSystemPath(directory)).c_str(),
		CELL_FS_S_IRWXU | CELL_FS_S_IRWXG | CELL_FS_S_IRWXO
	);
	if (err == CELL_FS_SUCCEEDED || err == CELL_FS_EEXIST)
		return true;
	else
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
	Ref< IVolume > volume = new NativeVolume(L"/app_home");
	fileSystem.mount(L"C", volume);
	fileSystem.setCurrentVolume(volume);
}

std::wstring NativeVolume::getSystemPath(const Path& path) const
{
	std::wstringstream ss;

	if (path.isRelative())
	{
		std::wstring tmp = m_currentDirectory.getPathNameNoVolume();
		ss << tmp << L"/" << path.getPathName();
	}
	else
	{
		ss << path.getPathNameNoVolume();
	}
	
	return ss.str();
}

}
