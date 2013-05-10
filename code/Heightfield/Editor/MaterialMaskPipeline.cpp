#include <limits>
#include "Core/Containers/AlignedVector.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/MaterialMaskResource.h"
#include "Heightfield/MaterialMaskResourceLayer.h"
#include "Heightfield/Editor/MaterialMaskAsset.h"
#include "Heightfield/Editor/MaterialMaskAssetLayer.h"
#include "Heightfield/Editor/MaterialMaskPipeline.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.MaterialMaskPipeline", 3, MaterialMaskPipeline, editor::IPipeline)

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
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const MaterialMaskAsset* maskAsset = checked_type_cast< const MaterialMaskAsset* >(sourceAsset);

	pipelineDepends->addDependency(Path(m_assetPath), maskAsset->getFileName().getPathName());

	const RefArray< MaterialMaskAssetLayer >& maskLayers = maskAsset->getLayers();
	for (RefArray< MaterialMaskAssetLayer >::const_iterator i = maskLayers.begin(); i != maskLayers.end(); ++i)
	{
		const RefArray< ISerializable >& maskLayerParams = (*i)->getParams();
		for (RefArray< ISerializable >::const_iterator j = maskLayerParams.begin(); j != maskLayerParams.end(); ++j)
			pipelineDepends->addDependency(*j);
	}

	return true;
}

bool MaterialMaskPipeline::buildOutput(
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
	const MaterialMaskAsset* maskAsset = checked_type_cast< const MaterialMaskAsset* >(sourceAsset);
	const RefArray< MaterialMaskAssetLayer >& maskLayers = maskAsset->getLayers();
	if (maskLayers.empty())
	{
		log::error << L"Material mask pipeline failed; no layers defined." << Endl;
		return false;
	}

	Ref< IStream > file = pipelineBuilder->openFile(Path(m_assetPath), maskAsset->getFileName().getOriginal());
	if (!file)
	{
		log::error << L"Material mask pipeline failed; unable to open source (" << maskAsset->getFileName().getOriginal() << L")" << Endl;
		return false;
	}

	Ref< drawing::Image > image = drawing::Image::load(file, maskAsset->getFileName().getExtension());
	if (!image)
	{
		log::error << L"Material mask pipeline failed; unable to load source image \"" << maskAsset->getFileName().getOriginal() << L"\"." << Endl;
		return false;
	}

	uint32_t size = image->getWidth();
	if (size != image->getHeight())
	{
		log::error << L"Material mask pipeline failed; source image must be square." << Endl;
		return false;
	}

	// Create mask resource.
	Ref< MaterialMaskResource > resource = new MaterialMaskResource();
	resource->m_size = size;
	resource->m_layers.resize(maskLayers.size());
	for (uint32_t i = 0; i < maskLayers.size(); ++i)
	{
		resource->m_layers[i] = new MaterialMaskResourceLayer();
		resource->m_layers[i]->m_params = maskLayers[i]->getParams();
	}

	// Create instance's name.
	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
	{
		log::error << L"Material mask pipeline failed; unable to create instance." << Endl;
		return false;
	}

	instance->setObject(resource);

	Ref< IStream > stream = instance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Material mask pipeline failed; unable to create data stream." << Endl;
		instance->revert();
		return false;
	}

	AlignedVector< Color4f > maskLayerColors(maskLayers.size());
	for (uint32_t i = 0; i < maskLayers.size(); ++i)
	{
		const Color4ub& c = maskLayers[i]->getColor();
		maskLayerColors[i].set(
			c.r  / 255.0f,
			c.g  / 255.0f,
			c.b  / 255.0f,
			c.a  / 255.0f
		);
	}

	// Convert image pixels into mask values.
	Writer writer(stream);
	for (uint32_t y = 0; y < size; ++y)
	{
		for (uint32_t x = 0; x < size; ++x)
		{
			Color4f sourceColor;
			image->getPixelUnsafe(x, y, sourceColor);

			float minD = std::numeric_limits< float >::max();
			uint8_t minI = 0;

			for (uint8_t i = 0; i < uint8_t(maskLayers.size()); ++i)
			{
				Color4f dc = maskLayerColors[i] - sourceColor;
				float d = std::abs(dc.getRed()) + std::abs(dc.getGreen()) + std::abs(dc.getBlue());
				if (d < minD)
				{
					minI = i;
					minD = d;
				}
			}

			writer << minI;
		}
	}

	stream->close();
	return instance->commit();
}

Ref< ISerializable > MaterialMaskPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
