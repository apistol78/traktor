#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Database/Database.h"
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
#include "Render/Shader.h"
#include "Shape/Editor/Solid/PrimitiveEntityData.h"
#include "Shape/Editor/Solid/SolidEntityData.h"
#include "Shape/Editor/Solid/SolidEntityPipeline.h"
#include "Shape/Editor/Solid/SolidEntityReplicator.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace shape
	{
		namespace
		{

const resource::Id< render::Shader > c_defaultShader(Guid(L"{F01DE7F1-64CE-4613-9A17-899B44D5414E}"));

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SolidEntityPipeline", 1, SolidEntityPipeline, world::EntityPipeline)

SolidEntityPipeline::SolidEntityPipeline()
:	m_targetEditor(false)
{
}

bool SolidEntityPipeline::create(const editor::IPipelineSettings* settings)
{
    if (!world::EntityPipeline::create(settings))
        return false;

	m_targetEditor = settings->getProperty< bool >(L"Pipeline.TargetEditor");
    return true;
}

TypeInfoSet SolidEntityPipeline::getAssetTypes() const
{
	return makeTypeInfoSet<
		PrimitiveEntityData,
		SolidEntityData
	>();
}

bool SolidEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (m_targetEditor)
		pipelineDepends->addDependency(c_defaultShader, editor::PdfBuild | editor::PdfResource);

	if (auto solidEntityData = dynamic_type_cast< const SolidEntityData* >(sourceAsset))
	{
		for (auto id : solidEntityData->getCollisionGroup())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);
		for (auto id : solidEntityData->getCollisionMask())
			pipelineDepends->addDependency(id, editor::PdfBuild | editor::PdfResource);	
	}
	else if (auto primitiveEntityData = dynamic_type_cast< const PrimitiveEntityData* >(sourceAsset))
	{
		const auto& materials = primitiveEntityData->getMaterials();
		for (const auto& pair : materials)
			pipelineDepends->addDependency(pair.second, editor::PdfUse);
	}
	else
		return false;

	return world::EntityPipeline::buildDependencies(pipelineDepends, sourceInstance, sourceAsset, outputPath, outputGuid);
}

Ref< ISerializable > SolidEntityPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	if (auto solidEntityData = dynamic_type_cast< const SolidEntityData* >(sourceAsset))
	{
		Ref< model::Model > outputModel = SolidEntityReplicator().createModel(pipelineBuilder, L"", solidEntityData);
		if (!outputModel)
			return nullptr;

		Guid outputRenderMeshGuid = pipelineBuilder->synthesizeOutputGuid(1);
		Guid outputCollisionShapeGuid = pipelineBuilder->synthesizeOutputGuid(1);

		std::wstring outputRenderMeshPath = L"Generated/" + outputRenderMeshGuid.format();
		std::wstring outputCollisionShapePath = L"Generated/" + outputCollisionShapeGuid.format();

		// Create our output entity which will only contain the merged meshes.
		Ref< world::EntityData > outputEntityData = new world::EntityData();
		outputEntityData->setName(solidEntityData->getName());
		outputEntityData->setTransform(solidEntityData->getTransform());

		// Build output mesh from merged model.
		Ref< mesh::MeshAsset > visualMeshAsset = new mesh::MeshAsset();
		visualMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
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
		outputShapeDesc->setCollisionGroup(solidEntityData->getCollisionGroup());
		outputShapeDesc->setCollisionMask(solidEntityData->getCollisionMask());

		Ref< physics::StaticBodyDesc > outputBodyDesc = new physics::StaticBodyDesc();
		outputBodyDesc->setShape(outputShapeDesc);

		outputEntityData->setComponent(new physics::RigidBodyComponentData(
			outputBodyDesc
		));

		return outputEntityData;
	}
	else
		return world::EntityPipeline::buildOutput(
			pipelineBuilder,
			sourceInstance,
			sourceAsset,
			buildParams
		);
}

	}
}
