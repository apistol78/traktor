/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Editor/IPipelineDepends.h"
#include "World/Editor/WorldEntityPipeline.h"
#include "World/Entity/ScriptComponentData.h"
#include "World/Entity/DecalComponentData.h"
#include "World/Entity/DecalEventData.h"
#include "World/Entity/EventSetComponentData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/GroupComponentData.h"
#include "World/Entity/ProbeComponentData.h"
#include "World/Entity/VolumetricFogComponentData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldEntityPipeline", 0, WorldEntityPipeline, EntityPipeline)

TypeInfoSet WorldEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< ScriptComponentData >();
	typeSet.insert< DecalComponentData >();
	typeSet.insert< DecalEventData >();
	typeSet.insert< EventSetComponentData >();
	typeSet.insert< ExternalEntityData >();
	typeSet.insert< GroupComponentData >();
	typeSet.insert< ProbeComponentData >();
	typeSet.insert< VolumetricFogComponentData >();
	return typeSet;
}

bool WorldEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (auto scriptComponentData = dynamic_type_cast< const ScriptComponentData* >(sourceAsset))
		pipelineDepends->addDependency(scriptComponentData->getRuntimeClass(), editor::PdfBuild);
	else if (auto decalComponentData = dynamic_type_cast< const DecalComponentData* >(sourceAsset))
		pipelineDepends->addDependency(decalComponentData->getShader(), editor::PdfBuild | editor::PdfResource);
	else if (auto decalEventData = dynamic_type_cast< const DecalEventData* >(sourceAsset))
		pipelineDepends->addDependency(decalEventData->getShader(), editor::PdfBuild | editor::PdfResource);
	else if (auto eventSetComponentData = dynamic_type_cast< const EventSetComponentData* >(sourceAsset))
	{
		for (auto eventData : eventSetComponentData->m_eventData)
			pipelineDepends->addDependency(eventData.second);
	}
	else if (auto externalEntityData = dynamic_type_cast< const ExternalEntityData* >(sourceAsset))
		pipelineDepends->addDependency(externalEntityData->getEntityData(), editor::PdfBuild);
	else if (auto groupComponentData = dynamic_type_cast< const GroupComponentData* >(sourceAsset))
	{
		for (auto entityData : groupComponentData->getEntityData())
			pipelineDepends->addDependency(entityData);
	}
	else if (auto probeComponentData = dynamic_type_cast<const ProbeComponentData*>(sourceAsset))
		pipelineDepends->addDependency(probeComponentData->getTexture(), editor::PdfBuild | editor::PdfResource);
	else if (auto volumetricFogComponentData = dynamic_type_cast< const VolumetricFogComponentData* >(sourceAsset))
		pipelineDepends->addDependency(volumetricFogComponentData->getShader(), editor::PdfBuild | editor::PdfResource);
	else
	{
		log::error << L"Unsupported component data type in world pipeline, \"" << type_name(sourceAsset) << L"\"." << Endl;
		return false;
	}

	return true;
}

}
