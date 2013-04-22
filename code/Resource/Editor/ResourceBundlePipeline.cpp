#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/Pipeline/PipelineDependency.h"
#include "Resource/ResourceBundle.h"
#include "Resource/Editor/ResourceBundleAsset.h"
#include "Resource/Editor/ResourceBundlePipeline.h"

namespace traktor
{
	namespace resource
	{
		namespace
		{

void collectResources(editor::IPipelineBuilder* pipelineBuilder, const editor::PipelineDependency* dependency, std::vector< std::pair< const TypeInfo*, Guid > >& outResources)
{
	for (RefArray< editor::PipelineDependency >::const_iterator i = dependency->children.begin(); i != dependency->children.end(); ++i)
	{
		collectResources(pipelineBuilder, *i, outResources);
		if (((*i)->flags & editor::PdfResource) != 0)
		{
			Ref< db::Instance > assetInstance = pipelineBuilder->getOutputDatabase()->getInstance((*i)->outputGuid);
			if (!assetInstance)
				continue;

			outResources.push_back(std::make_pair(
				assetInstance->getPrimaryType(),
				(*i)->outputGuid
			));
		}
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.ResourceBundlePipeline", 1, ResourceBundlePipeline, editor::IPipeline)

bool ResourceBundlePipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void ResourceBundlePipeline::destroy()
{
}

TypeInfoSet ResourceBundlePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ResourceBundleAsset >());
	return typeSet;
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
	const std::vector< Guid >& resources = bundleAsset->get();
	for (std::vector< Guid >::const_iterator i = resources.begin(); i != resources.end(); ++i)
		pipelineDepends->addDependency(*i, editor::PdfBuild);
	return true;
}

bool ResourceBundlePipeline::buildOutput(
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
	const ResourceBundleAsset* bundleAsset = checked_type_cast< const ResourceBundleAsset* >(sourceAsset);
	std::vector< std::pair< const TypeInfo*, Guid > > resources;

	// Collect resources, should be in depth-first to reduce
	// change of inter-dependencies.
	collectResources(pipelineBuilder, dependency, resources);

	log::info << L"Resource bundle; preload resources:" << Endl;
	log::info << IncreaseIndent;

	for (std::vector< std::pair< const TypeInfo*, Guid > >::const_iterator i = resources.begin(); i != resources.end(); ++i)
	{
		Ref< db::Instance > instance = pipelineBuilder->getOutputDatabase()->getInstance(i->second);
		if (instance)
		{
			if (i->first->getName())
				log::info << L"\"" << instance->getPath() << L"\" as " << i->first->getName() << Endl;
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
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
