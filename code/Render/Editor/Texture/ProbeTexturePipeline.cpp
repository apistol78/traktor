/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include <cmft/image.h>
#include <cmft/cubemapfilter.h>
#include <cmft/clcontext.h>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Random.h"
#include "Core/Math/Quaternion.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
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

ProbeTexturePipeline::ProbeTexturePipeline()
:	m_clContext(nullptr)
{
}

bool ProbeTexturePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

	int32_t clLoaded = cmft::clLoad();
	if (clLoaded)
	{
		m_clContext = cmft::clInit(
			CMFT_CL_VENDOR_ANY_GPU,
			CMFT_CL_DEVICE_TYPE_GPU | CMFT_CL_DEVICE_TYPE_ACCELERATOR,
			0
		);
		if (!m_clContext)
			log::warning << L"Probe texture asset pipeline; Unable to initialize OpenCL." << Endl;
	}
	else
		log::warning << L"Probe texture asset pipeline; OpenCL not loaded." << Endl;

	return true;
}

void ProbeTexturePipeline::destroy()
{
	if (m_clContext)
	{
		cmft::clUnload();
		m_clContext = nullptr;
	}
	editor::DefaultPipeline::destroy();
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

	Ref< drawing::Image > assetImage = drawing::Image::load(file, asset->getFileName().getExtension());
	if (!assetImage)
	{
		log::error << L"Probe texture asset pipeline failed; unable to load source image \"" << asset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	file->close();

	// Scale asset image if required.
	if (asset->m_sizeDivisor > 1)
	{
		drawing::ScaleFilter scaleFilter(
			assetImage->getWidth() / asset->m_sizeDivisor,
			assetImage->getHeight() / asset->m_sizeDivisor,
			drawing::ScaleFilter::MnAverage,
			drawing::ScaleFilter::MgLinear
		);
		assetImage->apply(&scaleFilter);
	}

	// Convert image into 32-bit float point format.
	assetImage->convert(drawing::PixelFormat::getRGBAF32());
	assetImage->clearAlpha(0.0f);

	log::info << L"Probe image loaded successfully; size " << assetImage->getWidth() << L" * " << assetImage->getHeight() << Endl;

	// Convert into cmft image.
	cmft::Image tmp;
	tmp.m_width    = (uint16_t)assetImage->getWidth();
	tmp.m_height   = (uint16_t)assetImage->getHeight();
	tmp.m_dataSize = assetImage->getDataSize();
	tmp.m_format   = cmft::TextureFormat::RGBA32F;
	tmp.m_numMips  = 1;
	tmp.m_numFaces = 1;
	tmp.m_data     = assetImage->getData();

	cmft::Image image;
	cmft::imageCopy(image, tmp);

	if (cmft::imageIsCubeCross(image, true))
		cmft::imageCubemapFromCross(image);
	else if (cmft::imageIsLatLong(image))
		imageCubemapFromLatLong(image);
	else if (cmft::imageIsHStrip(image))
		cmft::imageCubemapFromStrip(image);
	else if (cmft::imageIsVStrip(image))
		cmft::imageCubemapFromStrip(image);
	else if (cmft::imageIsOctant(image))
		cmft::imageCubemapFromOctant(image);
	else
	{
		log::error << L"Probe texture asset pipeline failed; Image is not cubemap(6 faces), cubecross(ratio 3:4 or 4:3), latlong(ratio 2:1), hstrip(ratio 6:1), vstrip(ration 1:6)" << Endl;
		return false;
	}

	// Execute conversion filter.
	cmft::imageRadianceFilter(
		image,
		0,
		(cmft::LightingModel::Enum)0, // inputParameters.m_lightingModel,
		false, //(bool)inputParameters.m_excludeBase,
		1, //(uint8_t)inputParameters.m_mipCount,
		asset->m_glossScale, //(uint8_t)inputParameters.m_glossScale,	// smaller == blurry, 20 pretty sharp
		asset->m_glossBias, //(uint8_t)inputParameters.m_glossBias,
		(cmft::EdgeFixup::Enum)0, //inputParameters.m_edgeFixup,
		255, // (int8_t)inputParameters.m_numCpuProcessingThreads,
		m_clContext
	);

	log::info << L"Probe filter complete" << Endl;

	// Convert back into cross image.
	cmft::imageCrossFromCubemap(image);

	// Create output image.
	Ref< drawing::Image > outputImage = new drawing::Image(drawing::PixelFormat::getRGBAF32(), image.m_width, image.m_height);
	std::memcpy(outputImage->getData(), image.m_data, outputImage->getDataSize());

	// Cleanup.
	cmft::imageUnload(image);

	// Build runtime texture cube map.
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
		outputImage
	);
}

	}
}
