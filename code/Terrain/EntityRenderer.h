#ifndef traktor_terrain_EntityRenderer_H
#define traktor_terrain_EntityRenderer_H

#include "World/Entity/IEntityRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace terrain
	{

class T_DLLCLASS EntityRenderer : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
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
};

	}
}

#endif	// traktor_terrain_EntityRenderer_H
