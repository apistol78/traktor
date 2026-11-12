/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/SmallSet.h"
#include "Core/Guid.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Vector4.h"
#include "Core/RefArray.h"
#include "Core/Thread/ReaderWriterLock.h"
#include "Resource/Proxy.h"
#include "World/IWorldComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

class dtNavMesh;
class dtNavMeshQuery;
struct dtMeshTile;

namespace traktor::ai
{

class MoveQueryResult;
class NavMesh;
class NavMeshChunk;

/*! Navigation mesh component.
 * \ingroup AI
 *
 * Navigation mesh of a world, instantiated from a navigation mesh
 * resource; chunks of a chunked navigation mesh are added by
 * navigation mesh chunk components.
 */
class T_DLLCLASS NavMeshComponent : public world::IWorldComponent
{
	T_RTTI_CLASS;

public:
	NavMeshComponent() = default;

	explicit NavMeshComponent(const resource::Proxy< NavMesh >& navMesh);

	virtual ~NavMeshComponent();

	virtual void destroy() override final;

	virtual void update(const world::UpdateParams& update) override final;

	/*! Create a movement query from start to end position.
	 *
	 * Since calculating a movement query is really expensive
	 * we need to be able to amortize this call, thus
	 * it will return a "deferred result" which
	 * will become ready sometime in the future.
	 *
	 * A goal beyond added chunks of a chunked navigation mesh
	 * is approached as far as added; query again when there.
	 *
	 * \param startPosition Start of movement.
	 * \param endPosition End of movement.
	 * \return Movement query async result.
	 */
	Ref< MoveQueryResult > createMoveQuery(const Vector4& startPosition, const Vector4& endPosition) const;

	/*! Find closest point on navigation mesh.
	 *
	 * Searches a cube of half-extent \a searchDistance (uniform on all three axes)
	 * around \a searchFrom.
	 *
	 * \param searchFrom Search from point.
	 * \param searchDistance Search half-extent, applied uniformly on all axes.
	 * \param outPoint Closest point on navigation mesh.
	 * \return True if closest point found.
	 */
	bool findClosestPoint(const Vector4& searchFrom, float searchDistance, Vector4& outPoint) const;

	/*! Find closest point on navigation mesh in the XZ plane only.
	 *
	 * Searches \a searchDistance in X and Z but the whole vertical column (Y is
	 * effectively unbounded), so it resolves to ground directly beneath \a searchFrom
	 * no matter how far above the mesh the point is. When several walkable surfaces
	 * stack at the same spot (e.g. ground under a roof) the one whose height is closest
	 * to searchFrom.y is returned.
	 *
	 * \param searchFrom Search from point.
	 * \param searchDistance Search half-extent in the X and Z axes.
	 * \param outPoint Closest point on navigation mesh.
	 * \return True if closest point found.
	 */
	bool findClosestPointXZ(const Vector4& searchFrom, float searchDistance, Vector4& outPoint) const;

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

	/*!
	 */
	bool findDistanceToWall(const Vector4& center, float radius, float& outDistance) const;

	/*! Enable or disable navigation through a modifier's area.
	 *
	 * \param id Id of modifier's owner entity.
	 * \param enable True if area should be navigable.
	 * \return True if modifier found.
	 */
	bool setModifierEnable(const Guid& id, bool enable);

	/*! Add chunk of a chunked navigation mesh, as soon as there is room for it.
	 *
	 * \param chunk Chunk to add.
	 */
	void addChunk(const NavMeshChunk* chunk);

	/*! Remove chunk added earlier.
	 *
	 * \param chunk Chunk to remove.
	 */
	void removeChunk(const NavMeshChunk* chunk);

private:
	friend class NavMeshComponentEditor;

	struct AddedChunk
	{
		Ref< const NavMeshChunk > chunk;
		AlignedVector< uint64_t > tiles;	//!< References to chunk's tiles in navigation mesh.
	};

	resource::Proxy< NavMesh > m_navMesh;
	dtNavMesh* m_detourNavMesh = nullptr;
	SmallSet< Guid > m_disabledModifiers;
	AlignedVector< AddedChunk > m_added;
	RefArray< const NavMeshChunk > m_pending;	//!< Chunks waiting for room in navigation mesh.
	Aabb3 m_addedBounds;
	int32_t m_freeTiles = 0;
	mutable ReaderWriterLock m_lock;

	dtNavMeshQuery* createQuery() const;

	bool addTiles(const NavMeshChunk* chunk);

	void applyModifier(const dtMeshTile* tile, const Aabb3& bounds, uint8_t area, bool enable);
};

}
