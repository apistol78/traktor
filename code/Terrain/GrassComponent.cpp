/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/GrassComponent.h"

#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Half.h"
#include "Core/Math/Quasirandom.h"
#include "Core/Math/RandomGeometry.h"
#include "Heightfield/Heightfield.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/IRenderSystem.h"
#include "Render/VertexElement.h"
#include "Resource/IResourceManager.h"
#include "Terrain/GrassComponentData.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "World/Entity.h"
#include "World/Entity/DisplacementWorldComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/World.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

#include <cmath>
#include <cstring>
#include <limits>

namespace traktor::terrain
{
namespace
{

//! Number of vertical segments in a single blade; the top segment converges into the tip vertex.
const int32_t c_bladeSegments = 4;
const int32_t c_bladeVertexCount = 2 * c_bladeSegments + 1;
const int32_t c_bladeTriangleCount = 2 * ((c_bladeSegments - 1) * 2 + 1); // Both windings.

//! Number of heightfield grid cells spanned by a cluster of blades, in each direction.
const int32_t c_clusterGridSize = 16;

#pragma pack(1)

struct Vertex
{
	float position[2]; //!< Lateral offset and normalized height of blade.
	half_t texCoord[2];
};

#pragma pack()

#pragma pack(1)

struct BladeData
{
	float positionX;
	float positionZ;
	float rotation;
	float positionY;
	float scale;
	float random;
	float dummy2;
	float dummy3;
};

#pragma pack()

const render::Handle s_handleTerrain_Normals(L"Terrain_Normals");
const render::Handle s_handleTerrain_SurfaceAlbedo(L"Terrain_SurfaceAlbedo");
const render::Handle s_handleTerrain_WorldExtent(L"Terrain_WorldExtent");
const render::Handle s_handleGrass_Eye(L"Grass_Eye");
const render::Handle s_handleGrass_MaxDistance(L"Grass_MaxDistance");
const render::Handle s_handleGrass_Blades(L"Grass_Blades");
const render::Handle s_handleGrass_Order(L"Grass_Order");

Vertex packVertex(float x, float y, float u, float v)
{
	Vertex vtx;
	vtx.position[0] = x;
	vtx.position[1] = y;
	vtx.texCoord[0] = floatToHalf(u);
	vtx.texCoord[1] = floatToHalf(v);
	return vtx;
}

//! True if an attribute carry any density within a grid rectangle.
bool anyAttributeDensity(const hf::Heightfield* heightfield, int32_t gridX, int32_t gridZ, int32_t gridSize, uint8_t attribute)
{
	for (int32_t z = 0; z < gridSize; ++z)
	{
		for (int32_t x = 0; x < gridSize; ++x)
		{
			if (heightfield->getGridAttributeDensity(gridX + x, gridZ + z, attribute) != 0)
				return true;
		}
	}
	return false;
}

/*! Scatter blades of a grass type across a cluster, each candidate accepted by attribute density at it's position.
 * Deterministic, called once to count blades and once to write first maxBlades into outBlades; return number of blades.
 */
int32_t scatterCluster(
	const hf::Heightfield* heightfield,
	const Vector4& center,
	float clusterSize,
	uint8_t attribute,
	int32_t candidateCount,
	float scale,
	BladeData* outBlades,
	int32_t maxBlades)
{
	RandomGeometry random(int32_t(center.x() * 919.0f + center.z() * 463.0f));

	// A single shift for the entire set decorrelate it from neighbouring clusters
	// without disturbing the stratification within it.
	const Vector2 shift(random.nextFloat(), random.nextFloat());

	int32_t count = 0;
	for (int32_t i = 0; i < candidateCount; ++i)
	{
		const Vector2 ruv = Quasirandom::hammersley((uint32_t)i, (uint32_t)candidateCount, shift);

		const float px = center.x() + (ruv.x * 2.0f - 1.0f) * clusterSize;
		const float pz = center.z() + (ruv.y * 2.0f - 1.0f) * clusterSize;

		const float density = heightfield->getWorldAttributeDensity(px, pz, attribute);

		// Draw every random value unconditionally to keep counting and writing pass in lockstep.
		const float accept = random.nextFloat();
		const float rotation = random.nextFloat();
		const float size = random.nextFloat();
		const float variation = random.nextFloat();

		if (accept >= density)
			continue;

		if (outBlades != nullptr && count < maxBlades)
		{
			BladeData& bd = outBlades[count];
			bd.positionX = px;
			bd.positionZ = pz;
			bd.rotation = rotation * TWO_PI;
			bd.positionY = heightfield->getWorldHeight(px, pz);
			// Blades shrink as density fade out, thinned out regions read as shorter grass.
			bd.scale = scale * (0.5f + 0.5f * density) * (0.5f + 0.5f * size);
			bd.random = variation;
			bd.dummy2 = 0.0f;
			bd.dummy3 = 0.0f;
		}

		++count;
	}

	return count;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.GrassComponent", GrassComponent, TerrainLayerComponent)

bool GrassComponent::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const GrassComponentData& layerData)
{
	m_layerData = layerData;

	if (!resourceManager->bind(m_layerData.m_shader, m_shader))
		return false;

	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat2, offsetof(Vertex, position)));
	vertexElements.push_back(render::VertexElement(render::DataUsage::Custom, render::DtHalf2, offsetof(Vertex, texCoord)));
	T_ASSERT(render::getVertexSize(vertexElements) == sizeof(Vertex));
	m_vertexLayout = renderSystem->createVertexLayout(vertexElements);

