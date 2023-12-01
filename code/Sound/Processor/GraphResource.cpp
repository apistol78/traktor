/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Sound/Sound.h"
#include "Sound/Processor/Graph.h"
#include "Sound/Processor/GraphBuffer.h"
#include "Sound/Processor/GraphResource.h"
#include "Sound/Processor/Node.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.sound.GraphResource", 0, GraphResource, ISoundResource)

GraphResource::GraphResource(
	const std::wstring& category,
	float gain,
	float range,
	const Graph* graph
)
:	m_category(category)
,	m_gain(gain)
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
		m_range
	);
}

void GraphResource::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"category", m_category);
	s >> Member< float >(L"gain", m_gain);
	s >> Member< float >(L"range", m_range);
	s >> MemberRef< const Graph >(L"graph", m_graph);
}

}
