/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

class dtNavMeshQuery;
class dtQueryFilter;

namespace traktor::ai
{

/*! Continuous movement query.
 * \ingroup AI
 */
class T_DLLCLASS MoveQuery : public Object
{
	T_RTTI_CLASS;

public:
	MoveQuery();

	virtual ~MoveQuery();

	/*! Update query to get desired "move to" position.
	 *
	 * \param currentPosition Current entity position.
	 * \param outMoveToPosition Desired move to position to advance on movement query.
	 * \param nodeDistanceThreshold Distance threshold until current position is determined to be at intermediate positions.
	 * \return True if successfully determined "move to" position.
	 */
	bool update(const Vector4& currentPosition, Vector4& outMoveToPosition, float nodeDistanceThreshold);

private:
	friend class NavMesh;

	enum
	{
		MaxPathPolygons = 1024
	};

	Vector4 m_startPosition;
	Vector4 m_endPosition;
	dtQueryFilter* m_filter;
	dtNavMeshQuery* m_navQuery;
	uint32_t m_path[MaxPathPolygons];
	int32_t m_pathCount;
	AlignedVector< Vector4 > m_steerPath;
	int32_t m_steerIndex;
};

}
