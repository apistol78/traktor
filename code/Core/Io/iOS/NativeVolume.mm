/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <CoreFoundation/CFBundle.h>
#include <Foundation/Foundation.h>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include "Core/Date/DateTime.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/File.h"
#include "Core/Io/iOS/NativeVolume.h"
#include "Core/Io/iOS/NativeStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/WildCompare.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace
	{
	
DateTime fromUnixTime(const time_t& t)
{
	struct tm* tmp = ::localtime(&t);
	T_ASSERT (tmp);

	return DateTime(
		tmp->tm_year + 1900,
		tmp->tm_mon + 1,
		tmp->tm_mday + 1,
		tmp->tm_hour,
		tmp->tm_min,
		tmp->tm_sec
	);
}
	
	}

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
	struct stat sb;
	if (stat(wstombs(getSystemPath(path)).c_str(), &sb) != 0)
		return 0;
		
	uint32_t flags = 0;
	/*
	if (sb.st_mode & S_ISREG)
		flags |= File::FfNormal;
	if (sb.st_mode & S_ISDIR)
		flags |= File::FfDirectory;
	*/
	return new File(
		path,
		sb.st_size,
		flags,
		fromUnixTime(sb.st_mtime),
		fromUnixTime(sb.st_atime),
		fromUnixTime(sb.st_mtime)
	);
}

int NativeVolume::find(const Path& mask, RefArray< File >& out)
{
	struct dirent* dp;

	std::wstring maskPath = mask.getPathOnly();
	std::wstring systemPath = getSystemPath(maskPath);
	std::wstring fileMask = mask.getFileName();

	if (fileMask == L"*.*")
		fileMask = L"*";
		
	WildCompare maskCompare(fileMask);

	DIR* dirp = opendir(systemPath.empty() ? "." : wstombs(systemPath).c_str());
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
			int flags = 0;
			int size = 0;
			
			if (dp->d_type == DT_DIR)
			{
				flags = File::FfDirectory;
			}
			else	// Assumes it's a normal file.
			{
				flags = File::FfNormal;
			}
			
			out.push_back(new File(
				maskPath + mbstows(dp->d_name),
				size,
				flags
			));
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
	return ::remove(wstombs(getSystemPath(filename)).c_str()) == 0;
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
	int status = mkdir(
		wstombs(getSystemPath(directory)).c_str(),
		S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH
	);
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
	std::wstring workingDirectory = std::wstring(L"C:");

	CFBundleRef mainBundle = CFBundleGetMainBundle();
	if (mainBundle)
	{
		CFURLRef appUrl = CFBundleCopyBundleURL(mainBundle);
		if (appUrl)
		{
			char bundlePath[PATH_MAX];
			CFURLGetFileSystemRepresentation(appUrl, TRUE, (uint8_t*)bundlePath, PATH_MAX);
			CFRelease(appUrl);

			workingDirectory = std::wstring(L"C:") + mbstows(bundlePath);	
		}
	}

	Ref< IVolume > volume = new NativeVolume(workingDirectory);
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


