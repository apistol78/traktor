#ifndef traktor_scene_EntityRendererAdapter_H
#define traktor_scene_EntityRendererAdapter_H

#include <Core/Ref.h>
#include <World/IEntityRenderer.h>

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
	
	virtual const TypeInfoSet getRenderableTypes() const T_OVERRIDE T_FINAL;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		Object* renderable
	) T_OVERRIDE T_FINAL;

	virtual void flush(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	) T_OVERRIDE T_FINAL;

private:
	Ref< EntityRendererCache > m_cache;
	Ref< world::IEntityRenderer > m_entityRenderer;
};
	
	}
}

#endif	// traktor_scene_EntityRendererAdapter_H
