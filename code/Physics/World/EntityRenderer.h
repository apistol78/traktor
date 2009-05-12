#ifndef traktor_physics_EntityRenderer_H
#define traktor_physics_EntityRenderer_H

#include "World/Entity/EntityRenderer.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! \brief Physics entity renderer.
 * \ingroup Physics
 */
class T_DLLCLASS EntityRenderer : public world::EntityRenderer
{
	T_RTTI_CLASS(EntityRenderer)

public:
	virtual const TypeSet getEntityTypes() const;

	virtual void render(
		world::WorldContext* worldContext,
		world::WorldRenderView* worldRenderView,
		world::Entity* entity
	);

	virtual void flush(
		world::WorldContext* worldContext,
		world::WorldRenderView* worldRenderView
	)
	{ /* Not implemented */ }
};

	}
}

#endif	// traktor_physics_EntityRenderer_H
