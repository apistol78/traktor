#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/AmbientOcclusionImageStep.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"

namespace traktor
{
    namespace render
    {
		namespace
		{

const static Handle s_handleTime(L"Time");
const static Handle s_handleDeltaTime(L"DeltaTime");

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.AmbientOcclusionImageStep", AmbientOcclusionImageStep, IImageStep)

void AmbientOcclusionImageStep::setup(const ImageGraph* /*imageGraph*/, const ImageGraphContext& cx, RenderPass& pass) const
{
	for (const auto& source : m_sources)
	{
		auto targetSetId = cx.findTextureTargetSetId(source.textureId);
		if (targetSetId != 0)
			pass.addInput(targetSetId);
	}
}

void AmbientOcclusionImageStep::build(
	const ImageGraph* imageGraph,
	const ImageGraphContext& cx,
	const RenderGraph& renderGraph,
	RenderContext* renderContext
) const
{
	// Setup parameters for the shader.
	auto pp = renderContext->alloc< ProgramParameters >();
	pp->beginParameters(renderContext);
	pp->setFloatParameter(s_handleTime, cx.getParams().time);
	pp->setFloatParameter(s_handleDeltaTime, cx.getParams().deltaTime);
	for (const auto& source : m_sources)
	{
		auto texture = cx.findTexture(renderGraph, source.textureId);
		pp->setTextureParameter(source.parameter, texture);
	}
	pp->endParameters(renderContext);

	// Draw fullscreen quad with shader.
	cx.getScreenRenderer()->draw(renderContext, m_shader, pp);
}

    }
}