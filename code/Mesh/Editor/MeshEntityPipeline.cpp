#include "Editor/IPipelineDepends.h"
#include "Mesh/Editor/MeshEntityPipeline.h"
#include "Mesh/Blend/BlendMeshEntityData.h"
#include "Mesh/Composite/CompositeMeshEntityData.h"
#include "Mesh/Indoor/IndoorMeshEntityData.h"
#include "Mesh/Instance/InstanceMeshEntityData.h"
#include "Mesh/Skinned/SkinnedMeshEntityData.h"
#include "Mesh/Static/StaticMeshEntityData.h"
#include "Mesh/Stream/StreamMeshEntityData.h"
#include "World/Entity/EntityInstance.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshEntityPipeline", 0, MeshEntityPipeline, world::EntityPipeline)

TypeInfoSet MeshEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< BlendMeshEntityData >());
	typeSet.insert(&type_of< CompositeMeshEntityData >());
	typeSet.insert(&type_of< IndoorMeshEntityData >());
	typeSet.insert(&type_of< InstanceMeshEntityData >());
	typeSet.insert(&type_of< SkinnedMeshEntityData >());
	typeSet.insert(&type_of< StaticMeshEntityData >());
	typeSet.insert(&type_of< StreamMeshEntityData >());
	return typeSet;
}

bool MeshEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const BlendMeshEntityData* blendMeshEntityData = dynamic_type_cast< const BlendMeshEntityData* >(sourceAsset))
		pipelineDepends->addDependency(blendMeshEntityData->getMesh().getGuid(), editor::PdfBuild);
	if (const CompositeMeshEntityData* compositeMeshEntityData = dynamic_type_cast< const CompositeMeshEntityData* >(sourceAsset))
	{
		const RefArray< world::EntityInstance >& instances = compositeMeshEntityData->getInstances();
		for (RefArray< world::EntityInstance >::const_iterator i = instances.begin(); i != instances.end(); ++i)
			pipelineDepends->addDependency(*i);
	}
	if (const IndoorMeshEntityData* indoorMeshEntityData = dynamic_type_cast< const IndoorMeshEntityData* >(sourceAsset))
		pipelineDepends->addDependency(indoorMeshEntityData->getMesh().getGuid(), editor::PdfBuild);
	if (const InstanceMeshEntityData* instanceMeshEntityData = dynamic_type_cast< const InstanceMeshEntityData* >(sourceAsset))
		pipelineDepends->addDependency(instanceMeshEntityData->getMesh().getGuid(), editor::PdfBuild);
	if (const SkinnedMeshEntityData* skinnedMeshEntityData = dynamic_type_cast< const SkinnedMeshEntityData* >(sourceAsset))
		pipelineDepends->addDependency(skinnedMeshEntityData->getMesh().getGuid(), editor::PdfBuild);
	if (const StaticMeshEntityData* staticMeshEntityData = dynamic_type_cast< const StaticMeshEntityData* >(sourceAsset))
		pipelineDepends->addDependency(staticMeshEntityData->getMesh().getGuid(), editor::PdfBuild);
	if (const StreamMeshEntityData* streamMeshEntityData = dynamic_type_cast< const StreamMeshEntityData* >(sourceAsset))
		pipelineDepends->addDependency(streamMeshEntityData->getMesh().getGuid(), editor::PdfBuild);

	return true;
}

	}
}
