#include <limits>
#include "Core/Functor/Functor.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Thread/JobManager.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/MaterialMask.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/Context/RenderContext.h"
#include "Terrain/UndergrowthEntity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.UndergrowthEntity", UndergrowthEntity, world::Entity)

void UndergrowthEntity::Vertex::set(const Vector4& position_, const Vector4& normal_, float texCoordU, float texCoordV)
{
	position[0] = position_.x();
	position[1] = position_.y();
	position[2] = position_.z();
	normal[0] = floatToHalf(normal_.x());
	normal[1] = floatToHalf(normal_.z());
	texCoord[0] = floatToHalf(texCoordU);
	texCoord[1] = floatToHalf(texCoordV);
}

UndergrowthEntity::UndergrowthEntity(
	const resource::Proxy< hf::Heightfield >& heightfield,
	const resource::Proxy< hf::MaterialMask >& materialMask,
	const Settings& settings,
	render::VertexBuffer* vertexBuffer,
	render::IndexBuffer* indexBuffer,
	const render::Primitives& primitives,
	const resource::Proxy< render::Shader >& shader
)
:	m_heightfield(heightfield)
,	m_materialMask(materialMask)
,	m_settings(settings)
,	m_vertexBuffer(vertexBuffer)
,	m_indexBuffer(indexBuffer)
,	m_primitives(primitives)
,	m_shader(shader)
,	m_lastView(Matrix44::identity())
,	m_jobs(4)
,	m_sync(false)
{
	m_lastFrustum.buildPerspective(0.0f, 1.0f, 0.0f, 0.0f);

	int offset = 0;
	for (int i = 0; i < sizeof_array(m_cells); ++i)
	{
		m_cells[i].position.set(
			std::numeric_limits< float >::max(),
			std::numeric_limits< float >::max(),
			std::numeric_limits< float >::max(),
			1.0f
		);

		m_cells[i].offset = offset;
		m_cells[i].count = m_settings.density / sizeof_array(m_cells);

		offset += m_cells[i].count * 4 * 2;
	}
}

UndergrowthEntity::~UndergrowthEntity()
{
	synchronize();
}

void UndergrowthEntity::render(
	render::RenderContext* renderContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	m_lastView = worldRenderView.getView();
	m_lastFrustum = worldRenderView.getViewFrustum();

	worldRenderPass.setShaderTechnique(m_shader);
	worldRenderPass.setShaderCombination(m_shader);

	render::IProgram* program = m_shader->getCurrentProgram();
	if (!program)
		return;

	synchronize();

	// Create render blocks.
	render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >();

	renderBlock->distance = 0.0f;
	renderBlock->program = program;
	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->indexBuffer = m_indexBuffer;
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitives = &m_primitives;

	renderBlock->programParams->beginParameters(renderContext);
	worldRenderPass.setProgramParameters(renderBlock->programParams, false);
	renderBlock->programParams->setFloatParameter(L"MaxRadius", m_settings.spreadDistance + m_settings.cellRadius);
	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(
		render::RfAlphaBlend,
		renderBlock
	);
}

Aabb3 UndergrowthEntity::getBoundingBox() const
{
	return Aabb3();
}

void UndergrowthEntity::update(const UpdateParams& update)
{
	synchronize();

	Vertex* vertexTop = static_cast< Vertex* >(m_vertexBuffer->lock());
	T_ASSERT_M (vertexTop, L"Unable to lock vertex buffer");

	m_lastFrustum.setFarZ(m_lastFrustum.getNearZ() + Scalar(m_settings.spreadDistance));

	JobManager& jobManager = JobManager::getInstance();
	m_jobs[0] = jobManager.add(makeFunctor(this, &UndergrowthEntity::updateTask, 0, 4, vertexTop));
	m_jobs[1] = jobManager.add(makeFunctor(this, &UndergrowthEntity::updateTask, 4, 8, vertexTop));
	m_jobs[2] = jobManager.add(makeFunctor(this, &UndergrowthEntity::updateTask, 8, 12, vertexTop));
	m_jobs[3] = jobManager.add(makeFunctor(this, &UndergrowthEntity::updateTask, 12, 16, vertexTop));

	m_sync = true;
}

