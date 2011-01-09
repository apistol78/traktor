#ifndef traktor_world_WorldRendererPreLit_H
#define traktor_world_WorldRendererPreLit_H

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

/*! \brief World renderer (P)re-(L)ighting implementation.
 * \ingroup World
 *
 * Using light pre-pass to generate incoming
 * lighting to all pixel fragments.
 * Pre-light available as a texture to all
 * shaders during final visual pass.
 *
 * Operation
 * 1. Render depth pass.
 * For each light
 *   2. Render shadow map from light if directional.
 *   3. Render screen-space shadow. mask.
 *   4. Render screen-space lighting; accumulate in lighting target.
 * 5. Render visuals.
 *
 * Techniques used
 * "Default" - Visual, final, color output.
 * "Depth"   - Depth pass; write view distance into depth buffer.
 * "Shadow"  - Shadow map pass; write distance from light into shadow map.
 * "Light"   - Light pass; add lighting from light into light buffer.
 */
class T_DLLCLASS WorldRendererPreLit : public IWorldRenderer
{
	T_RTTI_CLASS;

public:
	WorldRendererPreLit();

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
		Ref< WorldContext > shadow[WorldRenderView::MaxLightCount];
		Ref< WorldContext > light[WorldRenderView::MaxLightCount];
		Ref< WorldContext > visual;
		Matrix44 projection;
		Matrix44 viewToLightSpace[WorldRenderView::MaxLightCount];
		Frustum viewFrustum;
		bool haveDepth;
		bool haveShadows[WorldRenderView::MaxLightCount];
		uint32_t lightCount;

		Frame()
		:	haveDepth(false)
		,	lightCount(0)
		{
			for (uint32_t i = 0; i < WorldRenderView::MaxLightCount; ++i)
				haveShadows[i] = false;
		}
	};

	static render::handle_t ms_techniqueDefault;
	static render::handle_t ms_techniqueLight;
	static render::handle_t ms_techniqueDepth;
	static render::handle_t ms_techniqueShadow;
	static render::handle_t ms_handleTime;
	static render::handle_t ms_handleProjection;

	WorldRenderSettings m_settings;
	Ref< render::IRenderView > m_renderView;
	Ref< render::RenderTargetSet > m_depthTargetSet;
	Ref< render::RenderTargetSet > m_shadowTargetSet;
	Ref< render::RenderTargetSet > m_shadowMaskTargetSet;
	Ref< render::RenderTargetSet > m_lightTargetSet;
	Ref< render::RenderContext > m_globalContext;
	Ref< PostProcess > m_shadowMaskProjection;
	AlignedVector< Frame > m_frames;
	uint32_t m_count;

	void buildLightWithShadows(WorldRenderView& worldRenderView, Entity* entity, int frame);

	void buildLightWithNoShadows(WorldRenderView& worldRenderView, Entity* entity, int frame);

	void buildVisual(WorldRenderView& worldRenderView, Entity* entity, int frame);
};

	}
}

#endif	// traktor_world_WorldRendererPreLit_H
