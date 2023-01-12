/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/Editor/NavMeshAsset.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::ai
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.ai.NavMeshAsset", 0, NavMeshAsset, ISerializable)

void NavMeshAsset::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"source", m_source);
	s >> Member< float >(L"cellSize", m_cellSize, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"cellHeight", m_cellHeight, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"agentSlope", m_agentSlope, AttributeRange(0.0f) | AttributeUnit(UnitType::Degrees));
	s >> Member< float >(L"agentHeight", m_agentHeight, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"agentClimb", m_agentClimb, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"agentRadius", m_agentRadius, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"maxEdgeLength", m_maxEdgeLength, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"maxSimplificationError", m_maxSimplificationError, AttributeRange(0.0f));
	s >> Member< float >(L"minRegionSize", m_minRegionSize, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"mergeRegionSize", m_mergeRegionSize, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"detailSampleDistance", m_detailSampleDistance, AttributeRange(0.0f));
	s >> Member< float >(L"detailSampleMaxError", m_detailSampleMaxError, AttributeRange(0.0f));
}

}
