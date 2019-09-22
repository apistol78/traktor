#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;
class IWorldRenderPass;
class WorldContext;
class WorldRenderView;

/*! Entity renderer.
 * \ingroup World
 *
 * Each renderable type should have
 * a matching EntityRenderer.
 */
class T_DLLCLASS IEntityRenderer : public Object
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getRenderableTypes() const = 0;

	/*! Render pass.
	 *
	 * Render instance into render context.
	 */
	virtual void render(
		WorldContext& worldContext,
		WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass,
		Object* renderable
	) = 0;

	/*! Render flush.
	 *
	 * Flush whatever queues that the entity
	 * renderer might have used.
	 */
	virtual void flush(
		WorldContext& worldContext,
		WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass,
		Entity* rootEntity
	) = 0;
};

	}
}

