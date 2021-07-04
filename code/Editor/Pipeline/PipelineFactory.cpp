#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
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
	TypeInfoSet pipelineTypes;
	type_of< IPipeline >().findAllOf(pipelineTypes, false);
	for (auto pipelineType : pipelineTypes)
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

		uint32_t pipelineHash = pipelineSettings.getHash() + type_of(pipeline).getVersion();

		log::info << L"Pipeline \"" << type_name(pipeline) << L" created successfully; configuration hash " << str(L"0x%08x", pipelineHash) << L"." << Endl;

		for (auto assetType : pipeline->getAssetTypes())
		{
			TypeInfoSet buildableTypes;
			assetType->findAllOf(buildableTypes);
			for (auto buildableType : buildableTypes)
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
