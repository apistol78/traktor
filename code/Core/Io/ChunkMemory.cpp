/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Io/ChunkMemory.h"
#include "Core/Memory/Alloc.h"

namespace traktor
{
	namespace
	{

const int64_t c_chunkSize = 65536;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ChunkMemory", ChunkMemory, Object)

ChunkMemory::~ChunkMemory()
{
	for (auto chunk : m_chunks)
		Alloc::freeAlign(chunk);
}

size_t ChunkMemory::size() const
{
	return m_size;
}

bool ChunkMemory::resize(size_t size)
{
	const size_t chunkCount = size / c_chunkSize + 1;
	
	// Allocate more chunks if necessary.
	while (chunkCount > m_chunks.size())
	{
		uint8_t* chunkPtr = (uint8_t*)Alloc::acquireAlign(c_chunkSize, 16, T_FILE_LINE);
		if (!chunkPtr)
			return false;
		m_chunks.push_back(chunkPtr);
	}

	// Drop chunks if necessary.
	while (chunkCount < m_chunks.size())
	{
		Alloc::freeAlign(m_chunks.back());
		m_chunks.pop_back();
	}

	m_size = size;
	return true;
}

ChunkMemory::Chunk ChunkMemory::getChunk(size_t position) const
{
	const size_t index = position / c_chunkSize;
	const size_t offset = position % c_chunkSize;

	if (index >= m_chunks.size())
		return { nullptr, nullptr, 0, 0 };

	uint8_t* head = m_chunks[index];
	T_FATAL_ASSERT(head != nullptr);

	Chunk chunk;
	chunk.head = head;
	chunk.ptr = head + offset;
	chunk.size = std::min< size_t >(c_chunkSize, m_size - position);
	chunk.avail = std::min< size_t >(c_chunkSize - offset, m_size - position);
	return chunk;
}

}
