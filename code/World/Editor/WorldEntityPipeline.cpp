/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/DeepClone.h"
#include "Editor/IPipelineDepends.h"
#include "World/Editor/WorldEntityPipeline.h"
#include "World/Entity/DecalComponentData.h"
#include "World/Entity/DecalEventData.h"
#include "World/Entity/DirectionalLightEntityData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/LightComponentData.h"
#include "World/Entity/PointLightEntityData.h"
#include "World/Entity/SpotLightEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldEntityPipeline", 0, WorldEntityPipeline, EntityPipeline)

TypeInfoSet WorldEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< DecalComponentData >());
	typeSet.insert(&type_of< DecalEventData >());
	typeSet.insert(&type_of< DirectionalLightEntityData >());
	typeSet.insert(&type_of< ExternalEntityData >());
	typeSet.insert(&type_of< GroupEntityData >());
	typeSet.insert(&type_of< LightComponentData >());
	typeSet.insert(&type_of< PointLightEntityData >());
	typeSet.insert(&type_of< SpotLightEntityData >());
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
	if (const DecalComponentData* decalComponentData = dynamic_type_cast< const DecalComponentData* >(sourceAsset))
		pipelineDepends->addDependency(decalComponentData->getShader(), editor::PdfBuild | editor::PdfResource);
	else if (const DecalEventData* decalEventData = dynamic_type_cast< const DecalEventData* >(sourceAsset))
		pipelineDepends->addDependency(decalEventData->getShader(), editor::PdfBuild | editor::PdfResource);
	else if (const DirectionalLightEntityData* directionalLightEntityData = dynamic_type_cast< const DirectionalLightEntityData* >(sourceAsset))
		pipelineDepends->addDependency(directionalLightEntityData->getCloudShadowTexture(), editor::PdfBuild | editor::PdfResource);
	else if (const ExternalEntityData* externalEntityData = dynamic_type_cast< const ExternalEntityData* >(sourceAsset))
		pipelineDepends->addDependency(externalEntityData->getEntityData(), editor::PdfBuild);
	else if (const GroupEntityData* groupEntityData = dynamic_type_cast< const GroupEntityData* >(sourceAsset))
	{
		const RefArray< EntityData >& entityData = groupEntityData->getEntityData();
		for (RefArray< EntityData >::const_iterator i = entityData.begin(); i != entityData.end(); ++i)
			pipelineDepends->addDependency(*i);
	}
	else if (const LightComponentData* lightComponentData = dynamic_type_cast<const LightComponentData*>(sourceAsset))
	{
		pipelineDepends->addDependency(lightComponentData->getProbeDiffuseTexture(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(lightComponentData->getProbeSpecularTexture(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(lightComponentData->getCloudShadowTexture(), editor::PdfBuild | editor::PdfResource);
	}
	return true;
}

	}
}
