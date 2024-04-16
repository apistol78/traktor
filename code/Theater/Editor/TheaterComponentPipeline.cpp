/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepHash.h"
#include "Editor/IPipelineBuilder.h"
#include "Theater/ActData.h"
#include "Theater/TheaterComponentData.h"
#include "Theater/TrackData.h"
#include "Theater/Editor/TheaterComponentPipeline.h"
#include "World/EntityData.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TheaterComponentPipeline", 2, TheaterComponentPipeline, editor::IPipeline)

bool TheaterComponentPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void TheaterComponentPipeline::destroy()
{
}

TypeInfoSet TheaterComponentPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< TheaterComponentData >();
}

bool TheaterComponentPipeline::shouldCache() const
{
	return false;
}

uint32_t TheaterComponentPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool TheaterComponentPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	return true;
}

bool TheaterComponentPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	return false;
}

Ref< ISerializable > TheaterComponentPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	const TheaterComponentData* sourceComponentData = checked_type_cast< const TheaterComponentData*, false >(sourceAsset);
	const RefArray< ActData >& sourceActs = sourceComponentData->getActs();

	Ref< TheaterComponentData > componentData = new TheaterComponentData();
	componentData->m_repeatActs = sourceComponentData->m_repeatActs;
	componentData->m_randomizeActs = sourceComponentData->m_randomizeActs;

	RefArray< ActData >& acts = componentData->getActs();
	acts.resize(sourceActs.size());

	for (uint32_t i = 0; i < sourceActs.size(); ++i)
	{
		const RefArray< TrackData >& sourceTracks = sourceActs[i]->getTracks();

		acts[i] = new ActData(*sourceActs[i]);

		RefArray< TrackData >& tracks = acts[i]->getTracks();
		tracks.resize(sourceTracks.size());
		for (uint32_t j = 0; j < sourceTracks.size(); ++j)
			tracks[j] = new TrackData(*sourceTracks[j]);
	}

	return componentData;
}

}
