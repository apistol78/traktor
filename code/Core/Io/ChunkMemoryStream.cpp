/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cstring>
#include "Core/Io/ChunkMemory.h"
#include "Core/Io/ChunkMemoryStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ChunkMemoryStream", ChunkMemoryStream, IStream)

ChunkMemoryStream::ChunkMemoryStream(ChunkMemory* memory, bool readAllowed, bool writeAllowed)
:	m_memory(memory)
,	m_position(0)
,	m_readAllowed(readAllowed)
,	m_writeAllowed(writeAllowed)
{
}

ChunkMemoryStream::~ChunkMemoryStream()
{
	close();
}

void ChunkMemoryStream::close()
{
	m_memory = nullptr;
	m_position = 0;
	m_readAllowed =
	m_writeAllowed = false;
}

bool ChunkMemoryStream::canRead() const
{
	return m_readAllowed;
}

bool ChunkMemoryStream::canWrite() const
{
	return m_writeAllowed;
}

bool ChunkMemoryStream::canSeek() const
{
	return true;
}

int64_t ChunkMemoryStream::tell() const
{
	if (m_memory)
		return m_position;
	else
		return -1;
}

int64_t ChunkMemoryStream::available() const
{
	if (m_memory)
		return m_memory->size() - m_position;
	else
		return -1;
}

int64_t ChunkMemoryStream::seek(SeekOriginType origin, int64_t offset)
{
	if (!m_memory)
		return -1;

	if (origin == SeekCurrent)
		m_position += offset;
	else if (origin == SeekEnd)
		m_position = m_memory->size() + offset;
	else if (origin == SeekSet)
		m_position = offset;

	return m_position;
}

int64_t ChunkMemoryStream::read(void* block, int64_t nbytes)
{
	if (!m_memory)
		return -1;
	if (!m_readAllowed)
		return 0;
	if (nbytes <= 0)
		return 0;

	uint8_t* blockPtr = (uint8_t*)block;
	int64_t nread = 0;

	while (nread < nbytes)
	{
		auto chunk = m_memory->getChunk(m_position);
		if (!chunk.ptr)
			break;

		int64_t chunkRead = std::min< int64_t >(nbytes - nread, chunk.avail);
		if (chunkRead <= 0)
			break;

		std::memcpy(blockPtr, chunk.ptr, chunkRead);

		blockPtr += chunkRead;
		nread += chunkRead;
		m_position += chunkRead;
	}

	return nread;
}

int64_t ChunkMemoryStream::write(const void* block, int64_t nbytes)
{
	if (!m_writeAllowed)
		return 0;
	if (nbytes <= 0)
		return 0;

	size_t newSize = std::max< size_t >(m_memory->size(), m_position + nbytes);
	if (!m_memory->resize(newSize))
		return 0;

	const uint8_t* blockPtr = (const uint8_t*)block;
	int64_t nwritten = 0;

	while (nwritten < nbytes)
	{
		auto chunk = m_memory->getChunk(m_position);
		if (!chunk.ptr)
			break;

		int64_t chunkWrite = std::min< int64_t >(nbytes - nwritten, chunk.avail);
		std::memcpy(chunk.ptr, blockPtr, chunkWrite);

		blockPtr += chunkWrite;
		nwritten += chunkWrite;
		m_position += chunkWrite;
	}

	return nwritten;
}

void ChunkMemoryStream::flush()
{
}

}

