#include "Core/Misc/SafeDestroy.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "World/Entity.h"
#include "World/WorldBuildContext.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"
#include "World/Simple/WorldRendererSimple.h"
#include "World/Simple/WorldRenderPassSimple.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererSimple", 0, WorldRendererSimple, IWorldRenderer)

bool WorldRendererSimple::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc
)
{
	m_entityRenderers = desc.entityRenderers;
	return true;
}

void WorldRendererSimple::destroy()
{
	m_entityRenderers = nullptr;
}

void WorldRendererSimple::setup(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId
)
{
	// Add additional passes by entity renderers.
	{
		WorldSetupContext context(m_entityRenderers, rootEntity, renderGraph);
		context.setup(worldRenderView, rootEntity);
		context.flush();
	}

	// Add passes to render graph.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Visual");

	render::Clear cl;
	cl.mask = render::CfColor | render::CfDepth;
	cl.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	cl.depth = 1.0f;
	rp->setOutput(outputTargetSetId, cl, render::TfNone, render::TfAll);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			WorldBuildContext wc(
				m_entityRenderers,
				rootEntity,
				renderContext
			);

			// Default visual parameters.
			auto globalProgramParams = renderContext->alloc< render::ProgramParameters >();
			globalProgramParams->beginParameters(renderContext);
			globalProgramParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			globalProgramParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			globalProgramParams->endParameters(renderContext);

			// Build visual context.
			WorldRenderPassSimple defaultPass(
				s_techniqueSimpleColor,
				globalProgramParams,
				worldRenderView.getView()
			);

			wc.build(worldRenderView, defaultPass, rootEntity);
			wc.flush(worldRenderView, defaultPass);
			renderContext->merge(render::RpAll);
		}
	);
	renderGraph.addPass(rp);
}

render::ImageGraphContext* WorldRendererSimple::getImageGraphContext() const
{
	return nullptr;
}

	}
}
