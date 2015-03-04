#include "Amalgam/Engine/Classes/TerrainClasses.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptManager.h"
#include "Terrain/OceanEntity.h"
#include "Terrain/RiverEntity.h"
#include "Terrain/TerrainEntity.h"

namespace traktor
{
	namespace amalgam
	{

void registerTerrainClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< terrain::OceanEntity > > classOceanEntity = new script::AutoScriptClass< terrain::OceanEntity >();
	classOceanEntity->addMethod("setShallowTint", &terrain::OceanEntity::setShallowTint);
	classOceanEntity->addMethod("getShallowTint", &terrain::OceanEntity::getShallowTint);
	classOceanEntity->addMethod("setReflectionTint", &terrain::OceanEntity::setReflectionTint);
	classOceanEntity->addMethod("getReflectionTint", &terrain::OceanEntity::getReflectionTint);
	classOceanEntity->addMethod("setDeepColor", &terrain::OceanEntity::setDeepColor);
	classOceanEntity->addMethod("getDeepColor", &terrain::OceanEntity::getDeepColor);
	classOceanEntity->addMethod("setOpacity", &terrain::OceanEntity::setOpacity);
	classOceanEntity->addMethod("getOpacity", &terrain::OceanEntity::getOpacity);
	classOceanEntity->addMethod("getMaxAmplitude", &terrain::OceanEntity::getMaxAmplitude);
	scriptManager->registerClass(classOceanEntity);

	Ref< script::AutoScriptClass< terrain::RiverEntity > > classRiverEntity = new script::AutoScriptClass< terrain::RiverEntity >();
	scriptManager->registerClass(classRiverEntity);

	Ref< script::AutoScriptClass< terrain::TerrainEntity > > classTerrainEntity = new script::AutoScriptClass< terrain::TerrainEntity >();
	scriptManager->registerClass(classTerrainEntity);
}

	}
}
