#include "Core/Log/Log.h"
#include "Core/Serialization/DeepHash.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Input/Binding/InputMappingResource.h"
#include "Input/Binding/InputMappingSourceData.h"
#include "Input/Binding/InputMappingStateData.h"
#include "Input/Editor/InputMappingAsset.h"
#include "Input/Editor/InputMappingPipeline.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputMappingPipeline", 0, InputMappingPipeline, editor::IPipeline)

bool InputMappingPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void InputMappingPipeline::destroy()
{
}

TypeInfoSet InputMappingPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< InputMappingAsset >();
}

bool InputMappingPipeline::shouldCache() const
{
	return false;
}

uint32_t InputMappingPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool InputMappingPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	auto inputAsset = mandatory_non_null_type_cast< const InputMappingAsset* >(sourceAsset);
	for (const auto& dependency : inputAsset->getDependencies())
		pipelineDepends->addDependency(dependency, editor::PdfUse);
	return true;
}

bool InputMappingPipeline::buildOutput(
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
	Ref< InputMappingResource > mappingResource = checked_type_cast< InputMappingResource*, true >(buildProduct(pipelineBuilder, sourceInstance, sourceAsset, buildParams));
	if (!mappingResource)
		return false;

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
	{
		log::error << L"Input mapping pipeline failed; unable to create output instance" << Endl;
		return false;
	}

	outputInstance->setObject(mappingResource);
	if (!outputInstance->commit())
	{
		log::error << L"Input mapping pipeline failed; unable to commit output instance" << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > InputMappingPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	auto inputAsset = mandatory_non_null_type_cast< const InputMappingAsset* >(sourceAsset);

	Ref< InputMappingSourceData > mergedSourceData = new InputMappingSourceData();
	Ref< InputMappingStateData > mergedStateData = new InputMappingStateData();

	// Merge data from all dependencies.
	for (const auto& dependency : inputAsset->getDependencies())
	{
		Ref< const InputMappingAsset > mergeInputAsset = pipelineBuilder->getObjectReadOnly< InputMappingAsset >(dependency);
		if (!mergeInputAsset)
		{
			log::error << L"Input mapping pipeline failed; unable to read dependent input asset \"" << dependency.format() << L"\"." << Endl;
			return nullptr;
		}

		if (mergeInputAsset->getSourceData())
		{
			for (const auto& it : mergeInputAsset->getSourceData()->getSourceData())
			{
				mergedSourceData->setSourceData(
					it.first,
					it.second
				);
			}
		}

		if (mergeInputAsset->getStateData())
		{
			for (const auto& it : mergeInputAsset->getStateData()->getStateData())
			{
				mergedStateData->setStateData(
					it.first,
					it.second
				);
			}
		}
	}

	// Finally merge asset's own input definitions.
	if (inputAsset->getSourceData())
	{
		for (const auto& it : inputAsset->getSourceData()->getSourceData())
		{
			mergedSourceData->setSourceData(
				it.first,
				it.second
			);
		}
	}

	if (inputAsset->getStateData())
	{
		for (const auto& it : inputAsset->getStateData()->getStateData())
		{
			mergedStateData->setStateData(
				it.first,
				it.second
			);
		}
	}

	// Create output instance.
	return new InputMappingResource(
		mergedSourceData,
		mergedStateData
	);
}

	}
}
