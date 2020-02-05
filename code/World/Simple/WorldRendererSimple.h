#pragma once

#include "World/IWorldRenderer.h"

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

class RenderGraph;

	}

	namespace world
	{

class GroupEntity;
class WorldEntityRenderers;

/*! World renderer implementation.
 * \ingroup World
 *
 * Simple world renderer, no support for
 * shadows, lights, postprocessing etc.
 *
 * Techniques used
 * "Simple" - Visual, final, color output.
 */
class T_DLLCLASS WorldRendererSimple : public IWorldRenderer
{
	T_RTTI_CLASS;

public:
	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const WorldCreateDesc& desc
	) override final;

	virtual void destroy() override final;

	virtual void attach(Entity* entity) override final;

	virtual void build(const WorldRenderView& worldRenderView, int32_t frame) override final;

	virtual void render(render::IRenderView* renderView, int32_t frame) override final;

private:
	struct Frame
	{
		Ref< render::RenderContext > renderContext;
	};

	Ref< render::RenderGraph > m_renderGraph;
	Ref< WorldEntityRenderers > m_entityRenderers;
	Ref< GroupEntity > m_rootEntity;
	AlignedVector< Frame > m_frames;
};

	}
}
