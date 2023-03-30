/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <DetourNavMeshQuery.h>
#include "Ai/MoveQuery.h"
#include "Core/Log/Log.h"
#include "Core/Math/Plane.h"

namespace traktor::ai
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.MoveQuery", MoveQuery, Object)

MoveQuery::MoveQuery()
:	m_startPosition(0.0f, 0.0f, 0.0f, 0.0f)
,	m_endPosition(0.0f, 0.0f, 0.0f, 0.0f)
,	m_filter(new dtQueryFilter())
,	m_navQuery(nullptr)
,	m_pathCount(0)
,	m_steerIndex(0)
{
}

MoveQuery::~MoveQuery()
{
	dtFreeNavMeshQuery(m_navQuery);
	delete m_filter;
}

bool MoveQuery::update(const Vector4& currentPosition, Vector4& outMoveToPosition, float nodeDistanceThreshold)
{
	const static Vector4 c_101(1.0f, 0.0f, 1.0f);
	const Scalar ndt2(nodeDistanceThreshold * nodeDistanceThreshold);
	while (m_steerIndex < m_steerPath.size())
	{
		const Vector4& steerTo = m_steerPath[m_steerIndex];
		const Scalar d2 = ((steerTo - currentPosition) * c_101).length2();
		if (d2 >= ndt2)
		{
			outMoveToPosition = steerTo;
			return true;
		}
		else
			++m_steerIndex;
	}
	return false;
}

}
