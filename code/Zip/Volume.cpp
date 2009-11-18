#include <sstream>
#include <ctime>
#include <zip.h>
#include <unzip.h>
#include "Zip/Volume.h"
#include "Zip/StreamZip.h"
#include "Zip/StreamUnzip.h"
#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/WildCompare.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace zip
	{
		namespace
		{
			
voidpf ZCALLBACK zlibOpenFile(voidpf opaque, const char* filename, int mode)
{
	Ref< IStream > s;
	
	if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER) == ZLIB_FILEFUNC_MODE_READ)
		s = FileSystem::getInstance().open(mbstows(filename), File::FmRead);
	else if ((mode & ZLIB_FILEFUNC_MODE_CREATE) == ZLIB_FILEFUNC_MODE_CREATE)
		s = FileSystem::getInstance().open(mbstows(filename), File::FmWrite);
	
	return (voidpf)(s ? new Ref< IStream >(s) : 0);	
}

uLong ZCALLBACK zlibReadFile(voidpf opaque, voidpf stream, void* buf, uLong size)
{
	Ref< IStream >* s = reinterpret_cast< Ref< IStream >* >(stream);
	return (uLong)(*s != 0 ? (*s)->read(buf, int(size)) : 0);
}

uLong ZCALLBACK zlibWriteFile(voidpf opaque, voidpf stream, const void* buf, uLong size)
{
	Ref< IStream >* s = reinterpret_cast< Ref< IStream >* >(stream);
	return (uLong)(*s != 0 ? (*s)->write(buf, int(size)) : 0);
}

long ZCALLBACK zlibTellFile(voidpf opaque, voidpf stream)
{
	Ref< IStream >* s = reinterpret_cast< Ref< IStream >* >(stream);
	return (long)(*s != 0 ? (*s)->tell() : 0);
}

long ZCALLBACK zlibSeekFile(voidpf opaque, voidpf stream, uLong offset, int origin)
{
	Ref< IStream >* s = reinterpret_cast< Ref< IStream >* >(stream);
	const IStream::SeekOriginType co[] =
	{
		IStream::SeekSet,
		IStream::SeekCurrent,
		IStream::SeekEnd
	};

	if (*s == 0)
		return -1;

	if (!(*s)->seek(co[origin], int(offset)))
		return -2;

	return 0;
}

int ZCALLBACK zlibCloseFile(voidpf opaque, voidpf stream)
{
	Ref< IStream >* s = reinterpret_cast< Ref< IStream >* >(stream);
	if (s != 0 && *s != 0)
	{
		(*s)->close();
		delete s;
	}
	return 0;
}

