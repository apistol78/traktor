/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <DetourNavMeshQuery.h>
#include "Ai/MoveQuery.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Plane.h"

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
,	m_steerIndex(0)
{
}

MoveQuery::~MoveQuery()
{
	dtFreeNavMeshQuery(m_navQuery);
	delete m_filter;
}

bool MoveQuery::update(const Vector4& currentPosition, Vector4& outMoveToPosition)
{
	while (m_steerIndex < m_steerPath.size())
	{
		const Vector4& steerTo = m_steerPath[m_steerIndex];
		Scalar d = ((steerTo - currentPosition) * Vector4(1.0f, 0.0f, 1.0f)).length();
		if (d > 7.5f)
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
}
