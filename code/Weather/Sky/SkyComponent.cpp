#include <limits>
#include "Core/Misc/SafeDestroy.h"
#include "Render/IndexBuffer.h"
#include "Render/ITexture.h"
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

render::handle_t s_handleWeather_SkyRadius;
render::handle_t s_handleWeather_SkyOffset;
render::handle_t s_handleWeather_SkyTexture;
render::handle_t s_handleWeather_SkySunDirection;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.SkyComponent", SkyComponent, world::IEntityComponent)

SkyComponent::SkyComponent(
	render::VertexBuffer* vertexBuffer,
	render::IndexBuffer* indexBuffer,
	const render::Primitives& primitives,
	const resource::Proxy< render::Shader >& shader,
	const resource::Proxy< render::ITexture >& texture,
	float offset
)
:	m_vertexBuffer(vertexBuffer)
,	m_indexBuffer(indexBuffer)
,	m_primitives(primitives)
,	m_shader(shader)
,	m_texture(texture)
,	m_transform(Transform::identity())
,	m_offset(offset)
{
	s_handleWeather_SkyRadius = render::getParameterHandle(L"Weather_SkyRadius");
	s_handleWeather_SkyOffset = render::getParameterHandle(L"Weather_SkyOffset");
	s_handleWeather_SkyTexture = render::getParameterHandle(L"Weather_SkyTexture");
	s_handleWeather_SkySunDirection = render::getParameterHandle(L"Weather_SkySunDirection");
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

void SkyComponent::setOwner(world::Entity* owner)
{
}

void SkyComponent::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Aabb3 SkyComponent::getBoundingBox() const
{
	const float c_radius = 1e4f;
	return Aabb3(Vector4(-c_radius, -c_radius, -c_radius, 1.0f), Vector4(c_radius, c_radius, c_radius, 1.0f));
}

void SkyComponent::update(const world::UpdateParams& update)
{
}

void SkyComponent::build(
	render::RenderContext* renderContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	auto sp = worldRenderPass.getProgram(m_shader);
	if (!sp)
		return;

	Vector4 sunDirection = m_transform.axisY();

	render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >(L"Sky");

	// Render sky after all opaques but before of all alpha blended.
	renderBlock->distance = std::numeric_limits< float >::max();
	renderBlock->program = sp.program;
	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->indexBuffer = m_indexBuffer;
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitives = m_primitives;

	renderBlock->programParams->beginParameters(renderContext);

	worldRenderPass.setProgramParameters(renderBlock->programParams);

	renderBlock->programParams->setFloatParameter(s_handleWeather_SkyRadius, worldRenderView.getViewFrustum().getFarZ() - abs(m_offset) - 10.0f);
	renderBlock->programParams->setFloatParameter(s_handleWeather_SkyOffset, m_offset);
	renderBlock->programParams->setVectorParameter(s_handleWeather_SkySunDirection, sunDirection);
	renderBlock->programParams->setTextureParameter(s_handleWeather_SkyTexture, m_texture);

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(sp.priority, renderBlock);
}

	}
}