	m_vertexBuffer = renderSystem->createBuffer(
		render::BuVertex,
		c_bladeVertexCount * sizeof(Vertex),
		false,
		T_FILE_LINE_W);
	if (!m_vertexBuffer)
		return false;

	Vertex* vertex = static_cast< Vertex* >(m_vertexBuffer->lock());
	if (!vertex)
		return false;

	// Segment rows, root at height 0; quadratic taper towards the tip.
	for (int32_t i = 0; i < c_bladeSegments; ++i)
	{
		const float h = float(i) / c_bladeSegments;
		const float taper = 1.0f - h * h;
		*vertex++ = packVertex(-taper, h, 0.0f, 1.0f - h);
		*vertex++ = packVertex(taper, h, 1.0f, 1.0f - h);
	}

	// Tip vertex.
	*vertex++ = packVertex(0.0f, 1.0f, 0.5f, 0.0f);

	m_vertexBuffer->unlock();

	m_indexBuffer = renderSystem->createBuffer(
		render::BuIndex,
		c_bladeTriangleCount * 3 * sizeof(uint16_t),
		false,
		T_FILE_LINE_W);
	if (!m_indexBuffer)
		return false;

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
	if (!index)
		return false;

	// Both windings so blades are visible from either side.
	for (int32_t i = 0; i < c_bladeSegments - 1; ++i)
	{
		const uint16_t bl = uint16_t(i * 2);
		const uint16_t br = uint16_t(i * 2 + 1);
		const uint16_t tl = uint16_t(i * 2 + 2);
		const uint16_t tr = uint16_t(i * 2 + 3);

		*index++ = bl;
		*index++ = tl;
		*index++ = tr;

		*index++ = bl;
		*index++ = tr;
		*index++ = br;

		*index++ = tr;
		*index++ = tl;
		*index++ = bl;

		*index++ = br;
		*index++ = tr;
		*index++ = bl;
	}

	const uint16_t ll = uint16_t((c_bladeSegments - 1) * 2);
	const uint16_t lr = uint16_t((c_bladeSegments - 1) * 2 + 1);
	const uint16_t tip = uint16_t(c_bladeVertexCount - 1);

	*index++ = ll;
	*index++ = tip;
	*index++ = lr;

	*index++ = lr;
	*index++ = tip;
	*index++ = ll;

	m_indexBuffer->unlock();

