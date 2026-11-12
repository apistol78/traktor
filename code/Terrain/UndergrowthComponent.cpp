/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/UndergrowthComponent.h"

#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Half.h"
#include "Core/Math/Quasirandom.h"
#include "Core/Math/RandomGeometry.h"
#include "Heightfield/Heightfield.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/IRenderSystem.h"
#include "Render/VertexElement.h"
#include "Resource/IResourceManager.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "Terrain/UndergrowthComponentData.h"
#include "World/Entity.h"
#include "World/Entity/DisplacementWorldComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/World.h"
#include "World/WorldRenderView.h"

#include <cmath>
#include <cstring>
#include <limits>

namespace traktor::terrain
{
namespace
{

//! Number of heightfield grid cells spanned by a cluster of plants, in each direction.
const int32_t c_clusterGridSize = 16;

#pragma pack(1)

struct Vertex
{
	float position[2];
	half_t texCoord[2];
};

#pragma pack()

#pragma pack(1)

struct PlantData
{
	float positionX;
	float positionZ;
	float plant;
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
const render::Handle s_handleUndergrowth_Eye(L"Undergrowth_Eye");
const render::Handle s_handleUndergrowth_MaxDistance(L"Undergrowth_MaxDistance");
const render::Handle s_handleUndergrowth_Plants(L"Undergrowth_Plants");
const render::Handle s_handleUndergrowth_Order(L"Undergrowth_Order");

Vertex packVertex(const Vector4& position, float u, float v)
{
	Vertex vtx;
	vtx.position[0] = position.x();
	vtx.position[1] = position.y();
	vtx.texCoord[0] = floatToHalf(u);
	vtx.texCoord[1] = floatToHalf(v);
	return vtx;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.UndergrowthComponent", UndergrowthComponent, TerrainLayerComponent)

bool UndergrowthComponent::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const UndergrowthComponentData& layerData)
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
		4 * sizeof(Vertex),
		false,
		T_FILE_LINE_W);
	if (!m_vertexBuffer)
		return false;

	Vertex* vertex = static_cast< Vertex* >(m_vertexBuffer->lock());
	if (!vertex)
		return false;

	const Vector4 position(0.0f, 0.0f, 0.0f);
	const Vector4 axisX(1.0f, 0.0f, 0.0f);
	const Vector4 axisY(0.0f, 1.0f, 0.0f);

	*vertex++ = packVertex(position - axisX - axisY, 0.0f, 1.0f);
	*vertex++ = packVertex(position - axisX + axisY, 0.0f, 0.0f);
	*vertex++ = packVertex(position + axisX + axisY, 1.0f, 0.0f);
	*vertex++ = packVertex(position + axisX - axisY, 1.0f, 1.0f);

	m_vertexBuffer->unlock();

	m_indexBuffer = renderSystem->createBuffer(
		render::BuIndex,
		3 * 2 * 2 * sizeof(uint16_t),
		false,
		T_FILE_LINE_W);
	if (!m_indexBuffer)
		return 0;

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());

	*index++ = 0;
	*index++ = 1;
	*index++ = 2;

	*index++ = 0;
	*index++ = 2;
	*index++ = 3;

	*index++ = 2;
	*index++ = 1;
	*index++ = 0;

	*index++ = 3;
	*index++ = 2;
	*index++ = 0;

	m_indexBuffer->unlock();

	m_renderSystem = renderSystem;
	return true;
}

void UndergrowthComponent::destroy()
{
	m_renderSystem = nullptr;
}

void UndergrowthComponent::setOwner(world::Entity* owner)
{
	TerrainLayerComponent::setOwner(owner);
	m_owner = owner;
}

void UndergrowthComponent::setTransform(const Transform& transform)
{
}

Aabb3 UndergrowthComponent::getBoundingBox() const
{
	return Aabb3();
}

void UndergrowthComponent::update(const world::UpdateParams& update)
{
	TerrainLayerComponent::update(update);
}

void UndergrowthComponent::setup(const world::WorldRenderView& worldRenderView)
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

	if (!m_plantsCount)
		return;

	ViewState& vs = m_viewState[worldRenderView.getIndex()];
	vs.drawInstanceCount = 0;

	if (vs.orderBuffer == nullptr || vs.orderBuffer->getBufferSize() / sizeof(int32_t) != m_plantsCount)
	{
		vs.orderBuffer = m_renderSystem->createBuffer(render::BufferUsage::BuStructured, m_plantsCount * sizeof(int32_t), true, T_FILE_LINE_W);
		if (!vs.orderBuffer)
			return;
	}

	const Matrix44 view = worldRenderView.getView();

	Frustum viewFrustum = worldRenderView.getViewFrustum();
	viewFrustum.setFarZ(Scalar(m_layerData.m_spreadDistance + m_clusterSize));

	int32_t* orderPtr = (int32_t*)vs.orderBuffer->lock();
	if (!orderPtr)
		return;

	// Compact indices of visible plants, plant data is resident and indexed through this list.
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

