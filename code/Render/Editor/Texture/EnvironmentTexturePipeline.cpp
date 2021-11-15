#include "Compress/Lzf/DeflateStreamLzf.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.EnvironmentTexturePipeline", 4, EnvironmentTexturePipeline, editor::DefaultPipeline)

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

	log::info << L"Building environment texture, " << sideSize << L"..." << Endl;

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

	for (int32_t side = 0; side < 6; ++side)
	{
		RefArray< drawing::Image > mipImages(mipCount);
		for (int32_t i = 0; i < mipCount; ++i)
		{
			int32_t mipSize = sideSize >> i;

			mipImages[i] = new drawing::Image(
				pixelFormat,
				mipSize,
				mipSize
			);

			for (int32_t y = 0; y < mipSize; ++y)
			{
				for (int32_t x = 0; x < mipSize; ++x)
				{
					Vector4 direction = cubeMap->getDirection(side, x << i, y << i);
					Color4f c = cubeMap->get(direction);
					
					// \fixme Implement roughness filtering.

					mipImages[i]->setPixelUnsafe(x, y, c);
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
