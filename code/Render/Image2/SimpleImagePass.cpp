#include "Render/IRenderTargetSet.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/SimpleImagePass.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleImagePass", SimpleImagePass, IImagePass)

SimpleImagePass::SimpleImagePass()
:	m_output(0)
{
}

void SimpleImagePass::setup(RenderGraph* renderGraph) const
{
}

void SimpleImagePass::add(RenderGraph* renderGraph) const
{
	renderGraph->addPass(
		L"Simple",
		[&](RenderPassBuilder& builder)
		{
			// Add dependencies to source inputs.
			for (const auto& source : m_sources)
				builder.addInput(source.input);

			// Set output target.
			builder.setOutput(m_output);
		},
		[=](RenderPassResources& resources, RenderContext* renderContext)
		{
			auto pp = renderContext->alloc< ProgramParameters >();
			pp->beginParameters(renderContext);
			// pp->attachParameters(sharedParams);
			// pp->setFloatParameter(s_handleTime, m_time);
			// pp->setFloatParameter(s_handleDeltaTime, params.deltaTime);
			for (const auto& source : m_sources)
			{
				auto target = resources.getInput(source.input);
				if (target)
					pp->setTextureParameter(source.parameter, target->getColorTexture(0));
			}
			pp->endParameters(renderContext);

			// m_screenRenderer->draw(renderContext, m_shader, pp);
		}
	);
}

    }
}