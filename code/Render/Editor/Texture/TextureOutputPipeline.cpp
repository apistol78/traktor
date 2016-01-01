#include <cstring>
#include "Compress/Lzf/DeflateStreamLzf.h"
#include "Core/Functor/Functor.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Log2.h"
#include "Core/Math/Vector4.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ChainFilter.h"
#include "Drawing/Filters/GammaFilter.h"
#include "Drawing/Filters/MirrorFilter.h"
#include "Drawing/Filters/NormalizeFilter.h"
#include "Drawing/Filters/NormalMapFilter.h"
#include "Drawing/Filters/PremultiplyAlphaFilter.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Drawing/Filters/SharpenFilter.h"
#include "Drawing/Filters/SwizzleFilter.h"
#include "Drawing/Filters/TransformFilter.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineReport.h"
#include "Editor/IPipelineSettings.h"
#include "Render/Types.h"
#include "Render/Editor/Texture/DxtnCompressor.h"
#include "Render/Editor/Texture/EtcCompressor.h"
#include "Render/Editor/Texture/PvrtcCompressor.h"
#include "Render/Editor/Texture/SphereMapFilter.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "Render/Editor/Texture/TextureOutputPipeline.h"
#include "Render/Editor/Texture/UnCompressor.h"
#include "Render/Resource/TextureResource.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct ScaleTextureTask : public Object
{
	Ref< drawing::Image > image;
	Ref< drawing::IImageFilter > filter;
	float alphaCoverageDesired;
	float alphaCoverageRef;

	void execute()
	{
		image->apply(filter);

		if (alphaCoverageDesired > 0.0f)
		{
			float alphaRefMin = 0.0f;
			float alphaRefMax = 1.0f;
			float alphaRefMid = 0.5f;

			for (int32_t i = 0; i < 10; ++i)
			{
				float alphaCoverageMip = 0.0f;

				for (int32_t y = 0; y < image->getHeight(); ++y)
				{
					for (int32_t x = 0; x < image->getWidth(); ++x)
					{
						Color4f color;
						image->getPixelUnsafe(x, y, color);
						alphaCoverageMip += (color.getAlpha() > alphaRefMid) ? 1.0f : 0.0f;
					}
				}

				alphaCoverageMip /= float(image->getWidth() * image->getHeight());

				if (alphaCoverageMip > alphaCoverageDesired + FUZZY_EPSILON)
					alphaRefMin = alphaRefMid;
				else if (alphaCoverageMip < alphaCoverageDesired - FUZZY_EPSILON)
					alphaRefMax = alphaRefMid;
				else
					break;

				alphaRefMid = (alphaRefMin + alphaRefMax) / 2.0f;
			}

			float alphaScale = alphaCoverageRef / alphaRefMid;

			for (int32_t y = 0; y < image->getHeight(); ++y)
			{
				for (int32_t x = 0; x < image->getWidth(); ++x)
				{
					Color4f color;
					image->getPixelUnsafe(x, y, color);
					color.setAlpha(clamp(color.getAlpha() * Scalar(alphaScale), Scalar(0.0f), Scalar(1.0f)));
					image->setPixelUnsafe(x, y, color);
				}
			}
		}
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TextureOutputPipeline", 31, TextureOutputPipeline, editor::IPipeline)

TextureOutputPipeline::TextureOutputPipeline()
:	m_generateMipsThread(false)
,	m_skipMips(0)
,	m_clampSize(0)
,	m_compressionMethod(CmDXTn)
,	m_compressionQuality(1)
,	m_gamma(2.2f)
,	m_sRGB(false)
{
}

bool TextureOutputPipeline::create(const editor::IPipelineSettings* settings)
{
	m_generateMipsThread = settings->getProperty< PropertyBoolean >(L"TexturePipeline.GenerateMipsThread", false);
	m_skipMips = settings->getProperty< PropertyInteger >(L"TexturePipeline.SkipMips", 0);
	m_clampSize = settings->getProperty< PropertyInteger >(L"TexturePipeline.ClampSize", 0);
	m_compressionQuality = settings->getProperty< PropertyInteger >(L"TexturePipeline.CompressionQuality", 1);
	m_gamma = settings->getProperty< PropertyFloat >(L"TexturePipeline.Gamma", 2.2f);
	m_sRGB = settings->getProperty< PropertyBoolean >(L"TexturePipeline.sRGB", false);

	std::wstring compressionMethod = settings->getProperty< PropertyString >(L"TexturePipeline.CompressionMethod", L"DXTn");
	if (compareIgnoreCase< std::wstring >(compressionMethod, L"None") == 0)
		m_compressionMethod = CmNone;
	else if (compareIgnoreCase< std::wstring >(compressionMethod, L"DXTn") == 0)
		m_compressionMethod = CmDXTn;
	else if (compareIgnoreCase< std::wstring >(compressionMethod, L"PVRTC") == 0)
		m_compressionMethod = CmPVRTC;
	else if (compareIgnoreCase< std::wstring >(compressionMethod, L"ETC1") == 0)
		m_compressionMethod = CmETC1;
	else
	{
		log::error << L"Unknown compression method \"" << compressionMethod << L"\"" << Endl;
		return false;
	}

	return true;
}

void TextureOutputPipeline::destroy()
{
}

TypeInfoSet TextureOutputPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TextureOutput >());
	return typeSet;
}

