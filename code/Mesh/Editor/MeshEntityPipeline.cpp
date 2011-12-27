#include "Editor/IPipelineDepends.h"
#include "Mesh/MeshEntityData.h"
#include "Mesh/Composite/CompositeMeshEntityData.h"
#include "Mesh/Editor/MeshEntityPipeline.h"
#include "Mesh/Lod/LodMeshEntityData.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshEntityPipeline", 0, MeshEntityPipeline, world::EntityPipeline)

TypeInfoSet MeshEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MeshEntityData >());
	typeSet.insert(&type_of< CompositeMeshEntityData >());
	typeSet.insert(&type_of< LodMeshEntityData >());
	return typeSet;
}

bool MeshEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const MeshEntityData* meshEntityData = dynamic_type_cast< const MeshEntityData* >(sourceAsset))
		pipelineDepends->addDependency(meshEntityData->getMesh().getGuid(), editor::PdfBuild);
	else if (const CompositeMeshEntityData* compositeMeshEntityData = dynamic_type_cast< const CompositeMeshEntityData* >(sourceAsset))
	{
		const RefArray< AbstractMeshEntityData >& entityData = compositeMeshEntityData->getEntityData();
		for (RefArray< AbstractMeshEntityData >::const_iterator i = entityData.begin(); i != entityData.end(); ++i)
			pipelineDepends->addDependency(*i);
	}
	else if (const LodMeshEntityData* lodMeshEntityData = dynamic_type_cast< const LodMeshEntityData* >(sourceAsset))
	{
		const RefArray< AbstractMeshEntityData >& lods = lodMeshEntityData->getLods();
		for (RefArray< AbstractMeshEntityData >::const_iterator i = lods.begin(); i != lods.end(); ++i)
			pipelineDepends->addDependency(*i);
	}

	return true;
}

	}
}
