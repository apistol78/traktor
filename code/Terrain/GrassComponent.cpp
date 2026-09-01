/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/GrassComponent.h"

#include "Core/Containers/StaticVector.h"
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

#include <limits>

namespace traktor::terrain
{
namespace
{

//! Number of vertical segments in a single blade; the top segment converges into the tip vertex.
const int32_t c_bladeSegments = 4;
const int32_t c_bladeVertexCount = 2 * c_bladeSegments + 1;
const int32_t c_bladeTriangleCount = 2 * ((c_bladeSegments - 1) * 2 + 1); // Both windings.

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
	float dummy1;
	float scale;
	float random;
	float dummy2;
	float dummy3;
};

#pragma pack()

const render::Handle s_handleTerrain_Normals(L"Terrain_Normals");
const render::Handle s_handleTerrain_Heightfield(L"Terrain_Heightfield");
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

bool GrassComponent::updateBladeBuffer()
{
	m_bladeBuffer = nullptr;

	if (!m_bladesCount)
		return true;

	m_bladeBuffer = m_renderSystem->createBuffer(render::BufferUsage::BuStructured, m_bladesCount * sizeof(BladeData), false, T_FILE_LINE_W);
	if (!m_bladeBuffer)
		return false;

	BladeData* bladeData = (BladeData*)m_bladeBuffer->lock();
	if (!bladeData)
	{
		m_bladeBuffer = nullptr;
		return false;
	}

	for (const Cluster& cluster : m_clusters)
	{
		RandomGeometry random(int32_t(cluster.center.x() * 919.0f + cluster.center.z() * 463.0f));
		for (int32_t j = cluster.from; j < cluster.to; ++j)
		{
			const Vector2 ruv = Quasirandom::hammersley(j - cluster.from, cluster.to - cluster.from, random);

			const float dx = (ruv.x * 2.2f - 1.1f) * m_clusterSize;
			const float dz = (ruv.y * 2.2f - 1.1f) * m_clusterSize;

			const float px = cluster.center.x() + dx;
			const float pz = cluster.center.z() + dz;

			auto& bd = bladeData[j];
			bd.positionX = px;
			bd.positionZ = pz;
			bd.rotation = random.nextFloat() * TWO_PI;
			bd.dummy1 = 0.0f;
			bd.scale = cluster.scale * (random.nextFloat() * 0.5f + 0.5f);
			bd.random = random.nextFloat();
			bd.dummy2 = 0.0f;
			bd.dummy3 = 0.0f;
		}
	}

	m_bladeBuffer->unlock();
	return true;
}

void GrassComponent::setup(const world::WorldRenderView& worldRenderView)
{
	if (!m_bladesCount)
		return;

	// Blade data is view independent and constant over time; build it on demand,
	// it's invalidated when the clusters are rebuilt.
	if (m_bladeBuffer == nullptr && !updateBladeBuffer())
		return;

	const Matrix44 view = worldRenderView.getView();

	// Get blade state for current view.
	ViewState& vs = m_viewState[worldRenderView.getIndex()];
	if (vs.orderBuffer == nullptr || vs.orderBuffer->getBufferSize() / sizeof(int32_t) != m_bladesCount)
	{
		vs.drawInstanceCount = 0;
		vs.orderBuffer = m_renderSystem->createBuffer(render::BufferUsage::BuStructured, m_bladesCount * sizeof(int32_t), true, T_FILE_LINE_W);
		if (!vs.orderBuffer)
			return;
	}

	Frustum viewFrustum = worldRenderView.getViewFrustum();
	viewFrustum.setFarZ(Scalar(m_layerData.m_spreadDistance + m_clusterSize));

	vs.drawInstanceCount = 0;

	int32_t* orderPtr = (int32_t*)vs.orderBuffer->lock();
	if (!orderPtr)
		return;

	// Compact indices of visible blades; the blade data itself is already resident
	// and indexed through this list.
	const Scalar clusterSize(m_clusterSize);
	for (const Cluster& cluster : m_clusters)
	{
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

	if (!m_bladesCount)
		return;

	const auto& terrain = terrainComponent->getTerrain();

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

	if (!m_bladeBuffer)
		return;

	ViewState& vs = m_viewState[worldRenderView.getIndex()];
	if (!vs.orderBuffer || vs.drawInstanceCount <= 0)
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
	renderBlock->programParams->setTextureParameter(s_handleTerrain_Normals, terrain->getNormalMap());
	renderBlock->programParams->setTextureParameter(s_handleTerrain_Heightfield, terrain->getHeightMap());
	renderBlock->programParams->setTextureParameter(s_handleTerrain_SurfaceAlbedo, terrainComponent->getSurfaceCache()->getBaseTexture());
	renderBlock->programParams->setVectorParameter(s_handleTerrain_WorldExtent, terrain->getHeightfield()->getWorldExtent());
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
	m_clusters.resize(0);
	m_bladesCount = 0;

	// Blade buffer is built from the clusters; invalidate it and let setup rebuild
	// it on the next frame.
	m_bladeBuffer = nullptr;

	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (!terrainComponent)
		return;

	const resource::Proxy< Terrain >& terrain = terrainComponent->getTerrain();
	const resource::Proxy< hf::Heightfield >& heightfield = terrain->getHeightfield();

	// Get set of materials which have grass.
	StaticVector< uint8_t, 16 > um;
	um.resize(16, 0);

	uint8_t maxMaterialIndex = 0;
	for (const auto& grass : m_layerData.m_grass)
		um[grass.attribute] = ++maxMaterialIndex;

	const int32_t size = heightfield->getSize();
	const Vector4 extentPerGrid = heightfield->getWorldExtent() / Scalar(float(size));

	m_clusterSize = (16.0f / 2.0f) * max< float >(extentPerGrid.x(), extentPerGrid.z());

	// Create clusters.
	RandomGeometry random;
	for (int32_t z = 0; z < size; z += 16)
	{
		for (int32_t x = 0; x < size; x += 16)
		{
			StaticVector< int32_t, 16 > cm;
			cm.resize(16, 0);

			int32_t totalDensity = 0;
			for (int32_t cz = 0; cz < 16; ++cz)
			{
				for (int32_t cx = 0; cx < 16; ++cx)
				{
					const uint8_t attribute = heightfield->getGridAttribute(x + cx, z + cz);
					const uint8_t index = um[attribute];
					if (index > 0)
					{
						cm[index - 1]++;
						totalDensity++;
					}
				}
			}
			if (totalDensity <= 0)
				continue;

			float wx, wz;
			heightfield->gridToWorld(x + 8, z + 8, wx, wz);

			const float wy = heightfield->getWorldHeight(wx, wz);

			for (uint32_t i = 0; i < maxMaterialIndex; ++i)
			{
				if (cm[i] <= 0)
					continue;

				for (const auto& grass : m_layerData.m_grass)
				{
					if (um[grass.attribute] == i + 1)
					{
						const int32_t densityFactor = cm[i];

						const int32_t density = (grass.density * densityFactor) / (16 * 16);
						if (density <= 4)
							continue;

						const int32_t from = m_bladesCount;
						const int32_t to = from + density;

						Cluster c;
						c.center = Vector4(wx, wy, wz, 1.0f);
						c.scale = grass.scale * (0.5f + 0.5f * densityFactor / (16.0f * 16.0f));
						c.from = from;
						c.to = to;
						m_clusters.push_back(c);

						m_bladesCount = to;
					}
				}
			}
		}
	}
}

}
