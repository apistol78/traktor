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
	world::WorldContext* worldContext,
	world::WorldRenderView* worldRenderView,
	world::Entity* entity
)
{
	EntityAdapter* entityAdapter = m_context->findAdapterFromEntity(entity);
	if (!entityAdapter || entityAdapter->isVisible())
	{
		m_entityRenderer->render(
			worldContext,
			worldRenderView,
			entity
		);
	}
}

void EntityRendererAdapter::flush(
	world::WorldContext* worldContext,
	world::WorldRenderView* worldRenderView
)
{
	m_entityRenderer->flush(
		worldContext,
		worldRenderView
	);
}
	
	}
}
