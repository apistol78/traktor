/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <utime.h>
#include <Foundation/Foundation.h>
#include "Core/Date/DateTime.h"
#include "Core/Io/File.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/OsX/NativeStream.h"
#include "Core/Io/OsX/NativeVolume.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Misc/WildCompare.h"

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
		tmp->tm_mday,
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
		return nullptr;

	uint32_t flags = 0;
	if (sb.st_mode & S_IFREG)
		flags |= File::FfNormal;
	if (sb.st_mode & S_IFDIR)
		flags |= File::FfDirectory;
	if (!(sb.st_mode & S_IWUSR))
		flags |= File::FfReadOnly;
	if (path.getFileName().front() == L'.')
		flags |= File::FfHidden;

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
			if (dp->d_type == DT_DIR)
			{
				out.push_back(new File(
					maskPath + mbstows(dp->d_name),
					0,
					File::FfDirectory | (dp->d_name[0] == '.' ? File::FfHidden : 0)
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

bool NativeVolume::modify(const Path& fileName, const DateTime* creationTime, const DateTime* lastAccessTime, const DateTime* lastWriteTime)
{
	std::string systemPath = wstombs(getSystemPath(fileName));

	utimbuf utb = {};

	if (lastAccessTime == nullptr || lastWriteTime == nullptr)
	{
		struct stat st = {};
		if (stat(systemPath.c_str(), &st) != 0)
			return false;

		utb.actime = st.st_atime;
		utb.modtime = st.st_mtime;
	}

	if (lastAccessTime != nullptr)
		utb.actime = lastAccessTime->getSecondsSinceEpoch();
	if (lastWriteTime != nullptr)
		utb.modtime = lastWriteTime->getSecondsSinceEpoch();

	if (utime(systemPath.c_str(), &utb) != 0)
		return false;

	return true;
}

Ref< IStream > NativeVolume::open(const Path& filename, uint32_t mode)
{
	const uint32_t mrw = (mode & (File::FmRead | File::FmWrite));

	const char* m = nullptr;
	if (mrw == File::FmRead)
		m = "rb";
	else if (mrw == File::FmWrite)
		m = "wb";
	else if (mrw == (File::FmRead | File::FmWrite))
		m = "w+b";
	
	if (!m)
		return nullptr;

	FILE* fp = fopen(
		wstombs(getSystemPath(filename)).c_str(),
		m
	);
	return bool(fp != nullptr) ? new NativeStream(fp, mode) : nullptr;
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
	std::wstring sourceName = getSystemPath(fileName);
	std::wstring destinationName = getSystemPath(fileName.getPathOnly() + L"/" + newName);
	return bool(::rename(wstombs(sourceName).c_str(), wstombs(destinationName).c_str()) == 0);
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
	int status = rmdir(
		wstombs(getSystemPath(directory)).c_str()
	);
	return status == 0;
}

bool NativeVolume::renameDirectory(const Path& directory, const std::wstring& newName)
{
	return false;
}

bool NativeVolume::setCurrentDirectory(const Path& directory)
{
	if (directory.isRelative())
		m_currentDirectory = m_currentDirectory + directory;
	else
		m_currentDirectory = directory;
	return true;
}

Path NativeVolume::getCurrentDirectory() const
{
	return m_currentDirectory;
}

void NativeVolume::mountVolumes(FileSystem& fileSystem)
{
	NSFileManager* fileManager = [NSFileManager defaultManager];
    NSString* directoryPath = [fileManager currentDirectoryPath];
	if (directoryPath)
	{
		char buffer[4096];
		[directoryPath getCString: buffer maxLength: sizeof_array(buffer) encoding: NSUTF8StringEncoding];

		std::wstring workingDirectory = std::wstring(L"C:") + mbstows(buffer);

		Ref< IVolume > volume = new NativeVolume(workingDirectory);
		fileSystem.mount(L"C", volume);
		fileSystem.setCurrentVolume(volume);
	}
	else
		log::error << L"Unable to retrieve current working directory" << Endl;
}

std::wstring NativeVolume::getSystemPath(const Path& path) const
{
	StringOutputStream ss;

	if (path.isRelative())
	{
		std::wstring tmp = m_currentDirectory.getPathNameNoVolume();
		ss << tmp << L"/" << path.getPathName();
	}
	else
		ss << path.getPathNameNoVolume();

	return ss.str();
}

}
