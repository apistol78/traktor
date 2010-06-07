#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Log2.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ConvolutionFilter.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Physics/Heightfield.h"
#include "Physics/HeightfieldResource.h"
#include "Physics/Editor/HeightfieldPipeline.h"
#include "Physics/Editor/HeightfieldAsset.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

drawing::PixelFormat s_pfRaw16(16, 16, 0, 0, 0, 0, 0, 0, 0, false, false);

Ref< drawing::Image > readRawTerrain(const Path& fileName)
{
	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!file)
		return 0;

	uint32_t fileSize = file->available();

	const uint32_t heightByteSize = 2;

	uint32_t heights = fileSize / heightByteSize;
	uint32_t size = uint32_t(std::sqrt(float(heights)));

	Ref< drawing::Image > image = new drawing::Image(
		s_pfRaw16,
		size,
		size
	);

	file->read(image->getData(), fileSize);
	file->close();

	return image;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.HeightfieldPipeline", 1, HeightfieldPipeline, editor::IPipeline)

bool HeightfieldPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	return true;
}

void HeightfieldPipeline::destroy()
{
}

TypeInfoSet HeightfieldPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< HeightfieldAsset >());
	return typeSet;
}

bool HeightfieldPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	Ref< const HeightfieldAsset > heightfieldAsset = checked_type_cast< const HeightfieldAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, heightfieldAsset->getFileName());
	pipelineDepends->addDependency(fileName);
	return true;
}

bool HeightfieldPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< const HeightfieldAsset > heightfieldAsset = checked_type_cast< const HeightfieldAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, heightfieldAsset->getFileName());
		
	// Load source image.
	Ref< drawing::Image > image;
	if (fileName.getExtension() == L"raw")
	{
		// Load 16-raw file.
		image = readRawTerrain(fileName);
	}
	else
	{
		// Load source image.
		image = drawing::Image::load(fileName);
		image->convert(s_pfRaw16);

		// Smooth image, hack to filter 8-bit heights into 16-bit.
		image = image->applyFilter(drawing::ConvolutionFilter::createGaussianBlur());
	}
	if (!image)
		return false;

	uint32_t size = image->getWidth();
	if (size != (1 << log2(size)))
	{
		log::error << L"Heightfield must have log2 size" << Endl;
		return false;
	}

	// Create heightfield resource.
	Ref< HeightfieldResource > resource = new HeightfieldResource(
		image->getWidth(),
		heightfieldAsset->m_worldExtent
	);

	// Create instance's name.
	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
	{
		log::error << L"Failed to build heightfield, unable to create instance" << Endl;
		return false;
	}

	instance->setObject(resource);

	Ref< IStream > stream = instance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Failed to build heightfield, unable to create data stream" << Endl;
		instance->revert();
		return false;
	}

	// Convert image pixels into heights.
	Writer writer(stream);
	for (int32_t y = 0; y < image->getWidth(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			drawing::Color imagePixel;
			image->getPixel(x, y, imagePixel);
			writer << float(imagePixel.getRed() * 2.0f - 1.0f);
		}
	}

	stream->close();
	return instance->commit();
}

	}
}
