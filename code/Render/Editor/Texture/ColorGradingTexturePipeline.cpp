#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Render/Editor/Texture/ColorGradingTextureAsset.h"
#include "Render/Editor/Texture/ColorGradingTexturePipeline.h"
#include "Render/Editor/Texture/TextureOutput.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ColorGradingTexturePipeline", 4, ColorGradingTexturePipeline, editor::DefaultPipeline)

TypeInfoSet ColorGradingTexturePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< ColorGradingTextureAsset >();
	return typeSet;
}

bool ColorGradingTexturePipeline::shouldCache() const
{
	return true;
}

bool ColorGradingTexturePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	pipelineDepends->addDependency< TextureOutput >();
	return true;
}

bool ColorGradingTexturePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	const ColorGradingTextureAsset* asset = checked_type_cast< const ColorGradingTextureAsset*, false >(sourceAsset);

	Ref< drawing::Image > image = new drawing::Image(drawing::PixelFormat::getRGBAF32(), 64 * 64, 64);
	for (int32_t r = 0; r < 64; ++r)
	{
		float fr = float(r) / 63.0f;
		float gr = std::pow(fr, asset->getGamma());
		for (int32_t g = 0; g < 64; ++g)
		{
			float fg = float(g) / 63.0f;
			float gg = std::pow(fg, asset->getGamma());
			for (int32_t b = 0; b < 64; ++b)
			{
				float fb = float(b) / 63.0f;
				float gb = std::pow(fb, asset->getGamma());
				image->setPixel(r + b * 64, g, Color4f(gr, gg, gb, 0.0f));
			}
		}
	}

	Ref< TextureOutput > output;
	output = new TextureOutput();
	output->m_textureFormat = TfR8G8B8A8;
	output->m_generateMips = false;
	output->m_keepZeroAlpha = false;
	output->m_textureType = Tt3D;
	output->m_enableCompression = false;
	output->m_linearGamma = true;
	output->m_systemTexture = true;

	return pipelineBuilder->buildAdHocOutput(
		output,
		outputPath,
		outputGuid,
		image
	);
}

	}
}
