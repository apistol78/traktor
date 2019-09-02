#include "Editor/IPipelineBuilder.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/Operations/Boolean.h"
#include "Model/Operations/Transform.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Shape/Editor/Solid/IShape.h"
#include "Shape/Editor/Solid/PrimitiveEntityData.h"
#include "Shape/Editor/Solid/SolidEntityData.h"
#include "Shape/Editor/Solid/SolidModelGenerator.h"
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

    // Merge all primitives.
	model::Model current;

    auto it = primitiveEntityDatas.begin();
    if (it != primitiveEntityDatas.end())
    {
		auto model = (*it)->getShape()->createModel();
		if (!model)
			return nullptr;

		current = *model;
		model::Transform((*it)->getTransform().toMatrix44()).apply(current);

        for (++it; it != primitiveEntityDatas.end(); ++it)
        {
			auto other = (*it)->getShape()->createModel();
			if (!other)
				continue;

			model::Model result;

            switch ((*it)->getOperation())
            {
            case BooleanOperation::BoUnion:
                {
					model::Boolean(
						current,
						Transform::identity(),
						*other,
						(*it)->getTransform(),
						model::Boolean::BoUnion
					).apply(result);
                }
                break;

            case BooleanOperation::BoIntersection:
                {
					model::Boolean(
						current,
						Transform::identity(),
						*other,
						(*it)->getTransform(),
						model::Boolean::BoIntersection
					).apply(result);
				}
                break;

            case BooleanOperation::BoDifference:
                {
					model::Boolean(
						current,
						Transform::identity(),
						*other,
						(*it)->getTransform(),
						model::Boolean::BoDifference
					).apply(result);
				}
                break;
            }

			current = std::move(result);
        }
    }

	//model::Triangulate().apply(current);
	//model::CalculateTangents().apply(current);

    return new model::Model(current);
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