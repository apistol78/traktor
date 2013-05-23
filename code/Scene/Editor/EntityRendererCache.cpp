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
	EntityAdapter* parentRenderAdapter = 0;

	if (!m_entityStack.empty())
		parentRenderAdapter = m_entityStack.back();
	else
		parentRenderAdapter = m_context->findAdapterFromEntity(entity);

	if (parentRenderAdapter)
	{
		EntityAdapter* currentEntityAdapter = 0;

		if (parentRenderAdapter->getEntity() == entity)
			currentEntityAdapter = parentRenderAdapter;
		else
			currentEntityAdapter = parentRenderAdapter->findChildAdapterFromEntity(entity);

		m_entityStack.push_back(currentEntityAdapter);
		return currentEntityAdapter;
	}
	else
		return 0;
}

void EntityRendererCache::end()
{
	if (!m_entityStack.empty())
		m_entityStack.pop_back();
}

	}
}
