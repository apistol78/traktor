#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/MergeModel.h"
#include "Model/Operations/Transform.h"
#include "Render/Editor/Texture/TextureSet.h"
#include "Scene/Editor/Traverser.h"
#include "Shape/Editor/Prefab/PrefabComponentData.h"
#include "Shape/Editor/Prefab/PrefabEntityReplicator.h"
#include "World/EntityData.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.PrefabEntityReplicator", 0, PrefabEntityReplicator, scene::IEntityReplicator)

bool PrefabEntityReplicator::create(const editor::IPipelineSettings* settings)
{
	m_modelCachePath = settings->getProperty< std::wstring >(L"Pipeline.ModelCache.Path");
    return true;
}

TypeInfoSet PrefabEntityReplicator::getSupportedTypes() const
{
    return makeTypeInfoSet< PrefabComponentData >();
}

bool PrefabEntityReplicator::addDependencies(
    editor::IPipelineDepends* pipelineDepends,
    const world::EntityData* entityData,
    const world::IEntityComponentData* componentData
) const
{
	const PrefabComponentData* prefabComponentData = mandatory_non_null_type_cast< const PrefabComponentData* >(componentData);
    scene::Traverser::visit(prefabComponentData, [&](const world::EntityData* entityData) -> scene::Traverser::VisitorResult
    {
        if (auto meshComponentData = entityData->getComponent< mesh::MeshComponentData >())
            pipelineDepends->addDependency(meshComponentData->getMesh(), editor::PdfUse);
        return scene::Traverser::VrContinue;
    });
	return true;
}

Ref< model::Model > PrefabEntityReplicator::createModel(
    editor::IPipelineBuilder* pipelineBuilder,
    const std::wstring& assetPath,
    const world::EntityData* entityData,
    const world::IEntityComponentData* componentData
) const
{
	const PrefabComponentData* prefabComponentData = mandatory_non_null_type_cast< const PrefabComponentData* >(componentData);
    Transform worldInv = entityData->getTransform().inverse();

    // Collect all models from prefab component.
    RefArray< model::Model > models;
    scene::Traverser::visit(prefabComponentData, [&](const world::EntityData* entityData) -> scene::Traverser::VisitorResult
    {
        if (auto meshComponentData = entityData->getComponent< mesh::MeshComponentData >())
        {
            Ref< const mesh::MeshAsset > meshAsset = pipelineBuilder->getObjectReadOnly< mesh::MeshAsset >(
                meshComponentData->getMesh()
            );
            if (!meshAsset)
                return scene::Traverser::VrFailed;

	        Path filePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath) + meshAsset->getFileName());

	        Ref< model::Model > model = model::ModelCache(m_modelCachePath).get(filePath, meshAsset->getImportFilter());
	        if (!model)
		        return scene::Traverser::VrFailed;

	        model::Transform(
		        translate(meshAsset->getOffset()) *
		        scale(meshAsset->getScaleFactor(), meshAsset->getScaleFactor(), meshAsset->getScaleFactor())
	        ).apply(*model);
            model::Transform(
                (worldInv * entityData->getTransform()).toMatrix44()
            ).apply(*model);

            model->clear(model::Model::CfColors | model::Model::CfJoints);
            models.push_back(model);
        }
        return scene::Traverser::VrContinue;
    });

    // Create merged model.
    Ref< model::Model > outputModel = new model::Model();
    for (int32_t i = 0; i < (int32_t)models.size(); ++i)
        model::MergeModel(*models[i], Transform::identity(), 0.001f).apply(*outputModel);

    return outputModel;
}

Ref< Object > PrefabEntityReplicator::modifyOutput(
    editor::IPipelineBuilder* pipelineBuilder,
    const std::wstring& assetPath,
    const Object* source,
    const model::Model* model,
    const Guid& outputGuid
) const
{
	const PrefabComponentData* prefabComponentData = mandatory_non_null_type_cast< const PrefabComponentData* >(source);

    // Collect all material textures.
    std::map< std::wstring, Guid > materialTextures;
    scene::Traverser::visit(prefabComponentData, [&](const world::EntityData* inoutEntityData) -> scene::Traverser::VisitorResult
    {
        if (auto meshComponentData = inoutEntityData->getComponent< mesh::MeshComponentData >())
        {
            Ref< const mesh::MeshAsset > meshAsset = pipelineBuilder->getObjectReadOnly< mesh::MeshAsset >(
                meshComponentData->getMesh()
            );
            if (!meshAsset)
                return scene::Traverser::VrFailed;

	        // First use textures from texture set.
	        const auto& textureSetId = meshAsset->getTextureSet();
	        if (textureSetId.isNotNull())
	        {
		        Ref< const render::TextureSet > textureSet = pipelineBuilder->getObjectReadOnly< render::TextureSet >(textureSetId);
		        if (!textureSet)
			        return scene::Traverser::VrFailed;

		        materialTextures = textureSet->get();
	        }

	        // Then let explicit material textures override those from a texture set.
	        for (const auto& mt : meshAsset->getMaterialTextures())
		        materialTextures[mt.first] = mt.second;
        }
        return scene::Traverser::VrContinue;
    });

	// Create a new mesh asset referencing the modified model.
    Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
    outputMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
	outputMeshAsset->setMaterialTextures(materialTextures);

	// Build output mesh from modified model.
    pipelineBuilder->buildAdHocOutput(
        outputMeshAsset,
        outputGuid,
        model
    );

    return new mesh::MeshComponentData(resource::Id< mesh::IMesh >(outputGuid));
}

    }
}