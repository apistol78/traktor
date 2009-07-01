#include "Mesh/Editor/MeshEntityPipeline.h"
#include "Mesh/Blend/BlendMeshEntityData.h"
#include "Mesh/Composite/CompositeMeshEntityData.h"
#include "Mesh/Indoor/IndoorMeshEntityData.h"
#include "Mesh/Instance/InstanceMeshEntityData.h"
#include "Mesh/Skinned/SkinnedMeshEntityData.h"
#include "Mesh/Static/StaticMeshEntityData.h"
#include "World/Entity/EntityInstance.h"
#include "Editor/PipelineManager.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.mesh.MeshEntityPipeline", MeshEntityPipeline, world::EntityPipeline)

TypeSet MeshEntityPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< BlendMeshEntityData >());
	typeSet.insert(&type_of< CompositeMeshEntityData >());
	typeSet.insert(&type_of< IndoorMeshEntityData >());
	typeSet.insert(&type_of< InstanceMeshEntityData >());
	typeSet.insert(&type_of< SkinnedMeshEntityData >());
	typeSet.insert(&type_of< StaticMeshEntityData >());
	return typeSet;
}

bool MeshEntityPipeline::buildDependencies(
	editor::PipelineManager* pipelineManager,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const BlendMeshEntityData* blendMeshEntityData = dynamic_type_cast< const BlendMeshEntityData* >(sourceAsset))
		pipelineManager->addDependency(blendMeshEntityData->getMesh().getGuid());
	if (const CompositeMeshEntityData* compositeMeshEntityData = dynamic_type_cast< const CompositeMeshEntityData* >(sourceAsset))
	{
		const RefArray< world::EntityInstance >& instances = compositeMeshEntityData->getInstances();
		for (RefArray< world::EntityInstance >::const_iterator i = instances.begin(); i != instances.end(); ++i)
			pipelineManager->addDependency(*i);
	}
	if (const IndoorMeshEntityData* indoorMeshEntityData = dynamic_type_cast< const IndoorMeshEntityData* >(sourceAsset))
		pipelineManager->addDependency(indoorMeshEntityData->getMesh().getGuid());
	if (const InstanceMeshEntityData* instanceMeshEntityData = dynamic_type_cast< const InstanceMeshEntityData* >(sourceAsset))
		pipelineManager->addDependency(instanceMeshEntityData->getMesh().getGuid());
	if (const SkinnedMeshEntityData* skinnedMeshEntityData = dynamic_type_cast< const SkinnedMeshEntityData* >(sourceAsset))
		pipelineManager->addDependency(skinnedMeshEntityData->getMesh().getGuid());
	if (const StaticMeshEntityData* staticMeshEntityData = dynamic_type_cast< const StaticMeshEntityData* >(sourceAsset))
		pipelineManager->addDependency(staticMeshEntityData->getMesh().getGuid());

	return true;
}

	}
}
