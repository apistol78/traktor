#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "World/IEntityBuilder.h"
#include "World/Entity/DecalEntity.h"
#include "World/Entity/DecalEntityData.h"
#include "World/Entity/DecalEvent.h"
#include "World/Entity/DecalEventData.h"
#include "World/Entity/DirectionalLightEntity.h"
#include "World/Entity/DirectionalLightEntityData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/GodRayEntity.h"
#include "World/Entity/GodRayEntityData.h"
#include "World/Entity/GroupEntity.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/NullEntity.h"
#include "World/Entity/NullEntityData.h"
#include "World/Entity/PointLightEntity.h"
#include "World/Entity/PointLightEntityData.h"
#include "World/Entity/SpotLightEntity.h"
#include "World/Entity/SpotLightEntityData.h"
#include "World/Entity/VolumeEntity.h"
#include "World/Entity/VolumeEntityData.h"
#include "World/Entity/WorldEntityFactory.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldEntityFactory", WorldEntityFactory, IEntityFactory)

WorldEntityFactory::WorldEntityFactory(resource::IResourceManager* resourceManager)
:	m_resourceManager(resourceManager)
{
}

const TypeInfoSet WorldEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ExternalEntityData >());
	typeSet.insert(&type_of< GodRayEntityData >());
	typeSet.insert(&type_of< GroupEntityData >());
	typeSet.insert(&type_of< DecalEntityData >());
	typeSet.insert(&type_of< DirectionalLightEntityData >());
	typeSet.insert(&type_of< NullEntityData >());
	typeSet.insert(&type_of< PointLightEntityData >());
	typeSet.insert(&type_of< SpotLightEntityData >());
	typeSet.insert(&type_of< VolumeEntityData >());
	return typeSet;
}

const TypeInfoSet WorldEntityFactory::getEntityEventTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< DecalEventData >());
	return typeSet;
}

Ref< Entity > WorldEntityFactory::createEntity(const IEntityBuilder* builder, const EntityData& entityData) const
{
	if (const ExternalEntityData* externalEntityData = dynamic_type_cast< const ExternalEntityData* >(&entityData))
	{
		resource::Proxy< EntityData > resolvedEntityData;
		if (!m_resourceManager->bind(externalEntityData->getEntityData(), resolvedEntityData))
			return 0;

		resolvedEntityData->setName(externalEntityData->getName());
		resolvedEntityData->setTransform(externalEntityData->getTransform());

		return builder->create(resolvedEntityData);
	}

	if (const GodRayEntityData* godRayData = dynamic_type_cast< const GodRayEntityData* >(&entityData))
	{
		return new GodRayEntity(godRayData->getTransform());
	}

	if (const GroupEntityData* groupData = dynamic_type_cast< const GroupEntityData* >(&entityData))
	{
		Ref< GroupEntity > groupEntity = new GroupEntity(groupData->getTransform());

		const RefArray< EntityData >& entityData = groupData->getEntityData();
		for (RefArray< EntityData >::const_iterator i = entityData.begin(); i != entityData.end(); ++i)
		{
			Ref< Entity > childEntity = builder->create(*i);
			if (childEntity)
				groupEntity->addEntity(childEntity);
		}

		return groupEntity;
	}

	if (const DecalEntityData* decalData = dynamic_type_cast< const DecalEntityData* >(&entityData))
	{
		resource::Proxy< render::Shader > shader;
		if (!m_resourceManager->bind(decalData->getShader(), shader))
			return 0;

		Ref< DecalEntity > decalEntity = new DecalEntity(
			decalData->getTransform(),
			decalData->getSize(),
			decalData->getThickness(),
			decalData->getAlpha(),
			decalData->getCullDistance(),
			shader
		);

		return decalEntity;
	}

	if (const DirectionalLightEntityData* directionalLightData = dynamic_type_cast< const DirectionalLightEntityData* >(&entityData))
	{
		return new DirectionalLightEntity(
			directionalLightData->getTransform(),
			directionalLightData->getSunColor(),
			directionalLightData->getBaseColor(),
			directionalLightData->getShadowColor(),
			directionalLightData->getCastShadow()
		);
	}

	if (const PointLightEntityData* pointLightData = dynamic_type_cast< const PointLightEntityData* >(&entityData))
	{
		return new PointLightEntity(
			pointLightData->getTransform(),
			pointLightData->getSunColor(),
			pointLightData->getBaseColor(),
			pointLightData->getShadowColor(),
			pointLightData->getRange(),
			pointLightData->getRandomFlickerAmount(),
			pointLightData->getRandomFlickerFilter()
		);
	}

	if (const SpotLightEntityData* spotLightData = dynamic_type_cast< const SpotLightEntityData* >(&entityData))
	{
		return new SpotLightEntity(
			spotLightData->getTransform(),
			spotLightData->getSunColor(),
			spotLightData->getBaseColor(),
			spotLightData->getShadowColor(),
			spotLightData->getRange(),
			spotLightData->getRadius(),
			spotLightData->getCastShadow()
		);
	}

	if (const NullEntityData* nullData = dynamic_type_cast< const NullEntityData* >(&entityData))
	{
		return new NullEntity(nullData->getTransform());
	}

	if (const VolumeEntityData* volumeData = dynamic_type_cast< const VolumeEntityData* >(&entityData))
	{
		return new VolumeEntity(volumeData);
	}

	return 0;
}

Ref< IEntityEvent > WorldEntityFactory::createEntityEvent(const IEntityBuilder* builder, const IEntityEventData& entityEventData) const
{
	if (const DecalEventData* decalData = dynamic_type_cast< const DecalEventData* >(&entityEventData))
	{
		Ref< DecalEvent > decal = new DecalEvent();

		decal->m_size = decalData->getSize();
		decal->m_thickness = decalData->getThickness();
		decal->m_alpha = decalData->getAlpha();
		decal->m_cullDistance = decalData->getCullDistance();

		if (m_resourceManager->bind(decalData->getShader(), decal->m_shader))
			return decal;
	}
	return 0;
}

	}
}
