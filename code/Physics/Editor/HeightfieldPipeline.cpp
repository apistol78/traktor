#include "Physics/Editor/HeightfieldPipeline.h"
#include "Physics/Editor/HeightfieldAsset.h"
#include "Physics/HeightfieldResource.h"
#include "Physics/Heightfield.h"
#include "Editor/IPipelineManager.h"
#include "Editor/Settings.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ConvolutionFilter.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Stream.h"
#include "Core/Io/Writer.h"
#include "Core/Math/Const.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

drawing::PixelFormat s_pfRaw16(16, 16, 0, 0, 0, 0, 0, 0, 0, false, false);

int log2(int v)
{
	for (int i = 31; i >= 0; --i)
		if (v & (1 << i))
			return i;
	return 0;
}

drawing::Image* readRawTerrain(const Path& fileName)
{
	Ref< Stream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!file)
		return 0;

	uint32_t fileSize = file->available();

	const uint32_t heightByteSize = 2;

	uint32_t heights = fileSize / heightByteSize;
	uint32_t size = uint32_t(std::sqrt(float(heights)));

	Ref< drawing::Image > image = gc_new< drawing::Image >(
		&s_pfRaw16,
		size,
		size
	);

	file->read(image->getData(), fileSize);
	file->close();

	return image;
}

		}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.HeightfieldPipeline", HeightfieldPipeline, editor::IPipeline)

bool HeightfieldPipeline::create(const editor::Settings* settings)
{
	m_assetPath = settings->getProperty< editor::PropertyString >(L"Pipeline.AssetPath", L"");
	return true;
}

void HeightfieldPipeline::destroy()
{
}

uint32_t HeightfieldPipeline::getVersion() const
{
	return 1;
}

TypeSet HeightfieldPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< HeightfieldAsset >());
	return typeSet;
}

bool HeightfieldPipeline::buildDependencies(
	editor::IPipelineManager* pipelineManager,
	const db::Instance* sourceInstance,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	const HeightfieldAsset* heightfieldAsset = checked_type_cast< const HeightfieldAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, heightfieldAsset->getFileName());
	pipelineManager->addDependency(fileName);
	return true;
}

bool HeightfieldPipeline::buildOutput(
	editor::IPipelineManager* pipelineManager,
	const Serializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	const HeightfieldAsset* heightfieldAsset = checked_type_cast< const HeightfieldAsset* >(sourceAsset);
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
		image->convert(&s_pfRaw16);

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
	Ref< HeightfieldResource > resource = gc_new< HeightfieldResource >(
		image->getWidth(),
		cref(heightfieldAsset->m_worldExtent)
	);

	// Create instance's name.
	Ref< db::Instance > instance = pipelineManager->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
	{
		log::error << L"Failed to build heightfield, unable to create instance" << Endl;
		return false;
	}

	instance->setObject(resource);

	Ref< Stream > stream = instance->writeData(L"Data");
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
