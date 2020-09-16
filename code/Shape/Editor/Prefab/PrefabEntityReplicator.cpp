#include "Core/Io/FileSystem.h"
#include "Database/Database.h"
#include "Editor/IPipelineBuilder.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/MergeModel.h"
#include "Model/Operations/Transform.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Scene/Editor/Traverser.h"
#include "Shape/Editor/Prefab/PrefabEntityData.h"
#include "Shape/Editor/Prefab/PrefabEntityReplicator.h"
#include "World/EntityData.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.PrefabEntityReplicator", 0, PrefabEntityReplicator, scene::IEntityReplicator)

TypeInfoSet PrefabEntityReplicator::getSupportedTypes() const
{
    return makeTypeInfoSet< PrefabEntityData >();
}

Ref< model::Model > PrefabEntityReplicator::createModel(
    editor::IPipelineBuilder* pipelineBuilder,
    const std::wstring& assetPath,
    const Object* source
) const
{
	const PrefabEntityData* prefabEntityData = mandatory_non_null_type_cast< const PrefabEntityData* >(source);

    RefArray< model::Model > models;
    std::map< std::wstring, Guid > materialTextures;

    // Collect all models and remove all mesh components from prefab.
    scene::Traverser::visit(prefabEntityData, [&](const world::EntityData* inoutEntityData) -> scene::Traverser::VisitorResult
    {
        if (auto meshComponentData = inoutEntityData->getComponent< mesh::MeshComponentData >())
        {
            Ref< mesh::MeshAsset > meshAsset = pipelineBuilder->getSourceDatabase()->getObjectReadOnly< mesh::MeshAsset >(
                meshComponentData->getMesh()
            );
            if (!meshAsset)
                return scene::Traverser::VrFailed;

            // \tbd We should probably ignore mesh assets with custom shaders.

            Path filePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath) + meshAsset->getFileName());
            Ref< model::Model > model = model::ModelFormat::readAny(filePath, meshAsset->getImportFilter(), [&](const Path& p) {
                return pipelineBuilder->openFile(p);
            });
            if (!model)
                return scene::Traverser::VrFailed;

            // Transform model into world space.
            model::Transform(inoutEntityData->getTransform().toMatrix44()).apply(*model);

            model->clear(model::Model::CfColors | model::Model::CfJoints);
            models.push_back(model);

            materialTextures.insert(
                meshAsset->getMaterialTextures().begin(),
                meshAsset->getMaterialTextures().end()
            );

            //inoutEntityData->removeComponent(meshComponentData);
        }			
        return scene::Traverser::VrContinue;
    });

    Ref< model::Model > outputModel = new model::Model();
    if (!models.empty())
    {
        // Calculate number of UV tiles.
        int32_t tiles = (int32_t)(std::ceil(std::sqrt(models.size())) + 0.5f);

        // Offset lightmap UV into tiles.
        for (int32_t i = 0; i < (int32_t)models.size(); ++i)
        {
            float tileU = (float)(i % tiles) / tiles;
            float tileV = (float)(i / tiles) / tiles;

            uint32_t channel = models[i]->getTexCoordChannel(L"Lightmap");
            if (channel != model::c_InvalidIndex)
            {
                // Offset all texcoords into tile.
                AlignedVector< model::Vertex > vertices = models[i]->getVertices();
                for (auto& vertex : vertices)
                {
                    Vector2 uv = models[i]->getTexCoord(vertex.getTexCoord(channel));
                    uv *= (float)(1.0f / tiles);
                    uv += Vector2(tileU, tileV);
                    vertex.setTexCoord(channel, models[i]->addUniqueTexCoord(uv));
                }
                models[i]->setVertices(vertices);
            }
        }

        // Create merged model.
        for (int32_t i = 0; i < (int32_t)models.size(); ++i)
            model::MergeModel(*models[i], Transform::identity(), 0.001f).apply(*outputModel);
    }

    return outputModel;
}

Ref< Object > PrefabEntityReplicator::modifyOutput(
    editor::IPipelineBuilder* pipelineBuilder,
    const std::wstring& assetPath,
    const Object* source,
    const model::Model* model
) const
{
    return nullptr;
}

    }
}