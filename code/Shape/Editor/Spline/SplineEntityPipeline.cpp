#include "Shape/Spline/ExtrudeShapeLayerData.h"
#include "Shape/Spline/SplineEntityData.h"
#include "Shape/Editor/Spline/SplineEntityPipeline.h"
#include "Editor/IPipelineDepends.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SplineEntityPipeline", 0, SplineEntityPipeline, world::EntityPipeline)

TypeInfoSet SplineEntityPipeline::getAssetTypes() const
{
	return makeTypeInfoSet(
		type_of< SplineEntityData >()
	);
}

bool SplineEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const SplineEntityData* entityData = mandatory_non_null_type_cast< const SplineEntityData* >(sourceAsset);

	if (!world::EntityPipeline::buildDependencies(pipelineDepends, sourceInstance, sourceAsset, outputPath, outputGuid))
		return false;

	for (const auto& layer : entityData->getLayers())
	{
		if (const auto extrudeShapeLayer = dynamic_type_cast< const ExtrudeShapeLayerData* >(layer))
			pipelineDepends->addDependency(extrudeShapeLayer->getShader(), editor::PdfResource | editor::PdfBuild);
	}

	return true;
}

	}
}
