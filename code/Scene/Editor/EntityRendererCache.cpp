#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityRendererCache.h"
#include "Scene/Editor/SceneEditorContext.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityRendererCache", EntityRendererCache, Object)

EntityRendererCache::EntityRendererCache(SceneEditorContext* context)
:	m_context(context)
{
}

EntityAdapter* EntityRendererCache::begin(const world::Entity* entity)
{
	EntityAdapter* parentRenderAdapter = m_context->findAdapterFromEntity(entity);
	if (parentRenderAdapter)
	{
		EntityAdapter* currentEntityAdapter = 0;

		if (parentRenderAdapter->getEntity() == entity)
			currentEntityAdapter = parentRenderAdapter;
		else
			currentEntityAdapter = parentRenderAdapter->findChildAdapterFromEntity(entity);

		return currentEntityAdapter;
	}
	else
		return 0;
}

void EntityRendererCache::end()
{
}

	}
}
