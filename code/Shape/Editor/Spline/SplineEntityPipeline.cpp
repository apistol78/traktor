#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Shape/Editor/Spline/ControlPointComponentData.h"
#include "Shape/Editor/Spline/ExtrudeShapeLayerData.h"
#include "Shape/Editor/Spline/SplineEntityData.h"
#include "Shape/Editor/Spline/SplineEntityPipeline.h"
#include "Shape/Editor/Spline/SplineEntityReplicator.h"

namespace traktor
{
	namespace shape
	{
		namespace
		{

const resource::Id< render::Shader > c_defaultShader(Guid(L"{F01DE7F1-64CE-4613-9A17-899B44D5414E}"));

const Guid c_renderMeshIdSeed(L"{E5CE60FA-9C1A-4DB3-9A55-634EB157D1B9}");
const Guid c_collisionShapeIdSeed(L"{4BF9C3FE-673B-4DA8-8DB0-6B4E0D8CFBA0}");

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SplineEntityPipeline", 1, SplineEntityPipeline, world::EntityPipeline)

SplineEntityPipeline::SplineEntityPipeline()
:	m_targetEditor(false)
{
}

bool SplineEntityPipeline::create(const editor::IPipelineSettings* settings)
{
	if (!world::EntityPipeline::create(settings))
		return false;

	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath");
	m_targetEditor = settings->getPropertyIncludeHash< bool >(L"Pipeline.TargetEditor");
	return true;
}

TypeInfoSet SplineEntityPipeline::getAssetTypes() const
{
	return makeTypeInfoSet<
		ControlPointComponentData,
		ExtrudeShapeLayerData,
		SplineEntityData
	>();
}

bool SplineEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (m_targetEditor)
		pipelineDepends->addDependency(c_defaultShader, editor::PdfBuild | editor::PdfResource);

	if (auto splineEntityData = dynamic_type_cast< const SplineEntityData* >(sourceAsset))
	{
		for (auto id : splineEntityData->getCollisionGroup())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);
		for (auto id : splineEntityData->getCollisionMask())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);	
	}

	if (auto extrudeShapeLayerData = dynamic_type_cast< const ExtrudeShapeLayerData* >(sourceAsset))
	{
		pipelineDepends->addDependency(extrudeShapeLayerData->getMeshStart(), editor::PdfBuild);
		pipelineDepends->addDependency(extrudeShapeLayerData->getMeshRepeat(), editor::PdfBuild);
		pipelineDepends->addDependency(extrudeShapeLayerData->getMeshEnd(), editor::PdfBuild);
	}

	return world::EntityPipeline::buildDependencies(pipelineDepends, sourceInstance, sourceAsset, outputPath, outputGuid);
}

Ref< ISerializable > SplineEntityPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	if (m_targetEditor)
	{
		// In editor we generate spline geometry dynamically thus
		// not necessary to explicitly build mesh when building for editor.
		return world::EntityPipeline::buildOutput(pipelineBuilder, sourceInstance, sourceAsset, buildParams);
	}

	if (auto splineEntityData = dynamic_type_cast< const SplineEntityData* >(sourceAsset))
	{
		// Create model from spline.
		Ref< model::Model > outputModel = SplineEntityReplicator().createModel(pipelineBuilder, splineEntityData, nullptr);
		if (!outputModel)
		{
			log::warning << L"Unable to create model from spline \"" << splineEntityData->getName() << L"\"." << Endl;
			return nullptr;
		}

		const Guid& entityId = splineEntityData->getId();
		if (entityId.isNull())
			return nullptr;

		Guid outputRenderMeshGuid = entityId.permutation(c_renderMeshIdSeed);
		Guid outputCollisionShapeGuid = entityId.permutation(c_collisionShapeIdSeed);

		std::wstring outputRenderMeshPath = L"Generated/" + outputRenderMeshGuid.format();
		std::wstring outputCollisionShapePath = L"Generated/" + outputCollisionShapeGuid.format();

		// Create our output entity which will only contain the merged meshes.
		Ref< world::EntityData > outputEntityData = new world::EntityData();
		outputEntityData->setId(splineEntityData->getId());
		outputEntityData->setName(splineEntityData->getName());
		outputEntityData->setTransform(splineEntityData->getTransform());

		// Build output mesh from merged model.
		Ref< mesh::MeshAsset > visualMeshAsset = new mesh::MeshAsset();
		visualMeshAsset->setMeshType(mesh::MeshAsset::MtPartition);
		pipelineBuilder->buildAdHocOutput(
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
		pipelineBuilder->buildAdHocOutput(
			physicsMeshAsset,
			outputCollisionShapePath,
			outputCollisionShapeGuid,
			outputModel
		);

		// Replace mesh component referencing our merged physics mesh.
		Ref< physics::MeshShapeDesc > outputShapeDesc = new physics::MeshShapeDesc();
		outputShapeDesc->setMesh(resource::Id< physics::Mesh >(outputCollisionShapeGuid));
		outputShapeDesc->setCollisionGroup(splineEntityData->getCollisionGroup());
		outputShapeDesc->setCollisionMask(splineEntityData->getCollisionMask());

		Ref< physics::StaticBodyDesc > outputBodyDesc = new physics::StaticBodyDesc();
		outputBodyDesc->setShape(outputShapeDesc);

		outputEntityData->setComponent(new physics::RigidBodyComponentData(
			outputBodyDesc
		));

		return outputEntityData;
	}
	else
		return world::EntityPipeline::buildOutput(pipelineBuilder, sourceInstance, sourceAsset, buildParams);
}

	}
}
