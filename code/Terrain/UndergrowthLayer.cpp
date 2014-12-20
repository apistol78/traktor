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
render::handle_t s_handleSpreadDistance;
render::handle_t s_handleCellRadius;
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
:	m_spreadDistance(0.0f)
,	m_cellRadius(0.0f)
,	m_eye(Vector4::origo())
{
	s_handleNormals = render::getParameterHandle(L"Normals");
	s_handleHeightfield = render::getParameterHandle(L"Heightfield");
	s_handleSurface = render::getParameterHandle(L"Surface");
	s_handleWorldExtent = render::getParameterHandle(L"WorldExtent");
	s_handleEye = render::getParameterHandle(L"Eye");
	s_handleSpreadDistance = render::getParameterHandle(L"SpreadDistance");
	s_handleCellRadius = render::getParameterHandle(L"CellRadius");
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
	const resource::Proxy< Terrain >& terrain = terrainEntity.getTerrain();

	if (!resourceManager->bind(layerData.m_shader, m_shader))
		return false;

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

	Random rnd;

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

	m_plants.resize(layerData.m_density * 2);

	RandomGeometry random;
	for (int32_t i = 0; i < layerData.m_density; i += /*InstanceCount*/120)
	{
		Cluster c;
		c.center = Vector4(
			(random.nextFloat() * 2.0f - 1.0f) * terrain->getHeightfield()->getWorldExtent().x(),
			0.0f,
			(random.nextFloat() * 2.0f - 1.0f) * terrain->getHeightfield()->getWorldExtent().z(),
			1.0f
		);
		c.distance = std::numeric_limits< float >::max();
		c.visible = false;
		c.plant = 0;
		c.from = i;
		c.to = min< int32_t >(i + /*InstanceCount*/120, layerData.m_density);
		m_clusters.push_back(c);
	}

	m_spreadDistance = layerData.m_spreadDistance;
	m_cellRadius = layerData.m_cellRadius;

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
		viewFrustum.setFarZ(Scalar(m_spreadDistance + m_cellRadius * 2.0f));

		// Only perform "replanting" when moved more than one unit.
		if ((eye - m_eye).length() >= 1.0f)
		{
			m_eye = eye;

			for (AlignedVector< Cluster >::iterator i = m_clusters.begin(); i != m_clusters.end(); ++i)
			{
				Vector4 delta = i->center - m_eye;
				Scalar distance = delta.length();

				RandomGeometry random(int32_t(i->center.x() * 919.0f + i->center.z() * 463.0f));

				if (distance > m_spreadDistance + m_cellRadius)
				{
					float phi = (random.nextFloat() - 0.5f) * HALF_PI;
					float err = distance - (m_spreadDistance + m_cellRadius);

					i->center = m_eye + rotateY(phi) * (-delta * Scalar((m_spreadDistance + m_cellRadius - err - FUZZY_EPSILON) / distance));

					float cy = terrain->getHeightfield()->getWorldHeight(i->center.x(), i->center.z());
					i->center = i->center * Vector4(1.0f, 0.0f, 1.0f, 0.0f) + Vector4(0.0f, cy, 0.0f, 1.0f);

					float gx, gz;
					terrain->getHeightfield()->worldToGrid(i->center.x(), i->center.z(), gx, gz);

					i->plant = 1;
					for (int32_t j = i->from; j < i->to; ++j)
					{
						float dx, dz;
									
						do 
						{
							dx = (random.nextFloat() * 2.0f - 1.0f) * m_cellRadius;
							dz = (random.nextFloat() * 2.0f - 1.0f) * m_cellRadius;
						}
						while (std::sqrt(dx * dx + dz * dz) > m_cellRadius);

						float px = i->center.x() + dx;
						float pz = i->center.z() + dz;

						m_plants[j * 2 + 0] = Vector4(
							px,
							pz,
							0.0f,
							0.0f
						);
						m_plants[j * 2 + 1] = Vector4(
							random.nextFloat() * 0.5f + 0.5f,
							random.nextFloat(),
							0.0f,
							0.0f
						);
					}
				}

				if (i->plant)
					i->visible = (viewFrustum.inside(view * i->center, Scalar(m_cellRadius)) != Frustum::IrOutside);
				else
					i->visible = false;

				i->distance = distance;
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

		float fc = 1.0f - clamp(i->distance / (m_spreadDistance + m_cellRadius), 0.0f, 1.0f);
		int32_t count = i->to - i->from;
		
		count = int32_t(count * (fc * fc));
		if (count <= 0)
			continue;
		
		for (int32_t j = 0; j < count; ++j)
		{
			instanceData1[j] = m_plants[(j + i->from) * 2 + 0];
			instanceData2[j] = m_plants[(j + i->from) * 2 + 1];
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
		renderBlock->instanceCount = count;

		renderBlock->programParams->beginParameters(renderContext);
		worldRenderPass.setProgramParameters(renderBlock->programParams, false);
		renderBlock->programParams->setTextureParameter(s_handleNormals, terrain->getNormalMap());
		renderBlock->programParams->setTextureParameter(s_handleHeightfield, terrain->getHeightMap());
		renderBlock->programParams->setTextureParameter(s_handleSurface, terrainEntity.getSurfaceCache()->getBaseTexture());
		renderBlock->programParams->setVectorParameter(s_handleWorldExtent, terrain->getHeightfield()->getWorldExtent());
		renderBlock->programParams->setVectorParameter(s_handleEye, eye);
		renderBlock->programParams->setFloatParameter(s_handleSpreadDistance, m_spreadDistance);
		renderBlock->programParams->setFloatParameter(s_handleCellRadius, m_cellRadius);
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
}
