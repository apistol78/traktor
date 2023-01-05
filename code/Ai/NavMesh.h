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
#include "Core/Ref.h"
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

namespace traktor::ai
{

class MoveQueryResult;

/*! Navigation mesh.
 * \ingroup AI
 */
class T_DLLCLASS NavMesh : public Object
{
	T_RTTI_CLASS;

public:
	virtual ~NavMesh();

	/*! Create a movement query from start to end position.
	 *
	 * Since calculating a movement query is really expensive
	 * we need to be able to amortize this call, thus
	 * it will return a "deferred result" which
	 * will become ready sometime in the future.
	 *
	 * \param startPosition Start of movement.
	 * \param endPosition End of movement.
	 * \return Movement query async result.
	 */
	Ref< MoveQueryResult > createMoveQuery(const Vector4& startPosition, const Vector4& endPosition);

	/*! Find closest point on navigation mesh.
	 *
	 * \param searchFrom Search from point.
	 * \param outPoint Closest point on navigation mesh.
	 * \return True if closest point found.
	 */
	bool findClosestPoint(const Vector4& searchFrom, Vector4& outPoint) const;

	/*! Find a random point which is guaranteed to be on navigation mesh.
	 *
	 * \param outPoint Random point on navigation mesh.
	 * \return True if random point found.
	 */
	bool findRandomPoint(Vector4& outPoint) const;

	/*! Find a random point inside a sphere which is also guaranteed to be on navigation mesh.
	 *
	 * \param center Center of search sphere.
	 * \param radius Radius of search sphere.
	 * \param outPoint Random point on navigation mesh and also inside sphere.
	 * \return True if random point found.
	 */
	bool findRandomPoint(const Vector4& center, float radius, Vector4& outPoint) const;

private:
	friend class NavMeshFactory;
	friend class NavMeshComponentEditor;

	dtNavMesh* m_navMesh;
	AlignedVector< Vector4 > m_navMeshVertices;
	AlignedVector< uint16_t > m_navMeshPolygons;
};

}
