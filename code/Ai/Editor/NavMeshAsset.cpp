/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/Editor/NavMeshAsset.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace ai
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.ai.NavMeshAsset", 0, NavMeshAsset, ISerializable)

NavMeshAsset::NavMeshAsset()
:	m_cellSize(0.3f)
,	m_cellHeight(0.2f)
,	m_agentSlope(45.0f)
,	m_agentHeight(2.0f)
,	m_agentClimb(0.9f)
,	m_agentRadius(0.6f)
,	m_maxEdgeLength(12.0f)
,	m_maxSimplificationError(1.3f)
,	m_minRegionSize(8.0f)
,	m_mergeRegionSize(20.0f)
,	m_detailSampleDistance(6.0f)
,	m_detailSampleMaxError(1.0f)
{
}

void NavMeshAsset::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"source", m_source);
	s >> Member< float >(L"cellSize", m_cellSize, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"cellHeight", m_cellHeight, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"agentSlope", m_agentSlope);
	s >> Member< float >(L"agentHeight", m_agentHeight, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"agentClimb", m_agentClimb, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"agentRadius", m_agentRadius, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"maxEdgeLength", m_maxEdgeLength, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"maxSimplificationError", m_maxSimplificationError);
	s >> Member< float >(L"minRegionSize", m_minRegionSize, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"mergeRegionSize", m_mergeRegionSize, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"detailSampleDistance", m_detailSampleDistance);
	s >> Member< float >(L"detailSampleMaxError", m_detailSampleMaxError);
}

	}
}
