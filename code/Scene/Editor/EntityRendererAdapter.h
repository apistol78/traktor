#ifndef traktor_scene_EntityRendererAdapter_H
#define traktor_scene_EntityRendererAdapter_H

#include <Core/Ref.h>
#include <World/Entity/IEntityRenderer.h>

namespace traktor
{
	namespace scene
	{
	
class SceneEditorContext;
	
class EntityRendererAdapter : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	EntityRendererAdapter(SceneEditorContext* context, world::IEntityRenderer* entityRenderer);
	
	virtual const TypeInfoSet getEntityTypes() const;

	virtual void render(
		world::WorldContext* worldContext,
		world::WorldRenderView* worldRenderView,
		world::Entity* entity
	);

	virtual void flush(
		world::WorldContext* worldContext,
		world::WorldRenderView* worldRenderView
	);

private:
	Ref< SceneEditorContext > m_context;
	Ref< world::IEntityRenderer > m_entityRenderer;
};
	
	}
}

#endif	// traktor_scene_EntityRendererAdapter_H