	m_renderSystem = renderSystem;
	return true;
}

void GrassComponent::destroy()
{
	m_renderSystem = nullptr;
}

void GrassComponent::setOwner(world::Entity* owner)
{
	TerrainLayerComponent::setOwner(owner);
	m_owner = owner;
}

void GrassComponent::setTransform(const Transform& transform)
{
}

Aabb3 GrassComponent::getBoundingBox() const
{
	return Aabb3();
}

void GrassComponent::update(const world::UpdateParams& update)
{
	TerrainLayerComponent::update(update);
}

void GrassComponent::setup(const world::WorldRenderView& worldRenderView)
{
	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (!terrainComponent)
		return;

	// Clusters follow primary view, scattered again as eye enter another cluster or when tiles change.
	if (worldRenderView.getIndex() == 0 && !worldRenderView.getSnapshot())
	{
		const int32_t tileCount = terrainComponent->getTileCount();
		bool changed = ((int32_t)m_tileRevisions.size() != tileCount);
		m_tileRevisions.resize(tileCount);
		for (int32_t i = 0; i < tileCount; ++i)
		{
			const uint32_t revision = terrainComponent->getTileRevision(i);
			changed |= (m_tileRevisions[i] != revision);
			m_tileRevisions[i] = revision;
		}
		updateClusters(terrainComponent, worldRenderView.getEyePosition(), changed);
	}

	if (!m_bladesCount)
		return;

	ViewState& vs = m_viewState[worldRenderView.getIndex()];
	vs.drawInstanceCount = 0;

	if (vs.orderBuffer == nullptr || vs.orderBuffer->getBufferSize() / sizeof(int32_t) != m_bladesCount)
	{
		vs.orderBuffer = m_renderSystem->createBuffer(render::BufferUsage::BuStructured, m_bladesCount * sizeof(int32_t), true, T_FILE_LINE_W);
		if (!vs.orderBuffer)
			return;
	}

	const Matrix44 view = worldRenderView.getView();

	Frustum viewFrustum = worldRenderView.getViewFrustum();
	viewFrustum.setFarZ(Scalar(m_layerData.m_spreadDistance + m_clusterSize));

	int32_t* orderPtr = (int32_t*)vs.orderBuffer->lock();
	if (!orderPtr)
		return;

	// Compact indices of visible blades, blade data is resident and indexed through this list.
	const Scalar clusterSize(m_clusterSize);
	for (const auto& it : m_clusters)
	{
		const Cluster& cluster = it.second;
		if (viewFrustum.inside(view * cluster.center, clusterSize) == Frustum::Result::Outside)
			continue;

		for (int32_t j = cluster.from; j < cluster.to; ++j)
			*orderPtr++ = j;

		vs.drawInstanceCount += cluster.to - cluster.from;
	}

	vs.orderBuffer->unlock();
}

