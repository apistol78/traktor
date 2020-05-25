#include "Core/Serialization/DeepClone.h"
#include "Editor/IPipelineDepends.h"
#include "World/Editor/WorldEntityPipeline.h"
#include "World/Entity/ScriptComponentData.h"
#include "World/Entity/DecalComponentData.h"
#include "World/Entity/DecalEventData.h"
#include "World/Entity/EventSetComponentData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/FacadeComponentData.h"
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
	typeSet.insert< FacadeComponentData >();
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
	else if (auto facadeComponentData = dynamic_type_cast< const FacadeComponentData* >(sourceAsset))
	{
		for (auto entityData : facadeComponentData->getEntityData())
			pipelineDepends->addDependency(entityData);
	}
	else if (auto groupComponentData = dynamic_type_cast< const GroupComponentData* >(sourceAsset))
	{
		for (auto entityData : groupComponentData->getEntityData())
			pipelineDepends->addDependency(entityData);
	}
	else if (auto groupEntityData = dynamic_type_cast< const GroupEntityData* >(sourceAsset))
	{
		for (auto entityData : groupEntityData->getEntityData())
			pipelineDepends->addDependency(entityData);
	}
	else if (auto probeComponentData = dynamic_type_cast<const ProbeComponentData*>(sourceAsset))
		pipelineDepends->addDependency(probeComponentData->getTexture(), editor::PdfBuild | editor::PdfResource);
	return true;
}

	}
}
