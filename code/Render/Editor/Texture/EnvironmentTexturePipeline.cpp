/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Compress/Lzf/DeflateStreamLzf.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quasirandom.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/CubeMap.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Editor/Pipeline/PipelineProfiler.h"
#include "Render/Types.h"
#include "Render/Editor/Texture/AstcCompressor.h"
#include "Render/Editor/Texture/DxtnCompressor.h"
#include "Render/Editor/Texture/EnvironmentTextureAsset.h"
#include "Render/Editor/Texture/EnvironmentTexturePipeline.h"
#include "Render/Editor/Texture/EtcCompressor.h"
#include "Render/Editor/Texture/PvrtcCompressor.h"
#include "Render/Editor/Texture/UnCompressor.h"
#include "Render/Resource/TextureResource.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.EnvironmentTexturePipeline", 6, EnvironmentTexturePipeline, editor::DefaultPipeline)

bool EnvironmentTexturePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
	m_compressionQuality = settings->getPropertyIncludeHash< int32_t >(L"TexturePipeline.CompressionQuality", 1);

	std::wstring compressionMethod = settings->getPropertyIncludeHash< std::wstring >(L"TexturePipeline.CompressionMethod", L"DXTn");
	if (compareIgnoreCase(compressionMethod, L"None") == 0)
		m_compressionMethod = CompressionMethod::None;
	else if (compareIgnoreCase(compressionMethod, L"DXTn") == 0)
		m_compressionMethod = CompressionMethod::DXTn;
	else if (compareIgnoreCase(compressionMethod, L"PVRTC") == 0)
		m_compressionMethod = CompressionMethod::PVRTC;
	else if (compareIgnoreCase(compressionMethod, L"ETC1") == 0)
		m_compressionMethod = CompressionMethod::ETC1;
	else if (compareIgnoreCase(compressionMethod, L"ASTC") == 0)
		m_compressionMethod = CompressionMethod::ASTC;
	else
	{
		log::error << L"Unknown compression method \"" << compressionMethod << L"\"." << Endl;
		return false;
	}

	return true;
}

TypeInfoSet EnvironmentTexturePipeline::getAssetTypes() const
{
	return makeTypeInfoSet< EnvironmentTextureAsset >();
}

bool EnvironmentTexturePipeline::shouldCache() const
{
	return true;
}

bool EnvironmentTexturePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const EnvironmentTextureAsset* asset = mandatory_non_null_type_cast< const EnvironmentTextureAsset* >(sourceAsset);
	pipelineDepends->addDependency(Path(m_assetPath), asset->getFileName().getOriginal());
	return true;
}

