#include <Core/Misc/SafeDestroy.h>
#include <World/IWorldRenderPass.h>
#include "Shared/QuadComponent.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"QuadComponent", QuadComponent, world::IEntityComponent)

QuadComponent::QuadComponent(
	const resource::Proxy< render::Shader >& shader,
	render::VertexBuffer* vertexBuffer
)
:	m_shader(shader)
,	m_vertexBuffer(vertexBuffer)
,	m_color(1.0f, 1.0f, 1.0f, 1.0f)
{
}

void QuadComponent::destroy()
{
	m_shader.clear();
	safeDestroy(m_vertexBuffer);
}

void QuadComponent::setTransform(const Transform& transform)
{
}

Aabb3 QuadComponent::getBoundingBox() const
{
	return Aabb3(
		Vector4(-1.0f, -1.0f, 0.0f),
		Vector4( 1.0f,  1.0f, 0.0f)
	);
}

void QuadComponent::update(const world::UpdateParams& update)
{
}

void QuadComponent::setColor(const Color4f& color)
{
	m_color = color;
}

void QuadComponent::render(
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

	render::NonIndexedRenderBlock* renderBlock = renderContext->alloc< render::NonIndexedRenderBlock >("Quad");

	renderBlock->distance = 0.0f;
	renderBlock->program = program;
	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitive = render::PtTriangleStrip;
	renderBlock->offset = 0;
	renderBlock->count = 2;

	renderBlock->programParams->beginParameters(renderContext);
	worldRenderPass.setProgramParameters(renderBlock->programParams, false);
	renderBlock->programParams->setVectorParameter(L"Quad_Color", m_color);
	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(m_shader->getCurrentPriority(), renderBlock);
}
