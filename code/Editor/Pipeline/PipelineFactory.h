/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

//#include <vector>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

}

namespace traktor::db
{

class Database;

}

namespace traktor::editor
{

class IPipeline;

class T_DLLCLASS PipelineFactory : public Object
{
	T_RTTI_CLASS;

public:
	explicit PipelineFactory(const PropertyGroup* settings, db::Database* database);

	virtual ~PipelineFactory();

	bool findPipelineType(const TypeInfo& sourceType, const TypeInfo*& outPipelineType, uint32_t& outPipelineHash) const;

	IPipeline* findPipeline(const TypeInfo& pipelineType) const;

private:
	struct PipelineMatch
	{
		IPipeline* pipeline;
		uint32_t hash;
		uint32_t distance;
	};

	SmallMap< const TypeInfo*, Ref< IPipeline > > m_pipelines;
	SmallMap< const TypeInfo*, PipelineMatch > m_pipelineMap;
};

}
