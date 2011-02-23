#ifndef traktor_world_WorldRendererForward_H
#define traktor_world_WorldRendererForward_H

#include "Core/Containers/AlignedVector.h"
#include "World/IWorldRenderer.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"

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

class ISimpleTexture;
class RenderContext;
class RenderTargetSet;

	}

	namespace world
	{

class PostProcess;
class WorldContext;
class WorldEntityRenderers;

/*! \brief World renderer implementation.
 * \ingroup World
 *
 * Simple and naive implementation supporting
 * only a single shadowing directional light
 * and max 2 overlapping lights.
 *
 * Operation
 * 1. Render global shadow map if directional light 0 enabled.
 * 2. Render depth pass if enabled (either explicitly or shadows enabled).
 * 3. Render screen-space shadow mask.
 * 4. Render visuals.
 *
 * Techniques used
 * "Default" - Visual, final, color output.
 * "Depth"   - Depth pass; write view distance into depth buffer.
 * "Shadow"  - Shadow map pass; write distance from light into shadow map.
 */
class T_DLLCLASS WorldRendererForward : public IWorldRenderer
{
	T_RTTI_CLASS;

public:
	WorldRendererForward();

	virtual bool create(
		const WorldRenderSettings& settings,
		WorldEntityRenderers* entityRenderers,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::IRenderView* renderView,
		uint32_t multiSample,
		uint32_t frameCount
	);

	virtual void destroy();

	virtual void createRenderView(const WorldViewPerspective& worldView, WorldRenderView& outRenderView) const;

	virtual void createRenderView(const WorldViewOrtho& worldView, WorldRenderView& outRenderView) const;

	virtual void build(WorldRenderView& worldRenderView, Entity* entity, int frame);

	virtual void render(uint32_t flags, int frame, render::EyeType eye);

	virtual void getTargets(RefArray< render::ITexture >& outTargets) const;

private:
	struct Frame
	{
		Ref< WorldContext > depth;
		Ref< WorldContext > shadow;
		Ref< WorldContext > visual;
		WorldRenderView depthRenderView;
		WorldRenderView shadowRenderView;
		Matrix44 projection;
		Matrix44 viewToLightSpace;
		Matrix44 squareProjection;
		Frustum viewFrustum;
		float A;
		float B;
		bool haveDepth;
		bool haveShadows;

		Frame()
		:	A(0.0f)
		,	B(0.0f)
		,	haveDepth(false)
		,	haveShadows(false)
		{
		}
	};

	static render::handle_t ms_techniqueDefault;
	static render::handle_t ms_techniqueDepth;
	static render::handle_t ms_techniqueShadow;
	static render::handle_t ms_handleTime;
	static render::handle_t ms_handleProjection;

	WorldRenderSettings m_settings;
	Ref< render::IRenderView > m_renderView;
	Ref< render::RenderTargetSet > m_depthTargetSet;
	Ref< render::RenderTargetSet > m_shadowTargetSet;
	Ref< render::RenderTargetSet > m_shadowMaskTargetSet;
	Ref< render::RenderContext > m_globalContext;
	Ref< PostProcess > m_shadowMaskProjection;
	AlignedVector< Frame > m_frames;
	uint32_t m_count;

	void buildShadows(WorldRenderView& worldRenderView, Entity* entity, int frame);

	void buildNoShadows(WorldRenderView& worldRenderView, Entity* entity, int frame);
};

	}
}

#endif	// traktor_world_WorldRendererForward_H
