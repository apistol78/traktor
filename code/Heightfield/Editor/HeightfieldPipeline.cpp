#include <limits>
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/HeightfieldResource.h"
#include "Heightfield/Editor/ErosionFilter.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldPipeline.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldPipeline", 2, HeightfieldPipeline, editor::IPipeline)

bool HeightfieldPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
	return true;
}

void HeightfieldPipeline::destroy()
{
}

TypeInfoSet HeightfieldPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< HeightfieldAsset >();
}

bool HeightfieldPipeline::shouldCache() const
{
	return true;
}

uint32_t HeightfieldPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool HeightfieldPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	return true;
}

bool HeightfieldPipeline::buildOutput(
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
	Ref< const HeightfieldAsset > heightfieldAsset = checked_type_cast< const HeightfieldAsset* >(sourceAsset);

	Ref< IStream > sourceData = sourceInstance->readData(L"Data");
	if (!sourceData)
	{
		log::error << L"Heightfield pipeline failed; unable to open heights" << Endl;
		return false;
	}

	Ref< Heightfield > heightfield = HeightfieldFormat().read(
		sourceData,
		heightfieldAsset->getWorldExtent()
	);
	if (!heightfield)
	{
		log::error << L"Heightfield pipeline failed; unable to read heights" << Endl;
		return false;
	}

	sourceData->close();
	sourceData = nullptr;

	// Apply erosion filter.
	if (heightfieldAsset->getErosionEnable())
		ErosionFilter(heightfieldAsset->getErodeIterations()).apply(heightfield);

	// Create height field resource.
	Ref< HeightfieldResource > resource = new HeightfieldResource();

	// Create instance's name.
	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
	{
		log::error << L"Heightfield pipeline failed; unable to create instance" << Endl;
		return false;
	}

	Ref< IStream > outputData = instance->writeData(L"Data");
	if (!outputData)
	{
		log::error << L"Heightfield pipeline failed; unable to create data stream" << Endl;
		instance->revert();
		return false;
	}

	HeightfieldFormat().write(outputData, heightfield);

	outputData->close();
	outputData = nullptr;

	resource->m_worldExtent = heightfieldAsset->getWorldExtent();

	instance->setObject(resource);

	return instance->commit();
}

Ref< ISerializable > HeightfieldPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	T_FATAL_ERROR;
	return nullptr;
}

	}
}
