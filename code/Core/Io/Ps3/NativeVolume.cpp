#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "Core/Io/Ps3/NativeVolume.h"
#include "Core/Io/Ps3/NativeStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Misc/StringUtils.h"
#include "Core/Misc/WildCompare.h"
#include "Core/Log/Log.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS("traktor.NativeVolume", NativeVolume, Volume)

NativeVolume::NativeVolume(const Path& currentDirectory) :
	m_currentDirectory(currentDirectory)
{
}

std::string NativeVolume::getDescription() const
{
	return "Native volume";
}

File* NativeVolume::get(const Path& path)
{
	return 0;
}

int NativeVolume::find(const Path& mask, RefArray< File >& out)
{
	struct dirent* dp;

	std::string maskPath = mask.getPathOnly();
	std::string systemPath = getSystemPath(maskPath);
	std::string fileMask = mask.getFileName();

	if (fileMask == "*.*")
		fileMask = "*";
		
	WildCompare maskCompare(fileMask);

	DIR* dirp = opendir(systemPath.empty() ? "." : systemPath.c_str());
	if (!dirp)
	{
		log::warning << "Unable to open directory \"" << systemPath << "\"" << Endl;
		return 0;
	}
	
	if (!maskPath.empty())
		maskPath += "/";
		
	while ((dp = readdir(dirp)) != 0)
	{
		if (maskCompare.match(dp->d_name))
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
			
			out.push_back(gc_new< File >(
				this,
				maskPath + dp->d_name,
				size,
				flags
			));
		}
	}
	closedir(dirp);
	
	return int(out.size());
}

Stream* NativeVolume::open(const Path& filename, File::Mode mode)
{
	FILE* fp = fopen(
		getSystemPath(filename).c_str(),
		bool(mode == File::FmRead) ? "rb" : "wb"
	);
	return bool(fp != 0) ? gc_new< NativeStream >(fp, mode) : 0;
}

bool NativeVolume::exist(const Path& filename)
{
	return true;
}

bool NativeVolume::remove(const Path& filename)
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

bool NativeVolume::renameDirectory(const Path& directory, const std::string& newName)
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
	Ref< Volume > volume = gc_new< NativeVolume >("/app_home");
	fileSystem.mount("C", volume);
	fileSystem.setCurrentVolume(volume);
}

std::string NativeVolume::getSystemPath(const Path& path) const
{
	std::stringstream ss;

	if (path.isRelative())
	{
		std::string tmp = m_currentDirectory.getPathNameNoVolume();
		ss << tmp << "/" << path.getPathName();
	}
	else
	{
		ss << path.getPathNameNoVolume();
	}
	
	return ss.str();
}

}


