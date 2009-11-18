#ifndef traktor_world_WorldContext_H
#define traktor_world_WorldContext_H

#include "Core/Object.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Frustum.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderView;
class RenderContext;

	}

	namespace world
	{

class Entity;
class IEntityRenderer;
class WorldRenderer;
class WorldEntityRenderers;
class WorldRenderView;

/*! \brief World render thread context.
 * \ingroup World
 */
class T_DLLCLASS WorldContext : public Object
{
	T_RTTI_CLASS;

public:
	WorldContext(WorldRenderer* worldRenderer, WorldEntityRenderers* entityRenderers, render::IRenderView* renderView);

	void build(WorldRenderView* worldRenderView, Entity* entity);

	void flush(WorldRenderView* worldRenderView);

	inline Ref< render::RenderContext > getRenderContext() { return m_renderContext; }

private:
	Ref< WorldRenderer > m_worldRenderer;
	Ref< WorldEntityRenderers > m_entityRenderers;
	Ref< render::RenderContext > m_renderContext;
	const TypeInfo* m_lastEntityType;
	Ref< IEntityRenderer > m_lastEntityRenderer;
};

	}
}

#endif	// traktor_world_WorldContext_H
