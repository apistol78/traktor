#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityRendererAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"

namespace traktor
{
	namespace scene
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityRendererAdapter", EntityRendererAdapter, world::IEntityRenderer)

EntityRendererAdapter::EntityRendererAdapter(SceneEditorContext* context, world::IEntityRenderer* entityRenderer)
:	m_context(context)
,	m_entityRenderer(entityRenderer)
{
}

const TypeInfoSet EntityRendererAdapter::getEntityTypes() const
{
	return m_entityRenderer->getEntityTypes();
}

void EntityRendererAdapter::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	world::Entity* entity
)
{
	EntityAdapter* entityAdapter = m_context->beginRenderEntity(entity);
	if (!entityAdapter || entityAdapter->isVisible())
	{
		m_entityRenderer->render(
			worldContext,
			worldRenderView,
			worldRenderPass,
			entity
		);
	}
	 m_context->endRenderEntity();
}

void EntityRendererAdapter::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	m_entityRenderer->flush(
		worldContext,
		worldRenderView,
		worldRenderPass
	);
}
	
	}
}
