#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/SimpleImageStep.h"

namespace traktor
{
    namespace render
    {
		namespace
		{

const static Handle s_handleTime(L"Time");
const static Handle s_handleDeltaTime(L"DeltaTime");

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleImageStep", SimpleImageStep, IImageStep)

void SimpleImageStep::setup(const ImageGraph* imageGraph, RenderPass& pass) const
{
	for (const auto& source : m_sources)
		pass.addInput(source.targetSet);
}

void SimpleImageStep::build(
	const ImageGraph* imageGraph,
	const RenderGraph& renderGraph,
	RenderContext* renderContext,
	const ImageGraphParams& data
) const
{
	// Setup parameters for the shader.
	auto pp = renderContext->alloc< ProgramParameters >();
	pp->beginParameters(renderContext);
	pp->setFloatParameter(s_handleTime, data.time);
	pp->setFloatParameter(s_handleDeltaTime, data.deltaTime);
	for (const auto& source : m_sources)
	{
		auto targetSet = renderGraph.getTargetSet(source.targetSet);
		if (targetSet)
			pp->setTextureParameter(source.parameter, targetSet->getColorTexture(
				source.colorIndex
			));
	}
	pp->endParameters(renderContext);

	// Draw fullscreen quad with shader.
	imageGraph->getScreenRenderer()->draw(renderContext, m_shader, pp);
}

    }
}