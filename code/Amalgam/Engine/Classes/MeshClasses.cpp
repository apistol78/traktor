#include "Amalgam/Engine/Classes/MeshClasses.h"
#include "Mesh/MeshEntity.h"
#include "Mesh/Blend/BlendMeshEntity.h"
#include "Mesh/Composite/CompositeMeshEntity.h"
#include "Mesh/Indoor/IndoorMeshEntity.h"
#include "Mesh/Instance/InstanceMeshEntity.h"
#include "Mesh/Lod/LodMeshEntity.h"
#include "Mesh/Partition/PartitionMeshEntity.h"
#include "Mesh/Skinned/SkinnedMeshEntity.h"
#include "Mesh/Static/StaticMeshEntity.h"
#include "Mesh/Stream/StreamMeshEntity.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace amalgam
	{

void registerMeshClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< mesh::MeshEntity > > classMeshEntity = new script::AutoScriptClass< mesh::MeshEntity >();
	scriptManager->registerClass(classMeshEntity);

	Ref< script::AutoScriptClass< mesh::BlendMeshEntity > > classBlendMeshEntity = new script::AutoScriptClass< mesh::BlendMeshEntity >();
	classBlendMeshEntity->addMethod("setBlendWeights", &mesh::BlendMeshEntity::setBlendWeights);
	classBlendMeshEntity->addMethod("getBlendWeights", &mesh::BlendMeshEntity::getBlendWeights);
	scriptManager->registerClass(classBlendMeshEntity);

	Ref< script::AutoScriptClass< mesh::CompositeMeshEntity > > classCompositeMeshEntity = new script::AutoScriptClass< mesh::CompositeMeshEntity >();
	scriptManager->registerClass(classCompositeMeshEntity);

	Ref< script::AutoScriptClass< mesh::IndoorMeshEntity > > classIndoorMeshEntity = new script::AutoScriptClass< mesh::IndoorMeshEntity >();
	scriptManager->registerClass(classIndoorMeshEntity);

	Ref< script::AutoScriptClass< mesh::InstanceMeshEntity > > classInstanceMeshEntity = new script::AutoScriptClass< mesh::InstanceMeshEntity >();
	scriptManager->registerClass(classInstanceMeshEntity);

	Ref< script::AutoScriptClass< mesh::LodMeshEntity > > classLodMeshEntity = new script::AutoScriptClass< mesh::LodMeshEntity >();
	scriptManager->registerClass(classLodMeshEntity);

	Ref< script::AutoScriptClass< mesh::PartitionMeshEntity > > classPartitionMeshEntity = new script::AutoScriptClass< mesh::PartitionMeshEntity >();
	scriptManager->registerClass(classPartitionMeshEntity);

	Ref< script::AutoScriptClass< mesh::SkinnedMeshEntity > > classSkinnedMeshEntity = new script::AutoScriptClass< mesh::SkinnedMeshEntity >();
	scriptManager->registerClass(classSkinnedMeshEntity);

	Ref< script::AutoScriptClass< mesh::StaticMeshEntity > > classStaticMeshEntity = new script::AutoScriptClass< mesh::StaticMeshEntity >();
	scriptManager->registerClass(classStaticMeshEntity);

	Ref< script::AutoScriptClass< mesh::StreamMeshEntity > > classStreamMeshEntity = new script::AutoScriptClass< mesh::StreamMeshEntity >();
	classStreamMeshEntity->addMethod("getFrameCount", &mesh::StreamMeshEntity::getFrameCount);
	classStreamMeshEntity->addMethod("setFrame", &mesh::StreamMeshEntity::setFrame);
	scriptManager->registerClass(classStreamMeshEntity);
}

	}
}
