/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Random.h"
#include "Core/Math/Quaternion.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Render/Editor/Texture/CubeMap.h"
#include "Render/Editor/Texture/ProbeTextureAsset.h"
#include "Render/Editor/Texture/ProbeTexturePipeline.h"
#include "Render/Editor/Texture/TextureOutput.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

Vector4 randomCone(Random& r, const Vector4& direction, float coneAngle)
{
	float c = std::cos(coneAngle);
	float z = r.nextFloat() * (1.0f - c) + c;
	float p = r.nextFloat() * TWO_PI;
	float s = std::sqrt(1.0f - z * z);
	return Quaternion(Vector4(0.0f, 0.0f, 1.0f), direction) * Vector4(s * std::cos(p), s * std::sin(p), z);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProbeTexturePipeline", 1, ProbeTexturePipeline, editor::DefaultPipeline)

bool ProbeTexturePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	return true;
}

TypeInfoSet ProbeTexturePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ProbeTextureAsset >());
	return typeSet;
}

bool ProbeTexturePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const ProbeTextureAsset* asset = checked_type_cast< const ProbeTextureAsset* >(sourceAsset);
	pipelineDepends->addDependency(Path(m_assetPath), asset->getFileName().getOriginal());
	pipelineDepends->addDependency< TextureOutput >();
	return true;
}

bool ProbeTexturePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	const ProbeTextureAsset* asset = checked_type_cast< const ProbeTextureAsset* >(sourceAsset);
	Ref< TextureOutput > output;

	Ref< IStream > file = pipelineBuilder->openFile(Path(m_assetPath), asset->getFileName().getOriginal());
	if (!file)
	{
		log::error << L"Probe texture asset pipeline failed; unable to open source image \"" << asset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	Ref< drawing::Image > image = drawing::Image::load(file, asset->getFileName().getExtension());
	if (!image)
	{
		log::error << L"Probe texture asset pipeline failed; unable to load source image \"" << asset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	file->close();

	// Ensure source image has high dynamic range.
	image->convert(drawing::PixelFormat::getARGBF32());

	Ref< const CubeMap > cubeInput = new CubeMap(image);
	Ref< CubeMap > cubeOutput = new CubeMap(cubeInput->getSize(), drawing::PixelFormat::getARGBF32());

	// Calculate filter radius in pixel space.
	int32_t filterRadius = int32_t(std::sin(asset->m_filterAngle) * cubeInput->getSize());
	if (filterRadius <= 0)
		filterRadius = 1;

	log::info << L"Probe filter radius " << filterRadius << " pixel(s)." << Endl;

	// Create output probe by filtering input.
	Random random;
	for (int32_t side = 0; side < 6; ++side)
	{
		for (int32_t y = 0; y < cubeInput->getSize(); ++y)
		{
			for (int32_t x = 0; x < cubeInput->getSize(); ++x)
			{
				Color4f accum(0.0f, 0.0f, 0.0f, 0.0f);
				float accumWeight = 0.0f;

				for (int32_t dky = -filterRadius; dky <= filterRadius; ++dky)
				{
					float fdky = dky / float(filterRadius);
					for (int32_t dkx = -filterRadius; dkx <= filterRadius; ++dkx)
					{
						float fdkx = dkx / float(filterRadius);
						float f = 1.0f - std::sqrt(fdkx * fdkx + fdky * fdky);
						if (f <= 0.0f)
							continue;

						Vector4 d = cubeInput->getDirection(side, x + dkx, y + dky);

						int32_t sampleSide, sampleX, sampleY;
						cubeInput->getPosition(d, sampleSide, sampleX, sampleY);

						Color4f sample;
						cubeInput->getSide(sampleSide)->getPixelUnsafe(sampleX, sampleY, sample);

						accum += sample * Scalar(f);
						accumWeight += f;
					}
				}
				accum /= Scalar(accumWeight);

				cubeOutput->getSide(side)->setPixelUnsafe(x, y, accum);
			}
		}
	}

	// Create cubemap from output probe.
	Ref< drawing::Image > cross = cubeOutput->createCrossImage();
	if (!cross)
	{
		log::error << L"Probe texture asset pipeline failed; unable to create cross image." << Endl;
		return false;
	}

	output = new TextureOutput();
	output->m_textureFormat = TfInvalid;
	output->m_generateMips = true;
	output->m_keepZeroAlpha = false;
	output->m_textureType = TtCube;
	output->m_hasAlpha = false;
	output->m_enableCompression = true;
	output->m_linearGamma = true;

	return pipelineBuilder->buildOutput(
		output,
		outputPath,
		outputGuid,
		cross
	);
}

	}
}
