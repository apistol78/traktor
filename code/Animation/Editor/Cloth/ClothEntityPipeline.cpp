#include "Animation/Cloth/ClothComponentData.h"
#include "Animation/Editor/Cloth/ClothEntityPipeline.h"
#include "Editor/IPipelineDepends.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.ClothEntityPipeline", 0, ClothEntityPipeline, world::EntityPipeline)

TypeInfoSet ClothEntityPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< ClothComponentData >();
}

bool ClothEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (auto clothComponentData = dynamic_type_cast< const ClothComponentData* >(sourceAsset))
		pipelineDepends->addDependency(clothComponentData->getShader(), editor::PdfBuild | editor::PdfResource);
	return true;
}

	}
}
