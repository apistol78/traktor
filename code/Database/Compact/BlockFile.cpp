/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Reader.h"
#include "Core/Io/StreamStream.h"
#include "Core/Io/Writer.h"
#include "Core/Thread/Acquire.h"
#include "Database/Compact/BlockFile.h"
#include "Database/Compact/BlockReadStream.h"
#include "Database/Compact/BlockWriteStream.h"

namespace traktor::db
{
	namespace
	{

const uint32_t c_version = 3;
const uint32_t c_maxBlockCount = 8192;
const uint32_t c_headerSize = 3 * sizeof(uint32_t);
const uint32_t c_dataOffset = c_headerSize + c_maxBlockCount * (sizeof(uint32_t) + sizeof(BlockFile::Block));

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.BlockFile", BlockFile, Object)

BlockFile::~BlockFile()
{
	T_ASSERT(!m_stream);
}

bool BlockFile::create(const Path& fileName, bool flushAlways)
{
	if (FileSystem::getInstance().exist(fileName))
	{
		if (!FileSystem::getInstance().remove(fileName))
			return false;
	}

	m_stream = FileSystem::getInstance().open(fileName, File::FmRead | File::FmWrite);
	if (!m_stream)
		return false;

	m_fileName = fileName;
	m_flushAlways = flushAlways;
	m_unusedReadStreams.push_back(m_stream);

	flushTOC();
	return true;
}

bool BlockFile::open(const Path& fileName, bool readOnly, bool flushAlways)
{
	if (!readOnly && !FileSystem::getInstance().exist(fileName))
		return false;

	m_stream = FileSystem::getInstance().open(fileName, readOnly ? File::FmRead | File::FmMapped : File::FmRead | File::FmWrite);
	if (!m_stream)
		return false;

	Reader reader(m_stream);

	uint32_t magic, version;
	reader >> magic;
	reader >> version;

	if (magic != 'TBLK' || version != c_version)
		return false;

	uint32_t blockCount;
	reader >> blockCount;

	m_blocks.resize(blockCount);
	for (uint32_t i = 0; i < blockCount; ++i)
	{
		Block& block = m_blocks[i];
		reader >> block.id;
		reader >> block.offset;
		reader >> block.size;
		if (block.offset < c_dataOffset)
			return false;
	}

#if defined(_DEBUG)
	int64_t last = 0;
	int64_t size = 0;
	for (const auto& block : m_blocks)
	{
		last = std::max(last, block.offset + block.size);
		size += block.size;
	}
	log::info << last << L" allocated, " << size << L" used (" << (100 * (last - size)) / last << L"% waste) in " << m_blocks.size() << L" blocks." << Endl;
#endif

	m_stream->seek(IStream::SeekSet, c_dataOffset);

	m_fileName = fileName;
	m_flushAlways = flushAlways;
	m_unusedReadStreams.push_back(m_stream);
	return true;
}

void BlockFile::close()
{
	if (m_stream)
	{
		if (m_needFlushTOC)
			flushTOC();

		for (auto stream : m_unusedReadStreams)
			stream->close();

		m_stream->close();

		m_unusedReadStreams.clear();
		m_stream = nullptr;
	}
}

uint32_t BlockFile::allocBlockId()
{
	uint32_t maxBlockId = 0;
	for (const auto& block : m_blocks)
		maxBlockId = std::max(maxBlockId, block.id);

	Block block;
	block.id = maxBlockId + 1;
	block.offset = 0;
	block.size = 0;
	m_blocks.push_back(block);

	return block.id;
}

void BlockFile::freeBlockId(uint32_t blockId)
{
	auto it = std::find_if(m_blocks.begin(), m_blocks.end(), [=](const Block& b) {
		return b.id == blockId;
	});
	if (it != m_blocks.end())
		m_blocks.erase(it);
	else
		log::warning << L"Unable to free block " << blockId << L", no such block allocated." << Endl;
}

int64_t BlockFile::allocateRegion(int64_t size)
{
	// Check for large enough gaps between blocks.
	if (m_blocks.size() >= 2)
	{
		// Need to sort blocks by offset so we can easily search for gaps.
		AlignedVector< Block > blocks = m_blocks;
		std::sort(blocks.begin(), blocks.end(), [](const Block& lh, const Block& rh) {
			return lh.offset < rh.offset;
		});

		// Linearly search for block which has enough gap but smallest waste.
		int64_t minWaste = std::numeric_limits< int64_t >::max();
		const Block* minBlock = nullptr;

		for (int32_t i = 0; i < (int32_t)blocks.size() - 1; ++i)
		{
			if (blocks[i].offset < c_dataOffset)
				continue;

			const int64_t ctail = blocks[i].offset + blocks[i].size;
			const int64_t nhead = blocks[i + 1].offset;
			const int64_t gap = nhead - ctail;
			if (gap >= size)
			{
				const int64_t waste = gap - size;
				if (waste < minWaste)
				{
					minBlock = &blocks[i];
					minWaste = waste;
				}
			}
		}

		if (minBlock != nullptr)
		{
			const int64_t ctail = minBlock->offset + minBlock->size;
			return ctail;
		}

		// No gaps large enough found, append after last block.
		const auto& last = blocks.back();
		if (last.offset >= c_dataOffset)
			return last.offset + last.size;
	}
	else if (m_blocks.size() == 1)
	{
		// No gaps possible, append after last block.
		const auto& last = m_blocks.back();
		if (last.offset >= c_dataOffset)
			return last.offset + last.size;
	}

	// No blocks, append after last block.
	return c_dataOffset;
}

Ref< IStream > BlockFile::readBlock(uint32_t blockId)
{
	auto it = std::find_if(m_blocks.begin(), m_blocks.end(), [=](const Block& block) { return block.id == blockId; });
	if (it == m_blocks.end())
		return nullptr;

	Ref< IStream > stream;

	// Pop unused read streams from cache.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		if (!m_unusedReadStreams.empty())
		{
			stream = m_unusedReadStreams.back();
			m_unusedReadStreams.pop_back();
		}
	}

	// No unused stream available; create new read stream.
	if (!stream)
	{
		stream = FileSystem::getInstance().open(m_fileName, File::FmRead | File::FmMapped);
		if (!stream)
			return nullptr;
	}

	if (stream->seek(IStream::SeekSet, it->offset) < 0)
		return nullptr;

	return new BlockReadStream(this, stream, it->offset + it->size);
}

Ref< IStream > BlockFile::writeBlock(uint32_t blockId)
{
	auto it = std::find_if(m_blocks.begin(), m_blocks.end(), [=](const Block& block) { return block.id == blockId; });
	if (it == m_blocks.end())
		return nullptr;

	return new BlockWriteStream(this, m_stream, *it);
}

void BlockFile::needFlushTOC()
{
	if (!m_flushAlways)
		m_needFlushTOC = true;
	else
		flushTOC();
}

void BlockFile::flushTOC()
{
	m_stream->seek(IStream::SeekSet, 0);

	Writer writer(m_stream);

	writer << uint32_t('TBLK');
	writer << c_version;

	uint32_t blockCount = uint32_t(m_blocks.size());
	writer << blockCount;

	for (const auto& block : m_blocks)
	{
		writer << block.id;
		writer << block.offset;
		writer << block.size;
	}

	const int64_t padSize = c_dataOffset - m_stream->tell();
	for (int64_t i = 0; i < padSize; ++i)
	{
		const uint8_t padDummy = 0x00;
		m_stream->write(&padDummy, 1);
	}

	m_needFlushTOC = false;
}

void BlockFile::returnReadStream(IStream* readStream)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_unusedReadStreams.push_back(readStream);
}

}
