#include "Core/Log/Log.h"
#include "Core/Serialization/DeepHash.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/PipelineDependencySet.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/PipelineDependency.h"
#include "Resource/ResourceBundle.h"
#include "Resource/Editor/ResourceBundleAsset.h"
#include "Resource/Editor/ResourceBundlePipeline.h"

namespace traktor
{
	namespace resource
	{
		namespace
		{

void collectResources(editor::IPipelineBuilder* pipelineBuilder, const editor::PipelineDependencySet* dependencySet, const editor::PipelineDependency* dependency, AlignedVector< std::pair< const TypeInfo*, Guid > >& outResources, SmallSet< Guid >& inoutHistory)
{
	if (inoutHistory.find(dependency->outputGuid) != inoutHistory.end())
		return;

	inoutHistory.insert(dependency->outputGuid);

	if ((dependency->flags & editor::PdfResource) != 0)
	{
		Ref< db::Instance > assetInstance = pipelineBuilder->getOutputDatabase()->getInstance(dependency->outputGuid);
		if (assetInstance)
		{
			outResources.push_back(std::make_pair(
				assetInstance->getPrimaryType(),
				dependency->outputGuid
			));
		}
	}

	for (auto child : dependency->children)
	{
		const editor::PipelineDependency* childDependency = dependencySet->get(child);
		T_ASSERT(childDependency);

		collectResources(pipelineBuilder, dependencySet, childDependency, outResources, inoutHistory);
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.ResourceBundlePipeline", 6, ResourceBundlePipeline, editor::IPipeline)

bool ResourceBundlePipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void ResourceBundlePipeline::destroy()
{
}

TypeInfoSet ResourceBundlePipeline::getAssetTypes() const
{
	return makeTypeInfoSet< ResourceBundleAsset >();
}

bool ResourceBundlePipeline::shouldCache() const
{
	return false;
}

uint32_t ResourceBundlePipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool ResourceBundlePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const ResourceBundleAsset* bundleAsset = checked_type_cast< const ResourceBundleAsset* >(sourceAsset);
	for (const auto& resource : bundleAsset->get())
		pipelineDepends->addDependency(resource, editor::PdfBuild);
	return true;
}

bool ResourceBundlePipeline::buildOutput(
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
	const ResourceBundleAsset* bundleAsset = checked_type_cast< const ResourceBundleAsset* >(sourceAsset);
	AlignedVector< std::pair< const TypeInfo*, Guid > > resources;
	SmallSet< Guid > history;

	// Collect resources, should be in depth-first to reduce
	// change of inter-dependencies.
	collectResources(pipelineBuilder, dependencySet, dependency, resources, history);
	log::info << int32_t(resources.size()) << L" resource(s) in bundle found." << Endl;

	// Sort bundle resources on type.
	std::sort(resources.begin(), resources.end(), [](const std::pair< const TypeInfo*, Guid >& lh, const std::pair< const TypeInfo*, Guid >& rh) {
		return lh.first < rh.first;
	});

	log::info << L"Resource bundle; preload resources:" << Endl;
	log::info << IncreaseIndent;

	for (const auto& resource : resources)
	{
		Ref< db::Instance > instance = pipelineBuilder->getOutputDatabase()->getInstance(resource.second);
		if (instance)
		{
			if (resource.first && resource.first->getName())
				log::info << L"\"" << instance->getPath() << L"\" as " << resource.first->getName() << Endl;
			else
				log::info << L"\"" << instance->getPath() << L"\"" << Endl;
		}
	}

	log::info << DecreaseIndent;

	Ref< ResourceBundle > bundle = new ResourceBundle(resources, bundleAsset->persistent());

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
	{
		log::error << L"ResourceBundlePipeline failed; unable to create output instance" << Endl;
		return false;
	}

	outputInstance->setObject(bundle);

	if (!outputInstance->commit())
	{
		log::error << L"ResourceBundlePipeline failed; unable to commit output instance" << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > ResourceBundlePipeline::buildOutput(
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
