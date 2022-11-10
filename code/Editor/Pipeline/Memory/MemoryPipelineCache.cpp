/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/ChunkMemoryStream.h"
#include "Core/Thread/Acquire.h"
#include "Editor/Pipeline/Memory/MemoryPipelineCache.h"

namespace traktor::editor
{

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

	auto it = m_committed.find(guid);
	if (it == m_committed.end())
		return nullptr;

	return new ChunkMemoryStream(it->second, true, false);
}

Ref< IStream > MemoryPipelineCache::put(const Guid& guid, const PipelineDependencyHash& hash)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	auto it = m_pending.find(guid);
	if (it != m_pending.end())
		return nullptr;

	Ref< ChunkMemory > cm = new ChunkMemory();
	Ref< ChunkMemoryStream > cms = new ChunkMemoryStream(cm, false, true);
	m_pending.insert(std::make_pair(guid, cm));
	return cms;
}

bool MemoryPipelineCache::commit(const Guid& guid, const PipelineDependencyHash& hash)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	auto it = m_pending.find(guid);
	if (it == m_pending.end())
		return false;

	m_committed.insert(std::make_pair(guid, it->second));
	m_pending.erase(it);
	return true;
}

Ref< IStream > MemoryPipelineCache::get(const Key& key)
{
	return nullptr;
}

Ref< IStream > MemoryPipelineCache::put(const Key& key)
{
	return nullptr;
}

void MemoryPipelineCache::getInformation(OutputStream& os)
{
}

}
