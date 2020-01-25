#include "Core/Misc/SafeDestroy.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "World/WorldRenderView.h"
#include "World/WorldContext.h"
#include "World/Entity/GroupEntity.h"
#include "World/Simple/WorldRendererSimple.h"
#include "World/Simple/WorldRenderPassSimple.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

// Techniques
const render::Handle s_techniqueSimpleColor(L"World_SimpleColor");

// Global parameters.
const render::Handle s_handleTime(L"World_Time");
const render::Handle s_handleView(L"World_View");
const render::Handle s_handleViewInverse(L"World_ViewInverse");
const render::Handle s_handleProjection(L"World_Projection");

// Render graph.
const render::Handle s_handleVisual(L"Visual");

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererSimple", 0, WorldRendererSimple, IWorldRenderer)

bool WorldRendererSimple::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc
)
{
	m_entityRenderers = desc.entityRenderers;
	m_rootEntity = new GroupEntity();

	// Create render graph.
	m_renderGraph = new render::RenderGraph(
		renderSystem,
		desc.width,
		desc.height
	);

	m_frames.resize(desc.frameCount);
	for (auto& frame : m_frames)
		frame.renderContext = new render::RenderContext(1 * 1024 * 1024);
	
	return true;
}

void WorldRendererSimple::destroy()
{
	safeDestroy(m_renderGraph);
	m_entityRenderers = nullptr;
	m_rootEntity = nullptr;
	m_frames.clear();
}

void WorldRendererSimple::attach(Entity* entity)
{
	m_rootEntity->addEntity(entity);
}

void WorldRendererSimple::build(const WorldRenderView& worldRenderView, int32_t frame)
{
	WorldContext wc(
		m_entityRenderers,
		m_frames[frame].renderContext,
		m_rootEntity
	);

	// Reset render context by flushing it.
	wc.getRenderContext()->flush();

	// \tbd Flush all entity renderers first, only used by probes atm and need to render to targets.
	// Until we have RenderGraph properly implemented we need to make sure
	// rendering probes doesn't nest render passes.
	wc.flush();
	wc.getRenderContext()->merge(render::RpAll);

	m_renderGraph->addPass(
		L"Visual",
		[&](render::RenderPassBuilder& builder)
		{
		},
		[=](render::RenderPassResources& resources, render::RenderContext* renderContext)
		{
			WorldContext wc(
				m_entityRenderers,
				renderContext,
				m_rootEntity
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

			wc.build(worldRenderView, defaultPass, m_rootEntity);
			wc.flush(worldRenderView, defaultPass);
			renderContext->merge(render::RpAll);
		}
	);

	// Validate render graph.
	if (!m_renderGraph->validate())
		return;

	// Build render context through render graph.
	m_renderGraph->build(
		m_frames[frame].renderContext
	);

	m_rootEntity->removeAllEntities();
}

void WorldRendererSimple::render(render::IRenderView* renderView, int32_t frame)
{
	m_frames[frame].renderContext->render(renderView);
}

render::ImageProcess* WorldRendererSimple::getVisualImageProcess()
{
	return nullptr;
}

void WorldRendererSimple::getDebugTargets(std::vector< render::DebugTarget >& outTargets) const
{
}

	}
}
