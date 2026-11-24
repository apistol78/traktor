/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/EnterLeave.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldTextureAsset.h"
#include "Heightfield/Editor/HeightfieldTexturePipeline.h"
#include "Heightfield/Editor/TileStorage.h"
#include "Render/Editor/Texture/TextureOutput.h"

namespace traktor::hf
{
	namespace
	{

const int32_t c_rowsPerTask = 64;

float fract(float v)
{
	return v - std::floor(v);
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldTexturePipeline", 8, HeightfieldTexturePipeline, editor::DefaultPipeline)

bool HeightfieldTexturePipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
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

	// Tiles and overviews depend on stamp of tiles read, not on every tile in heightfield instance.
	if (asset->m_tileX < 0 && asset->m_overviewSize <= 0)
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
	const HeightfieldTextureAsset* asset = checked_type_cast< const HeightfieldTextureAsset* >(sourceAsset);
	const bool region = (asset->m_tileX >= 0 || asset->m_overviewSize > 0);

	// Whole heightfield use a lot of memory; serialize those builds, tiles and overviews are built concurrently.
	T_ANONYMOUS_VAR(EnterLeave)(
		[&]() { if (!region) m_lock.wait(); },
		[&]() { if (!region) m_lock.release(); }
	);

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

	// Normals of tiles need a border of samples from neighbour tiles.
	const int32_t border = (asset->m_tileX >= 0 && asset->m_output == HeightfieldTextureAsset::OtNormals) ? 1 : 0;

	const TileStorage tileStorage(heightfieldAssetInstance, heightfieldAsset);

	Ref< Heightfield > heightfield;
	if (asset->m_tileX >= 0)
		heightfield = tileStorage.readHeightfieldTile(heightfieldAsset->getWorldExtent(), asset->m_tileX, asset->m_tileZ, border);
	else if (asset->m_overviewSize > 0)
		heightfield = tileStorage.readHeightfieldOverview(heightfieldAsset->getWorldExtent(), asset->m_overviewSize);
	else
		heightfield = tileStorage.readHeightfield(heightfieldAsset->getWorldExtent());

	if (!heightfield)
	{
		log::error << L"Heightfield pipeline failed; unable to read heights" << Endl;
		return false;
	}

	const int32_t size = heightfield->getSize() - 2 * border;

	if (asset->m_output == HeightfieldTextureAsset::OtHeights)
	{
		Ref< drawing::Image > outputMap = new drawing::Image(drawing::PixelFormat::getR32F(), size, size);

		// Rows are filled in parallel.
		AlignedVector< Job::task_t > tasks;
		for (int32_t v0 = 0; v0 < size; v0 += c_rowsPerTask)
		{
			tasks.push_back([&, v0]() {
				for (int32_t v = v0; v < std::min(v0 + c_rowsPerTask, size); ++v)
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
			});
		}
		JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());

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

		// Mipmapped tile maps are power-of-2 with first and last texel centered on first and seam sample.
		const int32_t normalSize = (asset->m_tileX >= 0) ? size - 1 : size;
		const float normalScale = (asset->m_tileX >= 0) ? (float)(size - 1) / (normalSize - 1) : 1.0f;

		Ref< drawing::Image > outputMap = new drawing::Image(drawing::PixelFormat::getR8G8B8(), normalSize, normalSize);

		// Rows are filled in parallel.
		AlignedVector< Job::task_t > tasks;
		for (int32_t v0 = 0; v0 < normalSize; v0 += c_rowsPerTask)
		{
			tasks.push_back([&, v0]() {
				for (int32_t v = v0; v < std::min(v0 + c_rowsPerTask, normalSize); ++v)
				{
					for (int32_t u = 0; u < normalSize; ++u)
					{
						const Vector4 normal = heightfield->normalAt(border + u * normalScale, border + v * normalScale) * Vector4(0.5f, 0.5f, 0.5f, 0.0f) + Vector4(0.5f, 0.5f, 0.5f, 0.0f);
						outputMap->setPixelUnsafe(u, v, Color4f(
							normal.x(),
							normal.z(),
							normal.y()
						));
					}
				}
			});
		}
		JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());

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
		bool holes = false;
		for (int32_t v = 0; v < size && !holes; ++v)
		{
			for (int32_t u = 0; u < size && !holes; ++u)
				holes = !heightfield->getGridCut(u, v);
		}

		// Tiles and overviews without holes get a single texel cut map.
		const int32_t cutSize = (region && !holes) ? 1 : size;

		Ref< drawing::Image > outputMap = new drawing::Image(drawing::PixelFormat::getR8(), cutSize, cutSize);
		for (int32_t v = 0; v < cutSize; ++v)
		{
			for (int32_t u = 0; u < cutSize; ++u)
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
