#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include "Core/Io/OsX/NativeVolume.h"
#include "Core/Io/OsX/NativeStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"
#include "Core/Misc/WildCompare.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.NativeVolume", NativeVolume, Volume)

NativeVolume::NativeVolume(const Path& currentDirectory)
:	m_currentDirectory(currentDirectory)
{
}

std::wstring NativeVolume::getDescription() const
{
	return L"Native volume";
}

File* NativeVolume::get(const Path& path)
{
	return 0;
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
			
			out.push_back(gc_new< File >(
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

Stream* NativeVolume::open(const Path& filename, uint32_t mode)
{
	FILE* fp = fopen(
		wstombs(getSystemPath(filename)).c_str(),
		bool(mode == File::FmRead) ? "rb" : "wb"
	);
	return bool(fp != 0) ? gc_new< NativeStream >(fp, mode) : 0;
}

bool NativeVolume::exist(const Path& filename)
{
	struct stat sb;
	return bool(stat(wstombs(getSystemPath(filename)).c_str(), &sb) == 0);
}

bool NativeVolume::remove(const Path& filename)
{
	return false;
}

bool NativeVolume::makeDirectory(const Path& directory)
{
	int status = mkdir(
		wstombs(directory.getPathName()).c_str(),
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
	char cwd[256];
	getcwd(cwd, sizeof(cwd));

	std::wstring workingDirectory = std::wstring(L"C:") + mbstows(cwd);
	log::debug << L"Initial working directory \"" << workingDirectory << L"\"" << Endl;

	Ref< Volume > volume = gc_new< NativeVolume >(workingDirectory);
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


