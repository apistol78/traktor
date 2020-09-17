#include "Editor/IPipelineDepends.h"
#include "Render/Editor/Texture/TextureSet.h"
#include "Render/Editor/Texture/TextureSetPipeline.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TextureSetPipeline", 0, TextureSetPipeline, editor::DefaultPipeline)

TypeInfoSet TextureSetPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< TextureSet >();
}

bool TextureSetPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const TextureSet* textureSet = checked_type_cast< const TextureSet* >(sourceAsset);
	for (const auto& ts : textureSet->get())
		pipelineDepends->addDependency(ts.second, editor::PdfBuild);
	return true;
}

	}
}
