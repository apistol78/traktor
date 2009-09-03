#include <limits>
#include "Terrain/Editor/MaterialMaskPipeline.h"
#include "Terrain/Editor/MaterialMaskAsset.h"
#include "Terrain/MaterialMaskResource.h"
#include "Editor/IPipelineManager.h"
#include "Drawing/Image.h"
#include "Database/Instance.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.terrain.MaterialMaskPipeline", MaterialMaskPipeline, editor::IPipeline)

bool MaterialMaskPipeline::create(const editor::Settings* settings)
{
	return true;
}

void MaterialMaskPipeline::destroy()
{
}

uint32_t MaterialMaskPipeline::getVersion() const
{
	return 1;
}

TypeSet MaterialMaskPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< MaterialMaskAsset >());
	return typeSet;
}

bool MaterialMaskPipeline::buildDependencies(
	editor::IPipelineManager* pipelineManager,
	const db::Instance* sourceInstance,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	const MaterialMaskAsset* maskAsset = checked_type_cast< const MaterialMaskAsset* >(sourceAsset);
	pipelineManager->addDependency(maskAsset->getFileName());
	return true;
}

bool MaterialMaskPipeline::buildOutput(
	editor::IPipelineManager* pipelineManager,
	const Serializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	const MaterialMaskAsset* maskAsset = checked_type_cast< const MaterialMaskAsset* >(sourceAsset);
	Path fileName = maskAsset->getFileName();

	Ref< drawing::Image > image = drawing::Image::load(fileName);
	if (!image)
	{
		log::error << L"Unable to load material mask source image \"" << fileName.getPathName() << L"\"" << Endl;
		return false;
	}

	if (image->getWidth() != image->getHeight())
	{
		log::error << L"Material mask source image must be square" << Endl;
		return false;
	}

	// @fixme Quantize image into a unique color per mask channel.

	uint32_t size = image->getWidth();

	// Create mask resource.
	Ref< MaterialMaskResource > resource = gc_new< MaterialMaskResource >(size);

	// Create instance's name.
	Ref< db::Instance > instance = pipelineManager->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
	{
		log::error << L"Failed to build mask, unable to create instance" << Endl;
		return false;
	}

	instance->setObject(resource);

	Ref< Stream > stream = instance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Failed to build mask, unable to create data stream" << Endl;
		instance->revert();
		return false;
	}

	// Convert image pixels into mask values.
	Writer writer(stream);
	for (uint32_t y = 0; y < size; ++y)
	{
		for (uint32_t x = 0; x < size; ++x)
		{
			drawing::Color imagePixel;
			image->getPixel(x, y, imagePixel);

			uint8_t mask = 0;
			if (imagePixel.getRed() >= 0.5f)
				mask = 1;

			writer << mask;
		}
	}

	stream->close();
	return instance->commit();
}

	}
}
