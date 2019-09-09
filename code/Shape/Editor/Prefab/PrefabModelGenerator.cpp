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
#include "Shape/Editor/Traverser.h"
#include "Shape/Editor/Prefab/PrefabEntityData.h"
#include "Shape/Editor/Prefab/PrefabModelGenerator.h"
#include "World/Entity/ComponentEntityData.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.PrefabModelGenerator", 0, PrefabModelGenerator, IModelGenerator)

TypeInfoSet PrefabModelGenerator::getSupportedTypes() const
{
    return makeTypeInfoSet< PrefabEntityData >();
}

Ref< model::Model > PrefabModelGenerator::createModel(
    editor::IPipelineBuilder* pipelineBuilder,
    const std::wstring& assetPath,
    const Object* source
) const
{
	const PrefabEntityData* prefabEntityData = mandatory_non_null_type_cast< const PrefabEntityData* >(source);

    RefArray< model::Model > models;
    std::map< std::wstring, Guid > materialTextures;

    // Collect all models and remove all mesh components from prefab.
    Traverser::visit(prefabEntityData, [&](const world::EntityData* inoutEntityData) -> bool
    {
        if (auto componentEntityData = dynamic_type_cast< const world::ComponentEntityData* >(inoutEntityData))
        {
            if (auto meshComponentData = componentEntityData->getComponent< mesh::MeshComponentData >())
            {
                Ref< mesh::MeshAsset > meshAsset = pipelineBuilder->getSourceDatabase()->getObjectReadOnly< mesh::MeshAsset >(
                    meshComponentData->getMesh()
                );
                if (!meshAsset)
                    return false;

                // \tbd We should probably ignore mesh assets with custom shaders.

                Ref< model::Model > model = model::ModelFormat::readAny(meshAsset->getFileName(), [&](const Path& p) {
                    return pipelineBuilder->openFile(Path(assetPath), p.getOriginal());
                });
                if (!model)
                    return false;

                // Transform model into world space.
                model::Transform(inoutEntityData->getTransform().toMatrix44()).apply(*model);

                model->clear(model::Model::CfColors | model::Model::CfJoints);
                models.push_back(model);

                materialTextures.insert(
                    meshAsset->getMaterialTextures().begin(),
                    meshAsset->getMaterialTextures().end()
                );

                //componentEntityData->removeComponent(meshComponentData);
            }			
        }
        return true;
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

Ref< Object > PrefabModelGenerator::modifyOutput(
    editor::IPipelineBuilder* pipelineBuilder,
    const std::wstring& assetPath,
    const Object* source,
    const Guid& lightmapId,
    const model::Model* model
) const
{
    return nullptr;
}

    }
}