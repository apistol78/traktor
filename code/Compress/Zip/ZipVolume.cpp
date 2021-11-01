#include "Compress/Zip/InflateStreamZip.h"
#include "Compress/Zip/ZipVolume.h"
#include "Core/Io/File.h"
#include "Core/Io/StreamStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Misc/TString.h"
#include "Core/Misc/WildCompare.h"

namespace traktor
{
	namespace compress
	{
		namespace
		{

#pragma pack(1)
struct EOCD
{
	uint32_t signature;
	uint16_t disk;
	uint16_t diskOfCD;
	uint16_t numberOfCDRecords;
	uint16_t totalNumberOfCDRecords;
	uint32_t cdSize;
	uint32_t cdOffset;
	uint16_t commentLength;
	// uint8_t comment[commentLength]
};

struct CDFH
{
	uint32_t signature;
	uint16_t version;
	uint16_t requiredVersion;
	uint16_t generalFlags;
	uint16_t compression;
	uint16_t lastModificationTime;
	uint16_t lastModificationDate;
	uint32_t crc32;
	uint32_t compressedSize;
	uint32_t uncompressedSize;
	uint16_t fileNameLength;
	uint16_t extraFieldLength;
	uint16_t commentLength;
	uint16_t startDisk;
	uint16_t internalAttributes;
	uint32_t externalAttributes;
	uint32_t lfhOffset;
	// uint8_t fileName[fileNameLength]
	// uint8_t extraField[extraFieldLength]
	// uint8_t comment[commentLength]
};

struct LFH
{
	uint32_t signature;
	uint16_t version;
	uint16_t generalFlags;
	uint16_t compression;
	uint16_t lastModificationTime;
	uint16_t lastModificationDate;
	uint32_t crc32;
	uint32_t compressedSize;
	uint32_t uncompressedSize;
	uint16_t fileNameLength;
	uint16_t extraFieldLength;
	// uint8_t fileName[fileNameLength]
	// uint8_t extraField[extraFieldLength]
};

#pragma pack()

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.compress.ZipVolume", ZipVolume, IVolume)

ZipVolume::ZipVolume(IStream* zipFile)
:	m_zipFile(zipFile)
{
	char fileName[4096];
	char extra[4096];
	char comment[4096];
	EOCD eocd = {};

	// Search for EOCD signature from the end of file.
	uint64_t size = m_zipFile->available();
	if (size < sizeof(EOCD))
	{
		log::error << L"Corrupt ZIP file; no data." << Endl;
		return;
	}

	uint64_t search = size - sizeof(EOCD);
	do
	{
		m_zipFile->seek(IStream::SeekSet, search);
		if (m_zipFile->read(&eocd.signature, 4) != 4)
		{
			log::error << L"Corrupt ZIP file; failed to find EOCD signature, read failed." << Endl;
			return;
		}
		if (eocd.signature == 0x06054b50)
			break;
		search--;
	}
	while (search >= 0);

	if (eocd.signature != 0x06054b50)
	{
		log::error << L"Corrupt ZIP file; failed to find EOCD signature." << Endl;
		return;
	}

	// Read rest of EOCD header.
	m_zipFile->read(&eocd.disk, sizeof(EOCD) - sizeof(uint32_t));

	// Add root entry; must be at index 0.
	auto& root = m_fileInfo.push_back();
	root.parent = -1;

	// Read central directory.
	m_zipFile->seek(IStream::SeekSet, eocd.cdOffset);
	for (uint32_t i = 0; i < eocd.numberOfCDRecords; ++i)
	{
		CDFH cdfh = {};

		if (m_zipFile->read(&cdfh, sizeof(CDFH)) != sizeof(CDFH))
		{
			log::error << L"Corrupt ZIP file; failed to find CDFH." << Endl;
			return;
		}

		if (cdfh.signature != 0x02014b50)
		{
			log::error << L"Corrupt ZIP file; incorrect CDFH signature." << Endl;
			return;
		}

		if (
			cdfh.fileNameLength >= sizeof_array(fileName) ||
			cdfh.extraFieldLength >= sizeof_array(extra) ||
			cdfh.commentLength >= sizeof_array(comment)
		)
		{
			log::error << L"Corrupt ZIP file; too long strings in header." << Endl;
			return;
		}

		m_zipFile->read(fileName, cdfh.fileNameLength);
		fileName[cdfh.fileNameLength] = '\0';

		m_zipFile->read(extra, cdfh.extraFieldLength);
		m_zipFile->read(comment, cdfh.commentLength);

		int32_t directory = 0;

		StringSplit< std::wstring > ss(mbstows(fileName), L"/");
		for (auto it = ss.begin(); it != ss.end(); )
		{
			std::wstring txt = *it++;
			if (it != ss.end())
			{
				auto fiit = std::find_if(m_fileInfo.begin(), m_fileInfo.end(), [&](const FileInfo& fi) {
					return fi.name == txt;
				});
				if (fiit == m_fileInfo.end())
				{
					int32_t index = (int32_t)(m_fileInfo.size());

					auto& fi = m_fileInfo.push_back();
					fi.name = txt;
					fi.parent = directory;

					m_fileInfo[directory].children.push_back(index);

					directory = index;
				}
				else
					directory = (int32_t)(std::distance(m_fileInfo.begin(), fiit));
			}
			else
			{
				int32_t index = (int32_t)(m_fileInfo.size());

				auto& fi = m_fileInfo.push_back();
				fi.name = txt;
				fi.parent = directory;
				fi.offset = cdfh.lfhOffset;
				fi.compressedSize = cdfh.compressedSize;
				fi.uncompressedSize = cdfh.uncompressedSize;
				fi.attributes = ((cdfh.version >> 8) == 3) ? (cdfh.externalAttributes >> 16) : 0;

				m_fileInfo[directory].children.push_back(index);
			}
		}
	}
}

std::wstring ZipVolume::getDescription() const
{
	return L"zip";
}

Ref< File > ZipVolume::get(const Path& path)
{
	return nullptr;
}

int ZipVolume::find(const Path& mask, RefArray< File >& out)
{
	int32_t directoryIndex = findFileInfoIndex(mask.getPathOnlyNoVolume());
	if (directoryIndex < 0)
		return 0;

	std::wstring fileMask = mask.getFileName();
	if (fileMask == L"*.*")
		fileMask = L"*";

	WildCompare maskCompare(fileMask);
	for (auto child : m_fileInfo[directoryIndex].children)
	{
		const auto& fi = m_fileInfo[child];
		if (maskCompare.match(fi.name))
		{
			if (!fi.isDirectory())
			{
				out.push_back(new File(
					L"zip:" + getPathName(child),
					fi.uncompressedSize,
					File::FfNormal | ( ((fi.attributes & 0111) != 0000) ? File::FfExecutable : 0 )
				));
			}
			else
			{
				out.push_back(new File(
					L"zip:" + getPathName(child),
					0,
					File::FfDirectory
				));
			}
		}
	}

	return (int)out.size();
}

bool ZipVolume::modify(const Path& fileName, uint32_t flags)
{
	return false;
}

bool ZipVolume::modify(const Path& fileName, const DateTime* creationTime, const DateTime* lastAccessTime, const DateTime* lastWriteTime)
{
	return false;
}

Ref< IStream > ZipVolume::open(const Path& fileName, uint32_t mode)
{
	char fileNameTmp[4096];
	char extra[4096];
	LFH lfh = {};

	int32_t fileIndex = findFileInfoIndex(fileName.getPathNameNoVolume());
	if (fileIndex < 0)
		return nullptr;

	const FileInfo& fi = m_fileInfo[fileIndex];
	if (fi.isDirectory())
		return nullptr;

	m_zipFile->seek(IStream::SeekSet, fi.offset);

	if (m_zipFile->read(&lfh, sizeof(LFH)) != sizeof(LFH))
	{
		log::error << L"Corrupt ZIP file; failed to find LFH." << Endl;
		return nullptr;
	}

	if (lfh.signature != 0x04034b50)
	{
		log::error << L"Corrupt ZIP file; incorrect LFH signature." << Endl;
		return nullptr;
	}

	if (
		lfh.fileNameLength >= sizeof_array(fileNameTmp) ||
		lfh.extraFieldLength >= sizeof_array(extra)
	)
	{
		log::error << L"Corrupt ZIP file; too long strings in header." << Endl;
		return nullptr;
	}

	m_zipFile->read(fileNameTmp, lfh.fileNameLength);
	fileNameTmp[lfh.fileNameLength] = '\0';

	m_zipFile->read(extra, lfh.extraFieldLength);

	return new StreamStream(
		new InflateStreamZip(m_zipFile, 4096, true),
		m_zipFile->tell() + fi.uncompressedSize
	);
}

bool ZipVolume::exist(const Path& fileName)
{
	return (bool)(get(fileName) != nullptr);
}

bool ZipVolume::remove(const Path& fileName)
{
	return false;
}

bool ZipVolume::move(const Path& fileName, const std::wstring& newName, bool overwrite)
{
	return false;
}

bool ZipVolume::copy(const Path& fileName, const std::wstring& newName, bool overwrite)
{
	return false;
}

bool ZipVolume::makeDirectory(const Path& directory)
{
	return false;
}

bool ZipVolume::removeDirectory(const Path& directory)
{
	return false;
}

bool ZipVolume::renameDirectory(const Path& directory, const std::wstring& newName)
{
	return false;
}

bool ZipVolume::setCurrentDirectory(const Path& directory)
{
	if (directory.isRelative())
		m_currentDirectory = m_currentDirectory + directory;
	else
		m_currentDirectory = directory;
	return true;
}

Path ZipVolume::getCurrentDirectory() const
{
	return m_currentDirectory;
}

std::wstring ZipVolume::getSystemPath(const Path& path) const
{
	StringOutputStream ss;
	
	if (path.isRelative())
	{
		std::wstring tmp = m_currentDirectory.getPathNameNoVolume();
		ss << tmp << L"/" << path.getPathNameNoVolume();
	}
	else
		ss << path.getPathNameNoVolume();

	std::wstring txt = ss.str();
	if (startsWith(txt, L"/"))
		return txt.substr(1);
	else
		return txt;
}

std::wstring ZipVolume::getPathName(int32_t index) const
{
	const auto& fi = m_fileInfo[index];
	if (fi.parent >= 0)
		return getPathName(fi.parent) + L"/" + fi.name;
	else
		return fi.name;
}

int32_t ZipVolume::findFileInfoIndex(const Path& path) const
{
	std::wstring sp = getSystemPath(path);
	int32_t index = 0;

	StringSplit< std::wstring > ss(sp, L"/");
	for (auto it = ss.begin(); it != ss.end(); ++it)
	{
		const std::wstring txt = *it;
		const FileInfo& fi = m_fileInfo[index];

		int32_t next = -1;
		for (auto child : fi.children)
		{
			if (m_fileInfo[child].name == txt)
			{
				next = child;
				break;
			}
		}
		if (next < 0)
			return -1;

		index = next;
	}

	return index;
}

void ZipVolume::dump(int32_t index) const
{
	const auto& fi = m_fileInfo[index];
	log::info << index << L". \"" << fi.name << L"\", parent " << fi.parent << L", " << (fi.isDirectory() ? L"directory" : L"file") << Endl;
	log::info << IncreaseIndent;

	for (auto c : fi.children)
		dump(c);

	log::info << DecreaseIndent;
}

	}
}
