#include <cstring>
#include "Compress/Zip/DeflateStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Core/Math/Vector4.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/JobManager.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/GammaFilter.h"
#include "Drawing/Filters/NormalMapFilter.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Drawing/Filters/SwizzleFilter.h"
#include "Drawing/Filters/TransformFilter.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineReport.h"
#include "Editor/IPipelineSettings.h"
#include "Render/Types.h"
#include "Render/Editor/Texture/DxtnCompressor.h"
#include "Render/Editor/Texture/SphereMapFilter.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Editor/Texture/TexturePipeline.h"
#include "Render/Editor/Texture/UnCompressor.h"
#include "Render/Resource/TextureResource.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

bool isBinaryAlpha(const drawing::Image* image)
{
	std::set< uint8_t > alphas;
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			Color4f color;
			image->getPixel(x, y, color);

			uint8_t alpha = uint8_t(color.getAlpha() * 255.0f);
			alphas.insert(alpha);
			if (alphas.size() > 2)
				return false;
		}
	}
	return true;
}

struct ScaleTextureTask : public Object
{
	Ref< const drawing::Image > image;
	Ref< drawing::ScaleFilter > filter;
	Ref< drawing::Image > output;

	void execute()
	{
		output = image->applyFilter(filter);
		T_ASSERT (output);
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TexturePipeline", 12, TexturePipeline, editor::IPipeline)

TexturePipeline::TexturePipeline()
:	m_skipMips(0)
,	m_clampSize(0)
,	m_allowCompression(true)
,	m_compressionQuality(1)
{
}

bool TexturePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	m_skipMips = settings->getProperty< PropertyInteger >(L"TexturePipeline.SkipMips", 0);
	m_clampSize = settings->getProperty< PropertyInteger >(L"TexturePipeline.ClampSize", 0);
	m_allowCompression = settings->getProperty< PropertyBoolean >(L"TexturePipeline.AllowCompression", true);
	m_compressionQuality = settings->getProperty< PropertyInteger >(L"TexturePipeline.CompressionQuality", 1);
	return true;
}

void TexturePipeline::destroy()
{
}

TypeInfoSet TexturePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TextureAsset >());
	return typeSet;
}

bool TexturePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	Ref< const TextureAsset > textureAsset = checked_type_cast< const TextureAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, textureAsset->getFileName());
	pipelineDepends->addDependency(fileName);
	return true;
}

