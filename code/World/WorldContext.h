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
	namespace render
	{

class RenderContext;

	}

	namespace world
	{

class Entity;
class IWorldRenderPass;
class WorldEntityRenderers;
class WorldRenderView;

/*! World render thread context.
 * \ingroup World
 */
class T_DLLCLASS WorldContext : public Object
{
	T_RTTI_CLASS;

public:
	WorldContext(WorldEntityRenderers* entityRenderers, Entity* rootEntity);

	WorldContext(WorldEntityRenderers* entityRenderers, render::RenderContext* renderContext, Entity* rootEntity);

	void gather(const Object* renderable, AlignedVector< Light >& outLights) const;

	void build(const WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass, Object* renderable) const;

	void flush(const WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass) const;

	void flush() const;

	WorldEntityRenderers* getEntityRenderers() const { return m_entityRenderers; }

	render::RenderContext* getRenderContext() const { return m_renderContext; }

	Entity* getRootEntity() const { return m_rootEntity; }

private:
	WorldEntityRenderers* m_entityRenderers;
	render::RenderContext* m_renderContext;
	Entity* m_rootEntity;
};

	}
}

