#include "Avalanche/BlobFile.h"
#include "Avalanche/Key.h"
#include "Core/Guid.h"
#include "Core/Io/File.h"
#include "Core/Io/FileSystem.h"

namespace traktor
{
	namespace avalanche
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.avalanche.BlobFile", BlobFile, Object)

BlobFile::BlobFile(const Path& path)
:	m_path(path)
{
}

int64_t BlobFile::size() const
{
	Ref< File > file = FileSystem::getInstance().get(m_path);
	return file != nullptr ? file->getSize() : 0;
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
