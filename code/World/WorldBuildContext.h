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

/*! World build context.
 * \ingroup World
 */
class T_DLLCLASS WorldBuildContext : public Object
{
	T_RTTI_CLASS;

public:
	WorldBuildContext(const WorldEntityRenderers* entityRenderers, const Entity* rootEntity, render::RenderContext* renderContext);

	void build(const WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass, const Object* renderable) const;

	void flush(const WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass) const;

	void flush() const;

	const WorldEntityRenderers* getEntityRenderers() const { return m_entityRenderers; }

	const Entity* getRootEntity() const { return m_rootEntity; }

	render::RenderContext* getRenderContext() const { return m_renderContext; }

private:
	const WorldEntityRenderers* m_entityRenderers;
	const Entity* m_rootEntity;
	render::RenderContext* m_renderContext;
};

	}
}

