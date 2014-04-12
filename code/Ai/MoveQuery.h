#ifndef traktor_ai_MoveQuery_H
#define traktor_ai_MoveQuery_H

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

namespace traktor
{
	namespace ai
	{

class T_DLLCLASS MoveQuery : public Object
{
	T_RTTI_CLASS;

public:
	MoveQuery();

	virtual ~MoveQuery();

	bool update(const Vector4& currentPosition, Vector4& outMoveToPosition);

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
}

#endif	// traktor_ai_MoveQuery_H
