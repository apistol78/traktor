#include "Shape/Spline/ExtrudeShapeLayerData.h"
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
		type_of< ExtrudeShapeLayerData >()
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
	auto extrudeShapeData = mandatory_non_null_type_cast< const ExtrudeShapeLayerData* >(sourceAsset);
	pipelineDepends->addDependency(extrudeShapeData->getShader(), editor::PdfResource | editor::PdfBuild);
	return true;
}

	}
}
