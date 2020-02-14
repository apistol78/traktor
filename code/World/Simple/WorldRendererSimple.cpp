#include "Core/Misc/SafeDestroy.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "World/WorldRenderView.h"
#include "World/WorldBuildContext.h"
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
	return true;
}

void WorldRendererSimple::destroy()
{
	m_entityRenderers = nullptr;
	m_rootEntity = nullptr;
}

void WorldRendererSimple::attach(Entity* entity)
{
	m_rootEntity->addEntity(entity);
}

void WorldRendererSimple::setup(const WorldRenderView& worldRenderView, render::RenderGraph& renderGraph)
{
	Ref< render::RenderPass > rp = new render::RenderPass(L"Visual");
	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			WorldBuildContext wc(
				m_entityRenderers,
				m_rootEntity,
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

			wc.build(worldRenderView, defaultPass, m_rootEntity);
			wc.flush(worldRenderView, defaultPass);
			renderContext->merge(render::RpAll);
		}
	);
	renderGraph.addPass(rp);

	// Flush attached entities.
	m_rootEntity->removeAllEntities();
}

	}
}
