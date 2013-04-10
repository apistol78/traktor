#include <limits>
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Weather/Clouds/CloudMaskResource.h"
#include "Weather/Editor/Clouds/CloudMaskPipeline.h"
#include "Weather/Editor/Clouds/CloudMaskAsset.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.weather.CloudMaskPipeline", 1, CloudMaskPipeline, editor::IPipeline)

bool CloudMaskPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	return true;
}

void CloudMaskPipeline::destroy()
{
}

TypeInfoSet CloudMaskPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< CloudMaskAsset >());
	return typeSet;
}

bool CloudMaskPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const CloudMaskAsset* maskAsset = checked_type_cast< const CloudMaskAsset* >(sourceAsset);
	pipelineDepends->addDependency(Path(m_assetPath), maskAsset->getFileName().getOriginal());
	return true;
}

bool CloudMaskPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	const CloudMaskAsset* maskAsset = checked_type_cast< const CloudMaskAsset* >(sourceAsset);

	Ref< IStream > file = pipelineBuilder->openFile(Path(m_assetPath), maskAsset->getFileName().getOriginal());
	if (!file)
	{
		log::error << L"Cloud mask pipeline failed; unable to open source image \"" << maskAsset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	Ref< drawing::Image > image = drawing::Image::load(file, maskAsset->getFileName().getExtension());
	if (!image)
	{
		log::error << L"Cloud mask pipeline failed; unable to load material mask source image \"" << maskAsset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	if (image->getWidth() != image->getHeight())
	{
		log::error << L"Cloud mask pipeline failed; source image must be square" << Endl;
		return false;
	}

	uint32_t size = image->getWidth();

	// Create mask resource.
	Ref< CloudMaskResource > resource = new CloudMaskResource(size);

	// Create instance's name.
	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
	{
		log::error << L"Cloud mask pipeline failed; unable to create instance" << Endl;
		return false;
	}

	instance->setObject(resource);

	Ref< IStream > stream = instance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Cloud mask pipeline failed; unable to create data stream" << Endl;
		instance->revert();
		return false;
	}

	// Convert image pixels into mask values.
	Writer writer(stream);
	for (uint32_t y = 0; y < size; ++y)
	{
		for (uint32_t x = 0; x < size; ++x)
		{
			Color4f imagePixel;
			image->getPixel(x, y, imagePixel);

			uint8_t opacity = uint8_t(imagePixel.getRed() * 255);
			uint8_t size = uint8_t(imagePixel.getGreen() * 255);

			writer << opacity;
			writer << size;
		}
	}

	stream->close();
	return instance->commit();
}

Ref< ISerializable > CloudMaskPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
