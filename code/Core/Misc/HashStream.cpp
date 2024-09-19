/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
	T_ASSERT(m_hash);
	m_hash->end();
	m_hash = nullptr;
}

bool HashStream::canRead() const
{
	T_ASSERT(m_hash);
	return false;
}

bool HashStream::canWrite() const
{
	T_ASSERT(m_hash);
	return true;
}

bool HashStream::canSeek() const
{
	T_ASSERT(m_hash);
	return false;
}

int64_t HashStream::tell() const
{
	T_ASSERT(m_hash);
	return 0;
}

int64_t HashStream::available() const
{
	T_ASSERT(m_hash);
	return 0;
}

int64_t HashStream::seek(SeekOriginType origin, int64_t offset)
{
	T_ASSERT(m_hash);
	return 0;
}

int64_t HashStream::read(void* block, int64_t nbytes)
{
	T_ASSERT(m_hash);
	return 0;
}

int64_t HashStream::write(const void* block, int64_t nbytes)
{
	T_ASSERT(m_hash);
	m_hash->feedBuffer(block, nbytes);
	return nbytes;
}

void HashStream::flush()
{
}

}
