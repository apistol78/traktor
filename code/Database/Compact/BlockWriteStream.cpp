/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/ChunkMemory.h"
#include "Core/Io/ChunkMemoryStream.h"
#include "Core/Io/StreamCopy.h"
#include "Database/Compact/BlockWriteStream.h"

namespace traktor::db
{

BlockWriteStream::BlockWriteStream(BlockFile* blockFile, IStream* stream, BlockFile::Block& outBlock)
:	m_blockFile(blockFile)
,	m_outputStream(stream)
,	m_outBlock(outBlock)
{
	// Initialize output block first, to ensure not to be invalid in case of error.
	m_outBlock.offset = 0;
	m_outBlock.size = 0;

	// Initialize in-memory, recording, stream.
	m_memory = new ChunkMemory();
	m_memoryStream = new ChunkMemoryStream(m_memory, false, true);
}

BlockWriteStream::~BlockWriteStream()
{
	close();
}

void BlockWriteStream::close()
{
	if (!m_outputStream)
		return;

	const size_t size = m_memory->size();

	// Find region in block file which has enough room to write gathered data.
	const int64_t offset = m_blockFile->allocateRegion(size);

	// Move write pointer to region.
	m_outputStream->seek(IStream::SeekSet, offset);

	// Write entire block of memory to output stream.
	ChunkMemoryStream source(m_memory, true, false);
	StreamCopy(m_outputStream, &source).execute();

	// Update block descriptor.
	m_outBlock.offset = offset;
	m_outBlock.size = size;

	// Flush table of content and close output stream.
	m_blockFile->needFlushTOC();
	m_outputStream = nullptr;
	m_memoryStream = nullptr;
	m_memory = nullptr;
}

bool BlockWriteStream::canRead() const
{
	return false;
}

bool BlockWriteStream::canWrite() const
{
	return true;
}

bool BlockWriteStream::canSeek() const
{
	return false;
}

int64_t BlockWriteStream::tell() const
{
	return 0;
}

int64_t BlockWriteStream::available() const
{
	return 0;
}

int64_t BlockWriteStream::seek(SeekOriginType origin, int64_t offset)
{
	return -1;
}

int64_t BlockWriteStream::read(void* block, int64_t nbytes)
{
	return -1;
}

int64_t BlockWriteStream::write(const void* block, int64_t nbytes)
{
	T_ASSERT(m_memoryStream != nullptr);
	return m_memoryStream->write(block, nbytes);
}

void BlockWriteStream::flush()
{
	T_ASSERT(m_memoryStream != nullptr);
	m_memoryStream->flush();
}

}
