#include <DetourNavMeshQuery.h>
#include "Ai/MoveQuery.h"
#include "Ai/NavMesh.h"
#include "Core/Log/Log.h"
#include "Core/Math/Random.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor
{
	namespace ai
	{
		namespace
		{

const float c_searchExtents[3] = { 32.0f, 32.0f, 32.0f };

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

Ref< MoveQuery > NavMesh::createMoveQuery(const Vector4& startPosition, const Vector4& endPosition)
{
	dtNavMeshQuery* navQuery = dtAllocNavMeshQuery();
	if (!navQuery)
		return 0;

	dtStatus status = navQuery->init(m_navMesh, 2048);
	if (dtStatusFailed(status))
		return 0;

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
		startPosN
	);
	if (dtStatusFailed(status))
	{
		log::error << L"NavQuery; Unable to find start reference, status "; FormatHex(log::error, status, 8); log::error << Endl;
		return 0;
	}

	status = outputQuery->m_navQuery->findNearestPoly(
		endPos,
		c_searchExtents,
		outputQuery->m_filter,
		&endRef,
		endPosN
	);
	if (dtStatusFailed(status))
	{
		log::error << L"NavQuery; Unable to find end reference, status "; FormatHex(log::error, status, 8); log::error << Endl;
		return 0;
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
		sizeof_array(outputQuery->m_path)
	);
	if (dtStatusFailed(status) || outputQuery->m_pathCount <= 0)
	{
		// Failed to create navmesh path; most probably no valid route exists.
		// Create a short-cut path to move navigation entity back on track.
		outputQuery->m_steerPath.push_back(outputQuery->m_endPosition);
		return outputQuery;
	}

	float steerPath[256 * 3 + 1];
	int32_t steerPathCount = 0;

	status = outputQuery->m_navQuery->findStraightPath(
		startPosN,
		endPosN,
		outputQuery->m_path,
		outputQuery->m_pathCount,
		steerPath,
		0,
		0,
		&steerPathCount,
		256
	);
	if (dtStatusFailed(status) || steerPathCount <= 0)
	{
		// Failed to create navmesh path; most probably no valid route exists.
		// Create a short-cut path to move navigation entity back on track.
		outputQuery->m_steerPath.push_back(outputQuery->m_endPosition);
		return outputQuery;
	}

	outputQuery->m_steerPath.reserve(steerPathCount);
	for (int32_t i = 0; i < steerPathCount; ++i)
		outputQuery->m_steerPath.push_back(Vector4::loadUnaligned(&steerPath[i * 3]).xyz1());

	return outputQuery;
}

bool NavMesh::findClosestPoint(const Vector4& searchFrom, Vector4& outPoint) const
{
	dtNavMeshQuery* navQuery = dtAllocNavMeshQuery();
	if (!navQuery)
		return false;

	dtStatus status = navQuery->init(m_navMesh, 2048);
	if (dtStatusFailed(status))
		return 0;

	AutoPtr< dtQueryFilter > filter(new dtQueryFilter());

	float T_MATH_ALIGN16 startPos[4];
	searchFrom.storeAligned(startPos);

	dtPolyRef startRef;
	float T_MATH_ALIGN16 startPosN[4];

	status = navQuery->findNearestPoly(
		startPos,
		c_searchExtents,
		filter.ptr(),
		&startRef,
		startPosN
	);
	if (dtStatusFailed(status))
	{
		dtFreeNavMeshQuery(navQuery);
		return false;
	}

	outPoint = Vector4::loadAligned(startPosN).xyz1();

	dtFreeNavMeshQuery(navQuery);
	return true;
}

bool NavMesh::findRandomPoint(Vector4& outPoint) const
{
	dtNavMeshQuery* navQuery = dtAllocNavMeshQuery();
	if (!navQuery)
		return false;

	dtStatus status = navQuery->init(m_navMesh, 2048);
	if (dtStatusFailed(status))
		return 0;

	AutoPtr< dtQueryFilter > filter(new dtQueryFilter());

	dtPolyRef randomRef;
	float T_MATH_ALIGN16 randomPosN[4];

	status = navQuery->findRandomPoint(
		filter.ptr(),
		&random,
		&randomRef,
		randomPosN
	);
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
		return 0;

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
		startPosN
	);

	dtPolyRef randomRef;
	float T_MATH_ALIGN16 randomPosN[4];

	status = navQuery->findRandomPointAroundCircle(
		startRef,
		centerPos,
		radius,
		filter.ptr(),
		&random,
		&randomRef,
		randomPosN
	);
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
}
