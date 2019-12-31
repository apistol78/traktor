#include "Render/Context/RenderContext.h"
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

render::handle_t s_techniqueSimpleColor = 0;
render::handle_t s_handleTime = 0;
render::handle_t s_handleView = 0;
render::handle_t s_handleViewInverse = 0;
render::handle_t s_handleProjection = 0;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererSimple", 0, WorldRendererSimple, IWorldRenderer)

WorldRendererSimple::WorldRendererSimple()
{
	// Techniques
	s_techniqueSimpleColor = render::getParameterHandle(L"World_SimpleColor");

	// Global parameters.
	s_handleTime = render::getParameterHandle(L"World_Time");
	s_handleView = render::getParameterHandle(L"World_View");
	s_handleViewInverse = render::getParameterHandle(L"World_ViewInverse");
	s_handleProjection = render::getParameterHandle(L"World_Projection");
}

bool WorldRendererSimple::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc
)
{
	m_frames.resize(desc.frameCount);
	for (auto& frame : m_frames)
		frame.visual = new WorldContext(desc.entityRenderers);
	
	m_rootEntity = new GroupEntity();
	return true;
}

void WorldRendererSimple::destroy()
{
	for (auto& frame : m_frames)
		frame.visual = nullptr;
}

void WorldRendererSimple::attach(Entity* entity)
{
	m_rootEntity->addEntity(entity);
}

void WorldRendererSimple::build(WorldRenderView& worldRenderView, int32_t frame)
{
	Frame& f = m_frames[frame];

	// Ensure no lights in view.
	worldRenderView.resetLights();

	// Flush render contexts.
	f.visual->getRenderContext()->flush();

	// Default visual parameters.
	auto globalProgramParams = f.visual->getRenderContext()->alloc< render::ProgramParameters >();
	globalProgramParams->beginParameters(f.visual->getRenderContext());
	globalProgramParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
	globalProgramParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
	globalProgramParams->endParameters(f.visual->getRenderContext());

	// Build visual context.
	WorldRenderPassSimple defaultPass(
		s_techniqueSimpleColor,
		globalProgramParams,
		worldRenderView.getView()
	);
	f.visual->build(worldRenderView, defaultPass, m_rootEntity);
	f.visual->flush(worldRenderView, defaultPass, m_rootEntity);

	m_rootEntity->removeAllEntities();
}

bool WorldRendererSimple::beginRender(render::IRenderView* renderView, int32_t frame, const Color4f& clearColor)
{
	return true;
}

void WorldRendererSimple::render(render::IRenderView* renderView, int32_t frame)
{
	Frame& f = m_frames[frame];
	f.visual->getRenderContext()->render(renderView, render::RpAll);
}

void WorldRendererSimple::endRender(render::IRenderView* renderView, int32_t frame, float deltaTime)
{
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
