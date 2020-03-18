#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Shape/Editor/Spline/ExtrudeShapeLayerData.h"
#include "Shape/Editor/Spline/SplineEntityData.h"
#include "Shape/Editor/Spline/SplineEntityPipeline.h"
#include "Shape/Editor/Spline/SplineEntityReplicator.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SplineEntityPipeline", 0, SplineEntityPipeline, world::EntityPipeline)

SplineEntityPipeline::SplineEntityPipeline()
:	m_targetEditor(false)
{
}

bool SplineEntityPipeline::create(const editor::IPipelineSettings* settings)
{
	if (!world::EntityPipeline::create(settings))
		return false;

	m_targetEditor = settings->getProperty< bool >(L"Pipeline.TargetEditor");
	return true;
}

TypeInfoSet SplineEntityPipeline::getAssetTypes() const
{
	return makeTypeInfoSet<
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
	if (auto extrudeShapeData = dynamic_type_cast< const ExtrudeShapeLayerData* >(sourceAsset))
	{
		if (m_targetEditor)
			pipelineDepends->addDependency(extrudeShapeData->getShader(), editor::PdfResource | editor::PdfBuild);
	}
	return true;
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
		// Editor support runtime editing of splines.
		return DeepClone(sourceAsset).create();
	}
	else if (auto splineEntityData = dynamic_type_cast< const SplineEntityData* >(sourceAsset))
	{
		// Create model from spline.
		Ref< model::Model > model = SplineEntityReplicator().createModel(pipelineBuilder, L"", splineEntityData);
		if (!model)
			return nullptr;

		// Create our output entity which will only contain the mesh.
		Ref< world::EntityData > entityData = new world::EntityData();
		entityData->setName(splineEntityData->getName());
		entityData->setTransform(splineEntityData->getTransform());

		// Build output mesh from model.
		Ref< mesh::MeshAsset > meshAsset = new mesh::MeshAsset();
		meshAsset->setMeshType(mesh::MeshAsset::MtStatic);

		Guid outputmMeshGuid = pipelineBuilder->synthesizeOutputGuid(1);
		std::wstring outputMeshPath = L"Generated/" + outputmMeshGuid.format();

		pipelineBuilder->buildOutput(
			sourceInstance,
			meshAsset,
			outputMeshPath,
			outputmMeshGuid,
			model
		);

		// Add mesh component to reference our mesh.
		entityData->setComponent(new mesh::MeshComponentData(
			resource::Id< mesh::IMesh >(outputmMeshGuid)
		));

		return entityData;
	}
	else
		return nullptr;
}

	}
}
