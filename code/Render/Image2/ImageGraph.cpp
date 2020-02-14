#include "Core/Misc/SafeDestroy.h"
#include "Render/ITexture.h"
#include "Render/ScreenRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/Image2/ImagePass.h"
#include "Render/Image2/ImageStep.h"
#include "Render/Image2/ImageTargetSet.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageGraph", ImageGraph, Object)

void ImageGraph::addTargetSets(RenderGraph& renderGraph) const
{
	for (auto targetSet : m_targetSets)
	{
		renderGraph.addTargetSet(
			targetSet->getTargetSetId(),
			targetSet->getTargetSetDesc()
		);
	}
}

void ImageGraph::addPasses(RenderGraph& renderGraph, RenderPass* parentPass, const ImageGraphContext& cx) const
{
	// Copy context and append our internal targets so
	// steps can have a single method of accessing input textures.
	ImageGraphContext context = cx;
	for (auto targetSet : m_targetSets)
	{
		const auto& desc = targetSet->getTargetSetDesc();
		for (int32_t i = 0; i < desc.count; ++i)
		{
			context.associateTextureTargetSet(
				targetSet->getTextureId(i),
				targetSet->getTargetSetId(),
				i
			);
		}
	}

	// Add all passes to render graph.
	for (auto pass : m_passes)
	{
		Ref< RenderPass > rp = new RenderPass();

		for (auto step : pass->m_steps)
			step->setup(this, context, *rp);

		rp->setOutput(pass->m_output);
		rp->addBuild(
			[=](const RenderGraph& renderGraph, RenderContext* renderContext)
			{
				auto sharedParams = renderContext->alloc< ProgramParameters >();
				sharedParams->beginParameters(renderContext);
				for (auto it : m_scalarParameters)
					sharedParams->setFloatParameter(it.first, it.second);
				for (auto it : m_vectorParameters)
					sharedParams->setVectorParameter(it.first, it.second);
				for (auto it : m_textureParameters)
					sharedParams->setTextureParameter(it.first, it.second);
				sharedParams->endParameters(renderContext);
				
				for (auto step : pass->m_steps)
					step->build(this, context, renderGraph, sharedParams, renderContext);
			}
		);

		renderGraph.addPass(rp);
	}

	// Add sub pass to parent render pass.
	Ref< RenderPass > rp = new RenderPass();

	for (auto step : m_steps)
		step->setup(this, context, *rp);

	rp->addBuild(
		[=](const RenderGraph& renderGraph, RenderContext* renderContext)
		{
			auto sharedParams = renderContext->alloc< ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			for (auto it : m_scalarParameters)
				sharedParams->setFloatParameter(it.first, it.second);
			for (auto it : m_vectorParameters)
				sharedParams->setVectorParameter(it.first, it.second);
			for (auto it : m_textureParameters)
				sharedParams->setTextureParameter(it.first, it.second);
			sharedParams->endParameters(renderContext);

			for (auto step : m_steps)
				step->build(this, context, renderGraph, sharedParams, renderContext);
		}
	);

	parentPass->addSubPass(rp);
}

void ImageGraph::setFloatParameter(handle_t handle, float value)
{
	m_scalarParameters[handle] = value;
}

void ImageGraph::setVectorParameter(handle_t handle, const Vector4& value)
{
	m_vectorParameters[handle] = value;
}

void ImageGraph::setTextureParameter(handle_t handle, const resource::Proxy< ITexture >& value)
{
	m_textureParameters[handle] = value;
}

	}
}