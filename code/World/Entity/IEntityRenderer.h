#ifndef traktor_world_IEntityRenderer_H
#define traktor_world_IEntityRenderer_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;
class WorldContext;
class WorldRenderView;

/*! \brief Entity renderer.
 * \ingroup World
 *
 * Each renderable entity type should have
 * a matching EntityRenderer.
 * The entity renderer is responsible of
 * producing render commands from entity instances.
 */
class T_DLLCLASS IEntityRenderer : public Object
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEntityTypes() const = 0;

	/*! \brief Render pass.
	 *
	 * Render entities into render context.
	 */
	virtual void render(
		world::WorldContext* worldContext,
		world::WorldRenderView* worldRenderView,
		world::Entity* entity
	) = 0;

	/*! \brief Render flush.
	 *
	 * Flush whatever queues that the entity renderer might have used.
	 */
	virtual void flush(
		world::WorldContext* worldContext,
		world::WorldRenderView* worldRenderView
	) = 0;
};

	}
}

#endif	// traktor_world_IEntityRenderer_H
