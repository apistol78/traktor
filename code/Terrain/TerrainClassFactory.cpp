#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedColor4f.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Heightfield/Heightfield.h"
#include "Render/ISimpleTexture.h"
#include "Resource/ResourceCast.h"
#include "Terrain/OceanComponent.h"
#include "Terrain/RiverComponent.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainClassFactory.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainSurfaceCache.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainClassFactory", 0, TerrainClassFactory, IRuntimeClassFactory)

void TerrainClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classOceanComponent = new AutoRuntimeClass< OceanComponent >();
	classOceanComponent->addProperty("shallowTint", &OceanComponent::setShallowTint, &OceanComponent::getShallowTint);
	classOceanComponent->addProperty("reflectionTint", &OceanComponent::setReflectionTint, &OceanComponent::getReflectionTint);
	classOceanComponent->addProperty("shadowTint", &OceanComponent::setShadowTint, &OceanComponent::getShadowTint);
	classOceanComponent->addProperty("deepColor", &OceanComponent::setDeepColor, &OceanComponent::getDeepColor);
	classOceanComponent->addProperty("opacity", &OceanComponent::setOpacity, &OceanComponent::getOpacity);
	classOceanComponent->addProperty("maxAmplitude", &OceanComponent::getMaxAmplitude);
	registrar->registerClass(classOceanComponent);

	auto classRiverComponent = new AutoRuntimeClass< RiverComponent >();
	registrar->registerClass(classRiverComponent);

	auto classTerrain = new AutoRuntimeClass< Terrain >();
	classTerrain->addProperty("detailSkip", &Terrain::getDetailSkip);
	classTerrain->addProperty("patchDim", &Terrain::getPatchDim);
	classTerrain->addProperty("heightfield", &Terrain::getHeightfield);
	classTerrain->addProperty("colorMap", &Terrain::getColorMap);
	classTerrain->addProperty("normalMap", &Terrain::getNormalMap);
	classTerrain->addProperty("heightMap", &Terrain::getHeightMap);
	classTerrain->addProperty("splatMap", &Terrain::getSplatMap);
	classTerrain->addProperty("cutMap", &Terrain::getCutMap);
	classTerrain->addProperty("materialMap", &Terrain::getMaterialMap);
	registrar->registerClass(classTerrain);

	auto classTerrainComponent = new AutoRuntimeClass< TerrainComponent >();
	classTerrainComponent->addProperty("terrain", &TerrainComponent::getTerrain);
	classTerrainComponent->addProperty("patchCount", &TerrainComponent::getPatchCount);
	classTerrainComponent->addMethod("getSurfaceCache", &TerrainComponent::getSurfaceCache);
	registrar->registerClass(classTerrainComponent);
}

}
