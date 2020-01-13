#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Misc/AutoPtr.h"
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
class IRenderTargetSet;
class ISimpleTexture;
class RenderContext;
class StructBuffer;

	}

	namespace world
	{

class GroupEntity;
class IrradianceGrid;
class IShadowProjection;
class LightRendererDeferred;
class WorldEntityRenderers;

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

	virtual void render(render::IRenderView* renderView, int32_t frame) override final;

	virtual render::ImageProcess* getVisualImageProcess() override final;

	virtual void getDebugTargets(std::vector< render::DebugTarget >& outTargets) const override final;

private:
	struct Frame
	{
		Ref< render::RenderContext > renderContext;
		Ref< render::StructBuffer > lightSBuffer;
		Ref< render::StructBuffer > tileSBuffer;
		int32_t lightCount;
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

	Ref< render::IRenderTargetSet > m_visualTargetSet;
	Ref< render::IRenderTargetSet > m_intermediateTargetSet;
	Ref< render::IRenderTargetSet > m_gbufferTargetSet;
	Ref< render::IRenderTargetSet > m_ambientOcclusionTargetSet;
	Ref< render::IRenderTargetSet > m_velocityTargetSet;
	Ref< render::IRenderTargetSet > m_colorTargetSet;
	Ref< render::IRenderTargetSet > m_reflectionsTargetSet;		//!< Reflections target.
	Ref< render::IRenderTargetSet > m_shadowCascadeTargetSet;	//!< Shadow map for directional lights.
	Ref< render::IRenderTargetSet > m_shadowMaskTargetSet;		//!< Screen space projected shadow mask, directional lights.
	Ref< render::IRenderTargetSet > m_shadowAtlasTargetSet;		//!< Shadow map atlas for spot and point lights.

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
	
	Ref< WorldEntityRenderers > m_entityRenderers;
	Ref< GroupEntity > m_rootEntity;
	AlignedVector< Frame > m_frames;

	float m_slicePositions[MaxSliceCount + 1];
	Vector4 m_fogDistanceAndDensity;
	Vector4 m_fogColor;

	void buildGBuffer(WorldRenderView& worldRenderView, int32_t frame);

	void buildVelocity(const WorldRenderView& worldRenderView, int32_t frame);

	void buildAmbientOcclusion(WorldRenderView& worldRenderView, int32_t frame);

	void buildLights(const WorldRenderView& worldRenderView, int32_t frame);

	void buildReflections(const WorldRenderView& worldRenderView, int32_t frame);

	void buildVisual(const WorldRenderView& worldRenderView, int32_t frame);

	void buildCopyFrame(const WorldRenderView& worldRenderView, int32_t frame);

	void buildEndFrame(WorldRenderView& worldRenderView, int32_t frame);
};

	}
}
