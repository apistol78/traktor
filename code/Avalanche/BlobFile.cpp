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

BlobFile::BlobFile(const Path& path, int64_t size)
:	m_path(path)
,	m_size(size)
{
}

int64_t BlobFile::size() const
{
	return m_size;
}

Ref< IStream > BlobFile::append()
{
	return FileSystem::getInstance().open(m_path, File::FmWrite);
}

Ref< IStream > BlobFile::read() const
{
	return FileSystem::getInstance().open(m_path, File::FmRead);
}

	}
}
