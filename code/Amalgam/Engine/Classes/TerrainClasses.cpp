#include "Amalgam/Engine/Classes/TerrainClasses.h"
#include "Script/AutoScriptClass.h"
#include "Script/IScriptManager.h"
#include "Terrain/OceanEntity.h"
#include "Terrain/RiverEntity.h"
#include "Terrain/TerrainEntity.h"
#include "Terrain/UndergrowthEntity.h"

namespace traktor
{
	namespace amalgam
	{

void registerTerrainClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< terrain::OceanEntity > > classOceanEntity = new script::AutoScriptClass< terrain::OceanEntity >();
	scriptManager->registerClass(classOceanEntity);

	Ref< script::AutoScriptClass< terrain::RiverEntity > > classRiverEntity = new script::AutoScriptClass< terrain::RiverEntity >();
	scriptManager->registerClass(classRiverEntity);

	Ref< script::AutoScriptClass< terrain::TerrainEntity > > classTerrainEntity = new script::AutoScriptClass< terrain::TerrainEntity >();
	scriptManager->registerClass(classTerrainEntity);

	Ref< script::AutoScriptClass< terrain::UndergrowthEntity > > classUndergrowthEntity = new script::AutoScriptClass< terrain::UndergrowthEntity >();
	scriptManager->registerClass(classUndergrowthEntity);
}

	}
}
