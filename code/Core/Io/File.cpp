#include "Core/Io/File.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.File", 0, File, ISerializable)

File::File()
:	m_size(0)
,	m_flags(FfInvalid)
{
}

File::File(
	const Path& path,
	uint64_t size,
	uint32_t flags,
	const DateTime& creationTime,
	const DateTime& lastAccessTime,
	const DateTime& lastWriteTime
)
:	m_path(path)
,	m_size(size)
,	m_flags(flags)
,	m_creationTime(creationTime)
,	m_lastAccessTime(lastAccessTime)
,	m_lastWriteTime(lastWriteTime)
{
}

File::File(
	const Path& path,
	uint64_t size,
	uint32_t flags
)
:	m_path(path)
,	m_size(size)
,	m_flags(flags)
{
}

const Path& File::getPath() const
{
	return m_path;
}

uint64_t File::getSize() const
{
	return m_size;
}

uint32_t File::getFlags() const
{
	return m_flags;
}

bool File::isNormal() const
{
	return bool((m_flags & FfNormal) == FfNormal);
}

bool File::isReadOnly() const
{
	return bool((m_flags & FfReadOnly) == FfReadOnly);
}

bool File::isHidden() const
{
	return bool((m_flags & FfHidden) == FfHidden);
}

bool File::isArchive() const
{
	return bool((m_flags & FfArchive) == FfArchive);
}

bool File::isDirectory() const
{
	return bool((m_flags & FfDirectory) == FfDirectory);
}

const DateTime& File::getCreationTime() const
{
	return m_creationTime;
}

const DateTime& File::getLastAccessTime() const
{
	return m_lastAccessTime;
}

const DateTime& File::getLastWriteTime() const
{
	return m_lastWriteTime;
}

void File::serialize(ISerializer& s)
{
	s >> Member< Path >(L"path", m_path);
	s >> Member< uint64_t >(L"size", m_size);
	s >> Member< uint32_t >(L"flags", m_flags);
	s >> MemberComposite< DateTime >(L"creationTime", m_creationTime);
	s >> MemberComposite< DateTime >(L"lastAccessTime", m_lastAccessTime);
	s >> MemberComposite< DateTime >(L"lastWriteTime", m_lastWriteTime);
}

}
