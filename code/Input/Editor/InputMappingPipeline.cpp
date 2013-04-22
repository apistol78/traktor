#include "Core/Log/Log.h"
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
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< InputMappingAsset >());
	return typeSet;
}

bool InputMappingPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const InputMappingAsset* inputAsset = checked_type_cast< const InputMappingAsset*, false >(sourceAsset);

	const std::list< Guid >& dependencies = inputAsset->getDependencies();
	for (std::list< Guid >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
		pipelineDepends->addDependency(*i, editor::PdfUse);

	return true;
}

bool InputMappingPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
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
	Ref< InputMappingResource > mappingResource = checked_type_cast< InputMappingResource*, true >(buildOutput(pipelineBuilder, sourceAsset));
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

Ref< ISerializable > InputMappingPipeline::buildOutput(editor::IPipelineBuilder* pipelineBuilder, const ISerializable* sourceAsset) const
{
	const InputMappingAsset* inputAsset = checked_type_cast< const InputMappingAsset*, false >(sourceAsset);

	Ref< InputMappingSourceData > mergedSourceData = new InputMappingSourceData();
	Ref< InputMappingStateData > mergedStateData = new InputMappingStateData();

	// Merge data from all dependencies.
	const std::list< Guid >& dependencies = inputAsset->getDependencies();
	for (std::list< Guid >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		Ref< const InputMappingAsset > mergeInputAsset = pipelineBuilder->getObjectReadOnly< InputMappingAsset >(*i);
		if (!mergeInputAsset)
		{
			log::error << L"Input mapping pipeline failed; unable to read dependent input asset \"" << i->format() << L"\"" << Endl;
			return 0;
		}

		if (mergeInputAsset->getSourceData())
		{
			const std::map< std::wstring, Ref< IInputSourceData > >& sourceData = mergeInputAsset->getSourceData()->getSourceData();
			for (std::map< std::wstring, Ref< IInputSourceData > >::const_iterator j = sourceData.begin(); j != sourceData.end(); ++j)
			{
				mergedSourceData->setSourceData(
					j->first,
					j->second
				);
			}
		}

		if (mergeInputAsset->getStateData())
		{
			const RefArray< IInputFilter >& filters = mergeInputAsset->getStateData()->getFilters();
			for (RefArray< IInputFilter >::const_iterator j = filters.begin(); j != filters.end(); ++j)
				mergedStateData->addFilter(*j);

			const std::map< std::wstring, Ref< InputStateData > >& stateData = mergeInputAsset->getStateData()->getStateData();
			for (std::map< std::wstring, Ref< InputStateData > >::const_iterator j = stateData.begin(); j != stateData.end(); ++j)
			{
				mergedStateData->setStateData(
					j->first,
					j->second
				);
			}
		}
	}

	// Finally merge asset's own input definitions.
	if (inputAsset->getSourceData())
	{
		const std::map< std::wstring, Ref< IInputSourceData > >& sourceData = inputAsset->getSourceData()->getSourceData();
		for (std::map< std::wstring, Ref< IInputSourceData > >::const_iterator j = sourceData.begin(); j != sourceData.end(); ++j)
		{
			mergedSourceData->setSourceData(
				j->first,
				j->second
			);
		}
	}

	if (inputAsset->getStateData())
	{
		const RefArray< IInputFilter >& filters = inputAsset->getStateData()->getFilters();
		for (RefArray< IInputFilter >::const_iterator j = filters.begin(); j != filters.end(); ++j)
			mergedStateData->addFilter(*j);

		const std::map< std::wstring, Ref< InputStateData > >& stateData = inputAsset->getStateData()->getStateData();
		for (std::map< std::wstring, Ref< InputStateData > >::const_iterator j = stateData.begin(); j != stateData.end(); ++j)
		{
			mergedStateData->setStateData(
				j->first,
				j->second
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
