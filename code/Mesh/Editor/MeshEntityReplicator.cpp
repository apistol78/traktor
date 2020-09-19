#include "Core/Io/FileSystem.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshEntityReplicator.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/Operations/Transform.h"

namespace traktor
{
    namespace mesh
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshEntityReplicator", 0, MeshEntityReplicator, scene::IEntityReplicator)

bool MeshEntityReplicator::create(const editor::IPipelineSettings* settings)
{
	m_modelCachePath = settings->getProperty< std::wstring >(L"Pipeline.ModelCachePath", L"");
	return true;
}

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
	model::ModelCache modelCache(
		m_modelCachePath,
		[&](const Path& p) {
			return pipelineBuilder->getFile(p);
		},
		[&](const Path& p) {
			return pipelineBuilder->openFile(p);
		}
	);

	Ref< model::Model > model = modelCache.get(filePath, meshAsset->getImportFilter());
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
	outputMeshAsset->setTextureSet(meshAsset->getTextureSet());

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