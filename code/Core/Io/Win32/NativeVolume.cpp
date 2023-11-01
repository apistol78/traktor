/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <direct.h>
#include <iostream>
#include <sstream>
#include "Core/Io/Win32/NativeMappedFile.h"
#include "Core/Io/Win32/NativeMappedStream.h"
#include "Core/Io/Win32/NativeStream.h"
#include "Core/Io/Win32/NativeVolume.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/System.h"

namespace traktor
{
	namespace
	{

DateTime createDateTime(const FILETIME& ft)
{
	SYSTEMTIME st = {};
	FileTimeToSystemTime(&ft, &st);

	SYSTEMTIME lt = {};
	SystemTimeToTzSpecificLocalTime(NULL, &st, &lt);

	return DateTime(
		lt.wYear,
		uint8_t(lt.wMonth),
		lt.wDay,
		uint8_t(lt.wHour),
		uint8_t(lt.wMinute),
		uint8_t(lt.wSecond)
	);
}

FILETIME createFileTime(const DateTime& dt)
{
	SYSTEMTIME lt = {};
	lt.wYear = dt.getYear();
	lt.wMonth = dt.getMonth();
	lt.wDayOfWeek = dt.getWeekDay();
	lt.wDay = dt.getDay();
	lt.wHour = dt.getHour();
	lt.wMinute = dt.getMinute();
	lt.wSecond = dt.getSecond();

	SYSTEMTIME st = {};
	TzSpecificLocalTimeToSystemTime(NULL, &lt, &st);

	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);
	return ft;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.NativeVolume", NativeVolume, IVolume)

NativeVolume::NativeVolume(const Path& currentDirectory)
:	m_currentDirectory(currentDirectory)
{
}

std::wstring NativeVolume::getDescription() const
{
	wchar_t volumeName[256] = { L"" };
	if (GetVolumeInformation(
		m_currentDirectory.getVolume().c_str(),
		volumeName,
		sizeof_array(volumeName),
		NULL,
		NULL,
		NULL,
		NULL,
		0
	))
		return volumeName;
	else
		return L"";
}

Ref< File > NativeVolume::get(const Path& path)
{
	WIN32_FILE_ATTRIBUTE_DATA fa;
	std::memset(&fa, 0, sizeof(fa));

	if (!GetFileAttributesEx(wstots(getSystemPath(path)).c_str(), GetFileExInfoStandard, &fa))
		return nullptr;

	int flags =
		(((fa.dwFileAttributes  & FILE_ATTRIBUTE_NORMAL   ) == FILE_ATTRIBUTE_NORMAL   ) ? File::FfNormal    : 0) |
		(((fa.dwFileAttributes  & FILE_ATTRIBUTE_READONLY ) == FILE_ATTRIBUTE_READONLY ) ? File::FfReadOnly  : 0) |
		(((fa.dwFileAttributes  & FILE_ATTRIBUTE_HIDDEN   ) == FILE_ATTRIBUTE_HIDDEN   ) ? File::FfHidden    : 0) |
		(((fa.dwFileAttributes  & FILE_ATTRIBUTE_ARCHIVE  ) == FILE_ATTRIBUTE_ARCHIVE  ) ? File::FfArchive   : 0) |
		(((fa.dwFileAttributes  & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) ? File::FfDirectory : 0);

	return new File(
		path,
		fa.nFileSizeLow,
		flags,
		createDateTime(fa.ftCreationTime),
		createDateTime(fa.ftLastAccessTime),
		createDateTime(fa.ftLastWriteTime)
	);
}

RefArray< File > NativeVolume::find(const Path& mask)
{
	RefArray< File > files;
	WIN32_FIND_DATA ffd;
	HANDLE hfd;

	hfd = FindFirstFileEx(
		wstots(getSystemPath(mask)).c_str(),
		FindExInfoBasic,
		&ffd,
		FindExSearchNameMatch,
		NULL,
		FIND_FIRST_EX_LARGE_FETCH
	);
	if (hfd != INVALID_HANDLE_VALUE)
	{
		std::wstring path = mask.getPathOnly();
		if (!path.empty())
			path += L'/';
		do
		{
			int flags =
				(((ffd.dwFileAttributes  & FILE_ATTRIBUTE_NORMAL   ) == FILE_ATTRIBUTE_NORMAL   ) ? File::FfNormal    : 0) |
				(((ffd.dwFileAttributes  & FILE_ATTRIBUTE_READONLY ) == FILE_ATTRIBUTE_READONLY ) ? File::FfReadOnly  : 0) |
				(((ffd.dwFileAttributes  & FILE_ATTRIBUTE_HIDDEN   ) == FILE_ATTRIBUTE_HIDDEN   ) ? File::FfHidden    : 0) |
				(((ffd.dwFileAttributes  & FILE_ATTRIBUTE_ARCHIVE  ) == FILE_ATTRIBUTE_ARCHIVE  ) ? File::FfArchive   : 0) |
				(((ffd.dwFileAttributes  & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) ? File::FfDirectory : 0);

			files.push_back(new File(
				Path(path + tstows(ffd.cFileName)),
				ffd.nFileSizeLow,
				flags,
				createDateTime(ffd.ftCreationTime),
				createDateTime(ffd.ftLastAccessTime),
				createDateTime(ffd.ftLastWriteTime)
			));
		}
		while (FindNextFile(hfd, &ffd));
		FindClose(hfd);
	}

	return files;
}

bool NativeVolume::modify(const Path& fileName, uint32_t flags)
{
	DWORD dwAttr = 0;

	if (flags & File::FfNormal)
		dwAttr |= FILE_ATTRIBUTE_NORMAL;
	if (flags & File::FfReadOnly)
		dwAttr |= FILE_ATTRIBUTE_READONLY;
	if (flags & File::FfHidden)
		dwAttr |= FILE_ATTRIBUTE_HIDDEN;
	if (flags & File::FfArchive)
		dwAttr |= FILE_ATTRIBUTE_ARCHIVE;

	std::wstring systemPath = getSystemPath(fileName);
	BOOL result = SetFileAttributes(
		wstots(systemPath).c_str(),
		dwAttr
	);

	return (bool)(result != FALSE);
}

bool NativeVolume::modify(const Path& fileName, const DateTime* creationTime, const DateTime* lastAccessTime, const DateTime* lastWriteTime)
{
	std::wstring systemPath = getSystemPath(fileName);

	HANDLE hFile = CreateFile(
		wstots(systemPath).c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	FILETIME ct, at, wt;
	if (creationTime)
		ct = createFileTime(*creationTime);
	if (lastAccessTime)
		at = createFileTime(*lastAccessTime);
	if (lastWriteTime)
		wt = createFileTime(*lastWriteTime);

	BOOL result = SetFileTime(
		hFile,
		creationTime != nullptr ? &ct : nullptr,
		lastAccessTime != nullptr ? &at : nullptr,
		lastWriteTime != nullptr ? &wt : nullptr
	);

	CloseHandle(hFile);
	return (bool)(result != FALSE);
}

Ref< IStream > NativeVolume::open(const Path& fileName, uint32_t mode)
{
	DWORD desiredAccess = 0, creationDisposition = 0;

	if (mode & File::FmRead)
	{
		desiredAccess |= GENERIC_READ;
		creationDisposition = OPEN_EXISTING;
	}
	if (mode & (File::FmWrite | File::FmAppend))
	{
		desiredAccess |= GENERIC_WRITE;
		creationDisposition = (mode & (File::FmRead | File::FmAppend)) ? OPEN_ALWAYS : CREATE_ALWAYS;
	}

	const std::wstring systemPath = getSystemPath(fileName);

	const HANDLE hFile = CreateFile(
		wstots(systemPath).c_str(),
		desiredAccess,
		FILE_SHARE_READ,
		NULL,
		creationDisposition,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE)
	{
#if defined(_DEBUG)
		DWORD errorCode = GetLastError();
		if ((mode & File::FmRead) != File::FmRead || errorCode != ERROR_FILE_NOT_FOUND)
			log::warning << L"Unable to open file \"" << systemPath << L"\"; error code " << int32_t(errorCode) << Endl;
#endif
		return 0;
	}

	// Try to map file if open for reading.
	if ((mode & (File::FmRead | File::FmMapped)) == (File::FmRead | File::FmMapped))
	{
		const int64_t fileSize = GetFileSize(hFile, NULL);
		const HANDLE hFileMapping = CreateFileMapping(
			hFile,
			NULL,
			PAGE_READONLY,
			0,
			0,
			NULL
		);
		if (hFileMapping != INVALID_HANDLE_VALUE)
		{
			void* ptr = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
			if (ptr)
				return new NativeMappedStream(hFile, hFileMapping, ptr, fileSize);
			CloseHandle(hFileMapping);
		}
	}

	return new NativeStream(hFile, mode);
}

Ref< IMappedFile > NativeVolume::map(const Path& fileName)
{
	const std::wstring systemPath = getSystemPath(fileName);

	const HANDLE hFile = CreateFile(
		wstots(systemPath).c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE)
	{
#if defined(_DEBUG)
		DWORD errorCode = GetLastError();
		log::warning << L"Unable to map file \"" << systemPath << L"\"; error code " << int32_t(errorCode) << Endl;
#endif
		return nullptr;
	}

	const int64_t fileSize = GetFileSize(hFile, NULL);
	const HANDLE hFileMapping = CreateFileMapping(
		hFile,
		NULL,
		PAGE_READONLY,
		0,
		0,
		NULL
	);
	if (hFileMapping == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		return nullptr;
	}

	void* ptr = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
	if (!ptr)
	{
		CloseHandle(hFileMapping);
		CloseHandle(hFile);
		return nullptr;
	}

	return new NativeMappedFile(hFile, hFileMapping, ptr, fileSize);
}

bool NativeVolume::exist(const Path& fileName)
{
	WIN32_FIND_DATA ffd;
	HANDLE ffh;

	const std::wstring systemPath = getSystemPath(fileName);

	if ((ffh = FindFirstFile(wstots(systemPath).c_str(), &ffd)) == INVALID_HANDLE_VALUE)
		return false;

	FindClose(ffh);
	return true;
}

bool NativeVolume::remove(const Path& fileName)
{
	const std::wstring systemPath = getSystemPath(fileName);
	return bool(DeleteFile(wstots(systemPath).c_str()) == TRUE);
}

bool NativeVolume::move(const Path& fileName, const std::wstring& newName, bool overwrite)
{
	const std::wstring sourceName = getSystemPath(fileName);
	const std::wstring destinationName = getSystemPath(fileName.getPathOnly() + L"/" + newName);
	return bool(MoveFileEx(wstots(sourceName).c_str(), wstots(destinationName).c_str(), overwrite ? MOVEFILE_REPLACE_EXISTING : 0) == TRUE);
}

bool NativeVolume::copy(const Path& fileName, const std::wstring& newName, bool overwrite)
{
	const std::wstring sourceName = getSystemPath(fileName);
	const std::wstring destinationName = getSystemPath(fileName.getPathOnly() + L"/" + newName);
	return bool(CopyFile(wstots(sourceName).c_str(), wstots(destinationName).c_str(), !overwrite) == TRUE);
}

bool NativeVolume::makeDirectory(const Path& directory)
{
	const std::wstring systemPath = getSystemPath(directory);
	if (!CreateDirectory(wstots(systemPath).c_str(), NULL))
		return bool(GetLastError() == ERROR_ALREADY_EXISTS);
	return true;
}

bool NativeVolume::removeDirectory(const Path& directory)
{
	const std::wstring systemPath = getSystemPath(directory);
	if (!RemoveDirectory(wstots(systemPath).c_str()))
		return false;
	return true;
}

bool NativeVolume::renameDirectory(const Path& directory, const std::wstring& newName)
{
	const std::wstring sourceName = getSystemPath(directory);
	const std::wstring destinationName = getSystemPath(directory.getPathOnly() + L"/" + newName);
	return bool(MoveFile(wstots(sourceName).c_str(), wstots(destinationName).c_str()) == TRUE);
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
	wchar_t mountPoint[] = { L"A" };
	wchar_t driveFormat[] = { L"A:" };

	wchar_t currentDirectory[_MAX_PATH];
	if (!GetCurrentDirectory(sizeof_array(currentDirectory), currentDirectory))
		return;

	for (uint32_t drives = _getdrives(), drive = 0; drives != 0; drives >>= 1, ++drive)
	{
		if ((drives & 1) == 0)
			continue;

		mountPoint[0] =
		driveFormat[0] = L'A' + drive;

		Ref< IVolume > volume = new NativeVolume(driveFormat);
		fileSystem.mount(mountPoint, volume);

		if ((L'A' + drive) == toupper(currentDirectory[0]))
		{
			volume->setCurrentDirectory(currentDirectory);
			fileSystem.setCurrentVolume(volume);
		}
	}
}

std::wstring NativeVolume::getSystemPath(const Path& path) const
{
	std::wstringstream ss;

	const Path npath = path.normalized();
	if (npath.hasVolume())
	{
		T_ASSERT(npath.getVolume() == m_currentDirectory.getVolume());
		ss << npath.getPathName();
	}
	else
	{
		if (npath.isRelative())
			ss << m_currentDirectory.getPathName() << L"/" << npath.getPathName();
		else
			ss << m_currentDirectory.getVolume() << L":" << npath.getPathName();
	}

	return replaceAll(ss.str(), L'/', L'\\');
}

}
