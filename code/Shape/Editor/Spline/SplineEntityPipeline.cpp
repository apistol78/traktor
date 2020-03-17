#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Shape/Editor/Spline/ExtrudeShapeLayerData.h"
#include "Shape/Editor/Spline/SplineComponentData.h"
#include "Shape/Editor/Spline/SplineEntityPipeline.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"

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
		SplineComponentData
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
		return DeepClone(sourceAsset).create();
	else
	{
		// Freeze static geometry from spline and create a static mesh and physics
		// for entire spline.
		return nullptr;
	}
}

	}
}
