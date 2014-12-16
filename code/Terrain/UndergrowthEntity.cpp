#include <limits>
#include "Core/Functor/Functor.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Thread/JobManager.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/MaterialMask.h"
#include "Heightfield/MaterialParams.h"
#include "Render/ISimpleTexture.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/Context/RenderContext.h"
#include "Terrain/Terrain.h"
#include "Terrain/UndergrowthEntity.h"
#include "Terrain/UndergrowthPlant.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

render::handle_t s_handleNormals;
render::handle_t s_handleHeightfield;
render::handle_t s_handleWorldExtent;
render::handle_t s_handleEye;
render::handle_t s_handleSpreadDistance;
render::handle_t s_handleCellRadius;
render::handle_t s_handleInstances1;
render::handle_t s_handleInstances2;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.UndergrowthEntity", UndergrowthEntity, world::Entity)

UndergrowthEntity::UndergrowthEntity(
	const resource::Proxy< Terrain >& terrain,
	const resource::Proxy< hf::MaterialMask >& materialMask,
	const Settings& settings,
	render::VertexBuffer* vertexBuffer,
	render::IndexBuffer* indexBuffer,
	const resource::Proxy< render::Shader >& shader
)
:	m_terrain(terrain)
,	m_materialMask(materialMask)
,	m_settings(settings)
,	m_vertexBuffer(vertexBuffer)
,	m_indexBuffer(indexBuffer)
,	m_shader(shader)
{
	m_plants.resize(m_settings.density * 2);

	RandomGeometry random;
	for (int32_t i = 0; i < m_settings.density; i += InstanceCount)
	{
		Cluster c;
		c.center = Vector4(
			(random.nextFloat() * 2.0f - 1.0f) * m_terrain->getHeightfield()->getWorldExtent().x(),
			0.0f,
			(random.nextFloat() * 2.0f - 1.0f) * m_terrain->getHeightfield()->getWorldExtent().z(),
			1.0f
		);
		c.distance = std::numeric_limits< float >::max();
		c.visible = false;
		c.plant = 0;
		c.from = i;
		c.to = min< int32_t >(i + InstanceCount, m_settings.density);
		m_clusters.push_back(c);
	}

	s_handleNormals = render::getParameterHandle(L"Normals");
	s_handleHeightfield = render::getParameterHandle(L"Heightfield");
	s_handleWorldExtent = render::getParameterHandle(L"WorldExtent");
	s_handleEye = render::getParameterHandle(L"Eye");
	s_handleSpreadDistance = render::getParameterHandle(L"SpreadDistance");
	s_handleCellRadius = render::getParameterHandle(L"CellRadius");
	s_handleInstances1 = render::getParameterHandle(L"Instances1");
	s_handleInstances2 = render::getParameterHandle(L"Instances2");
}

UndergrowthEntity::~UndergrowthEntity()
{
}

