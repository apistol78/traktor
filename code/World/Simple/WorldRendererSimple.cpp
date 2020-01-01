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
	{
		frame.renderContext = new render::RenderContext(1 * 1024 * 1024);
		frame.worldContext = new WorldContext(desc.entityRenderers, frame.renderContext);
	}
	
	m_rootEntity = new GroupEntity();
	return true;
}

void WorldRendererSimple::destroy()
{
	for (auto& frame : m_frames)
	{
		frame.renderContext = nullptr;
		frame.worldContext = nullptr;
	}
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
	f.renderContext->flush();

	// Default visual parameters.
	auto globalProgramParams = f.renderContext->alloc< render::ProgramParameters >();
	globalProgramParams->beginParameters(f.renderContext);
	globalProgramParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
	globalProgramParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
	globalProgramParams->endParameters(f.renderContext);

	// Build visual context.
	WorldRenderPassSimple defaultPass(
		s_techniqueSimpleColor,
		globalProgramParams,
		worldRenderView.getView()
	);

	T_ASSERT(!f.renderContext->havePendingDraws());
	f.worldContext->build(worldRenderView, defaultPass, m_rootEntity);
	f.worldContext->flush(worldRenderView, defaultPass, m_rootEntity);
	f.renderContext->merge(render::RpAll);

	m_rootEntity->removeAllEntities();
}

void WorldRendererSimple::render(render::IRenderView* renderView, int32_t frame)
{
	Frame& f = m_frames[frame];
	f.renderContext->render(renderView);
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
