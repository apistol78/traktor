#include "Avalanche/BlobFile.h"
#include "Core/Guid.h"
#include "Core/Io/File.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Misc/Key.h"

namespace traktor::avalanche
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

bool BlobFile::create(IStream* source)
{
	const Path tmpPath = m_path.getPathName() + L".tmp";

	// Create a temporary file and copy source stream.
	Ref< IStream > tmp = FileSystem::getInstance().open(tmpPath, File::FmWrite);
	if (!tmp)
		return false;

	bool result = StreamCopy(tmp, source).execute();

	tmp->close();
	tmp = nullptr;

	// If copy succeeded then move into place.
	if (result)
		result = FileSystem::getInstance().move(m_path, tmpPath, true);
	
	// Cleanup temporary file if something failed.
	if (!result)
		FileSystem::getInstance().remove(tmpPath);

	return result;
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
