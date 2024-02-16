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
#include "Terrain/ForestComponent.h"
#include "Terrain/ForestComponentData.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"
#include "World/Entity.h"

namespace traktor::terrain
{
	namespace
	{

const render::Handle s_techniqueShadowWrite(L"World_ShadowWrite");

const render::Handle s_handleTerrain_Normals(L"Terrain_Normals");
const render::Handle s_handleTerrain_Heightfield(L"Terrain_Heightfield");
const render::Handle s_handleTerrain_Surface(L"Terrain_Surface");
const render::Handle s_handleTerrain_WorldExtent(L"Terrain_WorldExtent");
const render::Handle s_handleForest_Eye(L"Forest_Eye");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.ForestComponent", ForestComponent, TerrainLayerComponent)

bool ForestComponent::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const ForestComponentData& layerData
)
{
	if (!resourceManager->bind(layerData.m_lod0mesh, m_lod0mesh))
		return false;
	if (!resourceManager->bind(layerData.m_lod1mesh, m_lod1mesh))
		return false;
	if (!resourceManager->bind(layerData.m_lod2mesh, m_lod2mesh))
		return false;

	// Create a union of all lod's bounding boxes for culling.
	m_boundingBox = m_lod0mesh->getBoundingBox();
	m_boundingBox.contain(m_lod1mesh->getBoundingBox());
	m_boundingBox.contain(m_lod2mesh->getBoundingBox());

	m_data = layerData;
	return true;
}

void ForestComponent::destroy()
{
}

void ForestComponent::setOwner(world::Entity* owner)
{
	TerrainLayerComponent::setOwner(owner);
	m_owner = owner;
}

void ForestComponent::setTransform(const Transform& transform)
{
}

Aabb3 ForestComponent::getBoundingBox() const
{
	return Aabb3();
}

void ForestComponent::update(const world::UpdateParams& update)
{
	TerrainLayerComponent::update(update);
}

void ForestComponent::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (!terrainComponent)
		return;

	const resource::Proxy< Terrain >& terrain = terrainComponent->getTerrain();

	const Frustum& cullFrustum = worldRenderView.getCullFrustum();
	const Matrix44& view = worldRenderView.getView();
	const Vector4 eye = view.inverse().translation();

	if (worldRenderPass.getTechnique() != s_techniqueShadowWrite)
	{
		const bool updateClusters = (bool)((worldRenderPass.getPassFlags() & world::IWorldRenderPass::First) != 0);
		if (updateClusters)
		{
			m_lod0Indices.resize(0);
			m_lod1Indices.resize(0);
			m_lod2Indices.resize(0);

			for (uint32_t i = 0; i < (uint32_t)m_trees.size(); ++i)
			{
				const auto& tree = m_trees[i];

				float distance = 0.0f;
				if (!worldRenderView.isBoxVisible(
					m_boundingBox,
					Transform(tree.position),
					distance
				))
					continue;

				if (distance < m_data.m_lod0distance)
					m_lod0Indices.push_back(i);
				else if (distance < m_data.m_lod1distance)
					m_lod1Indices.push_back(i);
				else if (distance < m_data.m_lod2distance)
					m_lod2Indices.push_back(i);
			}
		}
	}
	else
	{
		m_lodShadowIndices.resize(0);

		for (uint32_t i = 0; i < (uint32_t)m_trees.size(); ++i)
		{
			const auto& tree = m_trees[i];

			float distance = 0.0f;
			if (!worldRenderView.isBoxVisible(
				m_boundingBox,
				Transform(tree.position),
				distance
			))
				continue;

			m_lodShadowIndices.push_back(i);
		}
	}

	render::RenderContext* renderContext = context.getRenderContext();

