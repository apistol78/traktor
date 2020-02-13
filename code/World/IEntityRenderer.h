#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "World/WorldTypes.h"

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
class WorldBuildContext;
class WorldGatherContext;
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

	/*! Gather pass. 
	 *
	 * Called once per frame to gather active lights.
	 *
	 * \note
	 * Currently specialized in gathering lights but
	 * might be refactored in the future to be able
	 * to gather other data as well.
	 *
	 * \param context World context.
	 * \param renderable Renderable instance.
	 * \param outLights Gathered lights.
	 */
	virtual void gather(
		const WorldGatherContext& context,
		const Object* renderable,
		AlignedVector< Light >& outLights
	) = 0;

	/*! Build pass.
	 *
	 * Build entity render blocks into render context.
	 */
	virtual void build(
		const WorldBuildContext& context,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass,
		Object* renderable
	) = 0;

	/*! Render flush.
	 *
	 * Flush whatever queues that the entity
	 * renderer might have used.
	 */
	virtual void flush(
		const WorldBuildContext& context,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass
	) = 0;

	/*! Render flush, last of frame. */
	virtual void flush(const WorldBuildContext& context) = 0;
};

	}
}

