#include "Core/Misc/SafeDestroy.h"
#include "Render/ScreenRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/IImageStep.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImagePass.h"
#include "Render/Image2/ImageTargetSet.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageGraph", ImageGraph, Object)

bool ImageGraph::create(IRenderSystem* renderSystem, RenderGraph* renderGraph)
{
	// Create screen renderer.
	m_screenRenderer = new ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	// Add all transient render targets to render graph.
	for (auto targetSet : m_targetSets)
	{
		renderGraph->addTargetSet(
			targetSet->getTargetSetDesc()
		);
	}

	return true;
}

void ImageGraph::destroy()
{
	safeDestroy(m_screenRenderer);
}

void ImageGraph::addPasses(RenderGraph* renderGraph, RenderPass* parentPass, const ImageGraphParams& data) const
{
	for (auto pass : m_passes)
	{
		Ref< RenderPass > rp = new RenderPass();

		for (auto step : pass->m_steps)
			step->setup(this, *rp);

		rp->setOutput(pass->m_output);
		rp->addBuild(
			[=](const RenderGraph& renderGraph, RenderContext* renderContext)
			{
				auto sharedParams = renderContext->alloc< ProgramParameters >();
				sharedParams->beginParameters(renderContext);
				// for (SmallMap< handle_t, float >::const_iterator i = m_scalarParameters.begin(); i != m_scalarParameters.end(); ++i)
				// 	sharedParams->setFloatParameter(i->first, i->second);
				// for (SmallMap< handle_t, Vector4 >::const_iterator i = m_vectorParameters.begin(); i != m_vectorParameters.end(); ++i)
				// 	sharedParams->setVectorParameter(i->first, i->second);
				// for (SmallMap< handle_t, resource::Proxy< ITexture > >::const_iterator i = m_textureParameters.begin(); i != m_textureParameters.end(); ++i)
				// 	sharedParams->setTextureParameter(i->first, i->second);
				sharedParams->endParameters(renderContext);
				
				for (auto step : pass->m_steps)
					step->build(this, renderGraph, renderContext, data);
			}
		);

		renderGraph->addPass(rp);
	}

	Ref< RenderPass > rp = new RenderPass();

	for (auto step : m_steps)
		step->setup(this, *rp);

	rp->addBuild(
		[=](const RenderGraph& renderGraph, RenderContext* renderContext)
		{
			auto sharedParams = renderContext->alloc< ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			// for (SmallMap< handle_t, float >::const_iterator i = m_scalarParameters.begin(); i != m_scalarParameters.end(); ++i)
			// 	sharedParams->setFloatParameter(i->first, i->second);
			// for (SmallMap< handle_t, Vector4 >::const_iterator i = m_vectorParameters.begin(); i != m_vectorParameters.end(); ++i)
			// 	sharedParams->setVectorParameter(i->first, i->second);
			// for (SmallMap< handle_t, resource::Proxy< ITexture > >::const_iterator i = m_textureParameters.begin(); i != m_textureParameters.end(); ++i)
			// 	sharedParams->setTextureParameter(i->first, i->second);
			sharedParams->endParameters(renderContext);

			for (auto step : m_steps)
				step->build(this, renderGraph, renderContext, data);
		}
	);

	parentPass->addSubPass(rp);
}

	}
}