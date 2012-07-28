#ifndef traktor_scene_EntityRendererAdapter_H
#define traktor_scene_EntityRendererAdapter_H

#include <Core/Ref.h>
#include <World/Entity/IEntityRenderer.h>

namespace traktor
{
	namespace scene
	{
	
class EntityRendererCache;
	
class EntityRendererAdapter : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	EntityRendererAdapter(EntityRendererCache* cache, world::IEntityRenderer* entityRenderer);
	
	virtual const TypeInfoSet getEntityTypes() const;

	virtual void precull(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::Entity* entity
	);

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		world::Entity* entity
	);

	virtual void flush(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	);

private:
	Ref< EntityRendererCache > m_cache;
	Ref< world::IEntityRenderer > m_entityRenderer;
};
	
	}
}

#endif	// traktor_scene_EntityRendererAdapter_H
