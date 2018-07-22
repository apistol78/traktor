/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
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

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainClassFactory", 0, TerrainClassFactory, IRuntimeClassFactory)

void TerrainClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< OceanComponent > > classOceanComponent = new AutoRuntimeClass< OceanComponent >();
	classOceanComponent->addProperty("shallowTint", &OceanComponent::setShallowTint, &OceanComponent::getShallowTint);
	classOceanComponent->addProperty("reflectionTint", &OceanComponent::setReflectionTint, &OceanComponent::getReflectionTint);
	classOceanComponent->addProperty("shadowTint", &OceanComponent::setShadowTint, &OceanComponent::getShadowTint);
	classOceanComponent->addProperty("deepColor", &OceanComponent::setDeepColor, &OceanComponent::getDeepColor);
	classOceanComponent->addProperty("opacity", &OceanComponent::setOpacity, &OceanComponent::getOpacity);
	classOceanComponent->addProperty("maxAmplitude", &OceanComponent::getMaxAmplitude);
	registrar->registerClass(classOceanComponent);

	Ref< AutoRuntimeClass< RiverComponent > > classRiverComponent = new AutoRuntimeClass< RiverComponent >();
	registrar->registerClass(classRiverComponent);

	Ref< AutoRuntimeClass< Terrain > > classTerrain = new AutoRuntimeClass< Terrain >();
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

	Ref< AutoRuntimeClass< TerrainComponent > > classTerrainComponent = new AutoRuntimeClass< TerrainComponent >();
	classTerrainComponent->addProperty("terrain", &TerrainComponent::getTerrain);
	classTerrainComponent->addProperty("surfaceCache", &TerrainComponent::getSurfaceCache);
	classTerrainComponent->addProperty("patchCount", &TerrainComponent::getPatchCount);
	registrar->registerClass(classTerrainComponent);
}

	}
}
