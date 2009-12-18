#include <cstring>
#include <squish.h>
#include "Core/Io/FileSystem.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Thread/JobManager.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/GammaFilter.h"
#include "Drawing/Filters/NormalMapFilter.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineReport.h"
#include "Editor/IPipelineSettings.h"
#include "Render/TextureResource.h"
#include "Render/Types.h"
#include "Render/Editor/TextureAsset.h"
#include "Render/Editor/TexturePipeline.h"
#include "Zip/DeflateStream.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

int log2(int v)
{
	for (int i = 32; i >= 0; --i)
		if (v & (1 << i))
			return i;
	return 0;
}

bool isLog2(int v)
{
	return (1 << log2(v)) == v;
}


uint32_t nearestLog2(uint32_t num)
{
    uint32_t n = num > 0 ? num - 1 : 0;

    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;

    return n;
}

uint32_t previousLog2(uint32_t num)
{
	uint32_t lg2 = nearestLog2(num);
	return lg2 > 1 ? (lg2 >> 1) : 1;
}

bool isBinaryAlpha(const drawing::Image* image)
{
	std::set< uint8_t > alphas;
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			drawing::Color color;
			image->getPixel(x, y, color);

			uint8_t alpha = uint8_t(color.getAlpha() * 255.0f);
			alphas.insert(alpha);
			if (alphas.size() > 2)
				return false;
		}
	}
	return true;
}

struct ScaleTextureTask
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

struct CompressTextureTask
{
	const drawing::Image* image;
	int top;
	int bottom;
	TextureFormat textureFormat;
	bool hasAlpha;
	std::vector< uint8_t > output;

