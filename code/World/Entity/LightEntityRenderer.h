#ifndef traktor_world_LightEntityRenderer_H
#define traktor_world_LightEntityRenderer_H

#include "World/Entity/IEntityRenderer.h"

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

/*! \brief Light entity renderer.
 * \ingroup World
 */
class T_DLLCLASS LightEntityRenderer : public IEntityRenderer
{
	T_RTTI_CLASS(LightEntityRenderer)

public:
	virtual const TypeSet getEntityTypes() const;

	virtual void render(
		WorldContext* worldContext,
		WorldRenderView* worldRenderView,
		Entity* entity
	);

	virtual void flush(
		WorldContext* worldContext,
		WorldRenderView* worldRenderView
	);
};

	}
}

#endif	// traktor_world_LightEntityRenderer_H