void UndergrowthEntity::synchronize()
{
	if (!m_sync)
		return;

	m_jobs[0]->wait();
	m_jobs[1]->wait();
	m_jobs[2]->wait();
	m_jobs[3]->wait();

	m_vertexBuffer->unlock();

	m_sync = false;
}

void UndergrowthEntity::updateTask(int start, int end, Vertex* outVertex)
{
	const Vector4& worldExtent = m_heightfield->getWorldExtent();
	Scalar dw = worldExtent.x() / Scalar(m_heightfield->getSize());

	Scalar mmszs(m_materialMask->getSize());
	int32_t mmszf = int32_t(m_materialMask->getSize());

	Matrix44 inverseView = m_lastView.inverseOrtho();
	for (int32_t i = start; i < end; ++i)
	{
		Vector4 position = m_lastView * m_cells[i].position;
		if (m_lastFrustum.inside(position, Scalar(m_settings.cellRadius)) != Frustum::IrOutside)
			continue;

		Vector4 seed(
			float(m_random.nextDouble() * 2.0f - 1.0f) * m_settings.spreadDistance,
			0.0f,
			float(m_random.nextDouble()) * m_settings.spreadDistance,
			1.0f
		);

		seed = inverseView * seed;

		// Check plant type from material mask.
		int32_t mx = int32_t(Scalar(m_materialMask->getSize()) * (0.5f - seed.x() / worldExtent.x()));
		int32_t my = int32_t(Scalar(m_materialMask->getSize()) * (seed.z() / worldExtent.z() + 0.5f));
		if (mx < 0 || my < 0 || mx >= int32_t(m_materialMask->getSize()) || my >= int32_t(m_materialMask->getSize()))
			continue;
		if (m_materialMask->getId(mx, my))
			continue;

		m_cells[i].position = seed;

		Vertex* vertex = outVertex + m_cells[i].offset;

		for (int j = 0; j < m_cells[i].count; ++j)
		{
			float x = seed.x() + float(m_random.nextDouble() * 2.0f - 1.0f) * m_settings.cellRadius;
			float z = seed.z() + float(m_random.nextDouble() * 2.0f - 1.0f) * m_settings.cellRadius;

			Vector4 position(
				x,
				m_heightfield->getWorldHeight(x, z),
				z,
				1.0f
			);

			// Calculate heightfield normal, use to bias lighting.
			float hx = m_heightfield->getWorldHeight(position.x() + dw, position.z());
			float hz = m_heightfield->getWorldHeight(position.x(), position.z() + dw);

			Vector4 normal = cross(
				Vector4(0.0f, hz - position.y(), dw, 0.0f),
				Vector4(dw, hx - position.y(), 0.0f, 0.0f)
			).normalized();

			// Height, or extrusion from seed.
			float e = float(m_random.nextDouble() * 0.25f + 1.25f) * m_settings.plantScale;

			// Random rotation.
			float a = float(m_random.nextDouble() * PI * 2.0f);
			float s = sinf(a) * 0.5f * m_settings.plantScale;
			float c = cosf(a) * 0.5f * m_settings.plantScale;

			// Random sheering.
			float sx = float(m_random.nextDouble() * 0.2f - 0.1f);
			float sz = float(m_random.nextDouble() * 0.2f - 0.1f);

			Vector4 axisX(c, 0.0f, s);
			Vector4 axisY(sx, e, sz);
			Vector4 axisZ(-s, 0.0f, c);

			vertex++->set(position - axisX         + axisZ, normal, 0.0f, 1.0f);
			vertex++->set(position - axisX + axisY + axisZ, normal, 0.0f, 0.0f);
			vertex++->set(position + axisX + axisY - axisZ, normal, 0.5f, 0.0f);
			vertex++->set(position + axisX         - axisZ, normal, 0.5f, 1.0f);

			vertex++->set(position - axisX         - axisZ, normal, 0.5f, 1.0f);
			vertex++->set(position - axisX + axisY - axisZ, normal, 0.5f, 0.0f);
			vertex++->set(position + axisX + axisY + axisZ, normal, 1.0f, 0.0f);
			vertex++->set(position + axisX         + axisZ, normal, 1.0f, 1.0f);
		}
	}
}

	}
}