void UndergrowthComponent::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass)
{
	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (!terrainComponent)
		return;

	if (!m_plantBuffer)
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

	auto renderBlock = renderContext->allocNamed< render::IndexedInstancingRenderBlock >(L"Undergrowth");
	renderBlock->distance = 10000.0f;
	renderBlock->program = sp.program;
	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->indexBuffer = m_indexBuffer->getBufferView();
	renderBlock->indexType = render::IndexType::UInt16;
	renderBlock->vertexBuffer = m_vertexBuffer->getBufferView();
	renderBlock->vertexLayout = m_vertexLayout;
	renderBlock->primitive = render::PrimitiveType::Triangles;
	renderBlock->offset = 0;
	renderBlock->count = 2 * 2;
	renderBlock->instanceCount = vs.drawInstanceCount;

	renderBlock->programParams->beginParameters(renderContext);
	worldRenderPass.setProgramParameters(renderBlock->programParams);
	renderBlock->programParams->setTextureParameter(s_handleTerrain_Normals, overview.normalMap);
	renderBlock->programParams->setTextureParameter(s_handleTerrain_SurfaceAlbedo, terrainComponent->getSurfaceCache()->getBaseTexture());
	renderBlock->programParams->setVectorParameter(s_handleTerrain_WorldExtent, overview.heightfield->getWorldExtent());
	renderBlock->programParams->setVectorParameter(s_handleUndergrowth_Eye, eye);
	renderBlock->programParams->setFloatParameter(s_handleUndergrowth_MaxDistance, m_layerData.m_spreadDistance + m_clusterSize);
	renderBlock->programParams->setBufferViewParameter(s_handleUndergrowth_Plants, m_plantBuffer->getBufferView());
	renderBlock->programParams->setBufferViewParameter(s_handleUndergrowth_Order, vs.orderBuffer->getBufferView());
	world::DisplacementWorldComponent::setSharedParameters(displacement, renderBlock->programParams);
	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(
		sp.priority,
		renderBlock);
}

void UndergrowthComponent::updatePatches()
{
	// Every cluster is scattered again by next setup.
	m_clusters.clear();
	m_valid = false;
}

void UndergrowthComponent::updateClusters(const TerrainComponent* terrainComponent, const Vector4& eye, bool force)
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
	clusters.reserve(std::max((cx1 - cx0 + 1) * (cz1 - cz0 + 1), 0) * m_layerData.m_plants.size());

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

			for (int32_t i = 0; i < (int32_t)m_layerData.m_plants.size(); ++i)
			{
				const auto& plant = m_layerData.m_plants[i];
				const uint64_t key = ((uint64_t)cz << 40) | ((uint64_t)cx << 16) | (uint64_t)i;

				auto it = m_clusters.find(key);
				if (it != m_clusters.end() && it->second.revision == revision)
				{
					Cluster& c = clusters[key];
					c.center = center;
					c.revision = revision;
					c.plants.swap(it->second.plants);
					continue;
				}

				// Accumulated attribute density across the cluster, 0 to 16 * 16 when fully painted.
				int32_t densityFactor = 0;
				for (int32_t z = 0; z < c_clusterGridSize; ++z)
				{
					for (int32_t x = 0; x < c_clusterGridSize; ++x)
						densityFactor += heightfield->getGridAttributeDensity(gridX + x, gridZ + z, plant.attribute);
				}
				densityFactor /= 255;
				if (densityFactor <= 0)
					continue;

				const int32_t density = (plant.density * densityFactor) / (c_clusterGridSize * c_clusterGridSize);
				if (density <= 4)
					continue;

				const float plantScale = plant.scale * (0.5f + 0.5f * densityFactor / float(c_clusterGridSize * c_clusterGridSize));

				Cluster& c = clusters[key];
				c.center = center;
				c.revision = revision;
				c.plants.resize(density * 8);

				PlantData* plantData = (PlantData*)c.plants.ptr();
				RandomGeometry random(int32_t(center.x() * 919.0f + center.z() * 463.0f));
				for (int32_t j = 0; j < density; ++j)
				{
					const Vector2 ruv = Quasirandom::hammersley(j, density, random);

					const float px = center.x() + (ruv.x * 2.2f - 1.1f) * m_clusterSize;
					const float pz = center.z() + (ruv.y * 2.2f - 1.1f) * m_clusterSize;

					auto& pd = plantData[j];
					pd.positionX = px;
					pd.positionZ = pz;
					pd.plant = float(plant.plant);
					pd.positionY = heightfield->getWorldHeight(px, pz);
					pd.scale = plantScale * (random.nextFloat() * 0.5f + 0.5f);
					pd.random = random.nextFloat();
					pd.dummy2 = 0.0f;
					pd.dummy3 = 0.0f;
				}
			}
		}
	}

	m_clusters.swap(clusters);
	m_eyeCluster[0] = eyeClusterX;
	m_eyeCluster[1] = eyeClusterZ;
	m_valid = true;

	// Fill plant buffer with plants of every cluster.
	m_plantsCount = 0;
	for (auto& it : m_clusters)
	{
		it.second.from = m_plantsCount;
		it.second.to = it.second.from + (int32_t)it.second.plants.size() / 8;
		m_plantsCount = it.second.to;
	}

	m_plantBuffer = nullptr;
	if (!m_plantsCount)
		return;

	m_plantBuffer = m_renderSystem->createBuffer(render::BufferUsage::BuStructured, m_plantsCount * sizeof(PlantData), false, T_FILE_LINE_W);
	uint8_t* plantData = m_plantBuffer ? (uint8_t*)m_plantBuffer->lock() : nullptr;
	if (!plantData)
	{
		m_plantBuffer = nullptr;
		m_plantsCount = 0;
		return;
	}

	for (const auto& it : m_clusters)
		std::memcpy(plantData + it.second.from * sizeof(PlantData), it.second.plants.c_ptr(), it.second.plants.size() * sizeof(float));

	m_plantBuffer->unlock();
}

}
