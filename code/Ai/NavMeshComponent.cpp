/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/NavMeshComponent.h"

#include "Ai/MoveQuery.h"
#include "Ai/MoveQueryResult.h"
#include "Ai/NavMesh.h"
#include "Ai/NavMeshChunk.h"
#include "Core/Log/Log.h"
#include "Core/Math/Random.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Thread/JobManager.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <limits>

namespace traktor::ai
{
namespace
{

const float c_searchExtents[3] = { 32.0f, 1.0f, 32.0f };

float random()
{
	static thread_local Random s_rnd;
	return s_rnd.nextFloat();
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.NavMeshComponent", NavMeshComponent, world::IWorldComponent)

NavMeshComponent::NavMeshComponent(const resource::Proxy< NavMesh >& navMesh)
	: m_navMesh(navMesh)
{
	dtNavMeshParams params;
	params.orig[0] = navMesh->m_origin[0];
	params.orig[1] = navMesh->m_origin[1];
	params.orig[2] = navMesh->m_origin[2];
	params.tileWidth = navMesh->m_tileWidth;
	params.tileHeight = navMesh->m_tileHeight;
	params.maxTiles = navMesh->m_maxTiles;
	params.maxPolys = navMesh->m_maxPolys;

	m_detourNavMesh = dtAllocNavMesh();
	if (!m_detourNavMesh || dtStatusFailed(m_detourNavMesh->init(&params)))
	{
		log::error << L"Unable to create navigation mesh." << Endl;
		dtFreeNavMesh(m_detourNavMesh);
		m_detourNavMesh = nullptr;
		return;
	}
	m_freeTiles = params.maxTiles;

	if (navMesh->m_tiles)
		addTiles(navMesh->m_tiles);
}

NavMeshComponent::~NavMeshComponent()
{
	// Navigation mesh is freed last since pending queries might still use it.
	dtFreeNavMesh(m_detourNavMesh);
}

void NavMeshComponent::destroy()
{
	m_pending.clear();
	m_added.clear();
	m_navMesh.clear();
}

void NavMeshComponent::update(const world::UpdateParams& update)
{
	// Pending chunks get room as chunks streamed out are removed.
	for (int32_t i = 0; i < (int32_t)m_pending.size();)
		if (addTiles(m_pending[i]))
			m_pending.erase(m_pending.begin() + i);
		else
			++i;
}

Ref< MoveQueryResult > NavMeshComponent::createMoveQuery(const Vector4& startPosition, const Vector4& endPosition) const
{
	// Extent of tiles of added chunks.
	Aabb3 added;
	{
		T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(m_lock);
		added = m_addedBounds;
	}
	const bool chunked = m_navMesh && !m_navMesh->m_chunks.empty() && !added.empty();
	const float addedX0 = added.mn.x();
	const float addedZ0 = added.mn.z();
	const float addedX1 = added.mx.x();
	const float addedZ1 = added.mx.z();

	Ref< MoveQueryResult > result = new MoveQueryResult();
	JobManager::getInstance().add([=, self = Ref< const NavMeshComponent >(this)]() {
		T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(m_lock);

		dtNavMeshQuery* navQuery = createQuery();
		if (!navQuery)
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

		dtStatus status = outputQuery->m_navQuery->findNearestPoly(
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

		// Goal outside added chunks; aim for mesh nearest goal on way from start, inside added chunks, instead.
		if (!endRef && chunked && (endPos[0] < addedX0 || endPos[0] > addedX1 || endPos[2] < addedZ0 || endPos[2] > addedZ1))
		{
			const Vector4 delta = endPosition - startPosition;
			float t = 1.0f;
			if (endPos[0] < addedX0)
				t = std::min(t, (addedX0 - startPos[0]) / delta.x());
			if (endPos[0] > addedX1)
				t = std::min(t, (addedX1 - startPos[0]) / delta.x());
			if (endPos[2] < addedZ0)
				t = std::min(t, (addedZ0 - startPos[2]) / delta.z());
			if (endPos[2] > addedZ1)
				t = std::min(t, (addedZ1 - startPos[2]) / delta.z());
			t = std::max(t, 0.0f);

			const float searchExtents[3] = { c_searchExtents[0], 1.0e4f, c_searchExtents[2] };
			const float distance = t * std::sqrt(delta.x() * delta.x() + delta.z() * delta.z());
			const int32_t steps = std::max((int32_t)std::ceil(distance / c_searchExtents[0]), 1);
			for (int32_t i = 0; i <= steps && !endRef; ++i)
			{
				float T_MATH_ALIGN16 atPos[4];
				(startPosition + delta * Scalar(t * (1.0f - (float)i / steps))).storeAligned(atPos);
				outputQuery->m_navQuery->findNearestPoly(
					atPos,
					searchExtents,
					outputQuery->m_filter,
					&endRef,
					endPosN);
			}
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

bool NavMeshComponent::findClosestPoint(const Vector4& searchFrom, float searchDistance, Vector4& outPoint) const
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(m_lock);

	dtNavMeshQuery* navQuery = createQuery();
	if (!navQuery)
		return false;

	AutoPtr< dtQueryFilter > filter(new dtQueryFilter());

	const float searchExtents[3] = { searchDistance, searchDistance, searchDistance };

	float T_MATH_ALIGN16 startPos[4];
	searchFrom.storeAligned(startPos);

	dtPolyRef startRef;
	float T_MATH_ALIGN16 startPosN[4];

	dtStatus status = navQuery->findNearestPoly(
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

bool NavMeshComponent::findClosestPointXZ(const Vector4& searchFrom, float searchDistance, Vector4& outPoint) const
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(m_lock);

	dtNavMeshQuery* navQuery = createQuery();
	if (!navQuery)
		return false;

	AutoPtr< dtQueryFilter > filter(new dtQueryFilter());

	float T_MATH_ALIGN16 searchPos[4];
	searchFrom.storeAligned(searchPos);

	// Wide in X and Z but the entire vertical column (a large-but-finite Y half-extent,
	// so the query box stays well clear of infinities), so a point high above the mesh
	// -- a player up a tower or on a roof -- still resolves to the ground beneath it.
	const float searchExtents[3] = { searchDistance, 1.0e6f, searchDistance };

	dtPolyRef polys[256];
	int polyCount = 0;
	dtStatus status = navQuery->queryPolygons(
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

bool NavMeshComponent::findRandomPoint(Vector4& outPoint) const
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(m_lock);

	dtNavMeshQuery* navQuery = createQuery();
	if (!navQuery)
		return false;

	AutoPtr< dtQueryFilter > filter(new dtQueryFilter());

	dtPolyRef randomRef;
	float T_MATH_ALIGN16 randomPosN[4];

	dtStatus status = navQuery->findRandomPoint(
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

bool NavMeshComponent::findRandomPoint(const Vector4& center, float radius, Vector4& outPoint) const
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(m_lock);

	dtNavMeshQuery* navQuery = createQuery();
	if (!navQuery)
		return false;

	AutoPtr< dtQueryFilter > filter(new dtQueryFilter());

	float T_MATH_ALIGN16 centerPos[4];
	center.storeAligned(centerPos);

	dtPolyRef startRef;
	float T_MATH_ALIGN16 startPosN[4];

	dtStatus status = navQuery->findNearestPoly(
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

bool NavMeshComponent::findDistanceToWall(const Vector4& center, float radius, float& outDistance) const
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireReader)(m_lock);

	dtNavMeshQuery* navQuery = createQuery();
	if (!navQuery)
		return false;

	AutoPtr< dtQueryFilter > filter(new dtQueryFilter());

	float T_MATH_ALIGN16 centerPos[4];
	center.storeAligned(centerPos);

	dtPolyRef centerRef;
	float T_MATH_ALIGN16 centerPosN[4];

	dtStatus status = navQuery->findNearestPoly(
		centerPos,
		c_searchExtents,
		filter.ptr(),
		&centerRef,
		centerPosN);
	if (dtStatusFailed(status))
	{
		dtFreeNavMeshQuery(navQuery);
		return false;
	}

	float hitDist;
	float hitPos[4];
	float hitNormal[4];

	status = navQuery->findDistanceToWall(
		centerRef,
		centerPos,
		radius,
		filter.ptr(),
		&hitDist,
		hitPos,
		hitNormal);
	if (dtStatusFailed(status))
	{
		dtFreeNavMeshQuery(navQuery);
		return false;
	}

	outDistance = hitDist;

	dtFreeNavMeshQuery(navQuery);
	return true;
}

bool NavMeshComponent::setModifierEnable(const Guid& id, bool enable)
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireWriter)(m_lock);

	if (!m_navMesh || !m_detourNavMesh)
		return false;

	const auto it = m_navMesh->m_modifiers.find(id);
	if (it == m_navMesh->m_modifiers.end())
		return false;

	if (enable)
		m_disabledModifiers.erase(id);
	else
		m_disabledModifiers.insert(id);

	const dtNavMesh* navMesh = m_detourNavMesh;
	for (int32_t i = 0; i < navMesh->getMaxTiles(); ++i)
	{
		const dtMeshTile* tile = navMesh->getTile(i);
		if (tile->header)
			applyModifier(tile, it->second.bounds, it->second.area, enable);
	}

	return true;
}

void NavMeshComponent::addChunk(const NavMeshChunk* chunk)
{
	if (!addTiles(chunk))
		m_pending.push_back(chunk);
}

void NavMeshComponent::removeChunk(const NavMeshChunk* chunk)
{
	if (m_pending.remove(chunk))
		return;

	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireWriter)(m_lock);

	auto it = m_added.begin();
	while (it != m_added.end() && it->chunk != chunk)
		++it;
	if (it == m_added.end())
		return;

	for (const auto tileRef : it->tiles)
		if (dtStatusSucceed(m_detourNavMesh->removeTile((dtTileRef)tileRef, nullptr, nullptr)))
			++m_freeTiles;
	m_added.erase(it);

	m_addedBounds = Aabb3();
	for (const auto& added : m_added)
		m_addedBounds.contain(added.chunk->m_bounds);
}

dtNavMeshQuery* NavMeshComponent::createQuery() const
{
	if (!m_detourNavMesh)
		return nullptr;

	dtNavMeshQuery* navQuery = dtAllocNavMeshQuery();
	if (navQuery && dtStatusFailed(navQuery->init(m_detourNavMesh, 2048)))
	{
		dtFreeNavMeshQuery(navQuery);
		return nullptr;
	}

	return navQuery;
}

bool NavMeshComponent::addTiles(const NavMeshChunk* chunk)
{
	T_ANONYMOUS_VAR(ReaderWriterLock::AcquireWriter)(m_lock);

	if (!m_detourNavMesh || (int32_t)chunk->m_tiles.size() > m_freeTiles)
		return false;

	auto& added = m_added.push_back();
	added.chunk = chunk;

	// Tiles are copied since navigation mesh writes links into tile data.
	for (const auto& data : chunk->m_tiles)
	{
		uint8_t* tileData = (uint8_t*)dtAlloc((int)data.size(), DT_ALLOC_PERM);
		std::memcpy(tileData, data.c_ptr(), data.size());

		dtTileRef tileRef = 0;
		if (dtStatusFailed(m_detourNavMesh->addTile(tileData, (int)data.size(), DT_TILE_FREE_DATA, 0, &tileRef)))
		{
			log::warning << L"Unable to add tile of navigation mesh." << Endl;
			dtFree(tileData);
			continue;
		}
		added.tiles.push_back(tileRef);
		--m_freeTiles;

		// Restore state of disabled modifiers on new tile.
		const dtMeshTile* tile = m_detourNavMesh->getTileByRef(tileRef);
		for (const auto& id : m_disabledModifiers)
		{
			const auto it = m_navMesh->m_modifiers.find(id);
			applyModifier(tile, it->second.bounds, it->second.area, false);
		}
	}

	m_addedBounds.contain(chunk->m_bounds);
	return true;
}

void NavMeshComponent::applyModifier(const dtMeshTile* tile, const Aabb3& bounds, uint8_t area, bool enable)
{
	const dtPolyRef base = m_detourNavMesh->getPolyRefBase(tile);
	for (int32_t i = 0; i < tile->header->polyCount; ++i)
	{
		const dtPoly& poly = tile->polys[i];
		if (poly.getArea() != area)
			continue;

		// Match bounds as well since area ids are not unique.
		Vector4 center = Vector4::zero();
		for (int32_t j = 0; j < poly.vertCount; ++j)
		{
			const float* p = &tile->verts[poly.verts[j] * 3];
			center += Vector4(p[0], p[1], p[2], 0.0f);
		}
		center /= Scalar(poly.vertCount);

		if (bounds.inside(center))
			m_detourNavMesh->setPolyFlags(base | (dtPolyRef)i, enable ? 0xffff : 0);
	}
}

}
