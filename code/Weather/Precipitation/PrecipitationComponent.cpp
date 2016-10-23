#include "Core/Misc/SafeDestroy.h"
#include "Render/IndexBuffer.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/Context/RenderContext.h"
#include "Weather/Precipitation/PrecipitationComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.PrecipitationComponent", PrecipitationComponent, world::IEntityComponent)

PrecipitationComponent::PrecipitationComponent(
	render::VertexBuffer* vertexBuffer,
	render::IndexBuffer* indexBuffer,
	const render::Primitives& primitives,
	const resource::Proxy< render::Shader >& shader
)
:	m_vertexBuffer(vertexBuffer)
,	m_indexBuffer(indexBuffer)
,	m_primitives(primitives)
,	m_shader(shader)
{
}

void PrecipitationComponent::destroy()
{
	safeDestroy(m_vertexBuffer);
	safeDestroy(m_indexBuffer);
	m_shader.clear();
}

void PrecipitationComponent::setOwner(world::Entity* owner)
{
}

void PrecipitationComponent::setTransform(const Transform& transform)
{
}

Aabb3 PrecipitationComponent::getBoundingBox() const
{
	return Aabb3();
}

void PrecipitationComponent::update(const world::UpdateParams& update)
{
}

void PrecipitationComponent::render(
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

	const Frustum& viewFrustum = worldRenderView.getViewFrustum();

	Vector4 origins[] =
	{
		viewFrustum.corners[0],
		viewFrustum.corners[1],
		viewFrustum.corners[2],
		viewFrustum.corners[3]
	};

	Vector4 edges[] =
	{
		viewFrustum.corners[4] - viewFrustum.corners[0],
		viewFrustum.corners[5] - viewFrustum.corners[1],
		viewFrustum.corners[6] - viewFrustum.corners[2],
		viewFrustum.corners[7] - viewFrustum.corners[3]
	};

	render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("Precipitation");

	// Render precipitation after all opaques but first of all alpha blended.
	renderBlock->distance = std::numeric_limits< float >::max();
	renderBlock->program = program;
	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->indexBuffer = m_indexBuffer;
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitives = m_primitives;

	renderBlock->programParams->beginParameters(renderContext);

	worldRenderPass.setProgramParameters(renderBlock->programParams);

	renderBlock->programParams->setVectorArrayParameter(L"Precipitation_FrustumOrigins", origins, sizeof_array(origins));
	renderBlock->programParams->setVectorArrayParameter(L"Precipitation_FrustumEdges", edges, sizeof_array(edges));
	
	//renderBlock->programParams->setFloatParameter(s_handleSkyDomeRadius, worldRenderView.getViewFrustum().getFarZ() - 100.0f);
	//renderBlock->programParams->setFloatParameter(s_handleSkyDomeOffset, m_offset);
	//renderBlock->programParams->setVectorParameter(s_handleSunDirection, m_sunDirection);

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(m_shader->getCurrentPriority(), renderBlock);
}

	}
}
