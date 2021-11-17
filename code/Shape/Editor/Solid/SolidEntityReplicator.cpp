#include "Database/Database.h"
#include "Editor/IPipelineBuilder.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/Operations/Boolean.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/MergeCoplanarAdjacents.h"
#include "Model/Operations/Transform.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Shape/Editor/Solid/IShape.h"
#include "Shape/Editor/Solid/PrimitiveEntityData.h"
#include "Shape/Editor/Solid/SolidEntityData.h"
#include "Shape/Editor/Solid/SolidEntityReplicator.h"
#include "World/EntityData.h"
#include "World/Entity/GroupComponentData.h"

namespace traktor
{
	namespace shape
	{
//		namespace
//		{
//
//void associateMaterials(
//	db::Database* database,
//	model::Model* model,
//	const SmallMap< int32_t, Guid >& materialMap
//)
//{
//	uint32_t tc = model->addUniqueTexCoordChannel(L"UVMap");
//
//	AlignedVector< model::Material > materials = model->getMaterials();
//	for (const auto& m : materialMap)
//	{
//		if (m.first >= materials.size())
//			continue;
//
//		Ref< Material > sm = database->getObjectReadOnly< Material >(m.second);
//		if (!sm)
//			continue;
//
//		auto& material = materials[m.first];
//		sm->prepareMaterial(tc, material);
//	}
//	model->setMaterials(materials);	
//}
//
//		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SolidEntityReplicator", 0, SolidEntityReplicator, scene::IEntityReplicator)

bool SolidEntityReplicator::create(const editor::IPipelineSettings* settings)
{
    return true;
}

TypeInfoSet SolidEntityReplicator::getSupportedTypes() const
{
	return makeTypeInfoSet< SolidEntityData >();
}

// bool SolidEntityReplicator::addDependencies(
//     editor::IPipelineDepends* pipelineDepends,
//     const world::EntityData* entityData,
//     const world::IEntityComponentData* componentData
// ) const
// {
// 	return true;
// }

Ref< model::Model > SolidEntityReplicator::createVisualModel(
	editor::IPipelineBuilder* pipelineBuilder,
    const world::EntityData* entityData,
    const world::IEntityComponentData* componentData
) const
{
	const SolidEntityData* solidEntityData = mandatory_non_null_type_cast< const SolidEntityData* >(entityData);
	
	auto group = solidEntityData->getComponent< world::GroupComponentData >();
	if (!group)
		return nullptr;

	// Get all primitive entities with shape.
	RefArray< const PrimitiveEntityData > primitiveEntityDatas;
	for (auto entityData : group->getEntityData())
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

		//associateMaterials(
		//	pipelineBuilder->getSourceDatabase(),
		//	model,
		//	(*it)->getMaterials()
		//);

		current = *model;
		model::Transform((*it)->getTransform().toMatrix44()).apply(current);

		for (++it; it != primitiveEntityDatas.end(); ++it)
		{
			auto other = (*it)->getShape()->createModel();
			if (!other)
				continue;

			//associateMaterials(
			//	pipelineBuilder->getSourceDatabase(),
			//	other,
			//	(*it)->getMaterials()
			//);

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
			model::CleanDegenerate().apply(current);
			model::MergeCoplanarAdjacents().apply(current);
		}
	}

	model::Transform(solidEntityData->getTransform().inverse().toMatrix44()).apply(current);

	return new model::Model(current);
}

Ref< model::Model > SolidEntityReplicator::createCollisionModel(
	editor::IPipelineBuilder* pipelineBuilder,
    const world::EntityData* entityData,
    const world::IEntityComponentData* componentData
) const
{
	return nullptr;
}

// Ref< Object > SolidEntityReplicator::modifyOutput(
// 	editor::IPipelineBuilder* pipelineBuilder,
//     const world::EntityData* entityData,
//     const world::IEntityComponentData* /*componentData*/,
// 	const model::Model* model,
// 	const Guid& outputGuid
// ) const
// {
// 	const SolidEntityData* solidEntityData = mandatory_non_null_type_cast< const SolidEntityData* >(entityData);

// 	Guid outputRenderMeshGuid = outputGuid.permutation(1);
// 	Guid outputCollisionShapeGuid = outputGuid.permutation(2);

// 	std::wstring outputRenderMeshPath = L"Generated/" + outputRenderMeshGuid.format();
// 	std::wstring outputCollisionShapePath = L"Generated/" + outputCollisionShapeGuid.format();

// 	// Build visual mesh.
// 	Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
// 	outputMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
// 	pipelineBuilder->buildAdHocOutput(
// 		outputMeshAsset,
// 		outputRenderMeshPath,
// 		outputRenderMeshGuid,
// 		model
// 	);

// 	// Build collision mesh.
// 	Ref< physics::MeshAsset > physicsMeshAsset = new physics::MeshAsset();
// 	physicsMeshAsset->setMargin(0.0f);
// 	physicsMeshAsset->setCalculateConvexHull(false);
// 	pipelineBuilder->buildAdHocOutput(
// 		physicsMeshAsset,
// 		outputCollisionShapePath,
// 		outputCollisionShapeGuid,
// 		model
// 	);

// 	// Replace mesh component referencing our merged physics mesh.
// 	Ref< physics::MeshShapeDesc > outputShapeDesc = new physics::MeshShapeDesc();
// 	outputShapeDesc->setMesh(resource::Id< physics::Mesh >(outputCollisionShapeGuid));
// 	outputShapeDesc->setCollisionGroup(solidEntityData->getCollisionGroup());
// 	outputShapeDesc->setCollisionMask(solidEntityData->getCollisionMask());

// 	// Create our output entity which will replace the solid entity.
// 	Ref< world::EntityData > outputEntityData = new world::EntityData();
// 	outputEntityData->setId(solidEntityData->getId());
// 	outputEntityData->setName(solidEntityData->getName());
// 	outputEntityData->setTransform(solidEntityData->getTransform());
// 	outputEntityData->setComponent(new mesh::MeshComponentData(resource::Id< mesh::IMesh >(outputRenderMeshGuid)));
// 	outputEntityData->setComponent(new physics::RigidBodyComponentData(new physics::StaticBodyDesc(outputShapeDesc)));
// 	return outputEntityData;
// }

	}
}