void GrassComponent::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass)
{
	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (!terrainComponent)
		return;

	if (!m_bladeBuffer)
		return;

	ViewState& vs = m_viewState[worldRenderView.getIndex()];
	if (!vs.orderBuffer || vs.drawInstanceCount <= 0)
		return;

	const auto& terrain = terrainComponent->getTerrain();
	const Terrain::TileResources& overview = terrain->getOverview();

	const Matrix44 view = worldRenderView.getView();
	const Matrix44 viewInv = view.inverse();
	const Vector4 eye = viewInv.translation();

	const world::World* world = m_owner->getWorld();
	const world::DisplacementWorldComponent* displacement = world ? world->getComponent< world::DisplacementWorldComponent >() : nullptr;
	if (displacement != nullptr && displacement->getMask() == nullptr)
		displacement = nullptr;

	render::Shader::Permutation perm = worldRenderPass.getPermutation(m_shader);
	world::DisplacementWorldComponent::getPermutation(displacement, m_shader, perm);

	auto sp = m_shader->getProgram(perm);
	if (!sp)
		return;

	render::RenderContext* renderContext = context.getRenderContext();

	auto renderBlock = renderContext->allocNamed< render::IndexedInstancingRenderBlock >(L"Grass");
	renderBlock->distance = 10000.0f;
	renderBlock->program = sp.program;
	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->indexBuffer = m_indexBuffer->getBufferView();
	renderBlock->indexType = render::IndexType::UInt16;
	renderBlock->vertexBuffer = m_vertexBuffer->getBufferView();
	renderBlock->vertexLayout = m_vertexLayout;
	renderBlock->primitive = render::PrimitiveType::Triangles;
	renderBlock->offset = 0;
	renderBlock->count = c_bladeTriangleCount;
	renderBlock->instanceCount = vs.drawInstanceCount;

	renderBlock->programParams->beginParameters(renderContext);
	worldRenderPass.setProgramParameters(renderBlock->programParams);
	renderBlock->programParams->setTextureParameter(s_handleTerrain_Normals, overview.normalMap);
	renderBlock->programParams->setTextureParameter(s_handleTerrain_SurfaceAlbedo, terrainComponent->getSurfaceCache()->getBaseTexture());
	renderBlock->programParams->setVectorParameter(s_handleTerrain_WorldExtent, overview.heightfield->getWorldExtent());
	renderBlock->programParams->setVectorParameter(s_handleGrass_Eye, eye);
	renderBlock->programParams->setFloatParameter(s_handleGrass_MaxDistance, m_layerData.m_spreadDistance + m_clusterSize);
	renderBlock->programParams->setBufferViewParameter(s_handleGrass_Blades, m_bladeBuffer->getBufferView());
	renderBlock->programParams->setBufferViewParameter(s_handleGrass_Order, vs.orderBuffer->getBufferView());
	world::DisplacementWorldComponent::setSharedParameters(displacement, renderBlock->programParams);
	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(
		sp.priority,
		renderBlock);
}

void GrassComponent::updatePatches()
{
	// Every cluster is scattered again by next setup.
	m_clusters.clear();
	m_valid = false;
}

