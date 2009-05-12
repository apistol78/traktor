#ifndef traktor_world_WorldContext_H
#define traktor_world_WorldContext_H

#include "Core/Heap/Ref.h"
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

class RenderView;
class RenderContext;

	}

	namespace world
	{

class Entity;
class EntityRenderer;
class WorldRenderer;
class WorldRenderView;

/*! \brief World render thread context.
 * \ingroup World
 */
class T_DLLCLASS WorldContext : public Object
{
	T_RTTI_CLASS(WorldContext)

public:
	WorldContext(WorldRenderer* worldRenderer, render::RenderView* renderView);

	void render(WorldRenderView* worldRenderView, Entity* entity);

	void render(uint32_t flags);

	void flush();

	inline render::RenderContext* getRenderContext() { return m_renderContext; }

private:
	Ref< WorldRenderer > m_worldRenderer;
	Ref< render::RenderContext > m_renderContext;
	const Type* m_lastEntityType;
	Ref< EntityRenderer > m_lastEntityRenderer;
};

	}
}

#endif	// traktor_world_WorldContext_H