	void execute()
	{
		int width = image->getWidth();
		int height = image->getHeight();

		uint32_t outputSize = getTextureMipPitch(
			textureFormat,
			width,
			bottom - top
		);

		output.resize(outputSize);
		std::memset(&output[0], 0, outputSize);

		if (textureFormat == TfDXT1 || textureFormat == TfDXT3)
		{
			const uint8_t* data = static_cast< const uint8_t* >(image->getData());
			uint8_t* block = &output[0];

			for (int y = top; y < bottom; y += 4)
			{
				for (int x = 0; x < width; x += 4)
				{
					uint8_t rgba[4][4][4];
					int mask = 0;

					for (int iy = 0; iy < 4; ++iy)
					{
						for (int ix = 0; ix < 4; ++ix)
						{
							int sx = x + ix;
							int sy = y + iy;

							if (sx >= width || sy >= height)
								continue;

							uint32_t offset = (sx + sy * image->getWidth()) * 4;
							rgba[iy][ix][0] = data[offset + 0];
							rgba[iy][ix][1] = data[offset + 1];
							rgba[iy][ix][2] = data[offset + 2];
							rgba[iy][ix][3] = hasAlpha ? data[offset + 3] : 0xff;

							mask |= 1 << (ix + iy * 4);
						}
					}

					squish::CompressMasked(
						(const squish::u8*)rgba,
						mask,
						block,
						textureFormat == TfDXT1 ? squish::kDxt1 : squish::kDxt3
					);

					block += getTextureBlockSize(textureFormat);
				}
			}
		}
		else
		{
			const uint8_t* data = static_cast< const uint8_t* >(image->getData()) + top * image->getWidth() * 4;
			std::memcpy(&output[0], data, outputSize);
		}
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TexturePipeline", 7, TexturePipeline, editor::IPipeline)

TexturePipeline::TexturePipeline()
:	m_skipMips(0)
,	m_clampSize(0)
,	m_allowCompression(true)
{
}

bool TexturePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< editor::PropertyString >(L"Pipeline.AssetPath", L"");
	m_skipMips = settings->getProperty< editor::PropertyInteger >(L"TexturePipeline.SkipMips", 0);
	m_clampSize = settings->getProperty< editor::PropertyInteger >(L"TexturePipeline.ClampSize", 0);
	m_allowCompression = settings->getProperty< editor::PropertyBoolean >(L"TexturePipeline.AllowCompression", true);
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
	int32_t dataSize = 0;
	bool hasAlpha = image->getPixelFormat().getAlphaBits() > 0 && !textureAsset->m_ignoreAlpha;

	// Determine pixel and texel formats.
	drawing::PixelFormat pixelFormat;
	TextureFormat textureFormat;

	if (hasAlpha)
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

		Ref< drawing::Image > sphereImage = new drawing::Image(pixelFormat, height, height);

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < height; ++x)
			{
				float t = 2.0f * (float(y) / float(height - 1.0f) - 0.5f);
				float s = 2.0f * (float(x) / float(height - 1.0f) - 0.5f);

				if (s * s + t * t > 1.0f)
					continue;

				int offset = 0;
				int slicex = 0;
				int slicey = 0;
				float vsign = 0;

				Vector4 direction(s, t, sqrtf(1.0f - s * s - t * t), 0.0f);
				Vector4 reflection(
					direction.x() * direction.z() * 2.0f,
					direction.y() * direction.z() * 2.0f,
					direction.z() * direction.z() * 2.0f - 1.0f
				);

				switch (majorAxis3(reflection))
				{
				case 0:
					vsign = -sign(float(reflection.x()));
					offset = reflection.x() < 0.0f ? 0 : height;
					reflection = reflection * (Scalar(1.0f) / -reflection.x());
					slicex = int((reflection.z() * -0.5f + 0.5f) * (height - 1));
					slicey = int((reflection.y() * 0.5f * vsign + 0.5f) * (height - 1));
					break;

				case 1:
					vsign = sign(float(reflection.y()));
					offset = reflection.y() < 0.0f ? height * 2 : height * 3;
					reflection = reflection * (Scalar(1.0f) / -reflection.y());
					slicex = int((reflection.x() * 0.5f * vsign + 0.5f) * (height - 1));
					slicey = int((reflection.z() * 0.5f + 0.5f) * (height - 1));
					break;

				case 2:
					vsign = -sign(float(reflection.z()));
					offset = reflection.z() > 0.0f ? height * 4 : height * 5;
					reflection = reflection * (Scalar(1.0f) / -reflection.z());
					slicex = int((reflection.x() * 0.5f + 0.5f) * (height - 1));
					slicey = int((reflection.y() * 0.5f * vsign + 0.5f) * (height - 1));
					break;
				}

				drawing::Color color;
				if (image->getPixel(slicex + offset, slicey, color))
					sphereImage->setPixel(x, y, color);
			}
		}

		image = sphereImage;
	}

	// Convert into linear gamma, do it before we're converting image
	// format as it's possible source image has float format thus
	// resulting in greater accuracy.
	if (!textureAsset->m_linearGamma)
	{
		drawing::GammaFilter gammaFilter(1.0f / 2.2f);
		image = image->applyFilter(&gammaFilter);
	}

	// Convert image into proper format.
	image->convert(pixelFormat);

	// Generate normal map from image.
	if (textureAsset->m_generateNormalMap)
	{
		drawing::NormalMapFilter filter(textureAsset->m_scaleDepth);
		image = image->applyFilter(&filter);
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

	Ref< IStream > stream = outputInstance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Unable to create texture data stream" << Endl;
		outputInstance->revert();
		return false;
	}

	if (!textureAsset->m_isCubeMap || textureAsset->m_generateSphereMap)
	{
		mipCount = textureAsset->m_generateMips ? log2(std::max(width, height)) + 1 : 1;
		T_ASSERT (mipCount >= 1);

		// Determine texture compression format.
		if (m_allowCompression && textureAsset->m_enableCompression && isLog2(width) && isLog2(height))
		{
			bool binaryAlpha = isBinaryAlpha(image);
			if (hasAlpha && !binaryAlpha)
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
		else
			log::info << L"Using no compression" << Endl;

		Writer writer(stream);

		writer << uint32_t(4);
		writer << int32_t(width);
		writer << int32_t(height);
		writer << int32_t(mipCount);
		writer << int32_t(textureFormat);
		writer << bool(false);
		writer << bool(true);

		Ref< IStream > streamData = new zip::DeflateStream(stream);
		Writer writerData(streamData);

		RefArray< drawing::Image > mipImages(mipCount);

		// Generate each mip level.
		{
			std::vector< ScaleTextureTask* > tasks;
			std::vector< Job* > jobs;

			log::info << L"Executing mip generation task(s)..." << Endl;

			for (int32_t i = 0; i < mipCount; ++i)
			{
				int32_t mipWidth = std::max(width >> i, 1);
				int32_t mipHeight = std::max(height >> i, 1);

				log::info << L"Executing mip generation task " << i << L" (" << mipWidth << L"*" << mipHeight << L")..." << Endl;

				ScaleTextureTask* task = new ScaleTextureTask();

				task->image = image;
				task->filter = new drawing::ScaleFilter(
						mipWidth,
						mipHeight,
						drawing::ScaleFilter::MnAverage,
						drawing::ScaleFilter::MgLinear,
						textureAsset->m_keepZeroAlpha
					);

				Job* job = new Job(makeFunctor(task, &ScaleTextureTask::execute));
				JobManager::getInstance().add(*job);

				tasks.push_back(task);
				jobs.push_back(job);
			}

			log::info << L"Collecting task(s)..." << Endl;

			for (size_t i = 0; i < jobs.size(); ++i)
			{
				jobs[i]->wait();

				mipImages[i] = tasks[i]->output;
				T_ASSERT (mipImages[i]);

				delete tasks[i];
				delete jobs[i];
			}

			log::info << L"All task(s) collected" << Endl;
		}

		// Create multiple jobs for compressing mips; split big mips into several jobs.
		{
			std::vector< CompressTextureTask* > tasks;
			std::vector< Job* > jobs;

			for (int32_t i = 0; i < mipCount; ++i)
			{
				int32_t height = mipImages[i]->getHeight();

				int32_t split = height / 32;
				if (split < 1)
					split = 1;

				log::info << L"Executing mip compression " << i << L" in " << split << L" task(s)..." << Endl;

				for (int32_t j = 0; j < split; ++j)
				{
					CompressTextureTask* task = new CompressTextureTask();
					task->image = mipImages[i];
					task->top = (height * j) / split;
					task->bottom = (height * (j + 1)) / split;
					task->textureFormat = textureFormat;
					task->hasAlpha = hasAlpha;

					Job* job = new Job(makeFunctor(task, &CompressTextureTask::execute));
					JobManager::getInstance().add(*job);

					tasks.push_back(task);
					jobs.push_back(job);
				}
			}

			log::info << L"Collecting task(s)..." << Endl;

			for (size_t i = 0; i < jobs.size(); ++i)
			{
				jobs[i]->wait();
				dataSize += writerData.write(&tasks[i]->output[0], uint32_t(tasks[i]->output.size()), 1);

				delete tasks[i];
				delete jobs[i];
			}

			log::info << L"All task(s) collected" << Endl;
		}

		streamData->close();
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

		writer << uint32_t(4);
		writer << int32_t(sideSize);
		writer << int32_t(sideSize);
		writer << int32_t(mipCount);
		writer << int32_t(textureFormat);
		writer << bool(true);
		writer << bool(true);

		// Create data writer, use deflate compression if enabled.
		Ref< IStream > streamData = new zip::DeflateStream(stream);
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

				dataSize += writerData.write(
					mipImage->getData(),
					mipSize * mipSize,
					sizeof(unsigned int)
				);
			}
		}

		streamData->close();
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
		report->set(L"width", width);
		report->set(L"height", height);
		report->set(L"mipCount", mipCount);
		report->set(L"format", int32_t(textureFormat));
		report->set(L"dataSize", dataSize);
	}

	return true;
}

	}
}
