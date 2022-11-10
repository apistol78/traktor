/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Sound/Processor/Graph.h"
#include "Sound/Editor/SoundCategory.h"
#include "Sound/Editor/Processor/GraphAsset.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.sound.GraphAsset", 0, GraphAsset, ISerializable)

GraphAsset::GraphAsset()
:	m_gain(0.0f)
,	m_presence(0.0f)
,	m_presenceRate(1.0f)
,	m_range(0.0f)
,	m_graph(new Graph())
{
}

void GraphAsset::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"category", m_category, AttributeType(type_of< SoundCategory >()));
	s >> Member< float >(L"gain", m_gain);
	s >> Member< float >(L"presence", m_presence);
	s >> Member< float >(L"presenceRate", m_presenceRate);
	s >> Member< float >(L"range", m_range);
	s >> MemberRef< Graph >(L"graph", m_graph);
}

	}
}
