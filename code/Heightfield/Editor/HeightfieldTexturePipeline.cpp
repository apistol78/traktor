/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Acquire.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/ErosionFilter.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldTextureAsset.h"
#include "Heightfield/Editor/HeightfieldTexturePipeline.h"
#include "Render/Editor/Texture/TextureOutput.h"

namespace traktor::hf
{
	namespace
	{

float fract(float v)
{
	return v - std::floor(v);
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldTexturePipeline", 5, HeightfieldTexturePipeline, editor::DefaultPipeline)

bool HeightfieldTexturePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");

	const std::wstring hf = settings->getPropertyIncludeHash< std::wstring >(L"HeightfieldTexturePipeline.HeightFormat", L"HfFloat32");
	if (hf == L"HfFloat16")
		m_heightFormat = HfFloat16;
	else if (hf == L"HfFloat32")
		m_heightFormat = HfFloat32;
	else if (hf == L"HfARGBEncoded")
		m_heightFormat = HfARGBEncoded;
	else
	{
		log::error << L"Heightfield pipeline error; Unknown height format \"" << hf << L"\"." << Endl;
		return false;
	}

	m_compressNormals = settings->getPropertyIncludeHash< bool >(L"HeightfieldTexturePipeline.CompressNormals", false);
	return true;
}

TypeInfoSet HeightfieldTexturePipeline::getAssetTypes() const
{
	return makeTypeInfoSet< HeightfieldTextureAsset >();
}

bool HeightfieldTexturePipeline::shouldCache() const
{
	return true;
}

bool HeightfieldTexturePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const HeightfieldTextureAsset* asset = checked_type_cast< const HeightfieldTextureAsset* >(sourceAsset);
	pipelineDepends->addDependency(asset->m_heightfield, editor::PdfUse);
	pipelineDepends->addDependency< render::TextureOutput >();
	return true;
}

bool HeightfieldTexturePipeline::buildOutput(
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
	// Due to memory usage of this pipeline we need to serialize
	// building of these assets. Pipeline should try reduce number
	// of concurrent builds with same pipeline type.
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	const HeightfieldTextureAsset* asset = checked_type_cast< const HeightfieldTextureAsset* >(sourceAsset);

	// Get heightfield asset and instance.
	Ref< const db::Instance > heightfieldAssetInstance = pipelineBuilder->getSourceDatabase()->getInstance(asset->m_heightfield);
	if (!heightfieldAssetInstance)
	{
		log::error << L"Heightfield texture pipeline failed; unable to get heightfield asset instance" << Endl;
		return false;
	}

	Ref< const HeightfieldAsset > heightfieldAsset = heightfieldAssetInstance->getObject< const HeightfieldAsset >();
	if (!heightfieldAsset)
	{
		log::error << L"Heightfield texture pipeline failed; unable to get heightfield asset" << Endl;
		return false;
	}

	Ref< IStream > sourceData = heightfieldAssetInstance->readData(L"Data");
	if (!sourceData)
	{
		log::error << L"Heightfield pipeline failed; unable to open heights" << Endl;
		return false;
	}

	Ref< Heightfield > heightfield = HeightfieldFormat().read(
		sourceData,
		heightfieldAsset->getWorldExtent()
	);
	if (!heightfield)
	{
		log::error << L"Heightfield pipeline failed; unable to read heights" << Endl;
		return false;
	}

	sourceData->close();
	sourceData = nullptr;

	// Apply erosion filter.
	if (heightfieldAsset->getErosionEnable())
		ErosionFilter(heightfieldAsset->getErodeIterations()).apply(heightfield);

	const int32_t size = heightfield->getSize();

	if (asset->m_output == HeightfieldTextureAsset::OtHeights)
	{
		Ref< drawing::Image > outputMap = new drawing::Image(drawing::PixelFormat::getR32F(), size, size);

		for (int32_t v = 0; v < size; ++v)
		{
			for (int32_t u = 0; u < size; ++u)
			{
				const float height = heightfield->getGridHeightNearest(u, v) * asset->m_scale;
				if (m_heightFormat == HfARGBEncoded)
				{
					Vector4 enc = Vector4(
						fract(1.0f * height),
						fract(255.0f * height),
						fract(65025.0f * height),
						fract(160581375.0f * height)
					);
					enc -= enc.shuffle< 1, 2, 3, 3 >() * Vector4(1.0 / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 0.0f);
					outputMap->setPixelUnsafe(u, v, Color4f(enc));
				}
				else
					outputMap->setPixelUnsafe(u, v, Color4f(height, height, height, height));
			}
		}

		const render::TextureFormat c_heightTextureFormats[] = { render::TfR16F, render::TfR32F, render::TfR8G8B8A8 };

		Ref< render::TextureOutput > output = new render::TextureOutput();
		output->m_textureFormat = c_heightTextureFormats[m_heightFormat];
		output->m_normalMap = false;
		output->m_scaleDepth = 0.0f;
		output->m_generateMips = false;
		output->m_keepZeroAlpha = false;
		output->m_textureType = render::Tt2D;
		output->m_hasAlpha = bool(m_heightFormat == HfARGBEncoded);
		output->m_ignoreAlpha = false;
		output->m_scaleImage = false;
		output->m_scaleWidth = 0;
		output->m_scaleHeight = 0;
		output->m_enableCompression = false;
		output->m_inverseNormalMapY = false;
		output->m_assumeLinearGamma = true;
		output->m_generateSphereMap = false;
		output->m_preserveAlphaCoverage = false;
		output->m_alphaCoverageReference = 0.0f;
		output->m_sharpenRadius = 0;
		output->m_sharpenStrength = 0.0f;
		output->m_noiseStrength = 0.0f;
		output->m_systemTexture = true;

		return pipelineBuilder->buildAdHocOutput(
			output,
			outputPath,
			outputGuid,
			outputMap
		);
	}
	else if (asset->m_output == HeightfieldTextureAsset::OtNormals)
	{
		// Create normal map from heightfield; we're storing in XZY since
		// we assume B to be "primary" axis when compressing.
		Ref< drawing::Image > outputMap = new drawing::Image(drawing::PixelFormat::getR8G8B8(), size, size);
		for (int32_t v = 0; v < size; ++v)
		{
			for (int32_t u = 0; u < size; ++u)
			{
				const Vector4 normal = heightfield->normalAt((float)u, (float)v) * Vector4(0.5f, 0.5f, 0.5f, 0.0f) + Vector4(0.5f, 0.5f, 0.5f, 0.0f);
				outputMap->setPixelUnsafe(u, v, Color4f(
					normal.x(),
					normal.z(),
					normal.y()
				));
			}
		}

		Ref< render::TextureOutput > output = new render::TextureOutput();
		output->m_textureFormat = render::TfInvalid;
		output->m_normalMap = true;
		output->m_scaleDepth = 0.0f;
		output->m_generateMips = true;
		output->m_keepZeroAlpha = false;
		output->m_textureType = render::Tt2D;
		output->m_hasAlpha = false;
		output->m_ignoreAlpha = true;
		output->m_scaleImage = false;
		output->m_scaleWidth = 0;
		output->m_scaleHeight = 0;
		output->m_enableCompression = m_compressNormals;
		output->m_inverseNormalMapY = false;
		output->m_assumeLinearGamma = true;
		output->m_generateSphereMap = false;
		output->m_preserveAlphaCoverage = false;
		output->m_alphaCoverageReference = 0.0f;
		output->m_sharpenRadius = 0;
		output->m_sharpenStrength = 0.0f;
		output->m_noiseStrength = 0.0f;
		output->m_systemTexture = true;

		return pipelineBuilder->buildAdHocOutput(
			output,
			outputPath,
			outputGuid,
			outputMap
		);
	}
	else if (asset->m_output == HeightfieldTextureAsset::OtCuts)
	{
		Ref< drawing::Image > outputMap = new drawing::Image(drawing::PixelFormat::getR8(), size, size);

		for (int32_t v = 0; v < size; ++v)
		{
			for (int32_t u = 0; u < size; ++u)
			{
				const float cut = heightfield->getGridCut(u, v) ? 1.0f : 0.0f;
				outputMap->setPixelUnsafe(u, v, Color4f(cut, cut, cut, cut));
			}
		}

		Ref< render::TextureOutput > output = new render::TextureOutput();
		output->m_textureFormat = render::TfR8;
		output->m_normalMap = false;
		output->m_scaleDepth = 0.0f;
		output->m_generateMips = false;
		output->m_keepZeroAlpha = false;
		output->m_textureType = render::Tt2D;
		output->m_hasAlpha = false;
		output->m_ignoreAlpha = false;
		output->m_scaleImage = false;
		output->m_scaleWidth = 0;
		output->m_scaleHeight = 0;
		output->m_enableCompression = false;
		output->m_inverseNormalMapY = false;
		output->m_assumeLinearGamma = true;
		output->m_generateSphereMap = false;
		output->m_preserveAlphaCoverage = false;
		output->m_alphaCoverageReference = 0.0f;
		output->m_sharpenRadius = 0;

		return pipelineBuilder->buildAdHocOutput(
			output,
			outputPath,
			outputGuid,
			outputMap
		);
	}
	else if (asset->m_output == HeightfieldTextureAsset::OtUnwrap)
	{
		Ref< drawing::Image > outputMap = new drawing::Image(drawing::PixelFormat::getRGBAF32(), size, size);

		const Vector4& worldExtent = heightfield->getWorldExtent();
		for (int32_t v = 0; v < size; ++v)
		{
			for (int32_t u = 0; u < size; ++u)
			{
				const float h = heightfield->getGridHeightNearest(u, v) * worldExtent.y();
				const float hu = heightfield->getGridHeightNearest(u + 1, v) * worldExtent.y();
				const float hv = heightfield->getGridHeightNearest(u, v + 1) * worldExtent.y();
				outputMap->setPixelUnsafe(u, v, Color4f(abs(hu - h), abs(hv - h), 0.0f, 1.0f));
			}
		}

		Color4f ct;

		for (int32_t v = 0; v < size; ++v)
		{
			Scalar total(0.0f);
			for (int32_t u = 0; u < size; ++u)
			{
				outputMap->getPixelUnsafe(u, v, ct);

				const Scalar du = ct.getRed();

				ct.setRed(total);
				outputMap->setPixelUnsafe(u, v, ct);

				total += du * Scalar(0.5f) + Scalar(0.5f);
			}
			for (int32_t u = 0; u < size; ++u)
			{
				outputMap->getPixelUnsafe(u, v, ct);
				ct.setRed(ct.getRed() / total);
				outputMap->setPixelUnsafe(u, v, ct);
			}
		}

		for (int32_t u = 0; u < size; ++u)
		{
			Scalar total(0.0f);
			for (int32_t v = 0; v < size; ++v)
			{
				outputMap->getPixelUnsafe(u, v, ct);

				const Scalar dv = ct.getGreen();

				ct.setGreen(total);
				outputMap->setPixelUnsafe(u, v, ct);

				total += dv * Scalar(0.5f) + Scalar(0.5f);
			}
			for (int32_t v = 0; v < size; ++v)
			{
				outputMap->getPixelUnsafe(u, v, ct);
				ct.setGreen(ct.getGreen() / total);
				outputMap->setPixelUnsafe(u, v, ct);
			}
		}

		Ref< render::TextureOutput > output = new render::TextureOutput();
		output->m_textureFormat = render::TfR8G8B8A8;
		output->m_normalMap = false;
		output->m_scaleDepth = 0.0f;
		output->m_generateMips = false;
		output->m_keepZeroAlpha = false;
		output->m_textureType = render::Tt2D;
		output->m_hasAlpha = false;
		output->m_ignoreAlpha = false;
		output->m_scaleImage = false;
		output->m_scaleWidth = 0;
		output->m_scaleHeight = 0;
		output->m_enableCompression = false;
		output->m_inverseNormalMapY = false;
		output->m_assumeLinearGamma = true;
		output->m_generateSphereMap = false;
		output->m_preserveAlphaCoverage = false;
		output->m_alphaCoverageReference = 0.0f;
		output->m_sharpenRadius = 0;

		return pipelineBuilder->buildAdHocOutput(
			output,
			outputPath,
			outputGuid,
			outputMap
		);
	}
	else
		return false;
}

}
