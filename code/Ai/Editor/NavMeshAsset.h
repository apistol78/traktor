/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ai
	{

/*! Navigation mesh asset.
 * \ingroup AI
 */
class T_DLLCLASS NavMeshAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	NavMeshAsset();

	virtual void serialize(ISerializer& s) override final;

private:
	friend class NavMeshPipeline;

	Guid m_source;
	float m_cellSize;
	float m_cellHeight;
	float m_agentSlope;
	float m_agentHeight;
	float m_agentClimb;
	float m_agentRadius;
	float m_maxEdgeLength;
	float m_maxSimplificationError;
	float m_minRegionSize;
	float m_mergeRegionSize;
	float m_detailSampleDistance;
	float m_detailSampleMaxError;
};

	}
}

