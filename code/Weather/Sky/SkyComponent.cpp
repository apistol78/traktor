#include <limits>
#include "Core/Misc/SafeDestroy.h"
#include "Render/IndexBuffer.h"
#include "Render/VertexBuffer.h"
#include "Render/Context/RenderContext.h"
#include "Weather/Sky/SkyComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace weather
	{
		namespace
		{

render::handle_t s_handleSkyDomeRadius;
render::handle_t s_handleSkyDomeOffset;
render::handle_t s_handleSunDirection;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.SkyComponent", SkyComponent, world::IEntityComponent)

SkyComponent::SkyComponent(
	render::VertexBuffer* vertexBuffer,
	render::IndexBuffer* indexBuffer,
	const render::Primitives& primitives,
	const resource::Proxy< render::Shader >& shader,
	const Vector4& sunDirection,
	float offset
)
:	m_vertexBuffer(vertexBuffer)
,	m_indexBuffer(indexBuffer)
,	m_primitives(primitives)
,	m_shader(shader)
,	m_sunDirection(sunDirection)
,	m_offset(offset)
{
	s_handleSkyDomeRadius = render::getParameterHandle(L"SkyDomeRadius");
	s_handleSkyDomeOffset = render::getParameterHandle(L"SkyDomeOffset");
	s_handleSunDirection = render::getParameterHandle(L"SunDirection");
}

SkyComponent::~SkyComponent()
{
	destroy();
}

void SkyComponent::destroy()
{
	safeDestroy(m_indexBuffer);
	safeDestroy(m_vertexBuffer);
}

void SkyComponent::setTransform(const Transform& transform)
{
}

Aabb3 SkyComponent::getBoundingBox() const
{
	const float c_radius = 1e4f;
	return Aabb3(Vector4(-c_radius, -c_radius, -c_radius, 1.0f), Vector4(c_radius, c_radius, c_radius, 1.0f));
}

void SkyComponent::update(const world::UpdateParams& update)
{
}


void SkyComponent::setSunDirection(const Vector4& sunDirection)
{
	m_sunDirection = sunDirection;
}

void SkyComponent::render(
	render::RenderContext* renderContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	worldRenderPass.setShaderTechnique(m_shader);
	worldRenderPass.setShaderCombination(m_shader);

	render::IProgram* program = m_shader->getCurrentProgram();
	if (!program)
		return;

	render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("Sky");

	// Render sky after all opaques but first of all alpha blended.
	renderBlock->distance = std::numeric_limits< float >::max();
	renderBlock->program = program;
	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->indexBuffer = m_indexBuffer;
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitives = m_primitives;

	renderBlock->programParams->beginParameters(renderContext);

	worldRenderPass.setProgramParameters(renderBlock->programParams, false);
	
	renderBlock->programParams->setFloatParameter(s_handleSkyDomeRadius, worldRenderView.getViewFrustum().getFarZ() - 100.0f);
	renderBlock->programParams->setFloatParameter(s_handleSkyDomeOffset, m_offset);
	renderBlock->programParams->setVectorParameter(s_handleSunDirection, m_sunDirection);

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(m_shader->getCurrentPriority(), renderBlock);
}

	}
}
