#ifndef traktor_world_WorldContext_H
#define traktor_world_WorldContext_H

#include "Core/Object.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class RenderContext;

	}

	namespace world
	{

class Entity;
class IEntityRenderer;
class IWorldRenderPass;
class WorldEntityRenderers;
class WorldRenderView;

/*! \brief World render thread context.
 * \ingroup World
 */
class T_DLLCLASS WorldContext : public Object
{
	T_RTTI_CLASS;

public:
	WorldContext(WorldEntityRenderers* entityRenderers);

	void build(WorldRenderView& worldRenderView, IWorldRenderPass& worldRenderPass, Entity* entity);

	void flush(WorldRenderView& worldRenderView, IWorldRenderPass& worldRenderPass);

	render::RenderContext* getRenderContext() { return m_renderContext.ptr(); }

private:
	Ref< WorldEntityRenderers > m_entityRenderers;
	Ref< render::RenderContext > m_renderContext;
	const TypeInfo* m_lastEntityType;
	IEntityRenderer* m_lastEntityRenderer;
};

	}
}

#endif	// traktor_world_WorldContext_H
