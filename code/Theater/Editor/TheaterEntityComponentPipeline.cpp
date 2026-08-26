/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/Editor/TheaterEntityComponentPipeline.h"

#include "Core/Serialization/DeepHash.h"
#include "Editor/IPipelineDepends.h"
#include "Theater/ActData.h"
#include "Theater/TheaterEntityComponentData.h"
#include "Theater/TrackData.h"
#include "World/IEntityEventData.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TheaterEntityComponentPipeline", 0, TheaterEntityComponentPipeline, editor::IPipeline)

bool TheaterEntityComponentPipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
{
	return true;
}

void TheaterEntityComponentPipeline::destroy()
{
}

TypeInfoSet TheaterEntityComponentPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< TheaterEntityComponentData >();
}

bool TheaterEntityComponentPipeline::shouldCache() const
{
	return false;
}

uint32_t TheaterEntityComponentPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool TheaterEntityComponentPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const TheaterEntityComponentData* componentData = checked_type_cast< const TheaterEntityComponentData*, false >(sourceAsset);

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

bool TheaterEntityComponentPipeline::buildOutput(
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

Ref< ISerializable > TheaterEntityComponentPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	const TheaterEntityComponentData* sourceComponentData = checked_type_cast< const TheaterEntityComponentData*, false >(sourceAsset);

	// Keys captured in the editor remember the state of the component they animate.
	Ref< TheaterEntityComponentData > componentData = new TheaterEntityComponentData();
	for (auto act : sourceComponentData->getActs())
		componentData->getActs().push_back(act->stripped());

	return componentData;
}

}
