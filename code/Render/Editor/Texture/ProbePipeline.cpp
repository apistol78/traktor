#include <cstring>
#include <cmft/image.h>
#include <cmft/cubemapfilter.h>
#include <cmft/clcontext.h>
#include "Compress/Lzf/DeflateStreamLzf.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Random.h"
#include "Core/Math/Quaternion.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Render/Editor/Texture/DxtnCompressor.h"
#include "Render/Editor/Texture/EtcCompressor.h"
#include "Render/Editor/Texture/IrradianceProbeAsset.h"
#include "Render/Editor/Texture/ProbePipeline.h"
#include "Render/Editor/Texture/PvrtcCompressor.h"
#include "Render/Editor/Texture/RadianceProbeAsset.h"
#include "Render/Editor/Texture/UnCompressor.h"
#include "Render/Resource/TextureResource.h"

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProbePipeline", 1, ProbePipeline, editor::DefaultPipeline)

ProbePipeline::ProbePipeline()
:	m_clContext(nullptr)
{
}

bool ProbePipeline::create(const editor::IPipelineSettings* settings)
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

void ProbePipeline::destroy()
{
	if (m_clContext)
	{
		cmft::clUnload();
		m_clContext = nullptr;
	}
	editor::DefaultPipeline::destroy();
}

TypeInfoSet ProbePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< IrradianceProbeAsset >());
	typeSet.insert(&type_of< RadianceProbeAsset >());
	return typeSet;
}

bool ProbePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	 if (const IrradianceProbeAsset* irradianceAsset = checked_type_cast< const IrradianceProbeAsset* >(sourceAsset))
		pipelineDepends->addDependency(Path(m_assetPath), irradianceAsset->getFileName().getOriginal());
	else if (const RadianceProbeAsset* radianceAsset = checked_type_cast< const RadianceProbeAsset* >(sourceAsset))
		pipelineDepends->addDependency(Path(m_assetPath), radianceAsset->getFileName().getOriginal());
	else
		return false;

	return true;
}

