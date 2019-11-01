#include <numeric>
#include "Core/Log/Log.h"
#include "Core/Math/Vector2.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Shape/Spline/ExtrudeShapeLayer.h"
#include "Shape/Spline/SplineEntity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"

namespace traktor
{
	namespace shape
	{
		namespace
		{

#pragma pack(1)
struct ExtrudeVertex
{
	float position[3];
	float normal[3];
	float where;
};
#pragma pack()

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.ExtrudeShapeLayer", ExtrudeShapeLayer, ISplineLayer)

ExtrudeShapeLayer::ExtrudeShapeLayer(
	SplineEntity* owner,
	render::IRenderSystem* renderSystem,
	const resource::Proxy< render::Shader >& shader,
	bool automaticOrientation,
	float detail
)
:	m_owner(owner)
,	m_renderSystem(renderSystem)
,	m_shader(shader)
,	m_automaticOrientation(automaticOrientation)
,	m_detail(detail)
{
}

void ExtrudeShapeLayer::update(const world::UpdateParams& update)
{
}

void ExtrudeShapeLayer::pathChanged()
{
	const auto& path = m_owner->getPath();

	const auto& keys = path.getKeys();
	if (keys.size() < 2)
		return;

	safeDestroy(m_vertexBuffer);
	safeDestroy(m_indexBuffer);

	const uint32_t nsegments = (uint32_t)(keys.size() - 1);
	AlignedVector< uint32_t > stepsPerSegment;

	// Calculate number of steps per segment.
	for (uint32_t i = 0; i < nsegments; ++i)
	{
		const auto& k0 = keys[i];
		const auto& k1 = keys[i + 1];

		Scalar err(0.0f);
		for (uint32_t j = 0; j < 10; ++j)
		{
			float t = (float)j / (10 - 1);

			Vector4 pl = lerp(k0.position, k1.position, Scalar(t));
			Vector4 pc = path.evaluate(k0.T * (1.0f - t) + k1.T * t).transform().translation();

			err += (pl - pc).xyz0().length();
		}
		err /= Scalar(10.0f);

		stepsPerSegment.push_back(
			std::max< uint32_t >(
				(uint32_t)(err * m_detail),
				1
			)
		);
	}

	int32_t nsteps = std::accumulate(stepsPerSegment.begin(), stepsPerSegment.end(), 0);
	if (nsteps <= 1)
		return;

	// Extrude shape.
	const float c_scale = 0.1f;
	AlignedVector< Vector2 > shape;
	for (int32_t i = 0; i < 16; ++i)
	{
		float a = ((float)i / 16) * TWO_PI;
		shape.push_back(Vector2(
			cos(a) * c_scale,
			sin(a) * c_scale
		));
	}

	uint32_t nedges = (uint32_t)shape.size();
	uint32_t nvertices = nsteps * nedges;
	uint32_t ntriangles = (nsteps - 1) * nedges * 2;

	// Create vertices.
	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat3, offsetof(ExtrudeVertex, position)));
	vertexElements.push_back(render::VertexElement(render::DuNormal, render::DtFloat3, offsetof(ExtrudeVertex, normal)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat1, offsetof(ExtrudeVertex, where)));

	m_vertexBuffer = m_renderSystem->createVertexBuffer(
		vertexElements,
		nvertices * sizeof(ExtrudeVertex),
		false
	);

	ExtrudeVertex* vertex = (ExtrudeVertex*)m_vertexBuffer->lock();

	uint32_t baseStep = 0;
	for (uint32_t segment = 0; segment < nsegments; ++segment)
	{
		for (uint32_t step = 0; step < stepsPerSegment[segment]; ++step)
		{
			const float at = (float)(baseStep + step) / (nsteps - 1);

			Matrix44 T = path.evaluate(at).transform().toMatrix44();

			if (m_automaticOrientation)
			{
				const float c_atDelta = 0.001f;
				Transform Tp = path.evaluate(std::max(at - c_atDelta, 0.0f)).transform();
				Transform Tn = path.evaluate(std::min(at + c_atDelta, 1.0f)).transform();
				T = lookAt(Tp.translation().xyz1(), Tn.translation().xyz1()).inverse();
			}

			for (uint32_t i = 0; i < nedges; ++i)
			{
				Vector2 p = shape[i];
				Vector2 n = shape[i].normalized();

				Vector4 ep = T * Vector4(p.x, p.y, 0.0f, 1.0f);
				Vector4 en = T * Vector4(n.x, n.y, 0.0f, 0.0f);

				vertex->position[0] = ep.x();
				vertex->position[1] = ep.y();
				vertex->position[2] = ep.z();

				vertex->normal[0] = en.x();
				vertex->normal[1] = en.y();
				vertex->normal[2] = en.z();

				vertex->where = at;

				vertex++;
			}
		}
		baseStep += stepsPerSegment[segment];
	}
	m_vertexBuffer->unlock();

	// Create indices.
	m_indexBuffer = m_renderSystem->createIndexBuffer(render::ItUInt16, ntriangles * 3 * sizeof(uint16_t), false);

	uint16_t* index = (uint16_t*)m_indexBuffer->lock();
	for (int32_t ring = 0; ring < nsteps - 1; ++ring)
	{
		const uint16_t b0 = ring * nedges;
		const uint16_t b1 = (ring + 1) * nedges;

		for (uint32_t i = 0; i < nedges; ++i)
		{
			uint16_t a = (uint16_t)i;
			uint16_t b = (uint16_t)((i + 1) % nedges);

			*index++ = (uint16_t)(b0 + b);
			*index++ = (uint16_t)(b0 + a);
			*index++ = (uint16_t)(b1 + a);

			*index++ = (uint16_t)(b1 + a);
			*index++ = (uint16_t)(b1 + b);
			*index++ = (uint16_t)(b0 + b);
		}
	}
	m_indexBuffer->unlock();

	// Create primitives.
	m_primitives.setIndexed(
		render::PtTriangles,
		0,
		ntriangles,
		0,
		nvertices - 1
	);
}

void ExtrudeShapeLayer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	if (!m_indexBuffer || !m_vertexBuffer)
		return;

	worldRenderPass.setShaderTechnique(m_shader);
	worldRenderPass.setShaderCombination(m_shader);

	render::IProgram* program = m_shader->getCurrentProgram();
	if (!program)
		return;

	auto renderContext = worldContext.getRenderContext();

	render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("Extrude");

	renderBlock->distance = std::numeric_limits< float >::max();
	renderBlock->program = program;
	renderBlock->indexBuffer = m_indexBuffer;
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitives = m_primitives;

	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->programParams->beginParameters(renderContext);

	worldRenderPass.setProgramParameters(
		renderBlock->programParams
	);

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(m_shader->getCurrentPriority(), renderBlock);
}

	}
}
