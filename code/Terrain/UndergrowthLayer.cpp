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
#include "Terrain/TerrainEntity.h"
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
	const TerrainEntity& terrainEntity
)
{
	m_layerData = layerData;

	if (!resourceManager->bind(m_layerData.m_shader, m_shader))
		return false;

	const resource::Proxy< Terrain >& terrain = terrainEntity.getTerrain();
	const resource::Proxy< hf::Heightfield >& heightfield = terrain->getHeightfield();

	std::vector< render::VertexElement > vertexElements;
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

	updatePatches(terrainEntity);
	return true;
}

void UndergrowthLayer::update(const world::UpdateParams& update)
{
}

void UndergrowthLayer::render(
	TerrainEntity& terrainEntity,
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	const resource::Proxy< Terrain >& terrain = terrainEntity.getTerrain();

	// \fixme Assume depth pass enabled; need some information about first pass from camera POV.
	bool updateClusters = bool(
		worldRenderPass.getTechnique() == render::getParameterHandle(L"World_DepthWrite") ||
		worldRenderPass.getTechnique() == render::getParameterHandle(L"World_PreLitGBufferWrite") ||
		worldRenderPass.getTechnique() == render::getParameterHandle(L"World_DeferredGBufferWrite")
	);

	const Matrix44& view = worldRenderView.getView();
	Vector4 eye = view.inverse().translation();

	if (updateClusters)
	{
		Frustum viewFrustum = worldRenderView.getViewFrustum();
		viewFrustum.setFarZ(Scalar(m_layerData.m_spreadDistance + m_clusterSize));

		// Only perform "replanting" when moved more than one unit.
		//if ((eye - m_eye).length() >= 1.0f)
		{
			//m_eye = eye;

			for (AlignedVector< Cluster >::iterator i = m_clusters.begin(); i != m_clusters.end(); ++i)
			{
				i->distance = (i->center - eye).length();

				bool visible = i->visible;

				i->visible = (viewFrustum.inside(view * i->center, Scalar(m_clusterSize)) != Frustum::IrOutside);
				if (!i->visible)
					continue;

				if (i->visible && visible)
					continue;

				RandomGeometry random(int32_t(i->center.x() * 919.0f + i->center.z() * 463.0f));
				for (int32_t j = i->from; j < i->to; ++j)
				{
					float dx = (random.nextFloat() * 2.0f - 1.0f) * m_clusterSize;
					float dz = (random.nextFloat() * 2.0f - 1.0f) * m_clusterSize;

					float px = i->center.x() + dx;
					float pz = i->center.z() + dz;

					m_plants[j * 2 + 0] = Vector4(
						px,
						pz,
						float(i->plant),
						0.0f
					);
					m_plants[j * 2 + 1] = Vector4(
						i->plantScale * (random.nextFloat() * 0.5f + 0.5f),
						random.nextFloat(),
						0.0f,
						0.0f
					);
				}
			}
		}
	}

	worldRenderPass.setShaderTechnique(m_shader);
	worldRenderPass.setShaderCombination(m_shader);

	render::IProgram* program = m_shader->getCurrentProgram();
	if (!program)
		return;

	render::RenderContext* renderContext = worldContext.getRenderContext();

	Vector4 instanceData1[/*InstanceCount*/120];
	Vector4 instanceData2[/*InstanceCount*/120];
	uint32_t plantCount = 0;

	for (AlignedVector< Cluster >::const_iterator i = m_clusters.begin(); i != m_clusters.end(); ++i)
	{
		if (!i->visible)
			continue;

		int32_t count = i->to - i->from;
		for (int32_t j = 0; j < count; )
		{
			int32_t batch = std::min(count - j, /*InstanceCount*/120);

			for (int32_t k = 0; k < batch; ++k, ++j)
			{
				instanceData1[k] = m_plants[(j + i->from) * 2 + 0];
				instanceData2[k] = m_plants[(j + i->from) * 2 + 1];
			}

			render::IndexedInstancingRenderBlock* renderBlock = renderContext->alloc< render::IndexedInstancingRenderBlock >();

			renderBlock->distance = i->distance;
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
			worldRenderPass.setProgramParameters(renderBlock->programParams, false);
			renderBlock->programParams->setTextureParameter(s_handleNormals, terrain->getNormalMap());
			renderBlock->programParams->setTextureParameter(s_handleHeightfield, terrain->getHeightMap());
			renderBlock->programParams->setTextureParameter(s_handleSurface, terrainEntity.getSurfaceCache()->getBaseTexture());
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

void UndergrowthLayer::updatePatches(const TerrainEntity& terrainEntity)
{
	m_plants.resize(0);
	m_clusters.resize(0);

	const resource::Proxy< Terrain >& terrain = terrainEntity.getTerrain();
	const resource::Proxy< hf::Heightfield >& heightfield = terrain->getHeightfield();

	// Get set of materials which have undergrowth.
	std::set< uint8_t > undergrowthMaterials;
	for (std::vector< UndergrowthLayerData::Plant >::const_iterator i = m_layerData.m_plants.begin(); i != m_layerData.m_plants.end(); ++i)
		undergrowthMaterials.insert(i->material);

	int32_t size = heightfield->getSize();
	Vector4 extentPerGrid = heightfield->getWorldExtent() / Scalar(size);

	m_clusterSize = (16.0f / 2.0f) * max< float >(extentPerGrid.x(), extentPerGrid.z());

	// Create clusters.
	RandomGeometry random;
	for (int32_t z = 0; z < size; z += 16)
	{
		for (int32_t x = 0; x < size; x += 16)
		{
			std::set< uint8_t > heightfieldMaterials;

			// Get materials inside cluster cell.
			for (int32_t cz = 0; cz < 16; ++cz)
			{
				for (int32_t cx = 0; cx < 16; ++cx)
				{
					uint8_t material = heightfield->getGridMaterial(x + cx, z + cz);
					heightfieldMaterials.insert(material);
				}
			}

			// Determine which plants in this cluster cells.
			std::vector< uint8_t > materials(16);
			std::vector< uint8_t >::iterator it = std::set_intersection(
				undergrowthMaterials.begin(),
				undergrowthMaterials.end(),
				heightfieldMaterials.begin(),
				heightfieldMaterials.end(), 
				materials.begin()
			);
			materials.resize(it - materials.begin());
			if (materials.empty())
				continue;

			float wx, wz;
			heightfield->gridToWorld(x + 8, z + 8, wx, wz);

			float wy = heightfield->getWorldHeight(wx, wz);

			for (std::vector< uint8_t >::const_iterator i = materials.begin(); i != materials.end(); ++i)
			{
				const UndergrowthLayerData::Plant* plantDef = 0;
				for (std::vector< UndergrowthLayerData::Plant >::const_iterator j = m_layerData.m_plants.begin(); j != m_layerData.m_plants.end(); ++j)
				{
					if (j->material == *i)
					{
						plantDef = &(*j);
						break;
					}
				}

				int32_t densityFactor = 0;
				for (int32_t cz = 0; cz < 16; ++cz)
				{
					for (int32_t cx = 0; cx < 16; ++cx)
					{
						uint8_t material = heightfield->getGridMaterial(x + cx, z + cz);
						if (material == *i)
							++densityFactor;
					}
				}

				int32_t density = (plantDef->density * densityFactor) / (16 * 16);
				if (density <= 4)
					continue;

				size_t from = m_plants.size();
				for (int32_t j = 0; j < density; ++j)
				{
					m_plants.push_back(Vector4::zero());
					m_plants.push_back(Vector4::zero());
				}
				size_t to = m_plants.size();

				Cluster c;
				c.center = Vector4(wx, wy, wz, 1.0f);
				c.distance = std::numeric_limits< float >::max();
				c.visible = false;
				c.plant = plantDef->plant;
				c.plantScale = plantDef->scale * (0.5f + 0.5f * densityFactor / (16.0f * 16.0f));
				c.from = from / 2;
				c.to = to / 2;
				m_clusters.push_back(c);
			}
		}
	}
}

	}
}
