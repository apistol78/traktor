/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <sstream>
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/IVolume.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Singleton/SingletonManager.h"

#if defined(_WIN32)
#	include "Core/Io/Win32/NativeVolume.h"
#	define HAS_NATIVE_VOLUME
#elif defined(_XBOX_ONE)
#	include "Core/Io/XboxOne/NativeVolume.h"
#	define HAS_NATIVE_VOLUME
#elif defined(__IOS__)		// IOS
#	include "Core/Io/iOS/NativeVolume.h"
#	define HAS_NATIVE_VOLUME
#elif defined(__APPLE__)	// MAC
#	include "Core/Io/OsX/NativeVolume.h"
#	define HAS_NATIVE_VOLUME
#elif defined(__PS3__)			// PS3
#	include "Core/Io/Ps3/NativeVolume.h"
#	define HAS_NATIVE_VOLUME
#elif defined(__PS4__)			// PS4
#	include "Core/Io/Ps4/NativeVolume.h"
#	define HAS_NATIVE_VOLUME
#elif defined(__EMSCRIPTEN__)	// Emscripten
#	include "Core/Io/Emscripten/NativeVolume.h"
#	define HAS_NATIVE_VOLUME
#elif defined(__PNACL__)	// PNaCl
#	include "Core/Io/PNaCl/NativeVolume.h"
#	define HAS_NATIVE_VOLUME
#elif defined(__ANDROID__)
#	include "Core/Io/Android/NativeVolume.h"
#	define HAS_NATIVE_VOLUME
#else						// LINUX
#	include "Core/Io/Linux/NativeVolume.h"
#	define HAS_NATIVE_VOLUME
#endif

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.FileSystem", FileSystem, Object)

FileSystem::FileSystem()
{
#if defined(HAS_NATIVE_VOLUME)
	NativeVolume::mountVolumes(*this);
#endif
}

