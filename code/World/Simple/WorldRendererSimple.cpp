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

	m_globalContext = new render::RenderContext(16 * 1024);
	
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

	// Flush render contexts.
	f.visual->getRenderContext()->flush();

	// Begun building new frame.
	const Matrix44& view = worldRenderView.getView();
	Matrix44 viewInverse = view.inverse();

	worldRenderView.setEyePosition(viewInverse.translation().xyz1());
	worldRenderView.setEyeDirection(viewInverse.axisZ().xyz0());

	// Build visual context.
	WorldRenderPassSimple defaultPass(
		s_techniqueSimpleColor,
		worldRenderView.getView()
	);
	f.visual->build(worldRenderView, defaultPass, m_rootEntity);
	f.visual->flush(worldRenderView, defaultPass, m_rootEntity);

	// Store some global values.
	f.projection = worldRenderView.getProjection();
	f.view = worldRenderView.getView();
	f.viewFrustum = worldRenderView.getViewFrustum();
	f.time = worldRenderView.getTime();

	m_rootEntity->removeAllEntities();
}

bool WorldRendererSimple::beginRender(render::IRenderView* renderView, int32_t frame, const Color4f& clearColor)
{
	return true;
}

void WorldRendererSimple::render(render::IRenderView* renderView, int32_t frame)
{
	Frame& f = m_frames[frame];

	render::ProgramParameters defaultProgramParams;
	defaultProgramParams.beginParameters(m_globalContext);
	defaultProgramParams.setFloatParameter(s_handleTime, f.time);
	defaultProgramParams.setMatrixParameter(s_handleProjection, f.projection);
	defaultProgramParams.endParameters(m_globalContext);

	f.visual->getRenderContext()->render(renderView, render::RpAll, &defaultProgramParams);

	m_globalContext->flush();
}

void WorldRendererSimple::endRender(render::IRenderView* renderView, int32_t frame, float deltaTime)
{
	Frame& f = m_frames[frame];
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
