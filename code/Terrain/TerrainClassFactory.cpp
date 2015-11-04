#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Terrain/OceanEntity.h"
#include "Terrain/RiverEntity.h"
#include "Terrain/TerrainClassFactory.h"
#include "Terrain/TerrainComponent.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainClassFactory", 0, TerrainClassFactory, IRuntimeClassFactory)

void TerrainClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< OceanEntity > > classOceanEntity = new AutoRuntimeClass< OceanEntity >();
	classOceanEntity->addMethod("setShallowTint", &OceanEntity::setShallowTint);
	classOceanEntity->addMethod("getShallowTint", &OceanEntity::getShallowTint);
	classOceanEntity->addMethod("setReflectionTint", &OceanEntity::setReflectionTint);
	classOceanEntity->addMethod("getReflectionTint", &OceanEntity::getReflectionTint);
	classOceanEntity->addMethod("setDeepColor", &OceanEntity::setDeepColor);
	classOceanEntity->addMethod("getDeepColor", &OceanEntity::getDeepColor);
	classOceanEntity->addMethod("setOpacity", &OceanEntity::setOpacity);
	classOceanEntity->addMethod("getOpacity", &OceanEntity::getOpacity);
	classOceanEntity->addMethod("getMaxAmplitude", &OceanEntity::getMaxAmplitude);
	registrar->registerClass(classOceanEntity);

	Ref< AutoRuntimeClass< RiverEntity > > classRiverEntity = new AutoRuntimeClass< RiverEntity >();
	registrar->registerClass(classRiverEntity);

	Ref< AutoRuntimeClass< TerrainComponent > > classTerrainComponent = new AutoRuntimeClass< TerrainComponent >();
	registrar->registerClass(classTerrainComponent);
}

	}
}
