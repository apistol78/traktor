/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <limits>
#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Half.h"
#include "Core/Math/RandomGeometry.h"
#include "Heightfield/Heightfield.h"
#include "Resource/IResourceManager.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/ISimpleTexture.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "Terrain/UndergrowthLayer.h"
#include "Terrain/UndergrowthLayerData.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

#if !defined(__PS3__)
const int32_t c_maxInstanceCount = 180;
#else
const int32_t c_maxInstanceCount = 20;
#endif

#pragma pack(1)
struct Vertex
{
	float position[2];
	half_t texCoord[2];
};
#pragma pack()

render::handle_t s_handleNormals;
render::handle_t s_handleHeightfield;
render::handle_t s_handleSurface;
render::handle_t s_handleWorldExtent;
render::handle_t s_handleEye;
render::handle_t s_handleMaxDistance;
render::handle_t s_handleInstances1;
render::handle_t s_handleInstances2;

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.UndergrowthLayer", UndergrowthLayer, ITerrainLayer)

UndergrowthLayer::UndergrowthLayer()
:	m_clusterSize(0.0f)
,	m_plantsCount(0)
{
	s_handleNormals = render::getParameterHandle(L"Normals");
	s_handleHeightfield = render::getParameterHandle(L"Heightfield");
	s_handleSurface = render::getParameterHandle(L"Surface");
	s_handleWorldExtent = render::getParameterHandle(L"WorldExtent");
	s_handleEye = render::getParameterHandle(L"Eye");
	s_handleMaxDistance = render::getParameterHandle(L"MaxDistance");
	s_handleInstances1 = render::getParameterHandle(L"Instances1");
	s_handleInstances2 = render::getParameterHandle(L"Instances2");
}

bool UndergrowthLayer::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const UndergrowthLayerData& layerData,
	const TerrainComponent& terrainComponent
)
{
	m_layerData = layerData;

	if (!resourceManager->bind(m_layerData.m_shader, m_shader))
		return false;

	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, position)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtHalf2, offsetof(Vertex, texCoord)));
	T_ASSERT (render::getVertexSize(vertexElements) == sizeof(Vertex));

	m_vertexBuffer = renderSystem->createVertexBuffer(
		vertexElements,
		4 * sizeof(Vertex),
		false
	);
	if (!m_vertexBuffer)
		return false;

	Vertex* vertex = static_cast< Vertex* >(m_vertexBuffer->lock());
	if (!vertex)
		return false;

	Vector4 position(0.0f, 0.0f, 0.0f);
	Vector4 axisX(1.0f, 0.0f, 0.0f);
	Vector4 axisY(0.0f, 1.0f, 0.0f);

	*vertex++ = packVertex(position - axisX - axisY, 0.0f, 1.0f);
	*vertex++ = packVertex(position - axisX + axisY, 0.0f, 0.0f);
	*vertex++ = packVertex(position + axisX + axisY, 1.0f, 0.0f);
	*vertex++ = packVertex(position + axisX - axisY, 1.0f, 1.0f);

	m_vertexBuffer->unlock();

	m_indexBuffer = renderSystem->createIndexBuffer(
		render::ItUInt16,
		3 * 2 * 2 * sizeof(uint16_t),
		false
	);
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

	updatePatches(terrainComponent);
	return true;
}

void UndergrowthLayer::update(const world::UpdateParams& update)
{
}

