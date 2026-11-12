/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/RubbleComponent.h"

#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Half.h"
#include "Core/Math/RandomGeometry.h"
#include "Heightfield/Heightfield.h"
#include "Render/Context/RenderContext.h"
#include "Render/ITexture.h"
#include "Resource/IResourceManager.h"
#include "Terrain/RubbleComponentData.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

#include <limits>

namespace traktor::terrain
{
namespace
{

const render::Handle s_handleTerrain_Normals(L"Terrain_Normals");
const render::Handle s_handleTerrain_Heightfield(L"Terrain_Heightfield");
const render::Handle s_handleTerrain_Surface(L"Terrain_Surface");
const render::Handle s_handleTerrain_WorldOrigin(L"Terrain_WorldOrigin");
const render::Handle s_handleTerrain_WorldExtent(L"Terrain_WorldExtent");
const render::Handle s_handleRubble_Eye(L"Rubble_Eye");
const render::Handle s_handleRubble_MaxDistance(L"Rubble_MaxDistance");

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.RubbleComponent", RubbleComponent, TerrainLayerComponent)

bool RubbleComponent::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const RubbleComponentData& data)
{
	m_rubble.resize(data.m_rubble.size());
	for (size_t i = 0; i < m_rubble.size(); ++i)
	{
		if (!resourceManager->bind(data.m_rubble[i].mesh, m_rubble[i].mesh))
			return false;

		m_rubble[i].attribute = data.m_rubble[i].attribute;
		m_rubble[i].density = data.m_rubble[i].density;
		m_rubble[i].randomScaleAmount = data.m_rubble[i].randomScaleAmount;
		m_rubble[i].randomTilt = data.m_rubble[i].randomTilt;
		m_rubble[i].upness = data.m_rubble[i].upness;
	}

	m_data = data;
	return true;
}

void RubbleComponent::destroy()
{
}

void RubbleComponent::setOwner(world::Entity* owner)
{
	TerrainLayerComponent::setOwner(owner);
	m_owner = owner;
}

void RubbleComponent::setTransform(const Transform& transform)
{
}

Aabb3 RubbleComponent::getBoundingBox() const
{
	return Aabb3();
}

void RubbleComponent::update(const world::UpdateParams& update)
{
	TerrainLayerComponent::update(update);
}

void RubbleComponent::setup(const world::WorldRenderView& worldRenderView)
{
	// Get terrain from owner.
	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (!terrainComponent)
		return;

	// Clusters are created on tiles as they are loaded.
	const int32_t tileCount = terrainComponent->getTileCount();
	if ((int32_t)m_tiles.size() != tileCount)
	{
		m_tiles.resize(0);
		m_tiles.resize(tileCount);
	}

	bool changed = false;
	for (int32_t i = 0; i < tileCount; ++i)
	{
		const uint32_t revision = terrainComponent->getTileRevision(i);
		if (m_tiles[i].revision != revision)
		{
			updateTile(m_tiles[i], i, terrainComponent->getTileHeightfield(i));
			m_tiles[i].revision = revision;
			changed = true;
		}
	}
	if (changed)
	{
		m_clusters.resize(0);
		m_instances.resize(0);
		for (const auto& tile : m_tiles)
		{
			const int32_t offset = (int32_t)m_instances.size();
			for (const auto& cluster : tile.clusters)
			{
				Cluster& c = m_clusters.push_back();
				c = cluster;
				c.from += offset;
				c.to += offset;
			}
			m_instances.resize(offset + tile.instanceCount);
		}

		// Move last eye position, forces scatter of visible clusters.
		m_eye = Vector4::zero();
	}

	const Matrix44 view = worldRenderView.getView();
	const Vector4 eye = view.inverse().translation();
	const Vector4 fwd = view.axisZ();

	if (
		(eye - m_eye).length() >= m_clusterSize / 2.0f ||
		dot3(fwd, m_fwd) < cos(deg2rad(2.0f)))
	{
		Frustum viewFrustum = worldRenderView.getViewFrustum();
		viewFrustum.setFarZ(Scalar(m_data.m_spreadDistance + m_clusterSize));

		m_eye = eye;
		m_fwd = fwd;

		for (auto& cluster : m_clusters)
		{
			cluster.distance = (cluster.center - eye).length();

			const bool visible = cluster.visible;
			cluster.visible = (bool)(viewFrustum.inside(view * cluster.center, Scalar(m_clusterSize)) != Frustum::Result::Outside);
			if (!cluster.visible)
				continue;
			if (cluster.visible && visible)
				continue;

			const hf::Heightfield* heightfield = terrainComponent->getTileHeightfield(cluster.tile);
			if (!heightfield)
				continue;

			const float randomScaleAmount = cluster.rubbleDef->randomScaleAmount;
			const float randomTilt = cluster.rubbleDef->randomTilt;
			const float upness = cluster.rubbleDef->upness;

			RandomGeometry random(cluster.seed);
			for (int32_t j = cluster.from; j < cluster.to; ++j)
			{
				const float dx = (random.nextFloat() * 2.0f - 1.0f) * m_clusterSize;
				const float dz = (random.nextFloat() * 2.0f - 1.0f) * m_clusterSize;

				// Calculate world position.
				const float px = cluster.center.x() + dx;
				const float pz = cluster.center.z() + dz;
				const float py = heightfield->getWorldHeight(px, pz);

				// Get ground normal.
				float gx, gz;
				heightfield->worldToGrid(px, pz, gx, gz);
				const Vector4 normal = heightfield->normalAt(gx, gz);

				// Calculate rotation.
				const float rx = (random.nextFloat() * 2.0f - 1.0f) * randomTilt;
				const float rz = (random.nextFloat() * 2.0f - 1.0f) * randomTilt;
				const float head = random.nextFloat() * TWO_PI;
				const Quaternion Qu = slerp(Quaternion(Vector4(0.0f, 1.0f, 0.0f), normal), Quaternion::identity(), upness);
				const Quaternion Qr = Quaternion::fromAxisAngle(Vector4(1.0f, 0.0f, 0.0f), rx) * Quaternion::fromAxisAngle(Vector4(0.0f, 0.0f, 1.0f), rz);
				const Quaternion Qh = Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f), head);

				// Update instance data.
				m_instances[j].position = Vector4(px, py, pz, 0.0f);
				m_instances[j].rotation = Qr * Qu * Qh;
				m_instances[j].scale = random.nextFloat() * randomScaleAmount + (1.0f - randomScaleAmount);
			}
		}
	}
}

