#include <DetourNavMeshQuery.h>
#include "Ai/MoveQuery.h"
#include "Ai/NavMesh.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ai
	{
		namespace
		{

const float c_searchExtents[3] = { 512.0f, 1024.0f, 512.0f };

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
	if (dtStatusFailed(status))
	{
		log::error << L"NavQuery; findPath failed, status "; FormatHex(log::error, status, 8); log::error << Endl;
		return 0;
	}

	if (outputQuery->m_pathCount <= 0)
	{
		log::error << L"NavQuery; findPath failed, no path references" << Endl;
		return 0;
	}

	return outputQuery;
}

bool NavMesh::findClosestPoint(const Vector4& searchFrom, Vector4& outPoint) const
{
	dtStatus status;

	dtNavMeshQuery* navQuery = dtAllocNavMeshQuery();
	if (!navQuery)
		return false;

	dtQueryFilter* filter = new dtQueryFilter();

	float T_MATH_ALIGN16 startPos[4];
	searchFrom.storeAligned(startPos);

	dtPolyRef startRef;
	float T_MATH_ALIGN16 startPosN[4];

	status = navQuery->findNearestPoly(
		startPos,
		c_searchExtents,
		filter,
		&startRef,
		startPosN
	);
	if (dtStatusFailed(status))
	{
		dtFreeNavMeshQuery(navQuery);
		delete filter;
		return false;
	}

	outPoint = Vector4::loadAligned(startPosN).xyz1();

	dtFreeNavMeshQuery(navQuery);
	delete filter;
	return true;
}

	}
}
