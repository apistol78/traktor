#include "Database/Database.h"
#include "Editor/IPipelineBuilder.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/Transform.h"
#include "Shape/Editor/MeshModelGenerator.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.MeshModelGenerator", 0, MeshModelGenerator, IModelGenerator)

TypeInfoSet MeshModelGenerator::getSupportedTypes() const
{
    return makeTypeInfoSet< mesh::MeshComponentData >();
}

Ref< model::Model > MeshModelGenerator::createModel(
    editor::IPipelineBuilder* pipelineBuilder,
	const std::wstring& assetPath,
    const Object* source
) const
{
	const mesh::MeshComponentData* meshComponentData = mandatory_non_null_type_cast< const mesh::MeshComponentData* >(source);

	Ref< mesh::MeshAsset > meshAsset = pipelineBuilder->getSourceDatabase()->getObjectReadOnly< mesh::MeshAsset >(
		meshComponentData->getMesh()
	);
	if (!meshAsset)
		return nullptr;

	// \tbd We should probably ignore mesh assets with custom shaders.

	Ref< model::Model > model = model::ModelFormat::readAny(meshAsset->getFileName(), [&](const Path& p) {
		return pipelineBuilder->openFile(Path(assetPath), p.getOriginal());
	});
	if (!model)
		return nullptr;

	model::Transform(scale(meshAsset->getScaleFactor(), meshAsset->getScaleFactor(), meshAsset->getScaleFactor())).apply(*model);
	return model;
}

Ref< Object > MeshModelGenerator::modifyOutput(
    editor::IPipelineBuilder* pipelineBuilder,
	const std::wstring& assetPath,
    const Object* source,
    const Guid& lightmapId,
    const model::Model* model
) const
{
	const mesh::MeshComponentData* meshComponentData = mandatory_non_null_type_cast< const mesh::MeshComponentData* >(source);

	Ref< mesh::MeshAsset > meshAsset = pipelineBuilder->getSourceDatabase()->getObjectReadOnly< mesh::MeshAsset >(
		meshComponentData->getMesh()
	);
	if (!meshAsset)
		return nullptr;

	Guid outputRenderMeshGuid = Guid(meshComponentData->getMesh()).permutation(1);
	std::wstring outputRenderMeshPath = L"Generated/" + outputRenderMeshGuid.format();

	auto materialTextures = meshAsset->getMaterialTextures();
	materialTextures[L"__Illumination__"] = lightmapId;

    Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
    outputMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
    outputMeshAsset->setMaterialTextures(materialTextures);

    pipelineBuilder->buildOutput(
        outputMeshAsset,
        outputRenderMeshPath,
        outputRenderMeshGuid,
        model
    );

	return new mesh::MeshComponentData(resource::Id< mesh::IMesh >(outputRenderMeshGuid));
}

    }
}