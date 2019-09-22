#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Resource/Proxy.h"
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

class GroupEntity;
class IrradianceGrid;
class IShadowProjection;
class LightRendererDeferred;
class WorldContext;

/*! World renderer, using deferred rendering method.
 * \ingroup World
 *
 * Advanced deferred rendering path.
 *
 * Operation
 * \TBD
 *
 * Techniques used
 * \TBD
 */
class T_DLLCLASS WorldRendererDeferred : public IWorldRenderer
{
	T_RTTI_CLASS;

public:
	WorldRendererDeferred();

	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const WorldCreateDesc& desc
	) override final;

	virtual void destroy() override final;

	virtual void attach(Entity* entity) override final;

	virtual void build(WorldRenderView& worldRenderView, int32_t frame) override final;

	virtual bool beginRender(render::IRenderView* renderView, int32_t frame, const Color4f& clearColor) override final;

	virtual void render(render::IRenderView* renderView, int32_t frame) override final;

	virtual void endRender(render::IRenderView* renderView, int32_t frame, float deltaTime) override final;

	virtual render::ImageProcess* getVisualImageProcess() override final;

	virtual void getDebugTargets(std::vector< render::DebugTarget >& outTargets) const override final;

private:
	struct ShadowContext
	{
		Ref< WorldContext > shadow;
		Matrix44 shadowLightView;
		Matrix44 shadowLightProjection;
		Matrix44 viewToLightSpace;
	};

	struct Frame
	{
		ShadowContext slice[MaxSliceCount];
		ShadowContext atlas[16];
		Ref< WorldContext > gbuffer;
		Ref< WorldContext > reflections;
		Ref< WorldContext > irradiance;
		Ref< WorldContext > velocity;
		Ref< WorldContext > visual;
		AlignedVector< Light > lights;
		Ref< render::StructBuffer > lightSBuffer;
		Ref< render::StructBuffer > tileSBuffer;
		Frustum viewFrustum;
		Matrix44 projection;
		Matrix44 lastView;
		Matrix44 view;
		int32_t atlasCount;
		float time;

		Frame()
		:	atlasCount(0)
		,	time(0.0f)
		{
		}
	};

	WorldRenderSettings m_settings;
	WorldRenderSettings::ShadowSettings m_shadowSettings;
	Quality m_toneMapQuality;
	Quality m_motionBlurQuality;
	Quality m_shadowsQuality;
	Quality m_reflectionsQuality;
	Quality m_ambientOcclusionQuality;
	Quality m_antiAliasQuality;
	Ref< IShadowProjection > m_shadowProjection;
	Ref< render::RenderTargetSet > m_visualTargetSet;
	Ref< render::RenderTargetSet > m_intermediateTargetSet;
	Ref< render::RenderTargetSet > m_gbufferTargetSet;
	Ref< render::RenderTargetSet > m_velocityTargetSet;
	Ref< render::RenderTargetSet > m_colorTargetSet;
	Ref< render::RenderTargetSet > m_reflectionsTargetSet;		//!< Reflections target.
	Ref< render::RenderTargetSet > m_shadowCascadeTargetSet;	//!< Shadow map for directional lights.
	Ref< render::RenderTargetSet > m_shadowMaskTargetSet;		//!< Screen space projected shadow mask, directional lights.
	Ref< render::RenderTargetSet > m_shadowAtlasTargetSet;		//!< Shadow map atlas for spot and point lights.
	Ref< render::RenderContext > m_globalContext;
	Ref< render::ImageProcess > m_colorTargetCopy;
	Ref< render::ImageProcess > m_ambientOcclusion;
	Ref< render::ImageProcess > m_antiAlias;
	Ref< render::ImageProcess > m_visualImageProcess;
	Ref< render::ImageProcess > m_gammaCorrectionImageProcess;
	Ref< render::ImageProcess > m_motionBlurPrimeImageProcess;
	Ref< render::ImageProcess > m_motionBlurImageProcess;
	Ref< render::ImageProcess > m_toneMapImageProcess;
	Ref< render::ImageProcess > m_shadowMaskProject;
	Ref< LightRendererDeferred > m_lightRenderer;
	resource::Proxy< IrradianceGrid > m_irradianceGrid;
	Ref< GroupEntity > m_rootEntity;
	AlignedVector< Frame > m_frames;
	float m_slicePositions[MaxSliceCount + 1];
	uint32_t m_count;
	Vector4 m_fogDistanceAndDensity;
	Vector4 m_fogColor;
	bool m_includeObjectVelocity;

	void buildGBuffer(WorldRenderView& worldRenderView, int32_t frame);

	void buildReflections(const WorldRenderView& worldRenderView, int32_t frame);

	void buildIrradiance(const WorldRenderView& worldRenderView, int32_t frame);

	void buildVelocity(const WorldRenderView& worldRenderView, int32_t frame);

	void buildLights(const WorldRenderView& worldRenderView, int32_t frame);

	void buildVisual(const WorldRenderView& worldRenderView, int32_t frame);
};

	}
}
