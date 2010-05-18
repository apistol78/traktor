#include <limits>
#include "Weather/Sky/SkyEntity.h"
#include "World/WorldRenderView.h"
#include "World/Entity/EntityUpdate.h"
#include "Render/Context/RenderContext.h"

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

void SkyEntity::render(render::RenderContext* renderContext, const world::WorldRenderView* worldRenderView)
{
	if (!m_shader.validate() || !m_shader->hasTechnique(worldRenderView->getTechnique()))
		return;

	render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >();

	// Render sky after all opaques but first of all alpha blended.
	renderBlock->distance = std::numeric_limits< float >::max();
	renderBlock->shader = m_shader;
	renderBlock->shaderParams = renderContext->alloc< render::ShaderParameters >();
	renderBlock->indexBuffer = m_indexBuffer;
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitives = &m_primitives;

	renderBlock->shaderParams->beginParameters(renderContext);

	worldRenderView->setTechniqueParameters(renderBlock->shaderParams);
	worldRenderView->setShaderParameters(renderBlock->shaderParams);
	
	renderBlock->shaderParams->setFloatParameter(m_handleSkyDomeRadius, worldRenderView->getViewFrustum().getFarZ());

	renderBlock->shaderParams->endParameters(renderContext);

	renderContext->draw(render::RfAlphaBlend, renderBlock);
}

void SkyEntity::update(const world::EntityUpdate* update)
{
}

	}
}
