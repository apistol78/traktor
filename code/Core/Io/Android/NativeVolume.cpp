/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include "Core/Io/FileSystem.h"
#include "Core/Io/Android/NativeVolume.h"
#include "Core/Io/Android/NativeStream.h"
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
	struct stat st;

	std::wstring systemPath = getSystemPath(path);
	if (stat(wstombs(systemPath).c_str(), &st) != 0)
		return 0;

	DateTime adt(uint64_t(st.st_atime));
	DateTime mdt(uint64_t(st.st_mtime));

	uint32_t flags = File::FfNormal;

	if ((st.st_mode & S_IWUSR) == 0)
		flags |= File::FfReadOnly;

	return new File(
		path,
		st.st_size,
		flags,
		mdt,
		adt,
		mdt
	);
}

int NativeVolume::find(const Path& mask, RefArray< File >& out)
{
	struct dirent* dp;

	std::wstring maskPath = mask.getPathOnly();
	std::wstring systemPath = getSystemPath(maskPath);
	std::wstring fileMask = mask.getFileName();

	WildCompare maskCompare(fileMask);

	DIR* dirp = opendir(wstombs(systemPath.empty() ? L"." : systemPath).c_str());
	if (!dirp)
	{
		log::warning << L"Unable to open directory \"" << systemPath << L"\"" << Endl;
		return 0;
	}

	if (!maskPath.empty())
		maskPath += L"/";

	while ((dp = readdir(dirp)) != 0)
	{
		if (maskCompare.match(mbstows(dp->d_name)))
		{
			if (dp->d_type == DT_DIR)
			{
				out.push_back(new File(
					maskPath + mbstows(dp->d_name),
					0,
					File::FfDirectory
				));
			}
			else	// Assumes it's a normal file.
			{
				Path filePath = maskPath + mbstows(dp->d_name);
				Ref< File > file = get(filePath);
				if (file)
					out.push_back(file);
				else
					log::warning << L"Unable to stat file \"" << filePath.getPathName() << L"\"" << Endl;
			}
		}
	}
	closedir(dirp);

	return int(out.size());
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
	struct stat sb;
	return bool(stat(wstombs(getSystemPath(filename)).c_str(), &sb) == 0);
}

bool NativeVolume::remove(const Path& filename)
{
	return bool(std::remove(wstombs(getSystemPath(filename)).c_str()) == 0);
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
	int status = mkdir(wstombs(directory.getPathName()).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (status != 0 && errno != EEXIST)
		return false;
	return true;
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
	Ref< IVolume > volume = new NativeVolume(L"C:");
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
