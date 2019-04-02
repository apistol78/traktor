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
	namespace world
	{

class WorldContext;
class WorldEntityRenderers;

/*! \brief World renderer implementation.
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
	WorldRendererSimple();

	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::IRenderView* renderView,
		const WorldCreateDesc& desc
	) override final;

	virtual void destroy() override final;

	virtual bool beginBuild() override final;

	virtual void build(Entity* entity) override final;

	virtual void endBuild(WorldRenderView& worldRenderView, int frame) override final;

	virtual bool beginRender(int32_t frame, const Color4f& clearColor) override final;

	virtual void render(int32_t frame) override final;

	virtual void endRender(int32_t frame, float deltaTime) override final;

	virtual render::ImageProcess* getVisualImageProcess() override final;

	virtual void getDebugTargets(std::vector< render::DebugTarget >& outTargets) const override final;

private:
	struct Frame
	{
		Ref< WorldContext > visual;
		Matrix44 projection;
		Matrix44 view;
		Frustum viewFrustum;
		float time;

		Frame()
		:	time(0.0f)
		{
		}
	};

	Ref< render::IRenderView > m_renderView;
	Ref< render::RenderContext > m_globalContext;
	RefArray< Entity > m_buildEntities;
	AlignedVector< Frame > m_frames;
};

	}
}