bool ProbePipeline::buildOutput(
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
	const editor::Asset* asset = checked_type_cast< const editor::Asset* >(sourceAsset);

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

	cmft::Image image;

	if (const IrradianceProbeAsset* irradianceAsset = dynamic_type_cast< const IrradianceProbeAsset* >(asset))
	{
		// Scale asset image if required.
		// if (irradianceAsset->m_sizeDivisor > 1)
		// {
		// 	drawing::ScaleFilter scaleFilter(
		// 		assetImage->getWidth() / irradianceAsset->m_sizeDivisor,
		// 		assetImage->getHeight() / irradianceAsset->m_sizeDivisor,
		// 		drawing::ScaleFilter::MnAverage,
		// 		drawing::ScaleFilter::MgLinear
		// 	);
		// 	assetImage->apply(&scaleFilter);
		// }

		// Convert image into 32-bit float point format.
		assetImage->convert(drawing::PixelFormat::getRGBAF32());
		assetImage->clearAlpha(0.0f);

		log::info << L"Irradiance probe image loaded successfully; size " << assetImage->getWidth() << L" * " << assetImage->getHeight() << Endl;

		// Convert into cmft image.
		cmft::Image tmp;
		tmp.m_width    = (uint16_t)assetImage->getWidth();
		tmp.m_height   = (uint16_t)assetImage->getHeight();
		tmp.m_dataSize = assetImage->getDataSize();
		tmp.m_format   = cmft::TextureFormat::RGBA32F;
		tmp.m_numMips  = 1;
		tmp.m_numFaces = 1;
		tmp.m_data     = assetImage->getData();

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

		T_FATAL_ASSERT (image.m_width == image.m_height);

		cmft::imageIrradianceFilterSh(
			image,
			256 // inputParameters.m_dstFaceSize
		);

		log::info << L"Irradiance probe filter complete." << Endl;
	}
	else if (const RadianceProbeAsset* radianceAsset = dynamic_type_cast< const RadianceProbeAsset* >(asset))
	{
		// Scale asset image if required.
		if (radianceAsset->m_sizeDivisor > 1)
		{
			drawing::ScaleFilter scaleFilter(
				assetImage->getWidth() / radianceAsset->m_sizeDivisor,
				assetImage->getHeight() / radianceAsset->m_sizeDivisor,
				drawing::ScaleFilter::MnAverage,
				drawing::ScaleFilter::MgLinear
			);
			assetImage->apply(&scaleFilter);
		}

		// Convert image into 32-bit float point format.
		assetImage->convert(drawing::PixelFormat::getRGBAF32());
		assetImage->clearAlpha(0.0f);

		log::info << L"Radiance probe image loaded successfully; size " << assetImage->getWidth() << L" * " << assetImage->getHeight() << Endl;

		// Convert into cmft image.
		cmft::Image tmp;
		tmp.m_width    = (uint16_t)assetImage->getWidth();
		tmp.m_height   = (uint16_t)assetImage->getHeight();
		tmp.m_dataSize = assetImage->getDataSize();
		tmp.m_format   = cmft::TextureFormat::RGBA32F;
		tmp.m_numMips  = 1;
		tmp.m_numFaces = 1;
		tmp.m_data     = assetImage->getData();

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

		T_FATAL_ASSERT (image.m_width == image.m_height);

		uint32_t mipCount = log2(image.m_width) + 1;
		T_ASSERT (mipCount >= 1);

		// Execute conversion filter.
		cmft::imageRadianceFilter(
			image,
			0,
			(cmft::LightingModel::Enum)0, // inputParameters.m_lightingModel,
			false, //(bool)inputParameters.m_excludeBase,
			(uint8_t)mipCount, //(uint8_t)inputParameters.m_mipCount,
			radianceAsset->m_glossScale, //(uint8_t)inputParameters.m_glossScale,	// smaller == blurry, 20 pretty sharp
			radianceAsset->m_glossBias, //(uint8_t)inputParameters.m_glossBias,
			(cmft::EdgeFixup::Enum)0, //inputParameters.m_edgeFixup,
			255, // (int8_t)inputParameters.m_numCpuProcessingThreads,
			m_clContext
		);

		log::info << L"Radiance probe filter complete." << Endl;
	}

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

	uint32_t sideSize = image.m_width;
	uint32_t mipCount = image.m_numMips;

	Writer writer(stream);
	writer << uint32_t(12);
	writer << int32_t(sideSize);
	writer << int32_t(sideSize);
	writer << int32_t(6);
	writer << int32_t(mipCount);
	writer << int32_t(TfR32G32B32A32F);
	writer << bool(false);
	writer << uint8_t(TtCube);
	writer << bool(true);
	writer << bool(true);

	// Create data writer, use deflate compression if enabled.
	Ref< IStream > streamData = new compress::DeflateStreamLzf(stream);
	Writer writerData(streamData);

	uint32_t offsets[CUBE_FACE_NUM][MAX_MIP_NUM] = { 0 };
	cmft::imageGetMipOffsets(offsets, image);

	for (int side = 0; side < 6; ++side)
	{
		RefArray< drawing::Image > mipImages(mipCount);
		for (int i = 0; i < mipCount; ++i)
		{
			uint32_t mipSize = sideSize >> i;
			Ref< drawing::Image > mipImage = new drawing::Image(drawing::PixelFormat::getRGBAF32(), mipSize, mipSize);
			std::memcpy(mipImage->getData(), (uint8_t*)image.m_data + offsets[side][i], mipImage->getDataSize());
			mipImages[i] = mipImage;
		}
		UnCompressor().compress(writerData, mipImages, TfR32G32B32A32F, false, 1);
	}

	streamData->close();

	// Cleanup.
	cmft::imageUnload(image);

	if (!outputInstance->commit())
	{
		log::error << L"Unable to commit output instance." << Endl;
		return false;
	}

	return true;
}

	}
}
