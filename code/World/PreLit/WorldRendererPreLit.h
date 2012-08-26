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

class IWorldShadowProjection;
class LightRenderer;
class PostProcess;
class WorldContext;
class WorldCullingSwRaster;

/*! \brief World renderer (P)re-(L)ighting implementation.
 * \ingroup World
 *
 * Using light pre-pass to generate incoming
 * lighting to all pixel fragments.
 * Pre-light available as a texture to all
 * shaders during final visual pass.
 *
 * Operation
 * 1. Render gbuffer (depth+normals) pass.
 * For each light
 *   2. Clear shadow mask target.
 *   If light is directional
 *     For each slice
 *       3. Render slice shadow map.
 *       4. Render screen-space shadow mask; accumulate shadow mask target.
 *     End
 *     5. Filter shadow mask target.
 *   End
 *   6. Render screen-space lighting; accumulate in lighting target.
 * End
 * 7. Render visuals.
 *
 * Techniques used
 * "Default" - Visual, final, color output.
 * "GBuffer" - GBuffer pass; write geometry buffers.
 * "Shadow"  - Shadow map pass; write distance from light into shadow map.
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

	virtual render::RenderTargetSet* getDepthTargetSet();

	virtual render::RenderTargetSet* getShadowMaskTargetSet();

	virtual void getTargets(RefArray< render::ITexture >& outTargets) const;

private:
	struct Slice
	{
		Ref< WorldContext > shadow[MaxLightCount];
		Matrix44 viewToLightSpace[MaxLightCount];
		Matrix44 squareProjection[MaxLightCount];
	};

	struct Frame
	{
		Ref< WorldCullingSwRaster > culling;

		Slice slice[MaxSliceCount];
		Ref< WorldContext > gbuffer;
		Ref< WorldContext > visual;

		Matrix44 projection;
		Matrix44 view;
		Frustum viewFrustum;

		Light lights[MaxLightCount];
		uint32_t lightCount;

		bool haveGBuffer;
		bool haveShadows[MaxLightCount];

		Frame()
		:	haveGBuffer(false)
		,	lightCount(0)
		{
			for (uint32_t i = 0; i < MaxLightCount; ++i)
				haveShadows[i] = false;
		}
	};

	static render::handle_t ms_techniquePreLitColor;
	static render::handle_t ms_techniqueGBuffer;
	static render::handle_t ms_techniqueShadow;
	static render::handle_t ms_handleTime;
	static render::handle_t ms_handleProjection;

	WorldRenderSettings m_settings;
	Ref< IWorldShadowProjection > m_shadowProjection;
	Ref< render::IRenderView > m_renderView;
	Ref< render::RenderTargetSet > m_gbufferTargetSet;
	Ref< render::RenderTargetSet > m_shadowTargetSet;
	Ref< render::RenderTargetSet > m_shadowMaskProjectTargetSet;
	RefArray< render::RenderTargetSet > m_shadowMaskFilterTargetSet;
	Ref< render::RenderTargetSet > m_lightMapTargetSet;
	Ref< render::RenderContext > m_globalContext;
	Ref< PostProcess > m_shadowMaskProject;
	Ref< PostProcess > m_shadowMaskFilter;
	Ref< PostProcess > m_ambientOcclusion;
	Ref< LightRenderer > m_lightRenderer;
	AlignedVector< Frame > m_frames;
	float m_slicePositions[MaxSliceCount + 1];
	uint32_t m_count;
	Vector4 m_fogColor;

	void buildLightWithShadows(WorldRenderView& worldRenderView, Entity* entity, int frame);

	void buildLightWithNoShadows(WorldRenderView& worldRenderView, Entity* entity, int frame);

	void buildVisual(WorldRenderView& worldRenderView, Entity* entity, int frame);
};

	}
}

#endif	// traktor_world_WorldRendererPreLit_H
