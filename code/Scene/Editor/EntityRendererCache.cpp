#include "Core/Log/Log.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityRendererCache.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "World/Entity.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityRendererCache", EntityRendererCache, Object)

EntityRendererCache::EntityRendererCache(SceneEditorContext* context)
:	m_context(context)
{
}

EntityAdapter* EntityRendererCache::begin(const Object* renderable)
{
	if (const world::Entity* entity = dynamic_type_cast< const world::Entity* >(renderable))
	{
		EntityAdapter* parentRenderAdapter = m_context->findAdapterFromEntity(entity);
		if (parentRenderAdapter)
		{
			EntityAdapter* currentEntityAdapter = nullptr;

			if (parentRenderAdapter->getEntity() == entity)
				currentEntityAdapter = parentRenderAdapter;
			else
				currentEntityAdapter = parentRenderAdapter->findChildAdapterFromEntity(entity);

			return currentEntityAdapter;
		}
	}
	return nullptr;
}

void EntityRendererCache::end()
{
}

	}
}
