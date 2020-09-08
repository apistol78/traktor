#include "Render/ITexture.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/Image2/ImagePass.h"
#include "Render/Image2/ImagePassOp.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImagePass", ImagePass, IImageStep)

void ImagePass::addPasses(const ImageGraph* graph, const ImageGraphContext& context, const targetSetVector_t& targetSetIds, RenderGraph& renderGraph) const
{
	Ref< RenderPass > rp = new RenderPass(m_name);

	for (auto op : m_ops)
		op->setup(graph, context, *rp);

	if (m_outputTargetSet >= 0)
	{
		uint32_t load = render::TfNone;

		if ((m_clear.mask & render::CfColor) == 0)
			load |= render::TfColor;

		rp->setOutput(targetSetIds[m_outputTargetSet], m_clear, load, render::TfColor);
	}

	rp->addBuild(
		[=](const RenderGraph& renderGraph, RenderContext* renderContext)
		{
			auto sharedParams = renderContext->alloc< ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			for (auto it : context.getFloatParameters())
				sharedParams->setFloatParameter(it.first, it.second);
			for (auto it : context.getVectorParameters())
				sharedParams->setVectorParameter(it.first, it.second);
			for (auto it : context.getTextureParameters())
				sharedParams->setTextureParameter(it.first, it.second);
			sharedParams->endParameters(renderContext);
				
			for (auto op : m_ops)
				op->build(graph, context, renderGraph, sharedParams, renderContext);
		}
	);

	renderGraph.addPass(rp);
}

	}
}
