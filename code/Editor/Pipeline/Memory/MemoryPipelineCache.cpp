/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Editor/Pipeline/Memory/MemoryPipelineCache.h"

#include "Core/Guid.h"
#include "Core/Io/ChunkMemoryStream.h"
#include "Core/Thread/Acquire.h"

namespace traktor::editor
{
namespace
{

// Combine guid and hash into a single 128-bit storage key so that multiple
// hashes of the same guid can coexist; mirrors File/Avalanche cache addressing.
Guid combineKey(const Guid& guid, const PipelineDependencyHash& hash)
{
	return guid.permutation(Guid((const uint8_t*)&hash));
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.MemoryPipelineCache", MemoryPipelineCache, IPipelineCache)

bool MemoryPipelineCache::create(const PropertyGroup* settings)
{
	return true;
}

void MemoryPipelineCache::destroy()
{
}

Ref< IStream > MemoryPipelineCache::get(const Guid& guid, const PipelineDependencyHash& hash)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	const Guid key = combineKey(guid, hash);

	auto it = m_committed.find(key);
	if (it == m_committed.end())
		return nullptr;

	return new ChunkMemoryStream(it->second.memory, true, false);
}

Ref< IStream > MemoryPipelineCache::put(const Guid& guid, const PipelineDependencyHash& hash)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	const Guid key = combineKey(guid, hash);

	auto it = m_pending.find(key);
	if (it != m_pending.end())
		return nullptr;

	Ref< ChunkMemory > cm = new ChunkMemory();
	Ref< ChunkMemoryStream > cms = new ChunkMemoryStream(cm, false, true);
	m_pending.insert(std::make_pair(key, Chunk{ hash, cm }));
	return cms;
}

bool MemoryPipelineCache::commit(const Guid& guid, const PipelineDependencyHash& hash)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	const Guid key = combineKey(guid, hash);

	auto it = m_pending.find(key);
	if (it == m_pending.end())
		return false;

	m_committed[key] = it->second;
	m_pending.erase(it);
	return true;
}

Ref< IStream > MemoryPipelineCache::get(const Key& key)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	auto it = m_blobs.find(key);
	if (it == m_blobs.end())
		return nullptr;

	return new ChunkMemoryStream(it->second, true, false);
}

Ref< IStream > MemoryPipelineCache::put(const Key& key)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	auto it = m_blobs.find(key);
	if (it != m_blobs.end())
		return nullptr;

	Ref< ChunkMemory > cm = new ChunkMemory();
	Ref< ChunkMemoryStream > cms = new ChunkMemoryStream(cm, false, true);
	m_blobs.insert(std::make_pair(key, cm));
	return cms;
}

void MemoryPipelineCache::getInformation(OutputStream& os)
{
}

}
