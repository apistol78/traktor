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

const float c_searchExtents[3] = { 16.0f, 32.0f, 16.0f };

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

	float startPos[4], endPos[4];
	startPosition.storeUnaligned(startPos);
	endPosition.storeUnaligned(endPos);

	Ref< MoveQuery > outputQuery = new MoveQuery();
	outputQuery->m_navQuery = navQuery;

	dtPolyRef startRef, endRef;
	float startPosN[4], endPosN[4];

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

	outputQuery->m_startPosition = Vector4::loadUnaligned(startPosN).xyz1();
	outputQuery->m_endPosition = Vector4::loadUnaligned(endPosN).xyz1();

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

	}
}
