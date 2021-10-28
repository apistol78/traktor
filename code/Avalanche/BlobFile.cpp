#include "Avalanche/BlobFile.h"
#include "Core/Guid.h"
#include "Core/Io/File.h"
#include "Core/Io/FileSystem.h"
#include "Core/Misc/Key.h"

namespace traktor
{
	namespace avalanche
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.avalanche.BlobFile", BlobFile, Object)

BlobFile::BlobFile(const Path& path, int64_t size, const DateTime& lastAccessed)
:	m_path(path)
,	m_size(size)
,	m_lastAccessed(lastAccessed)
{
}

BlobFile::~BlobFile()
{
	FileSystem::getInstance().modify(m_path, nullptr, &m_lastAccessed, nullptr);
}

int64_t BlobFile::size() const
{
	return m_size;
}

Ref< IStream > BlobFile::append()
{
	m_lastAccessed = DateTime::now();
	return FileSystem::getInstance().open(m_path, File::FmWrite);
}

Ref< IStream > BlobFile::read() const
{
	m_lastAccessed = DateTime::now();
	return FileSystem::getInstance().open(m_path, File::FmRead);
}

bool BlobFile::remove()
{
	return FileSystem::getInstance().remove(m_path);
}

bool BlobFile::touch()
{
	m_lastAccessed = DateTime::now();
	return true;
}

DateTime BlobFile::lastAccessed() const
{
	return m_lastAccessed;
}

	}
}