void GrassComponent::updateClusters(const TerrainComponent* terrainComponent, const Vector4& eye, bool force)
{
	const auto& terrain = terrainComponent->getTerrain();
	const hf::Heightfield* overviewHeightfield = terrain->getOverview().heightfield;
	const int32_t tileCount = terrain->getTileCount();
	const int32_t tileSize = terrain->getTileSize();
	if (!overviewHeightfield || tileCount <= 0 || terrainComponent->getTileCount() != tileCount * tileCount)
		return;

	const Vector4& worldExtent = overviewHeightfield->getWorldExtent();
	const int32_t size = tileCount * tileSize;
	const float cellSize = worldExtent.x() / size;
	const float clusterExtent = c_clusterGridSize * cellSize;
	const int32_t clusterCount = size / c_clusterGridSize;

	const int32_t eyeClusterX = (int32_t)std::floor((eye.x() + worldExtent.x() * 0.5f) / clusterExtent);
	const int32_t eyeClusterZ = (int32_t)std::floor((eye.z() + worldExtent.z() * 0.5f) / clusterExtent);
	if (!force && m_valid && eyeClusterX == m_eyeCluster[0] && eyeClusterZ == m_eyeCluster[1])
		return;

	m_clusterSize = clusterExtent * 0.5f;

	const float radius = m_layerData.m_spreadDistance + clusterExtent;
	const int32_t range = (int32_t)std::ceil(radius / clusterExtent);
	const int32_t cx0 = std::max(eyeClusterX - range, 0);
	const int32_t cz0 = std::max(eyeClusterZ - range, 0);
	const int32_t cx1 = std::min(eyeClusterX + range, clusterCount - 1);
	const int32_t cz1 = std::min(eyeClusterZ + range, clusterCount - 1);

	SmallMap< uint64_t, Cluster > clusters;
	clusters.reserve(std::max((cx1 - cx0 + 1) * (cz1 - cz0 + 1), 0) * m_layerData.m_grass.size());

	// Keys increase with iteration order, thus clusters are appended.
	for (int32_t cz = cz0; cz <= cz1; ++cz)
	{
		for (int32_t cx = cx0; cx <= cx1; ++cx)
		{
			const int32_t tileX = (cx * c_clusterGridSize) / tileSize;
			const int32_t tileZ = (cz * c_clusterGridSize) / tileSize;
			const int32_t tileIndex = tileX + tileZ * tileCount;

			const hf::Heightfield* heightfield = terrainComponent->getTileHeightfield(tileIndex);
			if (!heightfield)
				continue;

			const float wx = -worldExtent.x() * 0.5f + (cx * c_clusterGridSize + c_clusterGridSize / 2 + 0.5f) * cellSize;
			const float wz = -worldExtent.z() * 0.5f + (cz * c_clusterGridSize + c_clusterGridSize / 2 + 0.5f) * cellSize;
			const float dx = wx - eye.x();
			const float dz = wz - eye.z();
			if (dx * dx + dz * dz > radius * radius)
				continue;

			const Vector4 center(wx, heightfield->getWorldHeight(wx, wz), wz, 1.0f);
			const uint32_t revision = terrainComponent->getTileRevision(tileIndex);
			const int32_t gridX = cx * c_clusterGridSize - tileX * tileSize;
			const int32_t gridZ = cz * c_clusterGridSize - tileZ * tileSize;

			for (int32_t i = 0; i < (int32_t)m_layerData.m_grass.size(); ++i)
			{
				const auto& grass = m_layerData.m_grass[i];
				const uint64_t key = ((uint64_t)cz << 40) | ((uint64_t)cx << 16) | (uint64_t)i;

				auto it = m_clusters.find(key);
				if (it != m_clusters.end() && it->second.revision == revision)
				{
					Cluster& c = clusters[key];
					c.center = center;
					c.revision = revision;
					c.blades.swap(it->second.blades);
					continue;
				}

				// Density is filtered, thus a cluster next to a painted region also carry a fringe of grass.
				if (!anyAttributeDensity(heightfield, gridX - 1, gridZ - 1, c_clusterGridSize + 2, grass.attribute))
					continue;

				const int32_t count = scatterCluster(heightfield, center, m_clusterSize, grass.attribute, grass.density, grass.scale, nullptr, 0);
				if (count <= 0)
					continue;

				Cluster& c = clusters[key];
				c.center = center;
				c.revision = revision;
				c.blades.resize(count * 8);
				scatterCluster(heightfield, center, m_clusterSize, grass.attribute, grass.density, grass.scale, (BladeData*)c.blades.ptr(), count);
			}
		}
	}

	m_clusters.swap(clusters);
	m_eyeCluster[0] = eyeClusterX;
	m_eyeCluster[1] = eyeClusterZ;
	m_valid = true;

	// Fill blade buffer with blades of every cluster.
	m_bladesCount = 0;
	for (auto& it : m_clusters)
	{
		it.second.from = m_bladesCount;
		it.second.to = it.second.from + (int32_t)it.second.blades.size() / 8;
		m_bladesCount = it.second.to;
	}

	m_bladeBuffer = nullptr;
	if (!m_bladesCount)
		return;

	m_bladeBuffer = m_renderSystem->createBuffer(render::BufferUsage::BuStructured, m_bladesCount * sizeof(BladeData), false, T_FILE_LINE_W);
	uint8_t* bladeData = m_bladeBuffer ? (uint8_t*)m_bladeBuffer->lock() : nullptr;
	if (!bladeData)
	{
		m_bladeBuffer = nullptr;
		m_bladesCount = 0;
		return;
	}

	for (const auto& it : m_clusters)
		std::memcpy(bladeData + it.second.from * sizeof(BladeData), it.second.blades.c_ptr(), it.second.blades.size() * sizeof(float));

	m_bladeBuffer->unlock();
}

}