	// Expose some more shader parameters, such as terrain color etc.
	render::ProgramParameters* extraParameters = renderContext->alloc< render::ProgramParameters >();
	extraParameters->beginParameters(renderContext);
	extraParameters->setTextureParameter(s_handleTerrain_Normals, terrain->getNormalMap());
	extraParameters->setTextureParameter(s_handleTerrain_Heightfield, terrain->getHeightMap());
	extraParameters->setTextureParameter(s_handleTerrain_Surface, terrainComponent->getSurfaceCache(worldRenderView.getIndex())->getBaseTexture());
	extraParameters->setVectorParameter(s_handleTerrain_WorldExtent, terrain->getHeightfield()->getWorldExtent());
	extraParameters->setVectorParameter(s_handleForest_Eye, eye);
	extraParameters->endParameters(renderContext);
/*
	if (worldRenderPass.getTechnique() != s_techniqueShadowWrite)
	{
		for (uint32_t i = 0; i < m_lod2Indices.size(); )
		{
			const uint32_t batch = std::min< uint32_t >(m_lod2Indices.size() - i, mesh::InstanceMesh::MaxInstanceCount);

			m_instanceData.resize(batch);
			for (int32_t j = 0; j < batch; ++j, ++i)
			{
				m_trees[m_lod2Indices[i]].rotation.e.storeAligned(m_instanceData[j].data.rotation);
				m_trees[m_lod2Indices[i]].position.storeAligned(m_instanceData[j].data.translation);
				m_instanceData[j].data.scale = m_trees[m_lod2Indices[i]].scale;
				m_instanceData[j].distance = 0.0f;
			}

			m_lod2mesh->build(
				renderContext,
				worldRenderPass,
				m_instanceData,
				extraParameters
			);
		}

		for (uint32_t i = 0; i < m_lod1Indices.size(); )
		{
			const uint32_t batch = std::min< uint32_t >(m_lod1Indices.size() - i, mesh::InstanceMesh::MaxInstanceCount);

			m_instanceData.resize(batch);
			for (int32_t j = 0; j < batch; ++j, ++i)
			{
				m_trees[m_lod1Indices[i]].rotation.e.storeAligned(m_instanceData[j].data.rotation);
				m_trees[m_lod1Indices[i]].position.storeAligned(m_instanceData[j].data.translation);
				m_instanceData[j].data.scale = m_trees[m_lod1Indices[i]].scale;
				m_instanceData[j].distance = 0.0f;
			}

			m_lod1mesh->build(
				renderContext,
				worldRenderPass,
				m_instanceData,
				extraParameters
			);
		}

		for (uint32_t i = 0; i < m_lod0Indices.size(); )
		{
			const uint32_t batch = std::min< uint32_t >(m_lod0Indices.size() - i, mesh::InstanceMesh::MaxInstanceCount);

			m_instanceData.resize(batch);
			for (int32_t j = 0; j < batch; ++j, ++i)
			{
				m_trees[m_lod0Indices[i]].rotation.e.storeAligned(m_instanceData[j].data.rotation);
				m_trees[m_lod0Indices[i]].position.storeAligned(m_instanceData[j].data.translation);
				m_instanceData[j].data.scale = m_trees[m_lod0Indices[i]].scale;
				m_instanceData[j].distance = 0.0f;
			}

			m_lod0mesh->build(
				renderContext,
				worldRenderPass,
				m_instanceData,
				extraParameters
			);
		}
	}
	else
	{
		for (uint32_t i = 0; i < m_lodShadowIndices.size(); )
		{
			const uint32_t batch = std::min< uint32_t >(m_lodShadowIndices.size() - i, mesh::InstanceMesh::MaxInstanceCount);

			m_instanceData.resize(batch);
			for (int32_t j = 0; j < batch; ++j, ++i)
			{
				m_trees[m_lodShadowIndices[i]].rotation.e.storeAligned(m_instanceData[j].data.rotation);
				m_trees[m_lodShadowIndices[i]].position.storeAligned(m_instanceData[j].data.translation);
				m_instanceData[j].data.scale = m_trees[m_lodShadowIndices[i]].scale;
				m_instanceData[j].distance = 0.0f;
			}

			m_lod2mesh->build(
				renderContext,
				worldRenderPass,
				m_instanceData,
				extraParameters
			);
		}
	}
*/
}

void ForestComponent::updatePatches()
{
	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (!terrainComponent)
		return;

	const auto& terrain = terrainComponent->getTerrain();
	const auto& heightfield = terrain->getHeightfield();
	const float densityInv = 1.0f / m_data.m_density;
	const int32_t size = heightfield->getSize();
	const Vector4 extentPerGrid = heightfield->getWorldExtent() / Scalar(float(size));
	Random random;

	m_trees.resize(0);
	for (float z = 0; z < size; z += densityInv)
	{
		for (float x = 0; x < size; x += densityInv)
		{
			// Check if trees are allowed on this position.
			if (heightfield->getGridAttribute(x, z) != m_data.m_attribute)
				continue;

			// Get world position.
			float wx, wy, wz;
			heightfield->gridToWorld(x, z, wx, wz);
			wx += extentPerGrid.x() * densityInv * (random.nextFloat() - 0.5f);
			wz += extentPerGrid.z() * densityInv * (random.nextFloat() - 0.5f);
			wy = heightfield->getWorldHeight(wx, wz);

			// Get ground normal.
			float gx, gz;
			heightfield->worldToGrid(wx, wz, gx, gz);
			Vector4 normal = heightfield->normalAt(gx, gz);

			// Check slope angle threshold.
			const float slopeAngle = acos(normal.y());
			if (slopeAngle > m_data.m_slopeAngleThreshold)
				continue;

			// Calculate rotation.
			const float rx = (random.nextFloat() * 2.0f - 1.0f) * m_data.m_randomTilt;
			const float rz = (random.nextFloat() * 2.0f - 1.0f) * m_data.m_randomTilt;
			const float head = random.nextFloat() * TWO_PI;
			const Quaternion Qu = slerp(Quaternion(Vector4(0.0f, 1.0f, 0.0f), normal), Quaternion::identity(), m_data.m_upness);
			const Quaternion Qr = Quaternion::fromAxisAngle(Vector4(1.0f, 0.0f, 0.0f), rx) * Quaternion::fromAxisAngle(Vector4(0.0f, 0.0f, 1.0f), rz);
			const Quaternion Qh = Quaternion::fromAxisAngle(Vector4(0.0f, 1.0f, 0.0f), head);

			// Add tree on position.
			auto& tree = m_trees.push_back();
			tree.position = Vector4(wx, wy, wz, 1.0f);
			tree.rotation = Qr * Qu * Qh;
			tree.scale = random.nextFloat() * m_data.m_randomScale + (1.0f - m_data.m_randomScale);
		}
	}
}

}
