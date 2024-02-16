/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Half.h"
#include "Core/Math/RandomGeometry.h"
#include "Heightfield/Heightfield.h"
#include "Render/ITexture.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "Terrain/RubbleComponent.h"
#include "Terrain/RubbleComponentData.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

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
	const RubbleComponentData& data
)
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

void RubbleComponent::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	// Get terrain from owner.
	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (!terrainComponent)
		return;

	const auto& terrain = terrainComponent->getTerrain();
	if (!terrain)
		return;

	const auto& heightfield = terrain->getHeightfield();
	if (!heightfield)
		return;

	// Update clusters at first pass from eye pow.
	const bool updateClusters = (bool)((worldRenderPass.getPassFlags() & world::IWorldRenderPass::First) != 0);

	const Matrix44 view = worldRenderView.getView();
	const Vector4 eye = view.inverse().translation();
	const Vector4 fwd = view.axisZ();

	if (updateClusters)
	{
		if (
			(eye - m_eye).length() >= m_clusterSize / 2.0f ||
			dot3(fwd, m_fwd) < cos(deg2rad(2.0f))
		)
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
					const float py = terrain->getHeightfield()->getWorldHeight(px, pz);

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

	render::RenderContext* renderContext = context.getRenderContext();

	// Expose some more shader parameters, such as terrain color etc.
	render::ProgramParameters* extraParameters = renderContext->alloc< render::ProgramParameters >();
	extraParameters->beginParameters(renderContext);
	extraParameters->setTextureParameter(s_handleTerrain_Normals, terrain->getNormalMap());
	extraParameters->setTextureParameter(s_handleTerrain_Heightfield, terrain->getHeightMap());
	extraParameters->setTextureParameter(s_handleTerrain_Surface, terrainComponent->getSurfaceCache(worldRenderView.getIndex())->getBaseTexture());
	extraParameters->setVectorParameter(s_handleTerrain_WorldOrigin, -terrain->getHeightfield()->getWorldExtent() * 0.5_simd);
	extraParameters->setVectorParameter(s_handleTerrain_WorldExtent, terrain->getHeightfield()->getWorldExtent());
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
	m_instances.resize(0);
	m_clusters.resize(0);

	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (!terrainComponent)
		return;

	const auto& terrain = terrainComponent->getTerrain();
	const auto& heightfield = terrain->getHeightfield();

	// Get set of materials which have undergrowth.
	StaticVector< uint8_t, 16 > um;
	um.resize(16, 0);

	uint8_t maxMaterialIndex = 0;
	for (const auto& rubble : m_rubble)
		um[rubble.attribute] = ++maxMaterialIndex;

	const int32_t size = heightfield->getSize();
	const Vector4 extentPerGrid = heightfield->getWorldExtent() / Scalar(float(size));

	m_clusterSize = (16.0f / 2.0f) * max< float >(extentPerGrid.x(), extentPerGrid.z());

	// Create clusters.
	Random random;
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

				for (auto& rubble : m_rubble)
				{
					if (um[rubble.attribute] != i + 1)
						continue;

					const int32_t densityFactor = cm[i];
					const int32_t density = (rubble.density * densityFactor) / (16 * 16);
					if (density <= 4)
						continue;

					const int32_t from = (int32_t)m_instances.size();
					for (int32_t k = 0; k < density; ++k)
					{
						Instance& instance = m_instances.push_back();
						instance.position = Vector4::zero();
						instance.rotation = Quaternion::identity();
						instance.scale = 0.0f;
					}
					const int32_t to = (int32_t)m_instances.size();

					Cluster& c = m_clusters.push_back();
					c.rubbleDef = &rubble;
					c.center = Vector4(wx, wy, wz, 1.0f);
					c.distance = std::numeric_limits< float >::max();
					c.seed = (int32_t)random.next();
					c.from = from;
					c.to = to;
					c.visible = false;
				}
			}
		}
	}

	// Move last eye position, forces rescatter of visible clusters.
	m_eye = Vector4::zero();
}

}
