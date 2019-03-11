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
#include "World/Entity/ProbeComponentData.h"
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
	typeSet.insert(&type_of< ProbeComponentData >());
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
	else if (const ExternalEntityData* externalEntityData = dynamic_type_cast< const ExternalEntityData* >(sourceAsset))
		pipelineDepends->addDependency(externalEntityData->getEntityData(), editor::PdfBuild);
	else if (const GroupEntityData* groupEntityData = dynamic_type_cast< const GroupEntityData* >(sourceAsset))
	{
		for (auto entityData : groupEntityData->getEntityData())
			pipelineDepends->addDependency(entityData);
	}
	else if (const LightComponentData* lightComponentData = dynamic_type_cast<const LightComponentData*>(sourceAsset))
	{
		pipelineDepends->addDependency(lightComponentData->getProbeDiffuseTexture(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(lightComponentData->getProbeSpecularTexture(), editor::PdfBuild | editor::PdfResource);
	}
	else if (const ProbeComponentData* probeComponentData = dynamic_type_cast<const ProbeComponentData*>(sourceAsset))
	{
		pipelineDepends->addDependency(probeComponentData->getProbeDiffuseTexture(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(probeComponentData->getProbeSpecularTexture(), editor::PdfBuild | editor::PdfResource);
	}
	return true;
}

	}
}