void UndergrowthLayer::render(
	TerrainComponent& terrainComponent,
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	const resource::Proxy< Terrain >& terrain = terrainComponent.getTerrain();

	// Update clusters at first pass from eye pow.
	bool updateClusters = bool((worldRenderPass.getPassFlags() & world::IWorldRenderPass::PfFirst) != 0);

	Matrix44 view = worldRenderView.getView();
	Matrix44 viewInv = view.inverse();
	Vector4 eye = viewInv.translation();

	// Get plant state for current view.
	ViewState& vs = m_viewState[worldRenderView.getIndex()];
	if (vs.plants.size() != m_plantsCount * 2)
	{
		vs.plants.resize(m_plantsCount * 2, Vector4::zero());
		vs.distances.resize(m_clusters.size(), 0.0f);
		vs.pvs.assign(m_clusters.size(), false);
		updateClusters = true;
	}

	if (updateClusters)
	{
		Frustum viewFrustum = worldRenderView.getViewFrustum();
		viewFrustum.setFarZ(Scalar(m_layerData.m_spreadDistance + m_clusterSize));

		// Only perform "replanting" half of clusters each frame.
		const Scalar clusterSize(m_clusterSize);
		for (uint32_t i = vs.count % 2; i < m_clusters.size(); i += 2)
		{
			const Cluster& cluster = m_clusters[i];

			vs.distances[i] = (cluster.center - eye).length();

			bool visible = vs.pvs[i];
			vs.pvs.set(i, viewFrustum.inside(view * cluster.center, clusterSize) != Frustum::IrOutside);
			if (!vs.pvs[i])
				continue;
			if (vs.pvs[i] && visible)
				continue;

			RandomGeometry random(int32_t(cluster.center.x() * 919.0f + cluster.center.z() * 463.0f));
			for (int32_t j = cluster.from; j < cluster.to; ++j)
			{
				float dx = (random.nextFloat() * 2.0f - 1.0f) * m_clusterSize;
				float dz = (random.nextFloat() * 2.0f - 1.0f) * m_clusterSize;

				float px = cluster.center.x() + dx;
				float pz = cluster.center.z() + dz;

				vs.plants[j * 2 + 0] = Vector4(
					px,
					pz,
					float(cluster.plant),
					0.0f
				);
				vs.plants[j * 2 + 1] = Vector4(
					cluster.plantScale * (random.nextFloat() * 0.5f + 0.5f),
					random.nextFloat(),
					0.0f,
					0.0f
				);
			}
		}

		vs.count++;
	}

	worldRenderPass.setShaderTechnique(m_shader);
	worldRenderPass.setShaderCombination(m_shader);

	render::IProgram* program = m_shader->getCurrentProgram();
	if (!program)
		return;

	render::RenderContext* renderContext = worldContext.getRenderContext();

	Vector4 instanceData1[c_maxInstanceCount];
	Vector4 instanceData2[c_maxInstanceCount];

	for (uint32_t i = 0; i < m_clusters.size(); ++i)
	{
		if (!vs.pvs[i])
			continue;

		const Cluster& cluster = m_clusters[i];

		int32_t count = cluster.to - cluster.from;
		for (int32_t j = 0; j < count; )
		{
			int32_t batch = std::min(count - j, c_maxInstanceCount);

			for (int32_t k = 0; k < batch; ++k, ++j)
			{
				instanceData1[k] = vs.plants[(j + cluster.from) * 2 + 0];
				instanceData2[k] = vs.plants[(j + cluster.from) * 2 + 1];
			}

			render::IndexedInstancingRenderBlock* renderBlock = renderContext->alloc< render::IndexedInstancingRenderBlock >();

			renderBlock->distance = vs.distances[i];
			renderBlock->program = program;
			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->indexBuffer = m_indexBuffer;
			renderBlock->vertexBuffer = m_vertexBuffer;
			renderBlock->primitive = render::PtTriangles;
			renderBlock->offset = 0;
			renderBlock->count = 2 * 2;
			renderBlock->minIndex = 0;
			renderBlock->maxIndex = 3;
			renderBlock->instanceCount = batch;

			renderBlock->programParams->beginParameters(renderContext);
			worldRenderPass.setProgramParameters(renderBlock->programParams);
			renderBlock->programParams->setTextureParameter(s_handleNormals, terrain->getNormalMap());
			renderBlock->programParams->setTextureParameter(s_handleHeightfield, terrain->getHeightMap());
			renderBlock->programParams->setTextureParameter(s_handleSurface, terrainComponent.getSurfaceCache()->getBaseTexture());
			renderBlock->programParams->setVectorParameter(s_handleWorldExtent, terrain->getHeightfield()->getWorldExtent());
			renderBlock->programParams->setVectorParameter(s_handleEye, eye);
			renderBlock->programParams->setFloatParameter(s_handleMaxDistance, m_layerData.m_spreadDistance + m_clusterSize);
			renderBlock->programParams->setVectorArrayParameter(s_handleInstances1, instanceData1, count);
			renderBlock->programParams->setVectorArrayParameter(s_handleInstances2, instanceData2, count);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->draw(
				render::RpOpaque,
				renderBlock
			);
		}
	}
}

void UndergrowthLayer::updatePatches(const TerrainComponent& terrainComponent)
{
	m_clusters.resize(0);
	m_plantsCount = 0;

	const resource::Proxy< Terrain >& terrain = terrainComponent.getTerrain();
	const resource::Proxy< hf::Heightfield >& heightfield = terrain->getHeightfield();

	// Get set of materials which have undergrowth.
	StaticVector< uint8_t, 16 > um(16, 0);
	uint8_t maxMaterialIndex = 0;
	for (std::vector< UndergrowthLayerData::Plant >::const_iterator i = m_layerData.m_plants.begin(); i != m_layerData.m_plants.end(); ++i)
		um[i->material] = ++maxMaterialIndex;

	int32_t size = heightfield->getSize();
	Vector4 extentPerGrid = heightfield->getWorldExtent() / Scalar(float(size));

	m_clusterSize = (16.0f / 2.0f) * max< float >(extentPerGrid.x(), extentPerGrid.z());

	// Create clusters.
	RandomGeometry random;
	for (int32_t z = 0; z < size; z += 16)
	{
		for (int32_t x = 0; x < size; x += 16)
		{
			StaticVector< int32_t, 16 > cm(16, 0);
			int32_t totalDensity = 0;

			for (int32_t cz = 0; cz < 16; ++cz)
			{
				for (int32_t cx = 0; cx < 16; ++cx)
				{
					uint8_t material = heightfield->getGridMaterial(x + cx, z + cz);
					uint8_t index = um[material];
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

			float wy = heightfield->getWorldHeight(wx, wz);

			for (uint32_t i = 0; i < maxMaterialIndex; ++i)
			{
				if (cm[i] <= 0)
					continue;

				for (std::vector< UndergrowthLayerData::Plant >::const_iterator j = m_layerData.m_plants.begin(); j != m_layerData.m_plants.end(); ++j)
				{
					if (um[j->material] == i + 1)
					{
						int32_t densityFactor = cm[i];

						int32_t density = (j->density * densityFactor) / (16 * 16);
						if (density <= 4)
							continue;

						int32_t from = m_plantsCount;
						int32_t to = from + density;

						Cluster c;
						c.center = Vector4(wx, wy, wz, 1.0f);
						c.plant = j->plant;
						c.plantScale = j->scale * (0.5f + 0.5f * densityFactor / (16.0f * 16.0f));
						c.from = from;
						c.to = to;
						m_clusters.push_back(c);

						m_plantsCount = to;
					}
				}
			}
		}
	}
}

	}
}
