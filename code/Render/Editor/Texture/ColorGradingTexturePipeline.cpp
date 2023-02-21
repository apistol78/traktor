/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Render/Editor/Texture/ColorGradingTextureAsset.h"
#include "Render/Editor/Texture/ColorGradingTexturePipeline.h"
#include "Render/Editor/Texture/TextureOutput.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ColorGradingTexturePipeline", 4, ColorGradingTexturePipeline, editor::DefaultPipeline)

TypeInfoSet ColorGradingTexturePipeline::getAssetTypes() const
{
	return makeTypeInfoSet< ColorGradingTextureAsset >();
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
		const float fr = float(r) / 63.0f;
		const float gr = std::pow(fr, asset->getRedGamma());
		for (int32_t g = 0; g < 64; ++g)
		{
			const float fg = float(g) / 63.0f;
			const float gg = std::pow(fg, asset->getGreenGamma());
			for (int32_t b = 0; b < 64; ++b)
			{
				const float fb = float(b) / 63.0f;
				const float gb = std::pow(fb, asset->getBlueGamma());
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
