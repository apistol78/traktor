/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <limits>
#include "Core/Log/Log.h"
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

	for (TypeInfoSet::iterator i = pipelineTypes.begin(); i != pipelineTypes.end(); ++i)
	{
		Ref< IPipeline > pipeline = dynamic_type_cast< IPipeline* >((*i)->createInstance());
		if (pipeline)
		{
			PipelineSettings pipelineSettings(settings);
			if (pipeline->create(&pipelineSettings))
			{
				TypeInfoSet assetTypes = pipeline->getAssetTypes();
				for (TypeInfoSet::const_iterator j = assetTypes.begin(); j != assetTypes.end(); ++j)
				{
					TypeInfoSet buildableTypes;
					(*j)->findAllOf(buildableTypes);

					for (TypeInfoSet::const_iterator k = buildableTypes.begin(); k != buildableTypes.end(); ++k)
					{
						SmallMap< const TypeInfo*, PipelineMatch >::iterator it = m_pipelineMap.find(*k);
						if (it != m_pipelineMap.end())
						{
							uint32_t currentDistance = type_difference(*(*j), *(*k));
							if (currentDistance < it->second.distance)
							{
								it->second.pipeline = pipeline;
								it->second.hash = pipelineSettings.getHash() + type_of(pipeline).getVersion();
								it->second.distance = currentDistance;
							}
						}
						else
						{
							PipelineMatch& pm = m_pipelineMap[*k];
							pm.pipeline = pipeline;
							pm.hash = pipelineSettings.getHash() + type_of(pipeline).getVersion();
							pm.distance = type_difference(*(*j), *(*k));
						}
					}
				}
				m_pipelines[&type_of(pipeline)] = pipeline;
			}
			else
				log::error << L"Failed to create pipeline \"" << type_name(pipeline) << L"\"" << Endl;
		}
	}
}

PipelineFactory::~PipelineFactory()
{
	for (SmallMap< const TypeInfo*, Ref< IPipeline > >::iterator i = m_pipelines.begin(); i != m_pipelines.end(); ++i)
		i->second->destroy();

	m_pipelines.clear();
	m_pipelineMap.clear();
}

bool PipelineFactory::findPipelineType(const TypeInfo& sourceType, const TypeInfo*& outPipelineType, uint32_t& outPipelineHash) const
{
	outPipelineType = 0;
	outPipelineHash = 0;

	SmallMap< const TypeInfo*, PipelineMatch >::const_iterator i = m_pipelineMap.find(&sourceType);
	if (i != m_pipelineMap.end())
	{
		outPipelineType = &type_of(i->second.pipeline);
		outPipelineHash = i->second.hash;
		return true;
	}
	else
		return false;
}

IPipeline* PipelineFactory::findPipeline(const TypeInfo& pipelineType) const
{
	SmallMap< const TypeInfo*, Ref< IPipeline > >::const_iterator i = m_pipelines.find(&pipelineType);
	return i != m_pipelines.end() ? i->second : 0;
}

	}
}
