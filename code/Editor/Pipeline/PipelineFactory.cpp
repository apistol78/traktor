#include <algorithm>
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IPipeline.h"
#include "Editor/Pipeline/PipelineFactory.h"
#include "Editor/Pipeline/PipelineSettings.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineFactory", PipelineFactory, Object)

PipelineFactory::PipelineFactory(const PropertyGroup* settings)
{
	const bool verbose = settings->getProperty< bool >(L"Pipeline.Verbose", false);

	TypeInfoSet pipelineTypes = type_of< IPipeline >().findAllOf(false);

	AlignedVector< const TypeInfo* > sortedPipelineTypes;
	sortedPipelineTypes.insert(sortedPipelineTypes.begin(), pipelineTypes.begin(), pipelineTypes.end());
	std::sort(sortedPipelineTypes.begin(), sortedPipelineTypes.end(), [](const TypeInfo* lh, const TypeInfo* rh) {
		return std::wcscmp(lh->getName(), rh->getName()) < 0;
	});

	if (verbose)
		log::info << L"Creating " << (int32_t)sortedPipelineTypes.size() << L" pipelines..." << Endl;

	uint32_t completeHash = 0;

	for (auto pipelineType : sortedPipelineTypes)
	{
		Ref< IPipeline > pipeline = dynamic_type_cast< IPipeline* >(pipelineType->createInstance());
		if (!pipeline)
			continue;

		PipelineSettings pipelineSettings(settings);
		if (!pipeline->create(&pipelineSettings))
		{
			log::error << L"Failed to create pipeline \"" << type_name(pipeline) << L"\"." << Endl;			
			continue;
		}

		const uint32_t pipelineHash = pipelineSettings.getHash() + type_of(pipeline).getVersion();
		completeHash += pipelineHash;

		if (verbose)
		{
			log::info << L"Pipeline \"" << type_name(pipeline) << L" created successfully (" << str(L"0x%08x", pipelineHash) << L"):" << Endl;
			log::info << IncreaseIndent;
			log::info << pipelineSettings.getLog();
			log::info << DecreaseIndent;
		}

		for (auto assetType : pipeline->getAssetTypes())
		{
			for (auto buildableType : assetType->findAllOf())
			{
				auto it = m_pipelineMap.find(buildableType);
				if (it != m_pipelineMap.end())
				{
					uint32_t currentDistance = type_difference(*assetType, *buildableType);
					if (currentDistance < it->second.distance)
					{
						it->second.pipeline = pipeline;
						it->second.hash = pipelineHash;
						it->second.distance = currentDistance;
					}
				}
				else
				{
					PipelineMatch& pm = m_pipelineMap[buildableType];
					pm.pipeline = pipeline;
					pm.hash = pipelineHash;
					pm.distance = type_difference(*assetType, *buildableType);
				}
			}
		}

		m_pipelines[&type_of(pipeline)] = pipeline;
	}

	if (verbose)
		log::info << L"Pipeline signature hash " << str(L"0x%08x", completeHash) << L"." << Endl;
}

PipelineFactory::~PipelineFactory()
{
	for (auto pipeline : m_pipelines)
		pipeline.second->destroy();

	m_pipelines.clear();
	m_pipelineMap.clear();
}

bool PipelineFactory::findPipelineType(const TypeInfo& sourceType, const TypeInfo*& outPipelineType, uint32_t& outPipelineHash) const
{
	outPipelineType = nullptr;
	outPipelineHash = 0;

	auto it = m_pipelineMap.find(&sourceType);
	if (it != m_pipelineMap.end())
	{
		outPipelineType = &type_of(it->second.pipeline);
		outPipelineHash = it->second.hash;
		return true;
	}
	else
		return false;
}

IPipeline* PipelineFactory::findPipeline(const TypeInfo& pipelineType) const
{
	auto it = m_pipelines.find(&pipelineType);
	return it != m_pipelines.end() ? it->second : nullptr;
}

	}
}
