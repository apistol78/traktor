#pragma once

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

class ImageProcess;
class ISimpleTexture;
class RenderContext;
class RenderTargetSet;
class StructBuffer;

	}

	namespace world
	{

class IWorldShadowProjection;
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
	struct FrameShadow
	{
		Ref< WorldContext > shadow;
		Matrix44 shadowLightView;
		Matrix44 shadowLightProjection;
		Matrix44 viewToLightSpace;
	};

	struct Frame
	{
		FrameShadow slice[MaxSliceCount];
		FrameShadow atlas[16];

		Ref< WorldContext > depth;
		Ref< WorldContext > visual;

		Ref< render::StructBuffer > lightSBuffer;

		Matrix44 projection;
		Matrix44 view;
		Frustum viewFrustum;
		float time;
		int32_t lightCount;
		bool haveDepth;
		bool haveShadows;

		Frame()
		:	time(0.0f)
		,	lightCount(0)
		,	haveDepth(false)
		,	haveShadows(false)
		{
		}
	};

	WorldRenderSettings m_settings;
	Quality m_toneMapQuality;
	Quality m_shadowsQuality;
	Quality m_ambientOcclusionQuality;
	Quality m_antiAliasQuality;

	Ref< render::IRenderView > m_renderView;
	Ref< render::RenderTargetSet > m_visualTargetSet;
	Ref< render::RenderTargetSet > m_intermediateTargetSet;
	Ref< render::RenderTargetSet > m_gbufferTargetSet;
	Ref< render::RenderContext > m_globalContext;
	resource::Proxy< render::ITexture > m_reflectionMap;
	Ref< render::ImageProcess > m_ambientOcclusion;
	Ref< render::ImageProcess > m_antiAlias;
	Ref< render::ImageProcess > m_visualImageProcess;
	Ref< render::ImageProcess > m_gammaCorrectionImageProcess;
	Ref< render::ImageProcess > m_toneMapImageProcess;

	// Directional shadow map.
	Ref< IWorldShadowProjection > m_shadowProjection;
	Ref< render::RenderTargetSet > m_shadowCascadeTargetSet;

	// Point/Spot shadow map.
	Ref< render::RenderTargetSet > m_shadowAtlasTargetSet;

	RefArray< Entity > m_buildEntities;
	AlignedVector< Frame > m_frames;
	float m_slicePositions[MaxSliceCount + 1];
	uint32_t m_count;

	void buildGBuffer(WorldRenderView& worldRenderView, int frame);

	void buildLights(WorldRenderView& worldRenderView, int frame);

	void buildVisual(WorldRenderView& worldRenderView, int frame);
};

	}
}
