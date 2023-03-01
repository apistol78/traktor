/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/Envelope.h"
#include "Core/Math/Float.h"
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

	const Envelope< float > redEnvelope(asset->m_redCurve);
	const Envelope< float > greenEnvelope(asset->m_greenCurve);
	const Envelope< float > blueEnvelope(asset->m_blueCurve);

	const float brightness = asset->m_brightness;
	const float contrast = asset->m_contrast;
	const float saturation = asset->m_saturation;

	Ref< drawing::Image > image = new drawing::Image(drawing::PixelFormat::getRGBAF32(), 64 * 64, 64);
	for (int32_t r = 0; r < 64; ++r)
	{
		const float fr = float(r) / 63.0f;
		
		float gr = redEnvelope(fr);
		gr = clamp(contrast * (gr - 0.5f) + 0.5f + brightness, 0.0f, 1.0f);

		for (int32_t g = 0; g < 64; ++g)
		{
			const float fg = float(g) / 63.0f;

			float gg = greenEnvelope(fg);
			gg = clamp(contrast * (gg - 0.5f) + 0.5f + brightness, 0.0f, 1.0f);

			for (int32_t b = 0; b < 64; ++b)
			{
				const float fb = float(b) / 63.0f;

				float gb = blueEnvelope(fb);
				gb = clamp(contrast * (gb - 0.5f) + 0.5f + brightness, 0.0f, 1.0f);

				const float intensity = (gr + gg + gb) / 3.0f;
				const float outr = clamp(lerp(intensity, gr, saturation), 0.0f, 1.0f);
				const float outg = clamp(lerp(intensity, gg, saturation), 0.0f, 1.0f);
				const float outb = clamp(lerp(intensity, gb, saturation), 0.0f, 1.0f);

				image->setPixel(r + b * 64, g,
					Color4f(
						outr,
						outg,
						outb,
						0.0f
					)
				);
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
