/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/NavMesh.h"

#include "Ai/MoveQuery.h"
#include "Ai/MoveQueryResult.h"
#include "Core/Log/Log.h"
#include "Core/Math/Random.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"

#include <cmath>
#include <limits>

#include <DetourNavMeshQuery.h>

namespace traktor::ai
{
namespace
{

const float c_searchExtents[3] = { 32.0f, 1.0f, 32.0f };

float random()
{
	static Random s_rnd;
	return s_rnd.nextFloat();
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.NavMesh", NavMesh, Object)

NavMesh::~NavMesh()
{
	dtFreeNavMesh(m_navMesh);
}

Ref< MoveQueryResult > NavMesh::createMoveQuery(const Vector4& startPosition, const Vector4& endPosition)
{
	Ref< MoveQueryResult > result = new MoveQueryResult();
	JobManager::getInstance().add([=]() {
		T_ANONYMOUS_VAR(Ref< NavMesh >)(this);

		dtNavMeshQuery* navQuery = dtAllocNavMeshQuery();
		if (!navQuery)
		{
			result->fail();
			return;
		}

		dtStatus status = navQuery->init(m_navMesh, 2048);
		if (dtStatusFailed(status))
		{
			result->fail();
			return;
		}

		float T_MATH_ALIGN16 startPos[4];
		float T_MATH_ALIGN16 endPos[4];
		startPosition.storeAligned(startPos);
		endPosition.storeAligned(endPos);

		Ref< MoveQuery > outputQuery = new MoveQuery();
		outputQuery->m_navQuery = navQuery;

		dtPolyRef startRef, endRef;
		float T_MATH_ALIGN16 startPosN[4];
		float T_MATH_ALIGN16 endPosN[4];

		status = outputQuery->m_navQuery->findNearestPoly(
			startPos,
			c_searchExtents,
			outputQuery->m_filter,
			&startRef,
			startPosN);
		if (dtStatusFailed(status))
		{
			result->fail();
			return;
		}

		status = outputQuery->m_navQuery->findNearestPoly(
			endPos,
			c_searchExtents,
			outputQuery->m_filter,
			&endRef,
			endPosN);
		if (dtStatusFailed(status))
		{
			result->fail();
			return;
		}

		outputQuery->m_startPosition = Vector4::loadAligned(startPosN).xyz1();
		outputQuery->m_endPosition = Vector4::loadAligned(endPosN).xyz1();

		status = outputQuery->m_navQuery->findPath(
			startRef,
			endRef,
			startPosN,
			endPosN,
			outputQuery->m_filter,
			outputQuery->m_path,
			&outputQuery->m_pathCount,
			sizeof_array(outputQuery->m_path));
		if (dtStatusFailed(status) || outputQuery->m_pathCount <= 0)
		{
			// Failed to create navmesh path; most probably no valid route exists.
			// Create a short-cut path to move navigation entity back on track.
			outputQuery->m_steerPath.push_back(outputQuery->m_endPosition);
			result->succeed(outputQuery);
			return;
		}

		float steerPath[256 * 3 + 1];
		int32_t steerPathCount = 0;

		status = outputQuery->m_navQuery->findStraightPath(
			startPosN,
			endPosN,
			outputQuery->m_path,
			outputQuery->m_pathCount,
			steerPath,
			nullptr,
			nullptr,
			&steerPathCount,
			256);
		if (dtStatusFailed(status) || steerPathCount <= 0)
		{
			// Failed to create navmesh path; most probably no valid route exists.
			// Create a short-cut path to move navigation entity back on track.
			outputQuery->m_steerPath.push_back(outputQuery->m_endPosition);
			result->succeed(outputQuery);
			return;
		}

		outputQuery->m_steerPath.reserve(steerPathCount);
		for (int32_t i = 0; i < steerPathCount; ++i)
			outputQuery->m_steerPath.push_back(Vector4::loadUnaligned(&steerPath[i * 3]).xyz1());

		result->succeed(outputQuery);
		return;
	});
	return result;
}

bool NavMesh::findClosestPoint(const Vector4& searchFrom, float searchDistance, Vector4& outPoint) const
{
	dtNavMeshQuery* navQuery = dtAllocNavMeshQuery();
	if (!navQuery)
		return false;

	dtStatus status = navQuery->init(m_navMesh, 2048);
	if (dtStatusFailed(status))
	{
		dtFreeNavMeshQuery(navQuery);
		return false;
	}

	AutoPtr< dtQueryFilter > filter(new dtQueryFilter());

	const float searchExtents[3] = { searchDistance, searchDistance, searchDistance };

	float T_MATH_ALIGN16 startPos[4];
	searchFrom.storeAligned(startPos);

	dtPolyRef startRef;
	float T_MATH_ALIGN16 startPosN[4];

	status = navQuery->findNearestPoly(
		startPos,
		searchExtents,
		filter.ptr(),
		&startRef,
		startPosN);
	if (dtStatusFailed(status))
	{
		dtFreeNavMeshQuery(navQuery);
		return false;
	}

	outPoint = Vector4::loadAligned(startPosN).xyz1();

	dtFreeNavMeshQuery(navQuery);
	return true;
}

bool NavMesh::findClosestPointXZ(const Vector4& searchFrom, float searchDistance, Vector4& outPoint) const
{
	dtNavMeshQuery* navQuery = dtAllocNavMeshQuery();
	if (!navQuery)
		return false;

	dtStatus status = navQuery->init(m_navMesh, 2048);
	if (dtStatusFailed(status))
	{
		dtFreeNavMeshQuery(navQuery);
		return false;
	}

	AutoPtr< dtQueryFilter > filter(new dtQueryFilter());

	float T_MATH_ALIGN16 searchPos[4];
	searchFrom.storeAligned(searchPos);

	// Wide in X and Z but the entire vertical column (a large-but-finite Y half-extent,
	// so the query box stays well clear of infinities), so a point high above the mesh
	// -- a player up a tower or on a roof -- still resolves to the ground beneath it.
	const float searchExtents[3] = { searchDistance, 1.0e6f, searchDistance };

	dtPolyRef polys[256];
	int polyCount = 0;
	status = navQuery->queryPolygons(
		searchPos,
		searchExtents,
		filter.ptr(),
		polys,
		&polyCount,
		sizeof_array(polys));
	if (dtStatusFailed(status) || polyCount <= 0)
	{
		dtFreeNavMeshQuery(navQuery);
		return false;
	}

	// Pick the polygon whose closest point is nearest in the XZ plane; break vertical
	// ties (surfaces stacked at the same spot) by whichever sits closest in Y to the
	// search point.
	bool found = false;
	float bestXZ = std::numeric_limits< float >::max();
	float bestDY = std::numeric_limits< float >::max();
	float T_MATH_ALIGN16 bestPos[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	for (int32_t i = 0; i < polyCount; ++i)
	{
		float closest[3];
		bool posOverPoly = false;
		if (dtStatusFailed(navQuery->closestPointOnPoly(polys[i], searchPos, closest, &posOverPoly)))
			continue;

		const float dx = closest[0] - searchPos[0];
		const float dz = closest[2] - searchPos[2];
		const float xz = dx * dx + dz * dz;
		const float ady = std::abs(closest[1] - searchPos[1]);

		if (xz < bestXZ - 1e-4f)
		{
			// Strictly closer in the XZ plane: new winner, reset the vertical tie-break.
			bestXZ = xz;
			bestDY = ady;
			bestPos[0] = closest[0];
			bestPos[1] = closest[1];
			bestPos[2] = closest[2];
			found = true;
		}
		else if (xz <= bestXZ + 1e-4f && ady < bestDY)
		{
			// Same spot in XZ (stacked surfaces): keep the vertically closer one.
			bestDY = ady;
			bestPos[0] = closest[0];
			bestPos[1] = closest[1];
			bestPos[2] = closest[2];
			found = true;
		}
	}

	dtFreeNavMeshQuery(navQuery);

	if (!found)
		return false;

	outPoint = Vector4::loadAligned(bestPos).xyz1();
	return true;
}

bool NavMesh::findRandomPoint(Vector4& outPoint) const
{
	dtNavMeshQuery* navQuery = dtAllocNavMeshQuery();
	if (!navQuery)
		return false;

	dtStatus status = navQuery->init(m_navMesh, 2048);
	if (dtStatusFailed(status))
		return false;

	AutoPtr< dtQueryFilter > filter(new dtQueryFilter());

	dtPolyRef randomRef;
	float T_MATH_ALIGN16 randomPosN[4];

	status = navQuery->findRandomPoint(
		filter.ptr(),
		&random,
		&randomRef,
		randomPosN);
	if (dtStatusFailed(status))
	{
		dtFreeNavMeshQuery(navQuery);
		return false;
	}

	outPoint = Vector4::loadAligned(randomPosN).xyz1();

	dtFreeNavMeshQuery(navQuery);
	return true;
}

bool NavMesh::findRandomPoint(const Vector4& center, float radius, Vector4& outPoint) const
{
	dtNavMeshQuery* navQuery = dtAllocNavMeshQuery();
	if (!navQuery)
		return false;

	dtStatus status = navQuery->init(m_navMesh, 2048);
	if (dtStatusFailed(status))
		return false;

	AutoPtr< dtQueryFilter > filter(new dtQueryFilter());

	float T_MATH_ALIGN16 centerPos[4];
	center.storeAligned(centerPos);

	dtPolyRef startRef;
	float T_MATH_ALIGN16 startPosN[4];

	status = navQuery->findNearestPoly(
		centerPos,
		c_searchExtents,
		filter.ptr(),
		&startRef,
		startPosN);

	dtPolyRef randomRef;
	float T_MATH_ALIGN16 randomPosN[4];

	status = navQuery->findRandomPointAroundCircle(
		startRef,
		centerPos,
		radius,
		filter.ptr(),
		&random,
		&randomRef,
		randomPosN);
	if (dtStatusFailed(status))
	{
		dtFreeNavMeshQuery(navQuery);
		return false;
	}

	outPoint = Vector4::loadAligned(randomPosN).xyz1();

	dtFreeNavMeshQuery(navQuery);
	return true;
}

}
