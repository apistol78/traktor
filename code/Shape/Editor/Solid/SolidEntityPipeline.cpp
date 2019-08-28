#include "Core/Io/FileSystem.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Shape/Editor/Solid/IShape.h"
#include "Shape/Editor/Solid/PrimitiveEntityData.h"
#include "Shape/Editor/Solid/SolidEntityData.h"
#include "Shape/Editor/Solid/SolidEntityPipeline.h"
#include "Shape/Editor/Solid/Utilities.h"
#include "World/Entity/ComponentEntityData.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SolidEntityPipeline", 0, SolidEntityPipeline, world::EntityPipeline)

bool SolidEntityPipeline::create(const editor::IPipelineSettings* settings)
{
    if (!world::EntityPipeline::create(settings))
        return false;

    FileSystem::getInstance().makeAllDirectories(L"data/Temp/Solid");
    return true;
}

TypeInfoSet SolidEntityPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< SolidEntityData >();
}

bool SolidEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const SolidEntityData* solidEntityData = mandatory_non_null_type_cast< const SolidEntityData* >(sourceAsset);

	if (!world::EntityPipeline::buildDependencies(pipelineDepends, sourceInstance, sourceAsset, outputPath, outputGuid))
		return false;

    // \tbd Add only shader for editor preview...
    pipelineDepends->addDependency(solidEntityData->getShader(), editor::PdfResource | editor::PdfBuild);
	return true;
}

Ref< ISerializable > SolidEntityPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	const SolidEntityData* solidEntityData = mandatory_non_null_type_cast< const SolidEntityData* >(sourceAsset);
    
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
        model::Polygon polygon;
        polygon.setMaterial(0);
        for (const auto& vx : winding.get())
        {
            model::Vertex vertex;
            vertex.setPosition(outputModel->addUniquePosition(vx));
            polygon.addVertex(outputModel->addUniqueVertex(vertex));
        }
        outputModel->addUniquePolygon(polygon);
    }

    // model::ModelFormat::writeAny(L"data/Temp/Solid/" + solidEntityData->getName() + L".tmd", outputModel);

	Guid outputRenderMeshGuid = solidEntityData->getOutputGuid().permutation(0);
	Guid outputCollisionShapeGuid = solidEntityData->getOutputGuid().permutation(1);

	std::wstring outputRenderMeshPath = L"Generated/" + outputRenderMeshGuid.format();
	std::wstring outputCollisionShapePath = L"Generated/" + outputCollisionShapeGuid.format();

	// Create our output entity which will only contain the merged meshes.
	Ref< world::ComponentEntityData > outputEntityData = new world::ComponentEntityData();
	outputEntityData->setName(solidEntityData->getName());
	outputEntityData->setTransform(solidEntityData->getTransform());

    // Build output mesh from merged model.
    Ref< mesh::MeshAsset > visualMeshAsset = new mesh::MeshAsset();
    visualMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);

    pipelineBuilder->buildOutput(
        visualMeshAsset,
        outputRenderMeshPath,
        outputRenderMeshGuid,
        outputModel
    );

    // Replace mesh component referencing our merged mesh.
    outputEntityData->setComponent(new mesh::MeshComponentData(
        resource::Id< mesh::IMesh >(outputRenderMeshGuid)
    ));

    // Build output mesh from merged model.
    Ref< physics::MeshAsset > physicsMeshAsset = new physics::MeshAsset();
    physicsMeshAsset->setMargin(0.0f);
    physicsMeshAsset->setCalculateConvexHull(false);

    pipelineBuilder->buildOutput(
        physicsMeshAsset,
        outputCollisionShapePath,
        outputCollisionShapeGuid,
        outputModel
    );

    // Replace mesh component referencing our merged physics mesh.
    Ref< physics::MeshShapeDesc > outputShapeDesc = new physics::MeshShapeDesc();
    outputShapeDesc->setMesh(resource::Id< physics::Mesh >(outputCollisionShapeGuid));
    outputShapeDesc->setCollisionGroup(solidEntityData->getCollisionGroup());
    outputShapeDesc->setCollisionMask(solidEntityData->getCollisionMask());

    Ref< physics::StaticBodyDesc > outputBodyDesc = new physics::StaticBodyDesc();
    outputBodyDesc->setShape(outputShapeDesc);

    outputEntityData->setComponent(new physics::RigidBodyComponentData(
        outputBodyDesc
    ));

    return outputEntityData;
}

	}
}
