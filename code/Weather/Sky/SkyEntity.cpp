#include <limits>
#include "Render/Context/RenderContext.h"
#include "Weather/Sky/SkyEntity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"
#include "World/Entity/EntityUpdate.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.SkyEntity", SkyEntity, world::Entity)

SkyEntity::SkyEntity(
	render::VertexBuffer* vertexBuffer,
	render::IndexBuffer* indexBuffer,
	const render::Primitives& primitives,
	const resource::Proxy< render::Shader >& shader
)
:	m_vertexBuffer(vertexBuffer)
,	m_indexBuffer(indexBuffer)
,	m_primitives(primitives)
,	m_shader(shader)
,	m_handleSkyDomeRadius(render::getParameterHandle(L"SkyDomeRadius"))
{
}

void SkyEntity::render(
	render::RenderContext* renderContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	if (!m_shader.validate())
		return;

	worldRenderPass.setShaderTechnique(m_shader);
	worldRenderPass.setShaderCombination(m_shader);

	render::IProgram* program = m_shader->getCurrentProgram();
	if (!program)
		return;

	render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >();

	// Render sky after all opaques but first of all alpha blended.
	renderBlock->distance = std::numeric_limits< float >::max();
	renderBlock->program = program;
	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->indexBuffer = m_indexBuffer;
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitives = &m_primitives;

	renderBlock->programParams->beginParameters(renderContext);

	m_shader->setProgramParameters(renderBlock->programParams);
	worldRenderPass.setProgramParameters(renderBlock->programParams);
	
	renderBlock->programParams->setFloatParameter(m_handleSkyDomeRadius, worldRenderView.getViewFrustum().getFarZ());

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(render::RfAlphaBlend, renderBlock);
}

void SkyEntity::update(const world::EntityUpdate* update)
{
}

	}
}