int ZCALLBACK zlibErrorFile(voidpf opaque, voidpf stream)
{
	return 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.zip.Volume", Volume, IVolume)

Volume::Volume(const Path& archive)
:	m_archive(archive)
,	m_zipFile(0)
,	m_unzFile(0)
{
	m_zlibFF = new struct zlib_filefunc_def_s;
	m_zlibFF->zopen_file = zlibOpenFile;
	m_zlibFF->zread_file = zlibReadFile;
	m_zlibFF->zwrite_file = zlibWriteFile;
	m_zlibFF->ztell_file = zlibTellFile;
	m_zlibFF->zseek_file = zlibSeekFile;
	m_zlibFF->zclose_file = zlibCloseFile;
	m_zlibFF->zerror_file = zlibErrorFile;
	m_zlibFF->opaque = (voidpf)this;
}

Volume::~Volume()
{
	flush();
	delete m_zlibFF;
}

std::wstring Volume::getDescription() const
{
	return L"Zip archive (" + m_archive.getPathName() + L")";
}

Ref< File > Volume::get(const Path& path)
{
	return 0;
}

int Volume::find(const Path& mask, RefArray< File >& out)
{
	std::vector< std::wstring > tmp;
	Split< std::wstring >::any(mask.getPathNameNoVolume(), L"/", tmp);
	if (tmp.empty())
		return 0;

	if (!readArchive())
		return 0;

	std::vector< WildCompare > wilds;
	for (std::vector< std::wstring >::iterator i = tmp.begin(); i != tmp.end(); ++i)
		wilds.push_back(WildCompare(toLower(*i)));

	if (unzGoToFirstFile(m_unzFile) == UNZ_OK)
	{
		do
		{
			unz_file_info info;
			char name[256];

			if (unzGetCurrentFileInfo(m_unzFile, &info, name, sizeof(name), 0, 0, 0, 0) == UNZ_OK)
			{
				std::wstring path = mbstows(name);
				if (path.empty())
					continue;

				std::vector< std::wstring > tmp;
				Split< std::wstring >::any(toLower(path), L"/", tmp);

				if (*(path.end() - 1) == '/')
				{
					// Directory

					if (tmp.size() != wilds.size())
						continue;

					bool match = true;
					for (size_t i = 0; i < tmp.size() - 1; ++i)
					{
						if (!(match = wilds[i].match(tmp[i])))
							break;
					}
					if (!match)
						continue;

					out.push_back(new File(
						mask.getVolume() + L":" + path.substr(0, path.length() - 1),
						info.uncompressed_size,
						File::FfDirectory
					));	
				}
				else
				{
					// File

					if (tmp.size() != wilds.size())
						continue;

					bool match = true;
					for (size_t i = 0; i < tmp.size(); ++i)
					{
						if (!(match = wilds[i].match(tmp[i])))
							break;
					}
					if (!match)
						continue;

					out.push_back(new File(
						mask.getVolume() + L":" + path,
						info.uncompressed_size,
						File::FfNormal
					));					
				}
			}
		}
		while (unzGoToNextFile(m_unzFile) == UNZ_OK);
	}

	return int(out.size());
}

bool Volume::modify(const Path& fileName, uint32_t flags)
{
	return false;
}

Ref< IStream > Volume::open(const Path& filename, File::Mode mode)
{
	Ref< IStream > stream;
	if (mode == File::FmRead)
	{
		if (!readArchive())
			return 0;

		if (unzLocateFile(m_unzFile, wstombs(filename.getPathNameNoVolume()).c_str(), 2) != UNZ_OK)
			return 0;
		
		if (unzOpenCurrentFile(m_unzFile) != UNZ_OK)
			return 0;

		stream = new StreamUnzip(m_unzFile);
	}
	else if (mode == File::FmWrite)
	{
		if (!writeArchive())
			return 0;

#if !defined(WINCE)
		time_t t; time(&t);
		tm* lt = localtime(&t);
		
		zip_fileinfo zfi =
		{
			{
				lt->tm_sec,
				lt->tm_min,
				lt->tm_hour,
				lt->tm_mday,
				lt->tm_mon,
				lt->tm_year
			},
			0,
			0,
			0
		};
#else
		// @fixme See if we can fill this structure in some other way.
		zip_fileinfo zfi;
		memset(&zfi, 0, sizeof(zfi));
#endif

		if (zipOpenNewFileInZip(m_zipFile, wstombs(filename.getPathName()).c_str(), &zfi, 0, 0, 0, 0, 0, Z_DEFLATED, Z_DEFAULT_COMPRESSION) != ZIP_OK)
			return 0;
	
		stream = new StreamZip(m_zipFile);
	}
	return stream;
}

bool Volume::exist(const Path& filename)
{
	if (!readArchive())
		return false;
	
	return bool(unzLocateFile(m_unzFile, wstombs(filename.getPathNameNoVolume()).c_str(), 2) == UNZ_OK);
}

bool Volume::remove(const Path& filename)
{
	return false;
}

bool Volume::makeDirectory(const Path& directory)
{
	return false;
}

bool Volume::removeDirectory(const Path& directory)
{
	return false;
}

bool Volume::renameDirectory(const Path& directory, const std::wstring& newName)
{
	return false;
}

bool Volume::setCurrentDirectory(const Path& directory)
{
	m_currentDirectory = directory;
	return true;
}
	
Path Volume::getCurrentDirectory() const
{
	return m_currentDirectory;
}

void Volume::flush()
{
	if (m_zipFile != 0)
	{
		zipClose(m_zipFile, 0);
		m_zipFile = 0;
	}
	if (m_unzFile != 0)
	{
		unzClose(m_unzFile);
		m_unzFile = 0;
	}
}

bool Volume::readArchive()
{
	if (m_zipFile != 0)
	{
		zipClose(m_zipFile, 0);
		m_zipFile = 0;
	}

	if (m_unzFile == 0)
	{
		m_unzFile = unzOpen2(wstombs(m_archive.getPathName()).c_str(), m_zlibFF);
		if (m_unzFile == 0)
		{
			log::error << L"Unable to open archive \"" << m_archive.getPathName() << L"\"" << Endl;
			return false;
		}
	}

	return true;
}

bool Volume::writeArchive()
{
	if (m_unzFile != 0)
	{
		unzClose(m_unzFile);
		m_unzFile = 0;
	}

	if (m_zipFile == 0)
	{
		std::stringstream comment;
		comment << "Traktor Framework, Copyright 2005-2008 (C) Anders Pistol";
		
		m_zipFile = zipOpen2(wstombs(m_archive.getPathName()).c_str(), 0, (zipcharpc*)comment.str().c_str(), m_zlibFF);
		if (m_zipFile == 0)
		{
			log::error << L"Unable to open archive \"" << m_archive.getPathName() << L"\"" << Endl;
			return false;
		}
	}

	return true;
}

	}
}
