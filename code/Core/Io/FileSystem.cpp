#include <sstream>
#include "Core/Io/FileSystem.h"
#include "Core/Io/Volume.h"
#include "Core/Io/Stream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Misc/String.h"
#include "Core/Misc/Split.h"
#include "Core/Log/Log.h"

#if defined(_WIN32)
#	if !defined(WINCE)
#		include "Core/Io/Win32/NativeVolume.h"
#	else
#		include "Core/Io/WinCE/NativeVolume.h"
#	endif
#	define HAS_NATIVE_VOLUME
#elif defined(__APPLE__)	// _MAC
#	include "Core/Io/OsX/NativeVolume.h"
#	define HAS_NATIVE_VOLUME
#else				// GNU
#	include "Core/Io/Linux/NativeVolume.h"
#	define HAS_NATIVE_VOLUME
#endif

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.FileSystem", FileSystem, Singleton)

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
		SingletonManager::getInstance().addBefore(s_instance, &Heap::getInstance());
	}
	return *s_instance;
}

void FileSystem::mount(const std::wstring& id, Volume* volume)
{
	m_volumes[toLower(id)] = volume;
}

void FileSystem::umount(const std::wstring& id)
{
	std::map< std::wstring, Ref< Volume > >::iterator i = m_volumes.find(id);
	if (i != m_volumes.end())
		m_volumes.erase(i);
}

int FileSystem::getVolumeCount() const
{
	return int(std::distance(m_volumes.begin(), m_volumes.end()));
}

Volume* FileSystem::getVolume(int index) const
{
	std::map< std::wstring, Ref< Volume > >::const_iterator i = m_volumes.begin();
	while (i != m_volumes.end() && index-- > 0)
		++i;
	return i != m_volumes.end() ? i->second.getPtr() : 0;
}

std::wstring FileSystem::getVolumeId(int index) const
{
	std::map< std::wstring, Ref< Volume > >::const_iterator i = m_volumes.begin();
	while (i != m_volumes.end() && index-- > 0)
		++i;
	return i != m_volumes.end() ? i->first : L"";
}

void FileSystem::setCurrentVolume(Volume* volume)
{
	m_currentVolume = volume;
}

Volume* FileSystem::getCurrentVolume() const
{
	return m_currentVolume;
}

File* FileSystem::get(const Path& fileName)
{
	Volume* volume = getVolume(fileName);
	return volume ? volume->get(fileName) : 0;
}

int FileSystem::find(const Path& fileMask, RefArray< File >& out)
{
	Volume* volume = getVolume(fileMask);
	return volume ? volume->find(fileMask, out) : 0;
}

bool FileSystem::modify(const Path& fileName, uint32_t flags)
{
	Volume* volume = getVolume(fileName);
	return volume ? volume->modify(fileName, flags) : false;
}

Stream* FileSystem::open(const Path& fileName, uint32_t mode)
{
	Volume* volume = getVolume(fileName);
	return volume ? volume->open(fileName, mode) : 0;
}

bool FileSystem::exist(const Path& fileName)
{
	Volume* volume = getVolume(fileName);
	return volume ? volume->exist(fileName) : false;
}

bool FileSystem::remove(const Path& fileName)	
{
	Volume* volume = getVolume(fileName);
	return volume ? volume->remove(fileName) : false;
}

bool FileSystem::move(const Path& destination, const Path& source, bool overwrite)
{
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
	// Verify that file doesn't exist.
	if (overwrite == false)
	{
		if (exist(destination) == true)
			return 0;
	}

	// Open source file.
	Ref< Stream > src = open(source, File::FmRead);
	if (!src)
	{
		log::error << L"Unable to open file \"" << source.getPathName() << L"\" for reading" << Endl;
		return 0;
	}

	// Create destination file.
	Ref< Stream > dst = open(destination, File::FmWrite);
	if (!dst)
	{
		log::error << L"Unable to open file \"" << destination.getPathName() << L"\" for writing" << Endl;
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
	Volume* volume = getVolume(directory);
	return volume ? volume->makeDirectory(directory) : false;
}

bool FileSystem::makeAllDirectories(const Path& directory)
{
	Volume* volume = getVolume(directory);
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
	Volume* volume = getVolume(directory);
	return volume ? volume->removeDirectory(directory) : false;
}

bool FileSystem::renameDirectory(const Path& directory, const std::wstring& newName)
{
	Volume* volume = getVolume(directory);
	return volume ? volume->renameDirectory(directory, newName) : false;
}

Path FileSystem::getAbsolutePath(const Path& relativePath) const
{
	Volume* volume = getVolume(relativePath);
	if (!volume)
		return relativePath;

	return getAbsolutePath(volume->getCurrentDirectory(), relativePath);
}

Path FileSystem::getAbsolutePath(const Path& basePath, const Path& relativePath) const
{
	Path absolutePath = relativePath.isRelative() ?
		Path(basePath.getPathName() + L"/" + relativePath.getPathNameNoVolume()) :
		Path(basePath.getVolume() + L":" + relativePath.getPathNameNoVolume());

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

		if (absolutePath.getVolume() != relativeToPath.getVolume())
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
	delete this;
}

Volume* FileSystem::getVolume(const Path& path) const
{
	Ref< Volume > volume;
	
	if (path.hasVolume() == true)
	{
		std::map< std::wstring, Ref< Volume > >::const_iterator it = m_volumes.find(toLower(path.getVolume()));
		if (it != m_volumes.end())
			volume = it->second;
		else
			log::error << L"No volume named \"" << path.getVolume() << L"\" mounted" << Endl;
	}
	else
		volume = m_currentVolume;

	return volume;
}

}
