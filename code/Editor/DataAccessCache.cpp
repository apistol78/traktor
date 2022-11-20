/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/EnterLeave.h"
#include "Editor/DataAccessCache.h"
#include "Editor/IPipelineCache.h"
#include "Editor/Pipeline/PipelineProfiler.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.DataAccessCache", DataAccessCache, Object)

DataAccessCache::DataAccessCache(PipelineProfiler* profiler, IPipelineCache* cache)
:	m_profiler(profiler)
,	m_cache(cache)
{
}

Ref< Object > DataAccessCache::readObject(
	const Key& key,
	const std::function< Ref< Object > (IStream* stream) >& read,
	const std::function< bool (const Object* object, IStream* stream) >& write,
	const std::function< Ref< Object > () >& create
)
{
	T_ANONYMOUS_VAR(EnterLeave)(
		[&](){ m_profiler->begin(type_of< DataAccessCache >()); },
		[&](){ m_profiler->end(type_of< DataAccessCache >()); }
	);

	Ref< IStream > s;

	// Try to read from cache first.
	if (m_cache != nullptr)
	{
		if ((s = m_cache->get(key)) != nullptr)
		{
			m_profiler->begin(L"DataAccessCache read");
			Ref< Object > object = read(s);
			m_profiler->end(L"DataAccessCache read");
			s->close();
			return object;
		}
	}

	// No cached entry; need to fabricate object.
	m_profiler->begin(L"DataAccessCache create");
	Ref< Object > object = create();
	m_profiler->end(L"DataAccessCache create");
	if (!object)
		return nullptr;

	// Upload to cache and then return object.
	if (m_cache != nullptr)
	{
		if ((s = m_cache->put(key)) != nullptr)
		{
			m_profiler->begin(L"DataAccessCache write");
			const bool result = write(object, s);
			m_profiler->end(L"DataAccessCache write");
			if (result)
				s->close();
			else
				log::error << L"Unable to upload memento object to cache." << Endl;
		}
	}
	
	return object;
}

}
