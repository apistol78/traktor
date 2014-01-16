#include "Amalgam/Engine/Classes/MeshClasses.h"
#include "Mesh/MeshEntity.h"
#include "Script/AutoScriptClass.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace amalgam
	{

void registerMeshClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< mesh::MeshEntity > > classMeshEntity = new script::AutoScriptClass< mesh::MeshEntity >();
	scriptManager->registerClass(classMeshEntity);
}

	}
}