void UndergrowthEntity::render(
	render::RenderContext* renderContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	// \fixme Assume depth pass enabled; need some information about first pass from camera POV.
	bool updateClusters = bool(
		worldRenderPass.getTechnique() == render::getParameterHandle(L"World_DepthWrite") ||
		worldRenderPass.getTechnique() == render::getParameterHandle(L"World_PreLitGBufferWrite") ||
		worldRenderPass.getTechnique() == render::getParameterHandle(L"World_DeferredGBufferWrite")
	);

	if (updateClusters)
	{
		Frustum viewFrustum = worldRenderView.getViewFrustum();
		viewFrustum.setFarZ(Scalar(m_settings.spreadDistance + m_settings.cellRadius * 2.0f));

		const Matrix44& view = worldRenderView.getView();
		Vector4 eye = view.inverse().translation();

		// Only perform "replanting" when moved more than one unit.
		if ((eye - m_eye).length() >= 1.0f)
		{
			m_eye = eye;

			for (AlignedVector< Cluster >::iterator i = m_clusters.begin(); i != m_clusters.end(); ++i)
			{
				Vector4 delta = i->center - m_eye;
				Scalar distance = delta.length();

				RandomGeometry random(int32_t(i->center.x() * 919.0f + i->center.z() * 463.0f));

				if (distance > m_settings.spreadDistance + m_settings.cellRadius)
				{
					float phi = (random.nextFloat() - 0.5f) * HALF_PI;
					float err = distance - (m_settings.spreadDistance + m_settings.cellRadius);

					i->center = m_eye + rotateY(phi) * (-delta * Scalar((m_settings.spreadDistance + m_settings.cellRadius - err - FUZZY_EPSILON) / distance));

					float cy = m_terrain->getHeightfield()->getWorldHeight(i->center.x(), i->center.z());
					i->center = i->center * Vector4(1.0f, 0.0f, 1.0f, 0.0f) + Vector4(0.0f, cy, 0.0f, 1.0f);

					float gx, gz;
					m_terrain->getHeightfield()->worldToGrid(i->center.x(), i->center.z(), gx, gz);

					gx *= float(m_materialMask->getSize()) / m_terrain->getHeightfield()->getSize();
					gz *= float(m_materialMask->getSize()) / m_terrain->getHeightfield()->getSize();
					
					int32_t igx = clamp< int32_t >(int32_t(gx), 0, m_materialMask->getSize() - 1);
					int32_t igz = clamp< int32_t >(int32_t(gz), 0, m_materialMask->getSize() - 1);

					i->plant = 0;

					const hf::MaterialParams* params = m_materialMask->getParams(igx, m_materialMask->getSize() - 1 - igz);
					if (params)
					{
						const UndergrowthPlant* plant = params->get< UndergrowthPlant >();
						if (plant)
						{
							const std::vector< int32_t >& plantIds = plant->getPlants();
							int32_t index = int32_t((random.nextFloat() + 0.5f) * (plantIds.size() - 1));
							if (index >= 0)
							{
								T_ASSERT (index < plantIds.size());
								i->plant = plantIds[index] + 1;
								for (int32_t j = i->from; j < i->to; ++j)
								{
									float dx, dz;
									
									do 
									{
										dx = (random.nextFloat() * 2.0f - 1.0f) * m_settings.cellRadius;
										dz = (random.nextFloat() * 2.0f - 1.0f) * m_settings.cellRadius;
									}
									while (std::sqrt(dx * dx + dz * dz) > m_settings.cellRadius);

									float px = i->center.x() + dx;
									float pz = i->center.z() + dz;

									m_plants[j * 2 + 0] = Vector4(
										px,
										pz,
										random.nextFloat() * TWO_PI,
										float(i->plant - 1)
									);
									m_plants[j * 2 + 1] = Vector4(
										random.nextFloat() * 0.5f + 0.5f,
										0.0f,
										0.0f,
										0.0f
									);
								}
							}
						}
					}
				}

				if (i->plant)
					i->visible = (viewFrustum.inside(view * i->center, Scalar(m_settings.cellRadius)) != Frustum::IrOutside);
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

	Vector4 instanceData1[InstanceCount];
	Vector4 instanceData2[InstanceCount];
	uint32_t plantCount = 0;

	for (AlignedVector< Cluster >::const_iterator i = m_clusters.begin(); i != m_clusters.end(); ++i)
	{
		if (!i->visible)
			continue;

		float fc = 1.0f - clamp(i->distance / (m_settings.spreadDistance + m_settings.cellRadius), 0.0f, 1.0f);
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
		renderBlock->count = 2 * 2 * 2;
		renderBlock->minIndex = 0;
		renderBlock->maxIndex = 4 * 2 - 1;
		renderBlock->instanceCount = count;

		renderBlock->programParams->beginParameters(renderContext);
		worldRenderPass.setProgramParameters(renderBlock->programParams, false);
		renderBlock->programParams->setTextureParameter(s_handleNormals, m_terrain->getNormalMap());
		renderBlock->programParams->setTextureParameter(s_handleHeightfield, m_terrain->getHeightMap());
		renderBlock->programParams->setVectorParameter(s_handleWorldExtent, m_terrain->getHeightfield()->getWorldExtent());
		renderBlock->programParams->setVectorParameter(s_handleEye, m_eye);
		renderBlock->programParams->setFloatParameter(s_handleSpreadDistance, m_settings.spreadDistance);
		renderBlock->programParams->setFloatParameter(s_handleCellRadius, m_settings.cellRadius);
		renderBlock->programParams->setVectorArrayParameter(s_handleInstances1, instanceData1, count);
		renderBlock->programParams->setVectorArrayParameter(s_handleInstances2, instanceData2, count);
		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(
			render::RpOpaque,
			renderBlock
		);

		// Only allowed to draw 1/4th of all clusters.
		if (++plantCount > m_settings.density / 4)
			break;
	}
}

Aabb3 UndergrowthEntity::getBoundingBox() const
{
	return Aabb3();
}

void UndergrowthEntity::update(const world::UpdateParams& update)
{
}

	}
}