bool TextureOutputPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	return true;
}

bool TextureOutputPipeline::buildOutput(
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
	const TextureOutput* textureOutput = checked_type_cast< const TextureOutput* >(sourceAsset);
	Ref< drawing::Image > image = const_cast< drawing::Image* >(checked_type_cast< const drawing::Image*, false >(buildParams));

	int32_t width = image->getWidth();
	int32_t height = image->getHeight();
	int32_t mipCount = 1;
	bool isNormalMap = false;
	bool sRGB = false;

	drawing::PixelFormat pixelFormat;
	TextureFormat textureFormat;
	bool needAlpha;

	// Use explicit texture format if specified.
	if (textureOutput->m_textureFormat != TfInvalid)
	{
		log::info << L"Using explicit texture format" << Endl;

		textureFormat = textureOutput->m_textureFormat;
		switch (textureFormat)
		{
		case TfR8:
			pixelFormat = drawing::PixelFormat::getR8();
			break;
		case TfR8G8B8A8:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfR5G6B5:
			pixelFormat = drawing::PixelFormat::getR5G6B5();
			break;
		case TfR5G5B5A1:
			pixelFormat = drawing::PixelFormat::getR5G5B5A1();
			break;
		case TfR4G4B4A4:
			pixelFormat = drawing::PixelFormat::getR4G4B4A4();
			break;
		case TfR16G16B16A16F:
			pixelFormat = drawing::PixelFormat::getABGRF16();
			break;
		case TfR32G32B32A32F:
			pixelFormat = drawing::PixelFormat::getABGRF32();
			break;
		//case TfR16G16F:
		//	pixelFormat = drawing::PixelFormat::getR16G16F();
		//	break;
		//case TfR32G32F:
		//	pixelFormat = drawing::PixelFormat::getR32G32F();
		//	break;
		case TfR16F:
			pixelFormat = drawing::PixelFormat::getR16F();
			break;
		case TfR32F:
			pixelFormat = drawing::PixelFormat::getR32F();
			break;
		//case TfR11G11B10F:
		//	break;
		case TfDXT1:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfDXT2:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfDXT3:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfDXT4:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfDXT5:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfPVRTC1:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfPVRTC2:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfPVRTC3:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfPVRTC4:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfETC1:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		default:
			log::error << L"TextureOutputPipeline failed; unsupported explicit texture format" << Endl;
			return false;
		}

		needAlpha = 
			(pixelFormat.getAlphaBits() > 0 && !textureOutput->m_ignoreAlpha) ||
			textureOutput->m_enableNormalMapCompression;
	}
	else
	{
		log::info << L"Using automatic texture format" << Endl;

		// Determine pixel and texture format from source image (and hints).
		needAlpha = 
			(image->getPixelFormat().getAlphaBits() > 0 && !textureOutput->m_ignoreAlpha) ||
			textureOutput->m_enableNormalMapCompression;

		if (needAlpha)
		{
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			textureFormat = TfR8G8B8A8;
		}
		else
		{
			pixelFormat = drawing::PixelFormat::getR8G8B8X8();
			textureFormat = TfR8G8B8A8;
		}

		// Determine texture compression format.
		if (
			(textureOutput->m_textureType == Tt2D || textureOutput->m_textureType == TtCube) &&
			(textureOutput->m_enableCompression || textureOutput->m_enableNormalMapCompression)
		)
		{
			if (m_compressionMethod == CmDXTn)
			{
				if (textureOutput->m_enableNormalMapCompression)
				{
					log::info << L"Using DXT5nm compression" << Endl;
					textureFormat = TfDXT5;
				}
				else
				{
					if (needAlpha)
					{
						log::info << L"Using DXT3 compression" << Endl;
						textureFormat = TfDXT3;
					}
					else
					{
						log::info << L"Using DXT1 compression" << Endl;
						textureFormat = TfDXT1;
					}
				}
			}
			else if (m_compressionMethod == CmPVRTC)
			{
				if (
					width == height &&
					width >= 8 && width <= 2048
				)
				{
					if (needAlpha)
					{
						log::info << L"Using PVRTC3 compression" << Endl;
						textureFormat = TfPVRTC3;
					}
					else
					{
						log::info << L"Using PVRTC1 compression" << Endl;
						textureFormat = TfPVRTC1;
					}
				}
				else
					log::info << L"Using no compression" << Endl;
			}
			else if (m_compressionMethod == CmETC1)
			{
				if (!needAlpha)
				{
					log::info << L"Using ETC1 compression" << Endl;
					textureFormat = TfETC1;
				}
				else
					log::info << L"Using no compression" << Endl;
			}
			else
				log::info << L"Using no compression" << Endl;
		}
		else
			log::info << L"Using no compression" << Endl;
	}

	// Data is stored in big endian as GPUs are big endian machines.
	pixelFormat = pixelFormat.endianSwapped();

	// Flip image if necessary.
	if (textureOutput->m_flipX || textureOutput->m_flipY)
	{
		drawing::MirrorFilter mirrorFilter(textureOutput->m_flipX, textureOutput->m_flipY);
		image->apply(&mirrorFilter);
	}

	// Generate sphere map from cube map.
	if (textureOutput->m_textureType == TtCube && textureOutput->m_generateSphereMap)
	{
		log::info << L"Generating sphere map..." << Endl;
		SphereMapFilter sphereMapFilter;
		image->apply(&sphereMapFilter);
	}

	// Convert into linear gamma, do it before we're converting image
	// format as it's possible source image has float format thus
	// resulting in greater accuracy.
	if (
		!textureOutput->m_enableNormalMapCompression &&
		!textureOutput->m_linearGamma &&
		std::abs(m_gamma - 1.0f) > FUZZY_EPSILON
	)
	{
		if (m_sRGB)
			sRGB = true;
		else
		{
			log::info << L"Converting into linear gamma..." << Endl;
			drawing::GammaFilter gammaFilter(m_gamma);
			image->apply(&gammaFilter);
		}
	}

	// Multiply with alpha.
	if (textureOutput->m_premultiplyAlpha)
	{
		log::info << L"Pre-multiply with alpha..." << Endl;
		drawing::PremultiplyAlphaFilter preAlphaFilter;
		image->apply(&preAlphaFilter);
	}

	// Convert image into proper format.
	image->convert(pixelFormat);

	// Generate normal map from image.
	if (textureOutput->m_generateNormalMap)
	{
		log::info << L"Generating normal map..." << Endl;
		drawing::NormalMapFilter filter(textureOutput->m_scaleDepth);
		image->apply(&filter);
		isNormalMap = true;
	}

	// Inverse normal map Y; assume it's a normal map to begin with.
	if (textureOutput->m_inverseNormalMapY)
	{
		log::info << L"Converting normal map..." << Endl;
		drawing::TransformFilter transformFilter(Color4f(1.0f, -1.0f, 1.0f, 1.0f), Color4f(0.0f, 1.0f, 0.0f, 0.0f));
		image->apply(&transformFilter);
		isNormalMap = true;
	}

	// Ensure normal map isn't using sRGB texture.
	if (isNormalMap)
		sRGB = false;

	Ref< drawing::ChainFilter > mipFilters;

	// Swizzle channels to prepare for DXT5nm compression.
	if (
		m_compressionMethod == CmDXTn &&
		textureOutput->m_enableNormalMapCompression
	)
	{
		log::info << L"Preparing for DXT5nm compression..." << Endl;

		mipFilters = new drawing::ChainFilter();

		// Inverse X axis; do it here instead of in shader.
		mipFilters->add(new drawing::TransformFilter(Color4f(-1.0f, 1.0f, 1.0f, 1.0f), Color4f(1.0f, 0.0f, 0.0f, 0.0f)));

		// [rgba] -> [0,g,0,r] (or [a,g,0,r] if we cannot ignore alpha)
		mipFilters->add(new drawing::SwizzleFilter(textureOutput->m_ignoreAlpha ? L"0g0r" : L"ag0r"));

		if (!textureOutput->m_ignoreAlpha)
			log::warning << L"Kept source alpha in red channel; compressed normals might have severe artifacts" << Endl;

		isNormalMap = true;
	}

	// Rescale image.
	if (textureOutput->m_scaleImage)
	{
		width = textureOutput->m_scaleWidth;
		height = textureOutput->m_scaleHeight;
	}

	if (textureOutput->m_textureType == Tt2D)
	{
		// Skip mips on non-system textures.
		if (!textureOutput->m_systemTexture)
		{
			width = std::max(1, width >> m_skipMips);
			height = std::max(1, height >> m_skipMips);
		}

		// Ensure image size doesn't exceed clamp size.
		if (m_clampSize > 0)
		{
			if (width > m_clampSize)
			{
				height = (height * m_clampSize) / width;
				width = m_clampSize;
			}
			if (height > m_clampSize)
			{
				width = (width * m_clampSize) / height;
				height = m_clampSize;
			}
		}

		// Ensure power-of-2 textures.
		if (!isLog2(width) || !isLog2(height))
		{
			log::warning << L"Texture dimension not power-of-2; resized to nearest valid dimension" << Endl;
			
			if (nearestLog2(width) - width < width - previousLog2(width))
				width = nearestLog2(width);
			else
				width = previousLog2(width);

			if (nearestLog2(height) - height < height - previousLog2(height))
				height = nearestLog2(height);
			else
				height = previousLog2(height);
		}
	}

	// Create output instance.
	Ref< TextureResource > outputResource = new TextureResource();
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!outputInstance)
	{
		log::error << L"Unable to create output instance" << Endl;
		return false;
	}

	outputInstance->setObject(outputResource);

	// Create output data stream.
	Ref< IStream > stream = outputInstance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Unable to create texture data stream" << Endl;
		outputInstance->revert();
		return false;
	}

	int32_t dataOffsetBegin = 0, dataOffsetEnd = 0;

	if (textureOutput->m_textureType == Tt2D || textureOutput->m_generateSphereMap)
	{
		mipCount = textureOutput->m_generateMips ? log2(std::max(width, height)) + 1 : 1;
		T_ASSERT (mipCount >= 1);

		Writer writer(stream);

		writer << uint32_t(12);
		writer << int32_t(width);
		writer << int32_t(height);
		writer << int32_t(1);
		writer << int32_t(mipCount);
		writer << int32_t(textureFormat);
		writer << bool(sRGB);
		writer << uint8_t(Tt2D);
		writer << bool(true);
		writer << bool(textureOutput->m_systemTexture);

		dataOffsetBegin = stream->tell();

		Ref< IStream > streamData = new BufferedStream(new compress::DeflateStreamLzf(stream), 64 * 1024);
		Writer writerData(streamData);

		RefArray< drawing::Image > mipImages(mipCount);

		// Generate each mip level.
		{
			RefArray< ScaleTextureTask > tasks(mipCount);
			RefArray< Job > jobs(mipCount);

			// Estimate alpha coverage if desired.
			float alphaCoverage = -1.0f;
			if (textureOutput->m_preserveAlphaCoverage)
			{
				alphaCoverage = 0.0f;
				for (int32_t y = 0; y < image->getHeight(); ++y)
				{
					for (int32_t x = 0; x < image->getWidth(); ++x)
					{
						Color4f color;
						image->getPixelUnsafe(x, y, color);
						alphaCoverage += (color.getAlpha() > textureOutput->m_alphaCoverageReference) ? 1.0f : 0.0f;
					}
				}
				alphaCoverage /= float(image->getWidth() * image->getHeight());
				log::info << L"Estimated alpha coverage " << toString(alphaCoverage * 100.0f, 2) << L"%" << Endl;
			}

			// Create task for each mip level.
			for (int32_t i = 0; i < mipCount; ++i)
			{
				if (ThreadManager::getInstance().getCurrentThread()->stopped())
					break;

				int32_t mipWidth = std::max(width >> i, 1);
				int32_t mipHeight = std::max(height >> i, 1);

				log::info << L"Generating mip " << i << L" (" << mipWidth << L"*" << mipHeight << L")..." << Endl;

				if (mipWidth != image->getWidth() || mipHeight != image->getHeight() || isNormalMap || mipFilters)
				{
					// Create chain of image filters.
					Ref< drawing::ChainFilter > taskFilters = new drawing::ChainFilter();

					// First add scaling filter to desired mip size.
					taskFilters->add(new drawing::ScaleFilter(
						mipWidth,
						mipHeight,
						drawing::ScaleFilter::MnAverage,
						drawing::ScaleFilter::MgLinear,
						textureOutput->m_keepZeroAlpha
					));

					// Append sharpen filter.
					if (!isNormalMap && textureOutput->m_sharpenRadius > 0)
					{
						taskFilters->add(new drawing::SharpenFilter(
							textureOutput->m_sharpenRadius,
							textureOutput->m_sharpenStrength * (float(i) / (mipCount - 1))
						));
					}

					// Ensure each pixel is renormalized after scaling.
					if (isNormalMap)
						taskFilters->add(new drawing::NormalizeFilter());

					// Append mip filters for compression etc.
					if (mipFilters)
						taskFilters->add(mipFilters);

					Ref< ScaleTextureTask > task = new ScaleTextureTask();
					task->image = image->clone();
					task->filter = taskFilters;
					task->alphaCoverageDesired = alphaCoverage;
					task->alphaCoverageRef = textureOutput->m_alphaCoverageReference;

					if (m_generateMipsThread)
					{
						Ref< Job > job = JobManager::getInstance().add(makeFunctor(task.ptr(), &ScaleTextureTask::execute));
						T_ASSERT (job);

						tasks[i] = task;
						jobs[i] = job;
					}
					else
					{
						task->execute();

						mipImages[i] = task->image;
						T_ASSERT (mipImages[i]);
					}
				}
				else
				{
					// No need to actually process the image; it's already in the format and size we want.
					// \note We're not cloning the image as we want to keep memory usage lower, this
					// assumes the image isn't modified.
					mipImages[i] = image;
					T_ASSERT (mipImages[i]);
				}
			}

			// Gather generated mips from jobs.
			if (m_generateMipsThread)
			{
				for (size_t i = 0; i < jobs.size(); ++i)
				{
					if (!mipImages[i])
					{
						jobs[i]->wait();
						jobs[i] = 0;

						mipImages[i] = tasks[i]->image;
						T_ASSERT (mipImages[i]);

						tasks[i] = 0;
					}
				}
			}

			if (ThreadManager::getInstance().getCurrentThread()->stopped())
			{
				log::info << L"Texture pipeline terminated. Pipeline aborted." << Endl;
				return false;
			}
		}

		Ref< ICompressor > compressor;
		if (textureFormat >= TfDXT1 && textureFormat <= TfDXT5)
			compressor = new DxtnCompressor();
		else if (textureFormat >= TfPVRTC1 && textureFormat <= TfPVRTC4)
			compressor = new PvrtcCompressor();
		else if (textureFormat == TfETC1)
			compressor = new EtcCompressor();
		else
			compressor = new UnCompressor();

		compressor->compress(writerData, mipImages, textureFormat, needAlpha, m_compressionQuality);

		streamData->close();

		dataOffsetEnd = stream->tell();
	}
	else if (textureOutput->m_textureType == Tt3D)
	{
		int32_t sliceWidth = height;
		int32_t sliceHeight = height;
		int32_t sliceDepth = height;

		if (width / sliceWidth != sliceDepth)
		{
			log::error << L"3D map invalid size, width must be height * height" << Endl;
			return false;
		}

		mipCount = textureOutput->m_generateMips ? log2(height) + 1 : 1;
		T_ASSERT (mipCount >= 1);

		Writer writer(stream);

		writer << uint32_t(12);
		writer << int32_t(sliceWidth);
		writer << int32_t(sliceHeight);
		writer << int32_t(sliceDepth);
		writer << int32_t(mipCount);
		writer << int32_t(textureFormat);
		writer << bool(sRGB);
		writer << uint8_t(Tt3D);
		writer << bool(true);
		writer << bool(textureOutput->m_systemTexture);

		dataOffsetBegin = stream->tell();

		// Create data writer, use deflate compression if enabled.
		Ref< IStream > streamData = new compress::DeflateStreamLzf(stream);
		Writer writerData(streamData);

		for (int32_t slice = 0; slice < sliceDepth; ++slice)
		{
			Ref< drawing::Image > sliceImage = new drawing::Image(image->getPixelFormat(), sliceWidth, sliceHeight);

			sliceImage->copy(
				image,
				slice * sliceWidth,
				0,
				sliceWidth,
				sliceHeight
			);

			for (int32_t i = 0; i < mipCount; ++i)
			{
				int32_t mipSize = sliceHeight >> i;

				drawing::ScaleFilter mipScaleFilter(
					mipSize,
					mipSize,
					drawing::ScaleFilter::MnAverage,
					drawing::ScaleFilter::MgLinear,
					textureOutput->m_keepZeroAlpha
				);
				sliceImage->apply(&mipScaleFilter);

				uint32_t outputSize = getTextureMipPitch(
					textureFormat,
					mipSize,
					mipSize
				);

				 writerData.write(
					sliceImage->getData(),
					outputSize,
					1
				);
			}
		}

		streamData->close();

		dataOffsetEnd = stream->tell();
	}
	else if (textureOutput->m_textureType == TtCube)
	{
		uint32_t layout = 0;
		uint32_t sideSize = height;

		if (height == width / 6)
		{
			// [+x][-x][+y][-y][+z][-z]
			layout = 0;
			sideSize = height;
		}
		else if (height / 3 == width / 4)
		{
			// [  ][+y][  ][  ]
			// [-x][+z][+x][-z]
			// [  ][-y][  ][  ]
			layout = 1;
			sideSize = height / 3;
		}
		else if (height / 4 == width / 3)
		{
			// [  ][+y][  ]
			// [-x][+z][+x]
			// [  ][-y][  ]
			// [  ][-z][  ]
			layout = 2;
			sideSize = height / 4;
		}
		else
		{
			log::error << L"Cube map must have either a 6:1, 4:3 or 3:4 width/height ratio" << Endl;
			return false;
		}

		mipCount = textureOutput->m_generateMips ? log2(sideSize) + 1 : 1;
		T_ASSERT (mipCount >= 1);

		Writer writer(stream);

		writer << uint32_t(12);
		writer << int32_t(sideSize);
		writer << int32_t(sideSize);
		writer << int32_t(6);
		writer << int32_t(mipCount);
		writer << int32_t(textureFormat);
		writer << bool(sRGB);
		writer << uint8_t(TtCube);
		writer << bool(true);
		writer << bool(textureOutput->m_systemTexture);

		dataOffsetBegin = stream->tell();

		// Create data writer, use deflate compression if enabled.
		Ref< IStream > streamData = new compress::DeflateStreamLzf(stream);
		Writer writerData(streamData);

		for (int side = 0; side < 6; ++side)
		{
			Ref< drawing::Image > sideImage = new drawing::Image(image->getPixelFormat(), sideSize, sideSize);

			if (layout == 0)
				sideImage->copy(image, side * sideSize, 0, sideSize, sideSize);
			else if (layout == 1)
			{
				const int32_t c_sideOffsets[][2] =
				{
					{ 2, 1 },
					{ 0, 1 },
					{ 1, 0 },
					{ 1, 2 },
					{ 1, 1 },
					{ 3, 1 }
				};
				sideImage->copy(
					image,
					c_sideOffsets[side][0] * sideSize,
					c_sideOffsets[side][1] * sideSize,
					sideSize,
					sideSize
				);
			}
			else if (layout == 2)
			{
				const int32_t c_sideOffsets[][2] =
				{
					{ 2, 1 },
					{ 0, 1 },
					{ 1, 0 },
					{ 1, 2 },
					{ 1, 1 },
					{ 1, 3 }
				};
				sideImage->copy(
					image,
					c_sideOffsets[side][0] * sideSize,
					c_sideOffsets[side][1] * sideSize,
					sideSize,
					sideSize
				);
				if (side == 5)
				{
					// Flip -Z as it's defined up-side down in this layout.
					drawing::MirrorFilter filter(true, true);
					sideImage->apply(&filter);
				}
			}

			RefArray< drawing::Image > mipImages(mipCount);

			// Generate each mip level.
			for (int i = 0; i < mipCount; ++i)
			{
				int mipSize = sideSize >> i;

				drawing::ScaleFilter mipScaleFilter(
					mipSize,
					mipSize,
					drawing::ScaleFilter::MnAverage,
					drawing::ScaleFilter::MgLinear,
					textureOutput->m_keepZeroAlpha
				);
				sideImage->apply(&mipScaleFilter);

				mipImages[i] = sideImage->clone();
			}

			Ref< ICompressor > compressor;
			if (textureFormat >= TfDXT1 && textureFormat <= TfDXT5)
				compressor = new DxtnCompressor();
			else if (textureFormat >= TfPVRTC1 && textureFormat <= TfPVRTC4)
				compressor = new PvrtcCompressor();
			else if (textureFormat == TfETC1)
				compressor = new EtcCompressor();
			else
				compressor = new UnCompressor();

			compressor->compress(writerData, mipImages, textureFormat, needAlpha, m_compressionQuality);
		}

		streamData->close();

		dataOffsetEnd = stream->tell();
	}

	stream->close();

	if (!outputInstance->commit())
	{
		log::error << L"Unable to commit output instance" << Endl;
		return false;
	}

	// Create report.
	Ref< editor::IPipelineReport > report = pipelineBuilder->createReport(L"Texture", outputGuid);
	if (report)
	{
		report->set(L"path", outputPath);
		report->set(L"width", width);
		report->set(L"height", height);
		report->set(L"mipCount", mipCount);
		report->set(L"format", int32_t(textureFormat));
		report->set(L"dataSize", dataOffsetEnd - dataOffsetBegin);
	}

	return true;
}

Ref< ISerializable > TextureOutputPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
