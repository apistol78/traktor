/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Editor/IPipelineDepends.h"
#include "Spray/EffectComponentData.h"
#include "Spray/SoundComponentData.h"
#include "Spray/SoundEventData.h"
#include "Spray/SpawnEffectEventData.h"
#include "Spray/Editor/EffectEntityPipeline.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EffectEntityPipeline", 1, EffectEntityPipeline, world::EntityPipeline)

TypeInfoSet EffectEntityPipeline::getAssetTypes() const
{
	return makeTypeInfoSet<
		EffectComponentData,
		SoundComponentData,
		SoundEventData,
		SpawnEffectEventData
	>();
}

bool EffectEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (auto effectComponentData = dynamic_type_cast< const EffectComponentData* >(sourceAsset))
		pipelineDepends->addDependency(effectComponentData->getEffect(), editor::PdfBuild | editor::PdfResource);
	else if (auto soundComponentData = dynamic_type_cast< const SoundComponentData* >(sourceAsset))
		pipelineDepends->addDependency(soundComponentData->getSound(), editor::PdfBuild | editor::PdfResource);
	else if (auto soundEventData = dynamic_type_cast< const SoundEventData* >(sourceAsset))
		pipelineDepends->addDependency(soundEventData->m_sound, editor::PdfBuild | editor::PdfResource);
	else if (auto spawnEventData = dynamic_type_cast< const SpawnEffectEventData* >(sourceAsset))
		pipelineDepends->addDependency(spawnEventData->getEffect(), editor::PdfBuild | editor::PdfResource);

	return true;
}

Ref< ISerializable > EffectEntityPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	if (auto soundEventData = dynamic_type_cast< const SoundEventData* >(sourceAsset))
	{
		if (soundEventData->m_sound.isNull())
			return nullptr;
	}
	else if (auto spawnEventData = dynamic_type_cast< const SpawnEffectEventData* >(sourceAsset))
	{
		if (spawnEventData->getEffect().isNull())
			return nullptr;
	}

	return world::EntityPipeline::buildProduct(pipelineBuilder, sourceInstance, sourceAsset, buildParams);
}

}
