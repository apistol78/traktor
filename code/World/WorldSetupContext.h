#pragma once

#include "Core/Object.h"
#include "World/WorldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class RenderGraph;

}

namespace traktor::world
{

class Entity;
class WorldEntityRenderers;

/*! World setup context.
 * \ingroup World
 */
class T_DLLCLASS WorldSetupContext : public Object
{
	T_RTTI_CLASS;

public:
	explicit WorldSetupContext(const WorldEntityRenderers* entityRenderers, const Entity* rootEntity, render::RenderGraph& renderGraph);

	const WorldEntityRenderers* getEntityRenderers() const { return m_entityRenderers; }

	const Entity* getRootEntity() const { return m_rootEntity; }

	render::RenderGraph& getRenderGraph() const { return m_renderGraph; }

private:
	const WorldEntityRenderers* m_entityRenderers;
	const Entity* m_rootEntity;
	render::RenderGraph& m_renderGraph;
};

}
