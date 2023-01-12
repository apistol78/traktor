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

namespace traktor::ai
{

/*! Navigation mesh asset.
 * \ingroup AI
 */
class T_DLLCLASS NavMeshAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

private:
	friend class NavMeshPipeline;

	Guid m_source;
	float m_cellSize = 0.3f;
	float m_cellHeight = 0.2f;
	float m_agentSlope = 45.0f;
	float m_agentHeight = 2.0f;
	float m_agentClimb = 0.9f;
	float m_agentRadius = 0.6f;
	float m_maxEdgeLength = 12.0f;
	float m_maxSimplificationError = 1.3f;
	float m_minRegionSize = 8.0f;
	float m_mergeRegionSize = 20.0f;
	float m_detailSampleDistance = 6.0f;
	float m_detailSampleMaxError = 1.0f;
};

}
