/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/Editor/TheaterWorldComponentPipeline.h"

#include "Core/Log/Log.h"
#include "Core/Serialization/DeepHash.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Theater/ActData.h"
#include "Theater/TheaterWorldComponentData.h"
#include "Theater/TrackData.h"
#include "World/EntityData.h"
#include "World/IEntityEventData.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TheaterWorldComponentPipeline", 2, TheaterWorldComponentPipeline, editor::IPipeline)

bool TheaterWorldComponentPipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
{
	return true;
}

void TheaterWorldComponentPipeline::destroy()
{
}

TypeInfoSet TheaterWorldComponentPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< TheaterWorldComponentData >();
}

bool TheaterWorldComponentPipeline::shouldCache() const
{
	return false;
}

uint32_t TheaterWorldComponentPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool TheaterWorldComponentPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const TheaterWorldComponentData* componentData = checked_type_cast< const TheaterWorldComponentData*, false >(sourceAsset);

	// Add dependencies of events issued from tracks.
	for (auto act : componentData->getActs())
	{
		for (auto track : act->getTracks())
		{
			for (const auto& eventKey : track->getEvents())
				pipelineDepends->addDependency(eventKey.event.c_ptr());
		}
	}

	return true;
}

bool TheaterWorldComponentPipeline::buildOutput(
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

Ref< ISerializable > TheaterWorldComponentPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	const TheaterWorldComponentData* sourceComponentData = checked_type_cast< const TheaterWorldComponentData*, false >(sourceAsset);
	const RefArray< ActData >& sourceActs = sourceComponentData->getActs();

	Ref< TheaterWorldComponentData > componentData = new TheaterWorldComponentData();
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
