#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Sound/Sound.h"
#include "Sound/Processor/Graph.h"
#include "Sound/Processor/GraphBuffer.h"
#include "Sound/Processor/GraphResource.h"
#include "Sound/Processor/Node.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.sound.GraphResource", 0, GraphResource, ISoundResource)

GraphResource::GraphResource()
:	m_gain(0.0f)
,	m_presence(0.0f)
,	m_presenceRate(1.0f)
,	m_range(0.0f)
{
}

GraphResource::GraphResource(
	const std::wstring& category,
	float gain,
	float presence,
	float presenceRate,
	float range,
	const Graph* graph
)
:	m_category(category)
,	m_gain(gain)
,	m_presence(presence)
,	m_presenceRate(presenceRate)
,	m_range(range)
,	m_graph(graph)
{
}

Ref< Sound > GraphResource::createSound(resource::IResourceManager* resourceManager, const db::Instance* resourceInstance) const
{
	for (auto node : m_graph->getNodes())
	{
		if (!node->bind(resourceManager))
			return nullptr;
	}

	return new Sound(
		new GraphBuffer(m_graph),
		getParameterHandle(m_category),
		m_gain,
		m_presence,
		m_presenceRate,
		m_range
	);
}

void GraphResource::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"category", m_category);
	s >> Member< float >(L"gain", m_gain);
	s >> Member< float >(L"presence", m_presence);
	s >> Member< float >(L"presenceRate", m_presenceRate);
	s >> Member< float >(L"range", m_range);	
	s >> MemberRef< const Graph >(L"graph", m_graph);
}

	}
}
