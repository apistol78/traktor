#include <android/native_activity.h>
#include "Core/Io/Android/AssetsStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.AssetsStream", AssetsStream, IStream)

AssetsStream::AssetsStream(AAsset* assetFile)
:	m_assetFile(assetFile)
,	m_size(0)
{
	m_size = AAsset_getLength(m_assetFile);
}

AssetsStream::~AssetsStream()
{
	close();
}

void AssetsStream::close()
{
	if (m_assetFile)
	{
		AAsset_close(m_assetFile);
		m_assetFile = 0;
	}
}

bool AssetsStream::canRead() const
{
	return m_assetFile != 0;
}

bool AssetsStream::canWrite() const
{
	return false;
}

bool AssetsStream::canSeek() const
{
	return true;
}

int64_t AssetsStream::tell() const
{
	return (m_assetFile != 0) ? (int64_t)(m_size - AAsset_getRemainingLength(m_assetFile)) : 0;
}

int64_t AssetsStream::available() const
{
	return (m_assetFile != 0) ? ((int64_t)AAsset_getRemainingLength(m_assetFile)) : 0;
}

int64_t AssetsStream::seek(SeekOriginType origin, int64_t offset)
{
	if (m_assetFile == 0)
		return 0;

	const int fo[] = { SEEK_CUR, SEEK_END, SEEK_SET };
	return (int64_t)AAsset_seek(m_assetFile, offset, fo[origin]);
}

int64_t AssetsStream::read(void* block, int64_t nbytes)
{
	if (m_assetFile == 0)
		return 0;

	return AAsset_read(m_assetFile, block, nbytes);
}

int64_t AssetsStream::write(const void* block, int64_t nbytes)
{
	return 0;
}

void AssetsStream::flush()
{
}

}
