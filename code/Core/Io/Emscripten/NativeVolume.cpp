#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include "Core/Io/FileSystem.h"
#include "Core/Io/Linux/NativeVolume.h"
#include "Core/Io/Linux/NativeStream.h"
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
	return bool(unlink(wstombs(getSystemPath(filename)).c_str()) == 0);
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
	int status = rmdir(wstombs(directory.getPathName()).c_str());
	if (status != 0)
		return false;
	return true;
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
	if (!getcwd(cwd, sizeof(cwd)))
	{
		log::error << L"Unable to get current working directory; failed to mount virtual volume" << Endl;
		return;
	}

	std::wstring workingDirectory = std::wstring(L"C:") + mbstows(cwd);
	log::info << L"Current working directory \"" << workingDirectory << L"\"" << Endl;

	Ref< IVolume > volume = new NativeVolume(workingDirectory);
	fileSystem.mount(L"C", volume);
	fileSystem.setCurrentVolume(volume);
}

std::wstring NativeVolume::getSystemPath(const Path& path) const
{
	std::wstring systemPath = path.getPathNameNoVolume();
	log::info << L"\"" << path.getPathName() << L"\" => \"" << systemPath << L"\"" << Endl;
	return systemPath;
}

}
