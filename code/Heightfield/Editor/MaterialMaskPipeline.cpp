#include <limits>
#include "Core/Io/FileSystem.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/MaterialMaskResource.h"
#include "Heightfield/Editor/MaterialMaskPipeline.h"
#include "Heightfield/Editor/MaterialMaskAsset.h"

namespace traktor
{
	namespace hf
	{
		namespace
		{

uint8_t findBestMatch(const Color4f& color, const Color4f* colorSet, uint8_t count)
{
	float minD = std::numeric_limits< float >::max();
	uint8_t minI = 0;

	for (uint8_t i = 0; i < count; ++i)
	{
		Color4f dc = colorSet[i] - color;
		float d = std::abs(dc.getRed()) + std::abs(dc.getGreen()) + std::abs(dc.getBlue());
		if (d < minD)
		{
			minI = i;
			minD = d;
		}
	}

	return minI;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.MaterialMaskPipeline", 2, MaterialMaskPipeline, editor::IPipeline)

bool MaterialMaskPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	return true;
}

void MaterialMaskPipeline::destroy()
{
}

TypeInfoSet MaterialMaskPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MaterialMaskAsset >());
	return typeSet;
}

bool MaterialMaskPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	const MaterialMaskAsset* maskAsset = checked_type_cast< const MaterialMaskAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, maskAsset->getFileName());
	pipelineDepends->addDependency(fileName);
	return true;
}

bool MaterialMaskPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	const MaterialMaskAsset* maskAsset = checked_type_cast< const MaterialMaskAsset* >(sourceAsset);
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, maskAsset->getFileName());

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

	uint32_t size = image->getWidth();

	// Create mask resource.
	Ref< MaterialMaskResource > resource = new MaterialMaskResource(size);

	// Create instance's name.
	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
	{
		log::error << L"Failed to build mask, unable to create instance" << Endl;
		return false;
	}

	instance->setObject(resource);

	Ref< IStream > stream = instance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Failed to build mask, unable to create data stream" << Endl;
		instance->revert();
		return false;
	}

	const Color4f c_materialColors[] =
	{
		Color4f(0.0f, 0.0f, 0.0f, 0.0f),
		Color4f(1.0f, 0.0f, 0.0f, 0.0f),
		Color4f(0.0f, 1.0f, 0.0f, 0.0f),
		Color4f(0.0f, 0.0f, 1.0f, 0.0f),
		Color4f(1.0f, 1.0f, 0.0f, 0.0f),
		Color4f(0.0f, 1.0f, 1.0f, 0.0f)
	};

	// Convert image pixels into mask values.
	Writer writer(stream);
	for (uint32_t y = 0; y < size; ++y)
	{
		for (uint32_t x = 0; x < size; ++x)
		{
			Color4f imagePixel;
			image->getPixelUnsafe(x, y, imagePixel);
			uint8_t mask = findBestMatch(imagePixel, c_materialColors, sizeof_array(c_materialColors));
			writer << mask;
		}
	}

	stream->close();
	return instance->commit();
}

	}
}
