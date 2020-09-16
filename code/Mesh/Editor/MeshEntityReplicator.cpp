#include "Core/Io/FileSystem.h"
#include "Database/Database.h"
#include "Editor/IPipelineBuilder.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshEntityReplicator.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/Transform.h"

namespace traktor
{
    namespace mesh
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshEntityReplicator", 0, MeshEntityReplicator, scene::IEntityReplicator)

TypeInfoSet MeshEntityReplicator::getSupportedTypes() const
{
    return makeTypeInfoSet< MeshComponentData >();
}

Ref< model::Model > MeshEntityReplicator::createModel(
    editor::IPipelineBuilder* pipelineBuilder,
	const std::wstring& assetPath,
    const Object* source
) const
{
	const MeshComponentData* meshComponentData = mandatory_non_null_type_cast< const MeshComponentData* >(source);

	Ref< MeshAsset > meshAsset = pipelineBuilder->getSourceDatabase()->getObjectReadOnly< MeshAsset >(
		meshComponentData->getMesh()
	);
	if (!meshAsset)
		return nullptr;

	Path filePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath) + meshAsset->getFileName());
	Ref< model::Model > model = model::ModelFormat::readAny(filePath, meshAsset->getImportFilter(), [&](const Path& p) {
		return pipelineBuilder->openFile(p);
	});
	if (!model)
		return nullptr;

	model::Transform(scale(meshAsset->getScaleFactor(), meshAsset->getScaleFactor(), meshAsset->getScaleFactor())).apply(*model);
	return model;
}

Ref< Object > MeshEntityReplicator::modifyOutput(
    editor::IPipelineBuilder* pipelineBuilder,
	const std::wstring& assetPath,
    const Object* source,
    const model::Model* model
) const
{
	const MeshComponentData* meshComponentData = mandatory_non_null_type_cast< const MeshComponentData* >(source);

	Ref< MeshAsset > meshAsset = pipelineBuilder->getSourceDatabase()->getObjectReadOnly< MeshAsset >(
		meshComponentData->getMesh()
	);
	if (!meshAsset)
		return nullptr;

	Guid outputRenderMeshGuid = pipelineBuilder->synthesizeOutputGuid(1);
	std::wstring outputRenderMeshPath = L"Generated/" + outputRenderMeshGuid.format();

    Ref< MeshAsset > outputMeshAsset = new MeshAsset();
    outputMeshAsset->setMeshType(meshAsset->getMeshType());
	outputMeshAsset->setMaterialTextures(meshAsset->getMaterialTextures());

    pipelineBuilder->buildOutput(
		nullptr,
        outputMeshAsset,
        outputRenderMeshPath,
        outputRenderMeshGuid,
        model
    );

	return new MeshComponentData(resource::Id< IMesh >(outputRenderMeshGuid));
}

    }
}