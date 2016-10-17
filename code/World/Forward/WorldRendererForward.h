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

class ImageProcess;
class ISimpleTexture;
class RenderContext;
class RenderTargetSet;

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
	) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool beginBuild() T_OVERRIDE T_FINAL;

	virtual void build(Entity* entity) T_OVERRIDE T_FINAL;

	virtual void endBuild(WorldRenderView& worldRenderView, int frame) T_OVERRIDE T_FINAL;

	virtual bool beginRender(int frame, render::EyeType eye, const Color4f& clearColor) T_OVERRIDE T_FINAL;

	virtual void render(uint32_t flags, int frame, render::EyeType eye) T_OVERRIDE T_FINAL;

	virtual void endRender(int frame, render::EyeType eye, float deltaTime) T_OVERRIDE T_FINAL;

	virtual render::ImageProcess* getVisualImageProcess() T_OVERRIDE T_FINAL;

	virtual void getDebugTargets(std::vector< render::DebugTarget >& outTargets) const T_OVERRIDE T_FINAL;

private:
	struct Slice
	{
		Ref< WorldContext > shadow;
		Matrix44 shadowLightView;
		Matrix44 shadowLightProjection;
		Matrix44 viewToLightSpace;
	};

	struct Frame
	{
		Slice slice[MaxSliceCount];
		Ref< WorldContext > depth;
		Ref< WorldContext > visual;
		Matrix44 projection;
		Matrix44 view;
		Matrix44 viewToLightSpace;
		Frustum viewFrustum;
		float time;
		float A;
		float B;
		bool haveDepth;
		bool haveShadows;

		Frame()
		:	time(0.0f)
		,	A(0.0f)
		,	B(0.0f)
		,	haveDepth(false)
		,	haveShadows(false)
		{
		}
	};

	WorldRenderSettings m_settings;
	WorldRenderSettings::ShadowSettings m_shadowSettings;
	Quality m_shadowsQuality;
	Quality m_ambientOcclusionQuality;
	Quality m_antiAliasQuality;

	Ref< render::IRenderView > m_renderView;
	Ref< IWorldShadowProjection > m_shadowProjection;
	Ref< render::RenderTargetSet > m_visualTargetSet;
	Ref< render::RenderTargetSet > m_intermediateTargetSet;
	Ref< render::RenderTargetSet > m_depthTargetSet;
	Ref< render::RenderTargetSet > m_shadowTargetSet;
	Ref< render::RenderTargetSet > m_shadowMaskProjectTargetSet;
	Ref< render::RenderTargetSet > m_shadowMaskFilterTargetSet;
	Ref< render::RenderContext > m_globalContext;
	resource::Proxy< render::ITexture > m_reflectionMap;
	Ref< render::ImageProcess > m_shadowMaskProject;
	Ref< render::ImageProcess > m_shadowMaskFilter;
	Ref< render::ImageProcess > m_ambientOcclusion;
	Ref< render::ImageProcess > m_antiAlias;
	Ref< render::ImageProcess > m_visualImageProcess;
	Ref< render::ImageProcess > m_gammaCorrectionImageProcess;
	RefArray< Entity > m_buildEntities;
	AlignedVector< Frame > m_frames;
	float m_slicePositions[MaxSliceCount + 1];
	uint32_t m_count;
	Vector4 m_fogColor;

	void buildShadows(WorldRenderView& worldRenderView, Entity* entity, int frame);

	void buildNoShadows(WorldRenderView& worldRenderView, Entity* entity, int frame);
};

	}
}

#endif	// traktor_world_WorldRendererForward_H
