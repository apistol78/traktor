//#pragma optimize("", off)

#include <DetourNavMeshQuery.h>
#include "Ai/MoveQuery.h"
#include "Core/Log/Log.h"
#include "Core/Math/Line2.h"
#include "Core/Math/Format.h"

namespace traktor
{
	namespace ai
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.MoveQuery", MoveQuery, Object)

MoveQuery::MoveQuery()
:	m_startPosition(0.0f, 0.0f, 0.0f, 0.0f)
,	m_endPosition(0.0f, 0.0f, 0.0f, 0.0f)
,	m_filter(new dtQueryFilter())
,	m_navQuery(0)
,	m_pathCount(0)
,	m_steerPathCount(0)
,	m_steerPathIter(0)
{
}

MoveQuery::~MoveQuery()
{
	dtFreeNavMeshQuery(m_navQuery);
	delete m_filter;
}

bool MoveQuery::update(const Vector4& currentPosition, Vector4& outMoveToPosition)
{
	if (m_steerPathCount <= 0)
	{
		float startPosition[4];
		currentPosition.storeUnaligned(startPosition);

		float endPosition[4];
		m_endPosition.storeUnaligned(endPosition);

		dtStatus status = m_navQuery->findStraightPath(
			startPosition,
			endPosition,
			m_path,
			m_pathCount,
			m_steerPath,
			m_steerPathFlags,
			m_steerPathPolys,
			&m_steerPathCount,
			MaxSteerPoints
		);

		if (dtStatusFailed(status) || m_steerPathCount < 2)
		{
			log::info << L"MoveQuery; findStraightPath no steer points found (" << m_steerPathCount << L"), status "; FormatHex(log::info, status, 8); log::info << Endl;
			return false;
		}
	}

	if (m_steerPathIter >= m_steerPathCount - 1)
		return false;

	Vector4 steerFrom = Vector4::loadUnaligned(&m_steerPath[m_steerPathIter * 3]).xyz1();
	Vector4 steerTo = Vector4::loadUnaligned(&m_steerPath[(m_steerPathIter + 1) * 3]).xyz1();

	Line2 sub(
		Vector2(steerFrom.x(), steerFrom.z()),
		Vector2(steerTo.x(), steerTo.z())
	);
	Vector2 pt = sub.project(Vector2(currentPosition.x(), currentPosition.z()));
	float k = dot(pt - sub.p[0], sub.delta()) / (sub.length() * sub.length());

	//log::info << L"Steer iter   " << (m_steerPathIter + 1) << L"/" << m_steerPathCount << Endl;
	//log::info << L"  current    " << currentPosition << Endl;
	//log::info << L"  target     " << m_endPosition << Endl;
	//log::info << L"  steer from " << steerFrom << Endl;
	//log::info << L"  steer to   " << steerTo << Endl;
	//log::info << L"  k          " << k << Endl;

	if (k > 1.0f)
		m_steerPathIter++;

	outMoveToPosition = steerTo;
	return true;
}

	}
}
