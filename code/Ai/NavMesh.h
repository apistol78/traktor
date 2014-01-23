#ifndef traktor_ai_NavMesh_H
#define traktor_ai_NavMesh_H

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

class dtNavMesh;

namespace traktor
{
	namespace ai
	{

class MoveQuery;

class T_DLLCLASS NavMesh : public Object
{
	T_RTTI_CLASS;

public:
	virtual ~NavMesh();

	Ref< MoveQuery > createMoveQuery(const Vector4& startPosition, const Vector4& endPosition);

	bool findClosestPoint(const Vector4& searchFrom, Vector4& outPoint) const;

	bool findRandomPoint(Vector4& outPoint) const;

	bool findRandomPoint(const Vector4& center, float radius, Vector4& outPoint) const;

private:
	friend class NavMeshFactory;
	friend class NavMeshEntityEditor;

	dtNavMesh* m_navMesh;
	AlignedVector< Vector4 > m_navMeshVertices;
	AlignedVector< uint16_t > m_navMeshPolygons;
};

	}
}

#endif	// traktor_ai_NavMesh_H
