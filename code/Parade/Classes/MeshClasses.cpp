#include "Mesh/MeshEntity.h"
#include "Parade/Classes/MeshClasses.h"
#include "Script/AutoScriptClass.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace parade
	{

void registerMeshClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< mesh::MeshEntity > > classMeshEntity = new script::AutoScriptClass< mesh::MeshEntity >();
	classMeshEntity->addMethod(L"setUserParameter", &mesh::MeshEntity::setUserParameter);
	classMeshEntity->addMethod(L"getUserParameter", &mesh::MeshEntity::getUserParameter);
	scriptManager->registerClass(classMeshEntity);
}

	}
}
