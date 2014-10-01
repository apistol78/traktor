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

int AssetsStream::tell() const
{
	return (m_assetFile != 0) ? (int)(m_size - AAsset_getRemainingLength(m_assetFile)) : 0;
}

int AssetsStream::available() const
{
	return (m_assetFile != 0) ? ((int)AAsset_getRemainingLength(m_assetFile)) : 0;
}

int AssetsStream::seek(SeekOriginType origin, int offset)
{
	if (m_assetFile == 0)
		return 0;

	const int fo[] = { SEEK_CUR, SEEK_END, SEEK_SET };
	return (int)AAsset_seek(m_assetFile, offset, fo[origin]);
}

int AssetsStream::read(void* block, int nbytes)
{
	if (m_assetFile == 0)
		return 0;

	return AAsset_read(m_assetFile, block, nbytes);
}

int AssetsStream::write(const void* block, int nbytes)
{
	return 0;
}

void AssetsStream::flush()
{
}

}