FileSystem& FileSystem::getInstance()
{
	static FileSystem* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new FileSystem();
		s_instance->addRef(0);
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

void FileSystem::mount(const std::wstring& id, IVolume* volume)
{
	m_volumes[toLower(id)] = volume;
}

void FileSystem::umount(const std::wstring& id)
{
	std::map< std::wstring, Ref< IVolume > >::iterator i = m_volumes.find(id);
	if (i != m_volumes.end())
		m_volumes.erase(i);
}

int32_t FileSystem::getVolumeCount() const
{
	return int(std::distance(m_volumes.begin(), m_volumes.end()));
}

IVolume* FileSystem::getVolume(int32_t index) const
{
	std::map< std::wstring, Ref< IVolume > >::const_iterator i = m_volumes.begin();
	while (i != m_volumes.end() && index-- > 0)
		++i;
	return i != m_volumes.end() ? i->second : 0;
}

std::wstring FileSystem::getVolumeId(int32_t index) const
{
	std::map< std::wstring, Ref< IVolume > >::const_iterator i = m_volumes.begin();
	while (i != m_volumes.end() && index-- > 0)
		++i;
	return i != m_volumes.end() ? i->first : L"";
}

void FileSystem::setCurrentVolume(IVolume* volume)
{
	m_currentVolume = volume;
}

IVolume* FileSystem::getCurrentVolume() const
{
	return m_currentVolume;
}

bool FileSystem::setCurrentVolumeAndDirectory(const Path& directory)
{
	Ref< IVolume > nextCurrentVolume = getVolume(directory);
	if (!nextCurrentVolume)
		return false;

	if (!nextCurrentVolume->setCurrentDirectory(directory))
		return false;

	setCurrentVolume(nextCurrentVolume);
	return true;
}

Path FileSystem::getCurrentVolumeAndDirectory() const
{
	return m_currentVolume ? m_currentVolume->getCurrentDirectory() : Path();
}

Ref< File > FileSystem::get(const Path& fileName)
{
	Ref< IVolume > volume = getVolume(fileName);
	return volume ? volume->get(fileName) : 0;
}

int FileSystem::find(const Path& fileMask, RefArray< File >& out)
{
	Ref< IVolume > volume = getVolume(fileMask);
	return volume ? volume->find(fileMask, out) : 0;
}

bool FileSystem::modify(const Path& fileName, uint32_t flags)
{
	Ref< IVolume > volume = getVolume(fileName);
	return volume ? volume->modify(fileName, flags) : false;
}

Ref< IStream > FileSystem::open(const Path& fileName, uint32_t mode)
{
	Ref< IVolume > volume = getVolume(fileName);
	return volume ? volume->open(fileName, mode) : 0;
}

bool FileSystem::exist(const Path& fileName)
{
	Ref< IVolume > volume = getVolume(fileName);
	return volume ? volume->exist(fileName) : false;
}

bool FileSystem::remove(const Path& fileName)	
{
	Ref< IVolume > volume = getVolume(fileName);
	return volume ? volume->remove(fileName) : false;
}

bool FileSystem::move(const Path& destination, const Path& source, bool overwrite)
{
	Path destinationPath = getAbsolutePath(destination).normalized();
	Path sourcePath = getAbsolutePath(source).normalized();

	if (compareIgnoreCase< std::wstring >(destinationPath.getPathOnly(), sourcePath.getPathOnly()) == 0)
	{
		// Actually performing a rename operation; thus call rename on volume directly
		// which is much faster than transferring the file's content.
		Ref< IVolume > volume = getVolume(destinationPath);
		if (volume && volume->move(sourcePath, destinationPath.getFileName(), overwrite))
			return true;
	}

	if (copy(destination, source, overwrite) == false)
		return false;

	if (remove(source) == false)
	{
		remove(destination);
		return false;
	}

	return true;
}

bool FileSystem::copy(const Path& destination, const Path& source, bool overwrite)
{
	Path destinationPath = getAbsolutePath(destination).normalized();
	Path sourcePath = getAbsolutePath(source).normalized();

	if (compareIgnoreCase< std::wstring >(destinationPath.getPathOnly(), sourcePath.getPathOnly()) == 0)
	{
		// Actually performing a rename operation; thus call rename on volume directly
		// which is much faster than transferring the file's content.
		Ref< IVolume > volume = getVolume(destinationPath);
		if (volume && volume->copy(sourcePath, destinationPath.getFileName(), overwrite))
			return true;
	}

	// Verify that file doesn't exist.
	if (overwrite == false)
	{
		if (exist(destination) == true)
			return 0;
	}

	// Open source file.
	Ref< IStream > src = open(source, File::FmRead);
	if (!src)
	{
#if defined(_DEBUG)
		log::error << L"Unable to open file \"" << source.getPathName() << L"\" for reading" << Endl;
#endif
		return 0;
	}

	// Create destination file.
	Ref< IStream > dst = open(destination, File::FmWrite);
	if (!dst)
	{
#if defined(_DEBUG)
		log::error << L"Unable to open file \"" << destination.getPathName() << L"\" for writing" << Endl;
#endif
		return 0;
	}

	// Copy entire content.
	bool result = StreamCopy(dst, src).execute();

	// Finished, close streams.
	dst->close();
	src->close();

	return result;
}

bool FileSystem::makeDirectory(const Path& directory)
{
	Ref< IVolume > volume = getVolume(directory);
	return volume ? volume->makeDirectory(directory) : false;
}

bool FileSystem::makeAllDirectories(const Path& directory)
{
	Ref< IVolume > volume = getVolume(directory);
	if (!volume)
		return false;

	std::vector< std::wstring > directories;
	if (!Split< std::wstring >::any(directory.getPathNameNoVolume(), L"/", directories))
		return true;

	std::wstring build = directory.isRelative() ? L"" : L"/";
	for (std::vector< std::wstring >::iterator i = directories.begin(); i != directories.end(); ++i)
	{
		build += *i;
		if (!volume->makeDirectory(build))
			return false;
		build += L"/";
	}

	return true;
}

bool FileSystem::removeDirectory(const Path& directory)
{
	Ref< IVolume > volume = getVolume(directory);
	return volume ? volume->removeDirectory(directory) : false;
}

bool FileSystem::renameDirectory(const Path& directory, const std::wstring& newName)
{
	Ref< IVolume > volume = getVolume(directory);
	return volume ? volume->renameDirectory(directory, newName) : false;
}

Path FileSystem::getAbsolutePath(const Path& relativePath) const
{
	if (!relativePath.isRelative() && relativePath.hasVolume())
		return relativePath;

	Ref< IVolume > volume = getVolume(relativePath);
	if (!volume)
		return relativePath;

	return getAbsolutePath(volume->getCurrentDirectory(), relativePath);
}

Path FileSystem::getAbsolutePath(const Path& basePath, const Path& relativePath) const
{
	Path absoluteBasePath = basePath.isRelative() ? getAbsolutePath(basePath) : basePath;

	if (relativePath.hasVolume())
	{
		if (compareIgnoreCase(relativePath.getVolume(), absoluteBasePath.getVolume()) != 0)
			return relativePath;
	}

	Path absolutePath = relativePath.isRelative() ?
		Path(absoluteBasePath.getPathName() + L"/" + relativePath.getPathNameNoVolume()) :
		Path(absoluteBasePath.getVolume() + L":" + relativePath.getPathNameNoVolume());

	return absolutePath;
}

bool FileSystem::getRelativePath(const Path& absolutePath, const Path& relativeToPath, Path& relativePath) const
{
	if (absolutePath.isRelative() || relativeToPath.isRelative())
		return false;
		
	if (absolutePath.hasVolume() || relativeToPath.hasVolume())
	{
		if (!absolutePath.hasVolume() || !relativeToPath.hasVolume())
			return false;

		if (compareIgnoreCase(absolutePath.getVolume(), relativeToPath.getVolume()) != 0)
			return false;
	}

	std::vector< std::wstring > absoluteParts, relativeParts;

	Split< std::wstring >::any(absolutePath.getPathName(), L"/", absoluteParts);
	Split< std::wstring >::any(relativeToPath.getPathName(), L"/", relativeParts);

	std::vector< std::wstring >::iterator i1 = absoluteParts.begin();
	std::vector< std::wstring >::iterator i2 = relativeParts.begin();
	while (i1 != absoluteParts.end() && i2 != relativeParts.end() && *i1 == *i2)
	{
		i1 = absoluteParts.erase(i1);
		i2 = relativeParts.erase(i2);
	}
	
	if (i1 == absoluteParts.end() && i2 == relativeParts.end())
		return true;
	
	for (; i2 != relativeParts.end(); ++i2)
		absoluteParts.insert(absoluteParts.begin(), L"..");

	std::vector< std::wstring >::iterator j = absoluteParts.begin();
	std::wstringstream ss;
	ss << *j++;
	for (; j != absoluteParts.end(); ++j)
		ss << L"/" << *j;
		
	relativePath = ss.str();
	return true;
}

void FileSystem::destroy()
{
	T_SAFE_RELEASE(this);
}

IVolume* FileSystem::getVolume(const Path& path) const
{
	IVolume* volume = 0;
	
	if (path.hasVolume() == true)
	{
		std::map< std::wstring, Ref< IVolume > >::const_iterator it = m_volumes.find(toLower(path.getVolume()));
		if (it != m_volumes.end())
			volume = it->second;
#if defined(_DEBUG)
		else
			log::error << L"No volume named \"" << path.getVolume() << L"\" mounted" << Endl;
#endif
	}
	else
		volume = m_currentVolume;

	return volume;
}

}
