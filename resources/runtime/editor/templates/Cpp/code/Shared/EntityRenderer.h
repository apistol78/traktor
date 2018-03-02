#ifndef EntityRenderer_H
#define EntityRenderer_H

#include <World/IEntityRenderer.h>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MYCPPTEST_SHARED_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

class T_DLLCLASS EntityRenderer : public traktor::world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	virtual const traktor::TypeInfoSet getRenderableTypes() const;

	virtual void render(
		traktor::world::WorldContext& worldContext,
		traktor::world::WorldRenderView& worldRenderView,
		traktor::world::IWorldRenderPass& worldRenderPass,
		traktor::Object* renderable
	);

	virtual void flush(
		traktor::world::WorldContext& worldContext,
		traktor::world::WorldRenderView& worldRenderView,
		traktor::world::IWorldRenderPass& worldRenderPass
	);
};

#endif	// EntityRenderer_H
