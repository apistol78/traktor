/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cell/cell_fs.h>
#include "Core/Io/Ps3/DrmVolume.h"
#include "Core/Io/Ps3/NativeStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Misc/WildCompare.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.DrmVolume", DrmVolume, IVolume)

DrmVolume::DrmVolume(const SceNpDrmKey& licensee)
:	m_licensee(licensee)
,	m_currentDirectory(L"/app_home")
{
}

std::wstring DrmVolume::getDescription() const
{
	return L"Native DRM volume";
}

Ref< File > DrmVolume::get(const Path& path)
{
	return 0;
}

int DrmVolume::find(const Path& mask, RefArray< File >& out)
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

bool DrmVolume::modify(const Path& fileName, uint32_t flags)
{
	return false;
}

Ref< IStream > DrmVolume::open(const Path& filename, uint32_t mode)
{
	int32_t fd;

	if (mode != File::FmRead)
		return 0;

	int32_t err = sceNpDrmOpen(
		&m_licensee,
		wstombs(getSystemPath(filename)).c_str(),
		CELL_FS_O_RDONLY,
		&fd,
		NULL,
		0
	);
	if (err < 0)
		return 0;

	return new NativeStream(fd, mode);
}

bool DrmVolume::exist(const Path& filename)
{
	CellFsStat sb;
	return cellFsStat(wstombs(getSystemPath(filename)).c_str(), &sb) == CELL_FS_SUCCEEDED;
}

bool DrmVolume::remove(const Path& filename)
{
	return cellFsUnlink(wstombs(getSystemPath(filename)).c_str()) == CELL_FS_SUCCEEDED;
}

bool DrmVolume::rename(const Path& fileName, const std::wstring& newName)
{
	return false;
}

bool DrmVolume::makeDirectory(const Path& directory)
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

bool DrmVolume::removeDirectory(const Path& directory)
{
	return false;
}

bool DrmVolume::renameDirectory(const Path& directory, const std::wstring& newName)
{
	return false;
}

bool DrmVolume::setCurrentDirectory(const Path& directory)
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

Path DrmVolume::getCurrentDirectory() const
{
	return m_currentDirectory;
}

std::wstring DrmVolume::getSystemPath(const Path& path) const
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