void RubbleComponent::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass)
{
	// Get terrain from owner.
	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (!terrainComponent)
		return;

	const auto& terrain = terrainComponent->getTerrain();
	if (!terrain)
		return;

	const Terrain::TileResources& overview = terrain->getOverview();
	const Vector4& worldExtent = overview.heightfield->getWorldExtent();

	const Matrix44 view = worldRenderView.getView();
	const Vector4 eye = view.inverse().translation();
	const Vector4 fwd = view.axisZ();

	render::RenderContext* renderContext = context.getRenderContext();

	// Expose some more shader parameters, such as terrain color etc.
	render::ProgramParameters* extraParameters = renderContext->alloc< render::ProgramParameters >();
	extraParameters->beginParameters(renderContext);
	extraParameters->setTextureParameter(s_handleTerrain_Normals, overview.normalMap);
	extraParameters->setTextureParameter(s_handleTerrain_Heightfield, overview.heightMap);
	extraParameters->setTextureParameter(s_handleTerrain_Surface, terrainComponent->getSurfaceCache()->getBaseTexture());
	extraParameters->setVectorParameter(s_handleTerrain_WorldOrigin, -worldExtent * 0.5_simd);
	extraParameters->setVectorParameter(s_handleTerrain_WorldExtent, worldExtent);
	extraParameters->setVectorParameter(s_handleRubble_Eye, eye);
	extraParameters->setFloatParameter(s_handleRubble_MaxDistance, m_data.m_spreadDistance + m_clusterSize);
	extraParameters->endParameters(renderContext);
	/*
		for (const auto& cluster : m_clusters)
		{
			if (!cluster.visible)
				continue;

			const int32_t count = cluster.to - cluster.from;
			for (int32_t j = 0; j < count; )
			{
				const int32_t batch = std::min< int32_t >(count - j, mesh::InstanceMesh::MaxInstanceCount);

				m_instanceData.resize(batch);
				for (int32_t k = 0; k < batch; ++k, ++j)
				{
					m_instances[j + cluster.from].position.storeAligned( m_instanceData[k].data.translation );
					m_instances[j + cluster.from].rotation.e.storeAligned( m_instanceData[k].data.rotation );
					m_instanceData[k].data.scale = m_instances[j + cluster.from].scale;
					m_instanceData[k].distance = cluster.distance;
				}

				cluster.rubbleDef->mesh->build(
					renderContext,
					worldRenderPass,
					m_instanceData,
					extraParameters
				);
			}
		}
	*/
}

void RubbleComponent::updatePatches()
{
	// Every tile is scattered again by next setup.
	for (auto& tile : m_tiles)
		tile.revision = ~0U;
}

void RubbleComponent::updateTile(Tile& tile, int32_t tileIndex, const hf::Heightfield* heightfield)
{
	tile.clusters.resize(0);
	tile.instanceCount = 0;

	if (!heightfield)
		return;

	// Last row and column of tile's heightfield belong to neighbour tile.
	const int32_t size = heightfield->getSize() - 1;
	const Vector4 extentPerGrid = heightfield->getWorldExtent() / Scalar(float(heightfield->getSize()));

	m_clusterSize = (16.0f / 2.0f) * max< float >(extentPerGrid.x(), extentPerGrid.z());

	Random random(tileIndex);
	for (int32_t z = 0; z < size; z += 16)
	{
		for (int32_t x = 0; x < size; x += 16)
		{
			float wx, wz;
			heightfield->gridToWorld(x + 8, z + 8, wx, wz);
			const float wy = heightfield->getWorldHeight(wx, wz);

			for (auto& rubble : m_rubble)
			{
				// Accumulated attribute density across the cluster, 0 to 16 * 16 when fully painted.
				int32_t densityFactor = 0;
				for (int32_t cz = 0; cz < 16; ++cz)
				{
					for (int32_t cx = 0; cx < 16; ++cx)
						densityFactor += heightfield->getGridAttributeDensity(x + cx, z + cz, rubble.attribute);
				}
				densityFactor /= 255;
				if (densityFactor <= 0)
					continue;

				const int32_t density = (rubble.density * densityFactor) / (16 * 16);
				if (density <= 4)
					continue;

				Cluster& c = tile.clusters.push_back();
				c.rubbleDef = &rubble;
				c.center = Vector4(wx, wy, wz, 1.0f);
				c.distance = std::numeric_limits< float >::max();
				c.seed = (int32_t)random.next();
				c.tile = tileIndex;
				c.from = tile.instanceCount;
				c.to = c.from + density;
				c.visible = false;

				tile.instanceCount = c.to;
			}
		}
	}
}

}