bool TexturePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< const TextureAsset > textureAsset = checked_type_cast< const TextureAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, textureAsset->getFileName());

	// Load source image.
	Ref< drawing::Image > image = drawing::Image::load(fileName);
	if (!image)
	{
		log::error << L"Unable to read source texture image \"" << fileName.getPathName() << L"\"" << Endl;
		return false;
	}

	int32_t width = image->getWidth();
	int32_t height = image->getHeight();
	int32_t mipCount = 1;

	bool needAlpha = 
		(image->getPixelFormat().getAlphaBits() > 0 && !textureAsset->m_ignoreAlpha) ||
		textureAsset->m_enableNormalMapCompression;

	// Determine pixel and texel formats.
	drawing::PixelFormat pixelFormat;
	TextureFormat textureFormat;

	if (needAlpha)
	{
#if defined(T_LITTLE_ENDIAN)
		pixelFormat = drawing::PixelFormat::getA8B8G8R8();
#else
		pixelFormat = drawing::PixelFormat::getR8G8B8A8();
#endif
		textureFormat = TfR8G8B8A8;
	}
	else
	{
#if defined(T_LITTLE_ENDIAN)
		pixelFormat = drawing::PixelFormat::getX8B8G8R8();
#else
		pixelFormat = drawing::PixelFormat::getR8G8B8X8();
#endif
		textureFormat = TfR8G8B8A8;
	}

	// Generate sphere map from cube map.
	if (textureAsset->m_isCubeMap && textureAsset->m_generateSphereMap)
	{
		log::info << L"Generating sphere map..." << Endl;
		SphereMapFilter sphereMapFilter;
		image = image->applyFilter(&sphereMapFilter);
	}

	// Convert into linear gamma, do it before we're converting image
	// format as it's possible source image has float format thus
	// resulting in greater accuracy.
	if (!textureAsset->m_linearGamma)
	{
		log::info << L"Converting into linear gamma..." << Endl;
		drawing::GammaFilter gammaFilter(1.0f / 2.2f);
		image = image->applyFilter(&gammaFilter);
	}

	// Convert image into proper format.
	image->convert(pixelFormat);

	// Generate normal map from image.
	if (textureAsset->m_generateNormalMap)
	{
		log::info << L"Generating normal map..." << Endl;
		drawing::NormalMapFilter filter(textureAsset->m_scaleDepth);
		image = image->applyFilter(&filter);
	}

	// Inverse normal map Y; assume it's a normal map to begin with.
	if (textureAsset->m_inverseNormalMapY)
	{
		log::info << L"Converting normal map..." << Endl;
		drawing::TransformFilter transformFilter(Color4f(1.0f, -1.0f, 1.0f, 1.0f), Color4f(0.0f, 1.0f, 0.0f, 0.0f));
		image = image->applyFilter(&transformFilter);
	}

	// Swizzle channels to prepare for DXT5nm compression.
	if (m_allowCompression && textureAsset->m_enableNormalMapCompression)
	{
		log::info << L"Preparing for DXT5nm compression..." << Endl;

		// Inverse X axis; do it here instead of in shader.
		drawing::TransformFilter transformFilter(Color4f(-1.0f, 1.0f, 1.0f, 1.0f), Color4f(1.0f, 0.0f, 0.0f, 0.0f));
		image = image->applyFilter(&transformFilter);

		// [rgba] -> [0,g,0,r] (or [a,g,0,r] if we cannot ignore alpha)
		drawing::SwizzleFilter swizzleFilter(textureAsset->m_ignoreAlpha ? L"0g0r" : L"ag0r");
		image = image->applyFilter(&swizzleFilter);

		if (!textureAsset->m_ignoreAlpha)
			log::warning << L"Kept source alpha in red channel; compressed normals might have artifacts" << Endl;
	}

	// Rescale image.
	if (textureAsset->m_scaleImage)
	{
		width = textureAsset->m_scaleWidth;
		height = textureAsset->m_scaleHeight;
	}

	// Skip mips.
	width = std::max(1, width >> m_skipMips);
	height = std::max(1, height >> m_skipMips);

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
	if (!textureAsset->m_isCubeMap && (!isLog2(width) || !isLog2(height)))
	{
		log::warning << L"Texture dimension not power-of-2; resized to nearest valid dimension" << Endl;
		width = nearestLog2(width);
		height = nearestLog2(height);
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

	if (!textureAsset->m_isCubeMap || textureAsset->m_generateSphereMap)
	{
		mipCount = textureAsset->m_generateMips ? log2(std::max(width, height)) + 1 : 1;
		T_ASSERT (mipCount >= 1);

		// Determine texture compression format.
		if (
			m_allowCompression &&
			(textureAsset->m_enableCompression || textureAsset->m_enableNormalMapCompression) &&
			isLog2(width) &&
			isLog2(height)
		)
		{
			if (textureAsset->m_enableNormalMapCompression)
			{
				log::info << L"Using DXT5nm compression" << Endl;
				textureFormat = TfDXT5;
			}
			else
			{
				bool binaryAlpha = false;
				if (textureAsset->m_hasAlpha && !textureAsset->m_ignoreAlpha)
					binaryAlpha = isBinaryAlpha(image);
				if (needAlpha && !binaryAlpha)
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
		else
			log::info << L"Using no compression" << Endl;

		Writer writer(stream);

		writer << uint32_t(5);
		writer << int32_t(width);
		writer << int32_t(height);
		writer << int32_t(mipCount);
		writer << int32_t(textureFormat);
		writer << bool(false);
		writer << bool(true);

		dataOffsetBegin = stream->tell();

		Ref< IStream > streamData = new compress::DeflateStream(stream);
		Writer writerData(streamData);

		RefArray< drawing::Image > mipImages(mipCount);

		// Generate each mip level.
		{
			RefArray< ScaleTextureTask > tasks;
			RefArray< Job > jobs;

			log::info << L"Executing mip generation task(s)..." << Endl;

			for (int32_t i = 0; i < mipCount; ++i)
			{
				int32_t mipWidth = std::max(width >> i, 1);
				int32_t mipHeight = std::max(height >> i, 1);

				log::info << L"Executing mip generation task " << i << L" (" << mipWidth << L"*" << mipHeight << L")..." << Endl;

				Ref< ScaleTextureTask > task = new ScaleTextureTask();

				task->image = image;
				task->filter = new drawing::ScaleFilter(
						mipWidth,
						mipHeight,
						drawing::ScaleFilter::MnAverage,
						drawing::ScaleFilter::MgLinear,
						textureAsset->m_keepZeroAlpha
					);

				Ref< Job > job = JobManager::getInstance().add(makeFunctor(task.ptr(), &ScaleTextureTask::execute));

				tasks.push_back(task);
				jobs.push_back(job);
			}

			log::info << L"Collecting task(s)..." << Endl;

			for (size_t i = 0; i < jobs.size(); ++i)
			{
				jobs[i]->wait();
				jobs[i] = 0;

				mipImages[i] = tasks[i]->output;
				T_ASSERT (mipImages[i]);

				tasks[i] = 0;
			}

			log::info << L"All task(s) collected" << Endl;
		}

		Ref< ICompressor > compressor;
		if (textureFormat == TfDXT1 || textureFormat == TfDXT3 || textureFormat == TfDXT5)
			compressor = new DxtnCompressor();
		else
			compressor = new UnCompressor();

		compressor->compress(writerData, mipImages, textureFormat, needAlpha, m_compressionQuality);

		streamData->close();

		dataOffsetEnd = stream->tell();
	}
	else
	{
		uint32_t sideSize = height;
		if (sideSize != width / 6)
		{
			log::error << L"Cube map must have a 6:1 width/height ratio" << Endl;
			return false;
		}

		mipCount = textureAsset->m_generateMips ? log2(sideSize) + 1 : 1;
		T_ASSERT (mipCount >= 1);

		Writer writer(stream);

		writer << uint32_t(5);
		writer << int32_t(sideSize);
		writer << int32_t(sideSize);
		writer << int32_t(mipCount);
		writer << int32_t(textureFormat);
		writer << bool(true);
		writer << bool(true);

		dataOffsetBegin = stream->tell();

		// Create data writer, use deflate compression if enabled.
		Ref< IStream > streamData = new compress::DeflateStream(stream);
		Writer writerData(streamData);

		for (int side = 0; side < 6; ++side)
		{
			Ref< drawing::Image > sideImage = new drawing::Image(image->getPixelFormat(), sideSize, sideSize);
			sideImage->copy(image, side * sideSize, 0, sideSize, sideSize);

			for (int i = 0; i < mipCount; ++i)
			{
				int mipSize = sideSize >> i;

				drawing::ScaleFilter mipScaleFilter(
					mipSize,
					mipSize,
					drawing::ScaleFilter::MnAverage,
					drawing::ScaleFilter::MgLinear,
					textureAsset->m_keepZeroAlpha
				);
				Ref< drawing::Image > mipImage = sideImage->applyFilter(&mipScaleFilter);
				T_ASSERT (mipImage);

				 writerData.write(
					mipImage->getData(),
					mipSize * mipSize,
					sizeof(uint32_t)
				);
			}
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

	}
}
