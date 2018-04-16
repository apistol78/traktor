/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/HashStream.h"
#include "Core/Misc/IHash.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.HashStream", HashStream, IStream);

HashStream::HashStream(IHash* hash)
:	m_hash(hash)
{
	m_hash->begin();
}

void HashStream::close()
{
	T_ASSERT (m_hash);
	m_hash->end();
	m_hash = 0;
}

bool HashStream::canRead() const
{
	T_ASSERT (m_hash);
	return false;
}

bool HashStream::canWrite() const
{
	T_ASSERT (m_hash);
	return true;
}

bool HashStream::canSeek() const
{
	T_ASSERT (m_hash);
	return false;
}

int64_t HashStream::tell() const
{
	T_ASSERT (m_hash);
	return 0;
}

int64_t HashStream::available() const
{
	T_ASSERT (m_hash);
	return 0;
}

int64_t HashStream::seek(SeekOriginType origin, int64_t offset)
{
	T_ASSERT (m_hash);
	return 0;
}

int64_t HashStream::read(void* block, int64_t nbytes)
{
	T_ASSERT (m_hash);
	return 0;
}

int64_t HashStream::write(const void* block, int64_t nbytes)
{
	T_ASSERT (m_hash);
	m_hash->feed(block, nbytes);
	return nbytes;
}

void HashStream::flush()
{
}

}
