#include "Core/Serialization/DeepClone.h"
#include "Editor/IPipelineDepends.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshComponentPipeline.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshComponentPipeline", 0, MeshComponentPipeline, editor::IPipeline)

bool MeshComponentPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void MeshComponentPipeline::destroy()
{
}

TypeInfoSet MeshComponentPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MeshComponentData >());
	return typeSet;
}

bool MeshComponentPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const MeshComponentData* meshComponentData = dynamic_type_cast< const MeshComponentData* >(sourceAsset))
		pipelineDepends->addDependency(meshComponentData->getMesh(), editor::PdfBuild | editor::PdfResource);
	return true;
}

bool MeshComponentPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	T_FATAL_ERROR;
	return false;
}

Ref< ISerializable > MeshComponentPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	return DeepClone(sourceAsset).create();
}

	}
}
