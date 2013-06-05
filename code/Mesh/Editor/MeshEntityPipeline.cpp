#include "Editor/IPipelineDepends.h"
#include "Mesh/MeshEntityData.h"
#include "Mesh/Editor/MeshEntityPipeline.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshEntityPipeline", 0, MeshEntityPipeline, world::EntityPipeline)

TypeInfoSet MeshEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MeshEntityData >());
	return typeSet;
}

bool MeshEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const MeshEntityData* meshEntityData = dynamic_type_cast< const MeshEntityData* >(sourceAsset))
		pipelineDepends->addDependency(meshEntityData->getMesh(), editor::PdfBuild | editor::PdfResource);

	return true;
}

	}
}
