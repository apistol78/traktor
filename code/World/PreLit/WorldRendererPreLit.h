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
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::IRenderView* renderView,
		const WorldCreateDesc& desc
	);

	virtual void destroy();

	virtual void createRenderView(const WorldViewPerspective& worldView, WorldRenderView& outRenderView) const;

	virtual void createRenderView(const WorldViewOrtho& worldView, WorldRenderView& outRenderView) const;

	virtual bool beginBuild();

	virtual void build(Entity* entity);

	virtual void endBuild(WorldRenderView& worldRenderView, int frame);

	virtual bool beginRender(int frame, render::EyeType eye, const Color4f& clearColor);

	virtual void render(uint32_t flags, int frame, render::EyeType eye);

	virtual void endRender(int frame, render::EyeType eye, float deltaTime);

	virtual PostProcess* getVisualPostProcess();

	virtual render::RenderTargetSet* getVisualTargetSet();

	virtual render::RenderTargetSet* getDepthTargetSet();

	virtual render::RenderTargetSet* getShadowMaskTargetSet();

	virtual void getTargets(RefArray< render::ITexture >& outTargets) const;

private:
	struct Slice
	{
		Ref< WorldContext > shadow[MaxLightShadowCount];
		Matrix44 shadowLightView[MaxLightShadowCount];
		Matrix44 shadowLightProjection[MaxLightShadowCount];
		Matrix44 shadowLightSquareProjection[MaxLightShadowCount];
		Matrix44 viewToLightSpace[MaxLightShadowCount];
	};

	struct Frame
	{
		Ref< WorldCullingSwRaster > culling;
		Slice slice[MaxSliceCount];
		Ref< WorldContext > gbuffer;
		Ref< WorldContext > visual;
		float time;
		Matrix44 projection;
		Matrix44 view;
		Frustum viewFrustum;
		Light lights[MaxLightCount];
		uint32_t lightCount;
		Vector4 godRayDirection;
		bool haveGBuffer;
		bool haveShadows[MaxLightCount];

		Frame()
		:	time(0.0f)
		,	lightCount(0)
		,	haveGBuffer(false)
		{
			for (uint32_t i = 0; i < MaxLightCount; ++i)
				haveShadows[i] = false;
		}
	};

	static render::handle_t ms_techniquePreLitColor;
	static render::handle_t ms_techniqueGBuffer;
	static render::handle_t ms_techniqueShadow;
	static render::handle_t ms_handleTime;
	static render::handle_t ms_handleView;
	static render::handle_t ms_handleViewInverse;
	static render::handle_t ms_handleProjection;
	static render::handle_t ms_handleSquareProjection;
	static render::handle_t ms_handleColorMap;
	static render::handle_t ms_handleDepthMap;
	static render::handle_t ms_handleLightMap;
	static render::handle_t ms_handleNormalMap;
	static render::handle_t ms_handleReflectionMap;
	static render::handle_t ms_handleFogDistanceAndDensity;
	static render::handle_t ms_handleFogColor;

	WorldRenderSettings m_settings;
	WorldRenderSettings::ShadowSettings m_shadowSettings;
	Quality m_shadowsQuality;
	Quality m_ambientOcclusionQuality;
	Quality m_antiAliasQuality;

	Ref< render::IRenderView > m_renderView;
	Ref< IWorldShadowProjection > m_shadowProjection0;
	Ref< IWorldShadowProjection > m_shadowProjection;
	Ref< render::RenderTargetSet > m_visualTargetSet;
	Ref< render::RenderTargetSet > m_intermediateTargetSet;
	Ref< render::RenderTargetSet > m_gbufferTargetSet;
	Ref< render::RenderTargetSet > m_colorTargetSet;
	Ref< render::RenderTargetSet > m_shadowTargetSet;
	Ref< render::RenderTargetSet > m_shadowMaskProjectTargetSet;
	Ref< render::RenderTargetSet > m_shadowMaskFilterTargetSet;
	Ref< render::RenderTargetSet > m_lightMapTargetSet;
	Ref< render::RenderContext > m_globalContext;
	resource::Proxy< render::ITexture > m_reflectionMap;
	Ref< PostProcess > m_shadowMaskProject;
	Ref< PostProcess > m_shadowMaskFilter;
	Ref< PostProcess > m_colorTargetCopy;
	Ref< PostProcess > m_ambientOcclusion;
	Ref< PostProcess > m_antiAlias;
	Ref< PostProcess > m_visualPostProcess;
	Ref< PostProcess > m_gammaCorrectionPostProcess;
	Ref< LightRenderer > m_lightRenderer;
	RefArray< Entity > m_buildEntities;
	AlignedVector< Frame > m_frames;
	float m_slicePositions[MaxSliceCount + 1];
	uint32_t m_count;
	Vector4 m_fogDistanceAndDensity;
	Vector4 m_fogColor;

	void buildLightWithShadows(WorldRenderView& worldRenderView, int frame);

	void buildLightWithNoShadows(WorldRenderView& worldRenderView, int frame);

	void buildVisual(WorldRenderView& worldRenderView, int frame);
};

	}
}

#endif	// traktor_world_WorldRendererPreLit_H
