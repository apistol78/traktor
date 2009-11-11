#include "World/Entity/LightEntityFactory.h"
#include "World/Entity/DirectionalLightEntityData.h"
#include "World/Entity/DirectionalLightEntity.h"
#include "World/Entity/PointLightEntityData.h"
#include "World/Entity/PointLightEntity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.LightEntityFactory", LightEntityFactory, IEntityFactory)

const TypeSet LightEntityFactory::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< DirectionalLightEntityData >());
	typeSet.insert(&type_of< PointLightEntityData >());
	return typeSet;
}

Ref< Entity > LightEntityFactory::createEntity(IEntityBuilder* builder, const std::wstring& name, const EntityData& entityData, const Object* instanceData) const
{
	if (const DirectionalLightEntityData* directionalLightData = dynamic_type_cast< const DirectionalLightEntityData* >(&entityData))
	{
		return gc_new< DirectionalLightEntity >(
			cref(directionalLightData->getTransform()),
			cref(directionalLightData->getSunColor()),
			cref(directionalLightData->getBaseColor()),
			cref(directionalLightData->getShadowColor())
		);
	}
	if (const PointLightEntityData* pointLightData = dynamic_type_cast< const PointLightEntityData* >(&entityData))
	{
		return gc_new< PointLightEntity >(
			cref(pointLightData->getTransform()),
			cref(pointLightData->getSunColor()),
			cref(pointLightData->getBaseColor()),
			cref(pointLightData->getShadowColor()),
			pointLightData->getRange()
		);
	}
	return 0;
}

	}
}
