#include "Editor/IPipelineBuilder.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Shape/Editor/Solid/IShape.h"
#include "Shape/Editor/Solid/PrimitiveEntityData.h"
#include "Shape/Editor/Solid/SolidEntityData.h"
#include "Shape/Editor/Solid/SolidModelGenerator.h"
#include "Shape/Editor/Solid/Utilities.h"
#include "World/Entity/ComponentEntityData.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SolidModelGenerator", 0, SolidModelGenerator, IModelGenerator)

TypeInfoSet SolidModelGenerator::getSupportedTypes() const
{
    return makeTypeInfoSet< SolidEntityData >();
}

Ref< model::Model > SolidModelGenerator::createModel(const Object* source) const
{
	const SolidEntityData* solidEntityData = mandatory_non_null_type_cast< const SolidEntityData* >(source);
    
    // Get all primitive entities with shape.
    RefArray< const PrimitiveEntityData > primitiveEntityDatas;
    for (auto entityData : solidEntityData->getEntityData())
    {
        if (const auto primitiveEntityData = dynamic_type_cast< const PrimitiveEntityData* >(entityData))
        {
            if (primitiveEntityData->getShape() != nullptr)
                primitiveEntityDatas.push_back(primitiveEntityData);
        }
    }

    // Merge all windings into a single group of windings.
    AlignedVector< Winding3 > outputWindings;

    auto it = primitiveEntityDatas.begin();
    if (it != primitiveEntityDatas.end())
    {
        (*it)->getShape()->createWindings(outputWindings);
        outputWindings = transform(outputWindings, (*it)->getTransform());

        for (++it; it != primitiveEntityDatas.end(); ++it)
        {
            AlignedVector< Winding3 > windings;
            (*it)->getShape()->createWindings(windings);
            if (windings.empty())
                continue;

            windings = transform(windings, (*it)->getTransform());

            switch ((*it)->getOperation())
            {
            case BooleanOperation::BoUnion:
                {
                    auto result = unioon(outputWindings, windings);
                    outputWindings.swap(result);
                }
                break;

            case BooleanOperation::BoIntersection:
                {
                    auto result = intersection(outputWindings, windings);
                    outputWindings.swap(result);
                }
                break;

            case BooleanOperation::BoDifference:
                {
                    auto result = difference(outputWindings, windings);
                    outputWindings.swap(result);
                }
                break;
            }
        }
    }

    // Create model from windings.
    Ref< model::Model > outputModel = new model::Model();
    outputModel->addMaterial(model::Material(
        L"Default"
    ));
    for (const auto& winding : outputWindings)
    {
		Plane pl;
		if (!winding.getPlane(pl))
			continue;

		uint32_t normal = outputModel->addUniqueNormal(pl.normal());

        model::Polygon polygon;
        polygon.setMaterial(0);
		polygon.setNormal(normal);
        for (const auto& vx : winding.get())
        {
            model::Vertex vertex;
            vertex.setPosition(outputModel->addUniquePosition(vx));
			vertex.setNormal(normal);
            polygon.addVertex(outputModel->addUniqueVertex(vertex));
        }
        outputModel->addUniquePolygon(polygon);
    }

    return outputModel;
}

Ref< Object > SolidModelGenerator::modifyOutput(
    editor::IPipelineBuilder* pipelineBuilder,
    const Object* source,
    const Guid& lightmapId,
    const model::Model* model
) const
{
    const SolidEntityData* solidEntityData = mandatory_non_null_type_cast< const SolidEntityData* >(source);

	Guid outputRenderMeshGuid = solidEntityData->getOutputGuid().permutation(0);
	Guid outputCollisionShapeGuid = solidEntityData->getOutputGuid().permutation(1);

	std::wstring outputRenderMeshPath = L"Generated/" + outputRenderMeshGuid.format();
	std::wstring outputCollisionShapePath = L"Generated/" + outputCollisionShapeGuid.format();

    // Build visual mesh.
    Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
    outputMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
    outputMeshAsset->setMaterialTextures({ { L"__Illumination__", lightmapId } });

    pipelineBuilder->buildOutput(
        outputMeshAsset,
        outputRenderMeshPath,
        outputRenderMeshGuid,
        model
    );

    // Build collision mesh.
    Ref< physics::MeshAsset > physicsMeshAsset = new physics::MeshAsset();
    physicsMeshAsset->setMargin(0.0f);
    physicsMeshAsset->setCalculateConvexHull(false);

    pipelineBuilder->buildOutput(
        physicsMeshAsset,
        outputCollisionShapePath,
        outputCollisionShapeGuid,
        model
    );

    // Replace mesh component referencing our merged physics mesh.
    Ref< physics::MeshShapeDesc > outputShapeDesc = new physics::MeshShapeDesc();
    outputShapeDesc->setMesh(resource::Id< physics::Mesh >(outputCollisionShapeGuid));
    outputShapeDesc->setCollisionGroup(solidEntityData->getCollisionGroup());
    outputShapeDesc->setCollisionMask(solidEntityData->getCollisionMask());

	// Create our output entity which will replace the solid entity.
	Ref< world::ComponentEntityData > outputEntityData = new world::ComponentEntityData();
	outputEntityData->setName(solidEntityData->getName());
	outputEntityData->setTransform(solidEntityData->getTransform());
    outputEntityData->setComponent(new mesh::MeshComponentData(resource::Id< mesh::IMesh >(outputRenderMeshGuid)));
    outputEntityData->setComponent(new physics::RigidBodyComponentData(new physics::StaticBodyDesc(outputShapeDesc)));
    return outputEntityData;
}

    }
}