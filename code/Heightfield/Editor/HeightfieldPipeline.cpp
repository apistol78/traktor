#include <limits>
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldResource.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldCompositor.h"
#include "Heightfield/Editor/HeightfieldLayer.h"
#include "Heightfield/Editor/HeightfieldPipeline.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldPipeline", 0, HeightfieldPipeline, editor::IPipeline)

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
	
	// Add dependency to file.
	Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, heightfieldAsset->getFileName());
	pipelineDepends->addDependency(fileName);

	// Pass source instance as parameter to build output.
	outBuildParams = sourceInstance;
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

	Ref< const db::Instance > assetInstance = checked_type_cast< const db::Instance*, true >(buildParams);
	if (!assetInstance)
		return false;

	Ref< HeightfieldCompositor > compositor = HeightfieldCompositor::createFromInstance(assetInstance, m_assetPath);
	if (!compositor)
		return false;

	Ref< HeightfieldLayer > mergedLayer = compositor->mergeLayers();
	if (!mergedLayer)
		return false;

	// Create height field resource.
	Ref< HeightfieldResource > resource = new HeightfieldResource();

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

	Ref< IStream > stream = instance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Failed to build heightfield, unable to create data stream" << Endl;
		instance->revert();
		return false;
	}

	const height_t* heights = mergedLayer->getHeights();
	uint32_t size = mergedLayer->getSize();

	Writer(stream).write(
		heights,
		size * size,
		sizeof(height_t)
	);

	stream->close();
	
	resource->m_size = size;
	resource->m_worldExtent = heightfieldAsset->getWorldExtent();
	resource->m_patchDim = heightfieldAsset->getPatchDim();
	resource->m_detailSkip = heightfieldAsset->getDetailSkip();

	instance->setObject(resource);

	return instance->commit();
}

	}
}
