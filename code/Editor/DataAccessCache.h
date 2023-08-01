/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/Ref.h"
#include "Core/Misc/Key.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

}

namespace traktor::editor
{

class IPipelineCache;
class PipelineProfiler;

/*! Data access cache.
 *
 * This cache is useful for storing expensive operations, aka memoization,
 * from previous runs.
 */
class T_DLLCLASS DataAccessCache : public Object
{
	T_RTTI_CLASS;

public:
	explicit DataAccessCache(PipelineProfiler* profiler, IPipelineCache* cache);

	template< typename ObjectType >
	Ref< ObjectType > read(
		const Key& key,
		const std::function< Ref< ObjectType > () >& create
	)
	{
		return dynamic_type_cast< ObjectType* >(readObject(
			key,
			[&]() -> Ref< ISerializable > { return dynamic_type_cast< ObjectType* >(create()); }
		));
	}

private:
	Ref< PipelineProfiler > m_profiler;
	IPipelineCache* m_cache;

	Ref< ISerializable > readObject(
		const Key& key,
		const std::function< Ref< ISerializable > () >& create
	);
};

}
