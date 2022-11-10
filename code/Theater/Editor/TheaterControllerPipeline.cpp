/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepHash.h"
#include "Editor/IPipelineBuilder.h"
#include "Theater/ActData.h"
#include "Theater/TheaterControllerData.h"
#include "Theater/TrackData.h"
#include "Theater/Editor/TheaterControllerPipeline.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TheaterControllerPipeline", 2, TheaterControllerPipeline, editor::IPipeline)

bool TheaterControllerPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void TheaterControllerPipeline::destroy()
{
}

TypeInfoSet TheaterControllerPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< TheaterControllerData >();
}

bool TheaterControllerPipeline::shouldCache() const
{
	return false;
}

uint32_t TheaterControllerPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool TheaterControllerPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	return true;
}

bool TheaterControllerPipeline::buildOutput(
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

Ref< ISerializable > TheaterControllerPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	const TheaterControllerData* sourceControllerData = checked_type_cast< const TheaterControllerData*, false >(sourceAsset);
	const RefArray< ActData >& sourceActs = sourceControllerData->getActs();

	Ref< TheaterControllerData > controllerData = new TheaterControllerData();
	controllerData->m_repeatActs = sourceControllerData->m_repeatActs;
	controllerData->m_randomizeActs = sourceControllerData->m_randomizeActs;

	RefArray< ActData >& acts = controllerData->getActs();
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

	return controllerData;
}

	}
}
