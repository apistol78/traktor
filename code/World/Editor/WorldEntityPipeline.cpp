#include "Core/Serialization/DeepClone.h"
#include "Editor/IPipelineDepends.h"
#include "World/Editor/WorldEntityPipeline.h"
#include "World/Entity/ScriptComponentData.h"
#include "World/Entity/DecalComponentData.h"
#include "World/Entity/DecalEventData.h"
#include "World/Entity/EventSetComponentData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/GroupComponentData.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/LightComponentData.h"
#include "World/Entity/ProbeComponentData.h"

namespace traktor
{
	namespace world
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
	typeSet.insert< GroupEntityData >();
	typeSet.insert< LightComponentData >();
	typeSet.insert< ProbeComponentData >();
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
	if (const ScriptComponentData* scriptComponentData = dynamic_type_cast< const ScriptComponentData* >(sourceAsset))
		pipelineDepends->addDependency(scriptComponentData->getRuntimeClass(), editor::PdfBuild);
	else if (const DecalComponentData* decalComponentData = dynamic_type_cast< const DecalComponentData* >(sourceAsset))
		pipelineDepends->addDependency(decalComponentData->getShader(), editor::PdfBuild | editor::PdfResource);
	else if (const DecalEventData* decalEventData = dynamic_type_cast< const DecalEventData* >(sourceAsset))
		pipelineDepends->addDependency(decalEventData->getShader(), editor::PdfBuild | editor::PdfResource);
	else if (const EventSetComponentData* eventSetComponentData = dynamic_type_cast< const EventSetComponentData* >(sourceAsset))
	{
		for (auto eventData : eventSetComponentData->m_eventData)
			pipelineDepends->addDependency(eventData.second);
	}
	else if (const ExternalEntityData* externalEntityData = dynamic_type_cast< const ExternalEntityData* >(sourceAsset))
		pipelineDepends->addDependency(externalEntityData->getEntityData(), editor::PdfBuild);
	else if (const GroupComponentData* groupComponentData = dynamic_type_cast< const GroupComponentData* >(sourceAsset))
	{
		for (auto entityData : groupComponentData->getEntityData())
			pipelineDepends->addDependency(entityData);
	}
	else if (const GroupEntityData* groupEntityData = dynamic_type_cast< const GroupEntityData* >(sourceAsset))
	{
		for (auto entityData : groupEntityData->getEntityData())
			pipelineDepends->addDependency(entityData);
	}
	else if (const ProbeComponentData* probeComponentData = dynamic_type_cast<const ProbeComponentData*>(sourceAsset))
		pipelineDepends->addDependency(probeComponentData->getTexture(), editor::PdfBuild | editor::PdfResource);
	return true;
}

	}
}