bool EnvironmentTexturePipeline::buildOutput(
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
	const EnvironmentTextureAsset* asset = mandatory_non_null_type_cast< const EnvironmentTextureAsset* >(sourceAsset);

	Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + asset->getFileName());
	Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
	if (!file)
	{
		log::error << L"Environment texture asset pipeline failed; unable to open source image \"" << asset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	Ref< drawing::Image > image = drawing::Image::load(file, asset->getFileName().getExtension());
	if (!image)
	{
		log::error << L"Environment texture asset pipeline failed; unable to load source image \"" << asset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	file->close();

	Ref< drawing::CubeMap > cubeMap = drawing::CubeMap::createFromImage(image);
	if (!cubeMap)
	{
		log::error << L"Unable to convert image into cubemap; unknown layout." << Endl;
		return false;
	}
	
	drawing::PixelFormat pixelFormat = drawing::PixelFormat::getR8G8B8A8().endianSwapped();
	TextureFormat textureFormat = TfR8G8B8A8;

	const int32_t sideSize = cubeMap->getSize();
	const int32_t mipCount = log2(sideSize) + 1;
	T_ASSERT(mipCount >= 1);

	log::info << L"Building environment texture..." << Endl;

	// Create output instance.
	Ref< TextureResource > outputResource = new TextureResource();
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!outputInstance)
	{
		log::error << L"Unable to create output instance." << Endl;
		return false;
	}

	outputInstance->setObject(outputResource);

	// Create output data stream.
	Ref< IStream > stream = outputInstance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Unable to create texture data stream." << Endl;
		outputInstance->revert();
		return false;
	}

	Writer writer(stream);

	writer << uint32_t(12);
	writer << int32_t(sideSize);
	writer << int32_t(sideSize);
	writer << int32_t(6);
	writer << int32_t(mipCount);
	writer << int32_t(textureFormat);
	writer << bool(false);
	writer << uint8_t(TtCube);
	writer << bool(true);
	writer << bool(false);

	// Create data writer, use deflate compression.
	Ref< IStream > streamData = new BufferedStream(new compress::DeflateStreamLzf(stream), 64 * 1024);
	Writer writerData(streamData);

	const int32_t sampleCounts[] = { 1, 200, 400, 600, 800, 1000 };
	for (int32_t side = 0; side < 6; ++side)
	{
		RefArray< drawing::Image > mipImages(mipCount);
		
		mipImages[0] = cubeMap->getSide(side)->clone();
		mipImages[0]->convert(pixelFormat);

		for (int32_t i = 1; i < mipCount; ++i)
		{
			const int32_t mipSize = sideSize >> i;
			const float roughness = float(i) / (mipCount - 1);
			const int32_t samples = sampleCounts[std::min< int32_t >(i, sizeof_array(sampleCounts) - 1)];

			log::info << L"Generating side " << side << L", mip " << i << L", size " << mipSize << L", roughness " << roughness << L", samples " << samples << L"..." << Endl;

			mipImages[i] = new drawing::Image(
				pixelFormat,
				mipSize,
				mipSize
			);

			for (int32_t y = 0; y < mipSize; ++y)
			{
				for (int32_t x = 0; x < mipSize; ++x)
				{
					const Vector4 direction = cubeMap->getDirection(side, x << i, y << i);
					
					Vector4 u, v;
					orthogonalFrame(direction, u, v);
					const Matrix44 M(u, v, direction, Vector4::zero());

					Color4f color(0.0f, 0.0f, 0.0f);
					Scalar weight(0.0f);

					for (int32_t i = 0; i < samples; ++i)
					{
						const Vector2 rnd = Quasirandom::hammersley(i, samples);

						const float a = roughness * roughness;
						const float phi = TWO_PI * rnd.x;
						const float cosTheta = std::sqrt((1.0f - rnd.y) / (1.0f + (a * a - 1.0f) * rnd.y));
						const float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);

						const Vector4 H(
							std::cos(phi) * sinTheta,
							std::sin(phi) * sinTheta,
							cosTheta
						);

						const Vector4 Hw = (M * H).normalized();
						const Vector4 L = Hw * (2.0_simd * dot3(direction, Hw)) - direction;

						color += min(cubeMap->get(L.normalized()), Color4f(10.0f, 10.0f, 10.0f, 0.0f));
						weight += max(dot3(direction, L), 0.0_simd);
					}

					color /= weight;
					mipImages[i]->setPixelUnsafe(x, y, color.rgb1());
				}
			}
		}

		Ref< ICompressor > compressor;
		if (textureFormat >= TfDXT1 && textureFormat <= TfDXT5)
			compressor = new DxtnCompressor();
		else if (textureFormat >= TfPVRTC1 && textureFormat <= TfPVRTC4)
			compressor = new PvrtcCompressor();
		else if (textureFormat == TfETC1)
			compressor = new EtcCompressor();
		else if (textureFormat == TfASTC4x4 && textureFormat <= TfASTC12x12)
			compressor = new AstcCompressor();
		else
			compressor = new UnCompressor();

		log::info << L"Compressing texture..." << Endl;
		pipelineBuilder->getProfiler()->begin(type_of(compressor));
		compressor->compress(writerData, mipImages, textureFormat, false, m_compressionQuality);
		pipelineBuilder->getProfiler()->end(type_of(compressor));
	}

	streamData->close();

	if (!outputInstance->commit())
	{
		log::error << L"Unable to commit output instance." << Endl;
		return false;
	}

	return true;
}

	}
}
