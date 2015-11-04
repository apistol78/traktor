#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Terrain/OceanComponent.h"
#include "Terrain/RiverComponent.h"
#include "Terrain/TerrainClassFactory.h"
#include "Terrain/TerrainComponent.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainClassFactory", 0, TerrainClassFactory, IRuntimeClassFactory)

void TerrainClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< OceanComponent > > classOceanComponent = new AutoRuntimeClass< OceanComponent >();
	classOceanComponent->addMethod("setShallowTint", &OceanComponent::setShallowTint);
	classOceanComponent->addMethod("getShallowTint", &OceanComponent::getShallowTint);
	classOceanComponent->addMethod("setReflectionTint", &OceanComponent::setReflectionTint);
	classOceanComponent->addMethod("getReflectionTint", &OceanComponent::getReflectionTint);
	classOceanComponent->addMethod("setDeepColor", &OceanComponent::setDeepColor);
	classOceanComponent->addMethod("getDeepColor", &OceanComponent::getDeepColor);
	classOceanComponent->addMethod("setOpacity", &OceanComponent::setOpacity);
	classOceanComponent->addMethod("getOpacity", &OceanComponent::getOpacity);
	classOceanComponent->addMethod("getMaxAmplitude", &OceanComponent::getMaxAmplitude);
	registrar->registerClass(classOceanComponent);

	Ref< AutoRuntimeClass< RiverComponent > > classRiverComponent = new AutoRuntimeClass< RiverComponent >();
	registrar->registerClass(classRiverComponent);

	Ref< AutoRuntimeClass< TerrainComponent > > classTerrainComponent = new AutoRuntimeClass< TerrainComponent >();
	registrar->registerClass(classTerrainComponent);
}

	}
}
